#include "MidiExport.hpp"
#include "app/Config.hpp"

bool midiExportLog = false;

#include <QDebug>


using namespace aurals;


std::unique_ptr<MidiFile> aurals::exportMidi(Tab* tab, size_t shiftTheCursor) {
    auto output = std::make_unique<MidiFile>();
    //time line track
    {
        auto timeLineTrack = MidiTrack();
        tab->createTimeLine(shiftTheCursor);
        size_t tlAccum = 0;

        const auto& timeLine = tab->getTimeLine();
        for (size_t i = 0; i < timeLine.size(); ++i) {
            if (timeLine[i].type==1) {
                const short int rhyBase = 120;
                const short int power2 = 2 << (3);
                int preRValue = rhyBase * power2 / 4;
                preRValue *= tlAccum;
                preRValue /= 1000;
                timeLineTrack.pushChangeBPM(timeLine[i].value, preRValue);
                tlAccum = 0;
            }

            if (timeLine[i].type == 0)
                tlAccum += timeLine[i].value;

            if (timeLine[i].type == 2) {
                int packedMeter = timeLine[i].value;
                const int newDen = packedMeter & 0xff;
                packedMeter -= newDen;
                const int newNum = (packedMeter >> 8);
                const short int rhyBase = 120;
                const short int power2 = 2 << (3);
                int preRValue = rhyBase * power2 / 4;
                preRValue *= tlAccum;
                preRValue /= 1000;
                timeLineTrack.pushMetricsSignature(newNum,newDen,preRValue);
                tlAccum = 0;
            }
        }

        timeLineTrack.pushEvent47();
        output->push_back(std::move(timeLineTrack));
    }

    if (CONF_PARAM("metronome")=="1") {
     //metronome track
        int metronomeClickSize = 0;
        bool metronomeTurnedOn = true;

        if (metronomeTurnedOn)
        {
            auto metronomeClickTrack = MidiTrack();
            bool firstRun = true;

            const auto& timeLoop = tab->at(0)->getTimeLoop();
            for (size_t barI=shiftTheCursor; barI< timeLoop.size(); ++barI)
            {
                    Bar *bar = timeLoop.at(barI);
                    int newDen = bar->getSignDenum();

                    if (newDen==32) //TODO log formula
                        metronomeClickSize = 60;
                    if (newDen==16)
                        metronomeClickSize = 120;
                    if (newDen == 8)
                        metronomeClickSize = 240;
                    else if (newDen==4)
                        metronomeClickSize = 480;
                    else if (newDen==2)
                        metronomeClickSize = 960;
                    else if (newDen==1)
                        metronomeClickSize = 1920;
                    else if (newDen==0)
                        metronomeClickSize = 3840;

                    uint8_t num = newDen = bar->getSignNum();
                    for (uint8_t i = 0; i < num; ++i)
                        if (firstRun){
                            auto noteOn = MidiMessage(NoteOnMask | DrumTrackMask, 33, 127,0);
                            metronomeClickTrack.push_back(std::move(noteOn));
                            firstRun = false;
                        }
                        else {
                            auto noteOn = MidiMessage(NoteOnMask | DrumTrackMask, 33, 127,metronomeClickSize);
                            metronomeClickTrack.push_back(std::move(noteOn));
                        }
                }
                metronomeClickTrack.pushEvent47();
                output->push_back(std::move(metronomeClickTrack));
            }
    }

    size_t tabLen = tab->size();
    int drumsTrack = 0;
    size_t startCursorBar = shiftTheCursor;

    //Get aware of solo tracks
    std::vector<size_t> indecesToSkip;
    bool soloTurnedOn = false;

    for (size_t trackIndex = 0; trackIndex < tabLen; ++trackIndex) {
        auto& currentTrack = tab->at(trackIndex);
        std::uint8_t trackStatus = currentTrack->getStatus();
        if (trackStatus != 2)
            indecesToSkip.push_back(trackIndex);
        else
            soloTurnedOn=true;
    }

    if (soloTurnedOn==false)
        indecesToSkip.clear();

    for (size_t i = 0; i < tabLen; ++i)
    {
        auto& track = tab->at(i);

        std::uint8_t trackStatus = track->getStatus();
        if (trackStatus == 1) //mute
            continue;

        if (soloTurnedOn) {
            bool skipThatTrack = false;
            for (size_t i2 = 0; i2 < indecesToSkip.size(); ++i2)
                if (indecesToSkip[i2] == i) {
                    skipThatTrack=true;
                    break;
                }
            if (skipThatTrack)
                continue;
        }


        auto mTrack = MidiTrack();
        mTrack.flushAccum();

        if (track->isDrums()) {
            ++drumsTrack;
            size_t realInd = DrumTrackMask;
            exportTrack(track.get(), &mTrack, realInd, startCursorBar);
        }
        else
            exportTrack(track.get(), &mTrack, i, startCursorBar);

        output->push_back(std::move(mTrack));
    }

    return output;
}



void aurals::exportTrack(Track* track, MidiTrack* midiTrack, size_t channel, size_t shiftCursorBar) {

    size_t instrument = track->getInstrument();
    std::uint8_t midiPan = midiTrack->calcMidiPanoramFromTab(track->getPan());
    std::uint8_t midiVol = midiTrack->calcMidiVolumeFromTab(track->getVolume());
    midiTrack->pushChangeInstrument(instrument, channel);
    midiTrack->pushChangePanoram(midiPan, channel);
    midiTrack->pushChangeVolume(midiVol, channel);

    std::uint8_t theTunes[11];

    auto tuning = track->getTuning();
    for (size_t i = 0; i < 10; ++i)
        if (track->isDrums())
            theTunes[i] = 0;
        else
            theTunes[i] = tuning.getTune(i);

    midiTrack->setTunes(theTunes);


    const auto& timeLoop = track->getTimeLoop();
    size_t trackLen = timeLoop.size();

    for (size_t i = shiftCursorBar ; i < trackLen; ++i)
    {
        Bar *bar = timeLoop.at(i);
        std::uint8_t completeStatus = bar->getCompleteStatus();

        size_t barLen = bar->size();
        short specialLast = 0;
        size_t completeIndex = 0;

        if (completeStatus == 2) {
           //barLen = bar->getCompleteIndex();
           completeIndex = bar->getCompleteIndex();
           specialLast = bar->getCompleteAbs();
        }

        for (size_t j = 0; j < barLen; ++j) {
            Beat *beat = bar->at(j).get();
            if ((completeStatus == 2) && (j == completeIndex)) {
                exportBeat(beat, midiTrack, channel, specialLast);
                break;
            }
               else
                   exportBeat(beat, midiTrack, channel);
        }

       midiTrack->closeLetRings(channel);

       if (completeStatus == 1)
           midiTrack->finishIncomplete(bar->getCompleteAbs()); //short

    }
    midiTrack->pushEvent47();
}


void aurals::exportBeat(Beat* beat, MidiTrack* midiTrack, size_t channel, short specialRhy) {

    uint8_t dur = beat->getDuration();
    uint8_t det = beat->getDurationDetail();
    uint8_t dot = beat->getDotted();

    const int baseAmount = 120;

    int rOffset = 0;
    if (specialRhy == 0)
    {
        std::uint8_t powOfTwo = 6 - dur;
        short int power2 = 2<<(powOfTwo-1);//-1 because 2 is 1 pow itself
        rOffset = baseAmount*power2/4;

        if (dot & 1)  { //single dotted
            rOffset *= 3;
            rOffset /= 2;
        }
       if (det)
            rOffset = midiTrack->calculateRhythmDetail(det,rOffset); //FEW MISSING
    }
    else {
        short int rhyBase = 120; //TODO этот коэфицент одинаковый при этом всегда делится на 4
        short int power2 = 2 << (3);
        int preRValue = rhyBase*power2/4;
        preRValue *= specialRhy;
        preRValue /= 1000;

        rOffset = preRValue;
    }

    if (beat->getPause()) {
        midiTrack->closeLetRings(channel);
        midiTrack->accumulate(rOffset);
        return;
    }

    //BEAT EFFECTS:
    if (beat->getEffects().getEffectAt(Effect::Changes))
    {
        //changes

        {
            ChangesList *changes = beat->getChangesPtr();

            for (size_t indexChange = 0; indexChange != changes->size(); ++indexChange)
            {
              if (changes->at(indexChange).changeType == 8)
                    ;//Генерация происходит в другом месте

              if (changes->at(indexChange).changeType == 1) {
                 size_t newInstr = changes->at(indexChange).changeValue;
                 midiTrack->pushChangeInstrument(newInstr, channel, midiTrack->getAccum());
                 midiTrack->flushAccum();
              }

              if (changes->at(indexChange).changeType == 2) {
                  std::uint8_t newVol = changes->at(indexChange).changeValue;
                  std::uint8_t midiNewVolume = newVol * 8;
                  if (midiNewVolume > 127)
                      midiNewVolume = 127;
                  midiTrack->pushChangeVolume(midiNewVolume, channel); //must take accum
              }

              if (changes->at(indexChange).changeType == 3) {
                    std::uint8_t newPan = changes->at(indexChange).changeValue;
                    std::uint8_t midiNewPanoram = newPan * 8;
                    if (midiNewPanoram > 127)
                        midiNewPanoram = 127;
                    midiTrack->pushChangePanoram(midiNewPanoram,channel); //must take accum
              }
            }
        }
    }

    short int strokeStep  = rOffset / 12;
    size_t beatLen = beat->size();
    for (size_t i = 0; i < beatLen; ++i) {

        size_t trueIndex = i;
        if (beat->getEffects().getEffectAt(Effect::DownStroke)) //down
            trueIndex = (beatLen - i - 1);

        if (beat->getEffects().inRange(Effect::UpStroke,Effect::DownStroke)) { //up down strokes
            midiTrack->accumulate(strokeStep);
            rOffset -= strokeStep;
        }

        Note *note = beat->at(trueIndex).get();
        exportSingalsFromNoteOn(note, midiTrack, channel);
    }

    midiTrack->accumulate(rOffset);

    if (beat->getEffects().getEffectAt(Effect::FadeIn)) { //fade in

        midiTrack->pushFadeIn(midiTrack->getAccum(), channel);
        midiTrack->flushAccum();
    }
    if (beat->getEffects().getEffectAt(Effect::Tremolo)) { //tremolo
        midiTrack->pushTremolo(channel, midiTrack->getAccum());
        midiTrack->flushAccum();
    }

    exportPostEffect(beat, midiTrack, channel);
    for (size_t i =0; i < beat->size(); ++i) {
        Note *note = beat->at(i).get();
        exportSingalsFromNoteOff(note, midiTrack, channel);
    }
}




bool aurals::exportSingalsFromNoteOn(Note* note, MidiTrack* midiTrack, std::uint8_t channel) {
    std::uint8_t noteState = note->getState();

    if ((noteState==Note::leegNote) || (noteState==Note::leegedLeeg))
        return false;

    std::uint8_t fret = note->getFret();
    std::uint8_t stringN = note->getStringNumber();
    std::uint8_t midiNote = fret + midiTrack->getTunes()[stringN-1];

    std::uint8_t volume = note->getVolume();
    std::uint8_t midiVelocy = volume*15; //calcMidiVolumeGP(volume);

    std::uint8_t lastVelocy = 95; //not real last one yet

    if (midiVelocy > 127)
        midiVelocy = 127;

    if (midiVelocy == 0)
        midiVelocy = lastVelocy;

    if (note->getEffects().getEffectAt(Effect::LetRing)) {
        midiTrack->openLetRing(stringN,midiNote,midiVelocy,channel);
        return true;
    }

    if (note->getEffects().getEffectAt(Effect::PalmMute))
        midiVelocy = midiTrack->calcPalmMuteVelocy(midiVelocy);

    if (note->getEffects().getEffectAt(Effect::GhostNote))
        midiVelocy = midiVelocy > 10 ? midiVelocy - 10 : 1 ;

    if (note->getEffects().getEffectAt(Effect::HeavyAccented))
        midiVelocy = midiVelocy < 110 ? midiVelocy+midiVelocy/10 : 127;

    if (note->getEffects().getEffectAt(Effect::GraceNote))
        midiNote += 2;

    if (note->getEffects().inRange(Effect::Harmonics,Effect::HarmonicsV6))
    {
      if (note->getEffects().getEffectAt(Effect::Harmonics)) {
          if (fret == 7) midiNote += 12;
          if (fret == 5) midiNote += 19;
      }
      if (note->getEffects().getEffectAt(Effect::HarmonicsV4))
          midiNote += 12;
      //2 artif+5; 3 artif+7; 6 artif+12;
      //3 - tapped; 5 - semi //TODO update enums
    }

    midiTrack->pushNoteOn(midiNote,midiVelocy,channel);
    return true;
}



bool aurals::exportSingalsFromNoteOff(Note* note, MidiTrack* midiTrack, std::uint8_t channel) {

    if (note->getEffects().getEffectAt(Effect::LetRing))
        return false;

    std::uint8_t noteState = note->getState();

    if ((noteState == Note::leegedLeeg) || (noteState == Note::leegedNormal))
        return false;

    if (note->getEffects().getEffectAt(Effect::Stokatto))
        return false;

    std::uint8_t fret = note->getFret();
    std::uint8_t stringN = note->getStringNumber();
    std::uint8_t midiNote = fret + midiTrack->getTunes()[stringN-1];

    std::uint8_t midiVelocy = 80; //calcMidiVolumeFromTab(volume);

    if (note->getEffects().inRange(Effect::Harmonics,Effect::HarmonicsV6)) {
      if (note->getEffects().getEffectAt(Effect::Harmonics)) {
          if (fret==7) midiNote += 12;
          if (fret==5) midiNote += 19;
      }
      if (note->getEffects().getEffectAt(Effect::HarmonicsV4))
          midiNote += 12;
    }

    midiTrack->pushNoteOff(midiNote,midiVelocy,channel);
    return true;
}


void aurals::exportPostEffect(Beat* beat, MidiTrack* midiTrack, std::uint8_t channel) {

    for (size_t i =0; i < beat->size(); ++i)
    {
        auto& note = beat->at(i);
        std::uint8_t fret = note->getFret();
        std::uint8_t stringN = note->getStringNumber();
        std::uint8_t midiNote = fret + midiTrack->getTunes()[stringN-1];
        std::uint8_t midiVelocy = 95; //calcMidiVolumeGP(volume);
        std::uint8_t noteState = note->getState();

        if (noteState == Note::deadNote) { //dead note
            short tempAccum = midiTrack->getAccum();
            midiTrack->setAccum(20);
            midiTrack->pushNoteOff(midiNote,midiVelocy,channel);
            if (tempAccum > 20)
                tempAccum -= 20;
            else
                tempAccum = 0;
            midiTrack->setAccum(tempAccum);
        }

        if (note->getEffects().getEffectAt(Effect::GraceNote))
        {
            short int graceLen = (midiTrack->getAccum()/8);
            if (graceLen == 0)
                graceLen = 1;

            auto mSignalGraceOff = MidiMessage(NoteOffMask | channel,midiNote+2,80,graceLen-1);
            midiTrack->push_back(std::move(mSignalGraceOff));
            auto mSignalOn = MidiMessage(NoteOnMask | channel,midiNote,midiVelocy,1);
            midiTrack->push_back(std::move(mSignalOn));
            midiTrack->accumulate(-graceLen);
        }

        if (note->getEffects().inRange(Effect::Hammer, Effect::Legato))
        {
            if ( (note->getEffects() == Effect::Slide) || (note->getEffects() == Effect::LegatoSlide))
            {
                //if (effects==5) velocyShift=19; //set decreace legatto slide
                short int slideStep = midiTrack->getAccum() / 8;
                midiTrack->pushSlideUp(channel, 2, slideStep);
                midiTrack->flushAccum();
            }
            else if ((note->getEffects() == Effect::SlideDownV2) || (note->getEffects() == Effect::SlideDownV1))
            {
                short int slideStep = midiTrack->getAccum() / 8;
                midiTrack->pushSlideDown(channel, 7, slideStep);
                midiTrack->flushAccum();
            }
            else if ((note->getEffects() == Effect::SlideUpV2)|| (note->getEffects() == Effect::SlideUpV1))
            {
                short int slideStep = midiTrack->getAccum() / 8;
                midiTrack->pushSlideUp(channel, 7, slideStep);
                midiTrack->flushAccum();

            }
            else if (note->getEffects() == Effect::Legato) {
                //legato - as normal one should decreace sound of next note //velocyShift=19; //set decreace
            }
        }

        if (note->getEffects().getEffectAt(Effect::Bend)) {
            BendPoints *bend = note->getBendPtr();
            pushBendToTrack(bend, midiTrack,channel);
        }

        if (note->getEffects().getEffectAt(Effect::TremoloPick)) {
            //pushTremoloPick - tremolo pick - trills
            short int tremoloStep = midiTrack->getAccum() / 4;
            for (size_t i = 0; i < 3; ++i) {
                auto mSignalOff = MidiMessage(NoteOffMask | channel, midiNote, midiVelocy, tremoloStep);
                midiTrack->push_back(std::move(mSignalOff));
                auto mSignalOn = MidiMessage(NoteOnMask | channel, midiNote, midiVelocy, 0);
                midiTrack->push_back(std::move(mSignalOn));
            }
            midiTrack->setAccum(tremoloStep);
            //flushAccum(); //Attention review
        }

        if (note->getEffects().getEffectAt(Effect::Stokatto)) {
            //stokato - stop earlier
            short halfAccum = midiTrack->getAccum() / 2;
            auto mSignalOff = MidiMessage(NoteOffMask | channel,midiNote,midiVelocy,halfAccum);
            midiTrack->push_back(std::move(mSignalOff));
            midiTrack->setAccum(halfAccum);
        }

        if (note->getEffects().getEffectAt(Effect::Vibrato)) {
            short int vibroStep = midiTrack->getAccum() / 6;
            midiTrack->pushVibration(channel, 3, vibroStep);
            midiTrack->flushAccum();
        }
    }
}



void aurals::pushBendToTrack(BendPoints* bend, MidiTrack* midiTrack, std::uint8_t channel) {
    short rOffset = midiTrack->getAccum();
    midiTrack->flushAccum();

    size_t lastAbs = 0;
    size_t lastH = 0;
    short rAccum = 0;

    if (midiExportLog)
        qDebug() << "Bend rOffset="<<rOffset;

    for (size_t i = 0 ; i < bend->size(); ++i)
    {
        size_t curAbs = bend->at(i).horizontal;
        size_t curH = bend->at(i).vertical;

        size_t shiftAbs = curAbs - lastAbs;
        size_t shiftH = curH - lastH;

        if (midiExportLog)
        qDebug() << "AbsShift="<<shiftAbs<<"; HShift="<<shiftH;

        if (shiftH != 0) {
            std::uint8_t lastShift = 64 + (lastH * 32) / 4; //decreased from 100 to 4
            std::uint8_t curShift = 64 + (curH * 32) / 4; //next

            if (midiExportLog)
                qDebug() <<"lastShiftPitch=" << lastShift << "; curShiftPitch=" << curShift;

            double rhyStep = (shiftAbs*rOffset) / 600.0; //10 steps
            double pitchStep = (curShift-lastShift) / 10.0;

            if (midiExportLog)
                qDebug() <<"rStep="<<rhyStep<<"; rAccum="<<rAccum<<"; pSh="<<pitchStep;

            auto mSignalBendOpen = MidiMessage(PitchWheelMask | channel,0,lastShift, rAccum);
            midiTrack->push_back(std::move(mSignalBendOpen));
            rAccum = 0;

            double stepsDone = 0.0;
            double shiftDone = 0.0;

            for (size_t i = 0; i < 10; ++i) {
                double thisStep = rhyStep*(i+1) - stepsDone;
                stepsDone += thisStep;
                double thisShift = pitchStep*(i+1) - shiftDone;
                shiftDone += thisShift;
                short thisROffset = thisStep;
                short thisMidiShift = thisShift;
                short totalShiftNow = lastShift + shiftDone;

                auto mSignalBend = MidiMessage(PitchWheelMask | channel,0,totalShiftNow,thisROffset);
                midiTrack->push_back(std::move(mSignalBend));

                if (midiExportLog)
                    qDebug() <<"rD="<<thisStep<<"; sD="<<thisShift<<" shiftDone="<<shiftDone;
                if (midiExportLog)
                    qDebug() <<"rOff="<<thisROffset<<"; midiSh="<<thisMidiShift;
            }
        }
        else
            rAccum += shiftAbs*rOffset / 60; //no change - just calculate rOffset

        lastAbs = curAbs;
        lastH = curH;
    }

    std::uint8_t lastShift = 64 + (lastH * 32) / 4; //decreased from 100
    auto mSignalBendLast = MidiMessage(PitchWheelMask | channel, 0, lastShift, rAccum);
    midiTrack->push_back(std::move(mSignalBendLast));
    auto mSignalBendClose = MidiMessage(PitchWheelMask | channel, 0, 64, 0);
    midiTrack->push_back(std::move(mSignalBendClose));

    if (midiExportLog)
        qDebug() << "done";
}


