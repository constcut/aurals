#include "GmyFile.hpp"
#include "tab/tools/GtpFiles.hpp"
#include <fstream>

#include <QDebug>


using namespace aural_sight;


bool logFlag = false;



GmyFile::GmyFile()
{
}

//bends operation

void writeBendGMY(std::ofstream& file, BendPoints *bend)
{
    std::uint8_t bendType = bend->getType();
     file.write((char*)&bendType,1);

    std::uint8_t pointsCount = bend->size();
     file.write((char*)&pointsCount,1);

    for (size_t pointInd=0; pointInd<pointsCount; ++pointInd)
    {
        BendPoint *point = &bend->at(pointInd);

        std::uint8_t absolutePosition = point->horizontal;
        std::uint8_t verticalPosition = point->vertical;
        std::uint8_t vibratoFlag = point->vFlag;

         file.write((char*)(char*)&absolutePosition,1);
         file.write((char*)(char*)&verticalPosition,1); //could be packed more
         file.write((char*)(char*)&vibratoFlag,1); //but don't optimize before there is a need
    }
}


void readBendGMY(std::ifstream& file, BendPoints *bend)
{
    std::uint8_t bendType=0;
     file.read((char*)&bendType,1);

    size_t pointsCount = 0;
     file.read((char*)&pointsCount,1); //cannot be more then 255

    if (logFlag) qDebug()<< "Type "<<bendType<<"; N= "<<pointsCount;
    bend->setType(bendType); //trem yet not handled anyway

    for (size_t pointInd=0; pointInd<pointsCount; ++pointInd)
    {
        std::uint8_t absolutePosition = 0;
        std::uint8_t verticalPosition = 0;
        std::uint8_t vibratoFlag = 0;

         file.read((char*)&absolutePosition,1);
         file.read((char*)&verticalPosition,1);
         file.read((char*)&vibratoFlag,1);

        if (logFlag) qDebug() << "Point# "<< pointInd << "; absPos="<<absolutePosition<<"; vertPos="
               <<verticalPosition<<"; vibrato- "<<vibratoFlag;

        BendPoint point;
        point.vertical = verticalPosition; //25 - 1/12 of 3 tones
        point.horizontal = absolutePosition;
        point.vFlag = vibratoFlag;
        bend->push_back(point);
    }

    if (logFlag) qDebug() << "Beng if (gtpLog)  logging finished with " << bend->size();
}



    //from the tab gp originally
bool GmyFile::saveToFile(std::ofstream& file, Tab *tab)
{
     file.write((char*)"GA",2);
    char byteZero = 0;
     file.write((char*)&byteZero,1);
    //fill file with 8bit 0


    std::uint8_t effectsSet = 0; //development one
    std::uint8_t tracksByteLen = 1; //1 byte for tracks
    std::uint8_t barsByteLen = 2;
    std::uint8_t noteByteLen = 1; //attention - all this fields yet not used directly

     file.write((char*)&effectsSet,1);
     file.write((char*)&tracksByteLen,1);
     file.write((char*)&barsByteLen,1);
     file.write((char*)&noteByteLen,1);

    //must not exceed the value!!
    //IDEA REFACT ATTENTION
    //make poly as double template
    //ChainContainer<ul, Bar>
    //ChainContainer<byte, Track>
    //ChainContainer<int, Beat>
    size_t tracksCount = tab->size();//attention please
    size_t barsCount = tab->at(0)->size(); //search largerst or not do such thing? refact

     file.write((char*)&tracksCount,1); //256 tracks are insane
     file.write((char*)&barsCount,2); //65 535 bars are insace
    //first 8 bytes of format

    //1 bpm
    size_t bpm = tab->getBPM();
     file.write((char*)&bpm,2); //values more then 10 bits used for the 0.025 of bpm

    for (size_t i = 0; i < tracksCount; ++i) {

        auto& track = tab->at(i);
        std::string trackName = track->getName();
        saveString(file, trackName);

        auto tuning = track->getTuning();
        std::uint8_t stringsCount = tuning.getStringsAmount();
        file.write((char*)&stringsCount,1);

        if (logFlag) qDebug() << "Write strings count "<<stringsCount;

        for (std::uint8_t sI=0; sI < stringsCount; ++ sI)
        {
            std::uint8_t tune = tuning.getTune(sI);
             file.write((char*)&tune,1);

            if (logFlag) qDebug() <<"Write tune "<<tune<<" for i="<<sI;
        }

        //maximum frets
        std::uint8_t fretsLimit = track->getMidiInfo(3); //frets
         file.write((char*)&fretsLimit,1);
        //prepare for capo
        std::uint8_t capoSet = track->getMidiInfo(4);
         file.write((char*)&capoSet,1);

        if (logFlag) qDebug() << "Write limits fret "<<fretsLimit<<"; capo "<<capoSet;

        bool isDrums = track->isDrums();
         file.write((char*)&isDrums,1);

        int instr = track->getInstrument();
        std::uint8_t pan = track->getPan();
        std::uint8_t vol = track->getVolume();

         file.write((char*)&instr,2);
        //cover under track functions refact: getVolume getPan GeInstrument
            /// pan
         file.write((char*)&pan,1);
         file.write((char*)&vol,1);

        if (logFlag) qDebug() <<"IsD "<<isDrums<<"; instr-"<<
                (int)track->getInstrument()<<
              "; +2more sets ";

        std::uint8_t lastSignNum = 0;
        std::uint8_t lastSignDen = 0;

        for (size_t j = 0; j < barsCount; ++j)
        {
            //Each bar for that track
            Bar *bar;
            static Bar staticBar;

            if (j < track->size())
                bar = track->at(j).get();
            else
                bar = &staticBar; //Отладить этот участок, раньше тут была утечка и сырое выделение - но в теории дефолный бар должен работать, но может нехватать в нем 1 бита

            if (i == 0) //first track
            {
                if (logFlag) qDebug() <<"First track bar "<<j;
                std::uint8_t barHead = 0;

                std::uint8_t barNum = bar->getSignNum();
                std::uint8_t barDen = bar->getSignDenum();

                if (barNum != lastSignNum)
                {
                    barHead |= 1;
                    lastSignNum = barNum;
                }

                if (barDen != lastSignDen)
                {
                    barHead |= 2;
                    lastSignDen = barDen;
                }

                if (bar->getRepeat() & 1)
                    barHead |= 4;
                if (bar->getRepeat() & 2)
                    barHead |= 8;

                std::uint8_t repeatTimes = bar->getRepeatTimes();

                std::uint8_t altRepeat = bar->getAltRepeat();
                if (altRepeat)
                    barHead |= 16;

                auto [markerText, markerColor] = bar->getMarker();

                if (markerColor != 0)
                    barHead |= 32;

                //Now according to barHead WRITE DOWN information

                if (logFlag) qDebug()<< "Bar head "<<barHead;
                 file.write((char*)&barHead,1);

                if (barHead & 1)
                     file.write((char*)&barNum,1);

                if (barHead & 2)
                     file.write((char*)&barDen,1);

                if (barHead & 8)
                     file.write((char*)&repeatTimes,1);

                if (barHead & 16)
                     file.write((char*)&altRepeat,1);

                if (barHead & 32)
                {
                   //WRITE TEXT function;
                   //saveString(file,markerText);
                    file.write((char*)&markerColor,4);
                }
            }


            size_t barLen = bar->size();
             file.write((char*)&barLen,1); //more then 256 notes in beat are strange

            for (size_t k = 0; k < bar->size(); ++k)
            {
                //Each beat
                auto& beat = (bar->at(k));

                bool isPause = beat->getPause();
                // file.write((char*)&isPause,1);

                std::uint8_t dur = beat->getDuration();
                // file.write((char*)&dur,1);

                std::uint8_t dot = beat->getDotted();
                // file.write((char*)&dot,1);

                std::uint8_t det = beat->getDurationDetail();
                // file.write((char*)&det,1);

                std::string beatText;
                beat->getText(beatText);

                std::uint8_t beatHead = 0;


                beatHead = dur; //3bits
                beatHead += dot << 3;

                if (det)
                    beatHead += 1 << 5;


                ABitArray effPackBeat = beat->getEffects();
                size_t effPackBeatValue = effPackBeat.takeBits();

                if (effPackBeatValue)
                    beatHead += 1 << 6;

                if (isPause)
                    beatHead += 1 << 7;

                 file.write((char*)&beatHead,1);


                if (det)
                     file.write((char*)&det,1);

                if (effPackBeatValue)
                     file.write((char*)&effPackBeatValue,4);


                if (effPackBeat == Effect::Changes)
                {
                    //Package *changePack = effPackBeat.getPack(28);

                    //if (changePack)
                    {
                        ChangesList* changes = beat->getChangesPtr();//(ChangesList*)changePack->getPointer();

                        size_t amountOfChanges = changes->size();

                         file.write((char*)&amountOfChanges,1);

                        for (size_t indexChange=0; indexChange != amountOfChanges; ++indexChange)
                        {
                            std::uint8_t changeType = (changes->at(indexChange)).changeType;
                            size_t changeValue = (changes->at(indexChange)).changeValue;

                            std::uint8_t changeDur = (changes->at(indexChange)).changeCount;
                            //change count (apply effect on few beats later)

                             file.write((char*)&changeType,1);
                             file.write((char*)&changeValue,2); //pack it
                             file.write((char*)&changeDur,1);
                        }
                    }

                }


                //tremolo, chord etc should be stored here

                size_t beatLen = beat->size();
                file.write((char*)&beatLen,1); //256 notes in beat are too much

                //and notes inside
                for (size_t el=0; el < beat->size(); ++el)
                {
                    auto& note = beat->at(el);
                    ABitArray effPackNote = note->getEffects();
                    std::uint8_t fret = note->getFret();
                    //merge
                    std::uint8_t stringNum = note->getStringNumber();
                    std::uint8_t packFret=0;
                    if (isDrums) {
                        fret -= 35;
                        packFret = (fret);
                        std::uint8_t stringSh = stringNum<<5;
                        packFret += stringSh;
                    }
                    else {
                       //4bit for fret 0-31 and string num 0-15
                       packFret = (fret);
                       std::uint8_t stringSh = stringNum<<5;
                       packFret += stringSh;
                    }

                    file.write((char*)&packFret,1);
                    std::uint8_t noteSpec = 0;
                    std::uint8_t vol = note->getVolume();
                    std::uint8_t state = note->getState();
                    std::uint8_t effectsPrec = effPackNote.empty() == true ? 0 : 1;

                    noteSpec = (vol & 0xF) + ((state&7)<<4) + (effectsPrec<<7);
                    file.write((char*)&noteSpec,1);

                    if (effectsPrec) {
                        size_t noteEffValue = effPackNote.takeBits();
                         file.write((char*)&noteEffValue,4);

                        if (effPackNote == Effect::Bend) //bend
                        {
                            BendPoints *bend = note->getBendPtr();
                            writeBendGMY(file,bend);
                        }
                    }
                }
                //data then effects
            }
        }
    }

    return true;
}


bool GmyFile::saveString(std::ofstream& file, std::string &strValue) {
    size_t stringLen = strValue.size();
     file.write((char*)&stringLen,2);
     file.write((char*)strValue.c_str(),strValue.size());
    return true;
}


bool GmyFile::loadString(std::ifstream& file, std::string &strValue)
{
    char bufer[2048];

    size_t stringLen = 0;
     file.read((char*)&stringLen,2);

     file.read((char*)&bufer,stringLen);
    bufer[stringLen] = 0;

    strValue = bufer;
    return true;
}

bool GmyFile::loadFromFile(std::ifstream& file, Tab *tab, bool skipVersion)
{
    //now reverse

    if (skipVersion==false)
    {
        char firstBytes[3]={0};
         file.read((char*)firstBytes,2);

        if ((firstBytes[0]!='G') ||
            (firstBytes[1]!='A'))
        {
            if (logFlag) qDebug() << "Attempt to open not guitarmy file";
            return false;
        }
    }

    char byteZero = 0;
     file.read((char*)&byteZero,1);
    if (byteZero != 0)
    {
        if (logFlag) qDebug() << "Not a base format version";
        return false;
    }

    std::uint8_t effectsSet = 0; //development one
    std::uint8_t tracksByteLen = 1; //1 byte for tracks
    std::uint8_t barsByteLen = 2;
    std::uint8_t noteByteLen = 1; //attention - all this fields yet not used directly

     file.read((char*)&effectsSet,1);
     file.read((char*)&tracksByteLen,1);
     file.read((char*)&barsByteLen,1);
     file.read((char*)&noteByteLen,1);


    size_t tracksCount = 0;
    size_t barsCount = 0;

     file.read((char*)&tracksCount,1);
     file.read((char*)&barsCount,2);
    //here could reserve values
    /*
    for (ul tI = 0; tI < tracksCount; ++tI)
    {
        for (ul bI = 0; bI < barsCount; ++bI)
        {
            //reserve
        }
    }
    */

    //General tab information

    //1 bpm
    size_t bpm = 0;
     file.read((char*)&bpm,2); //values more then 1200 could be used as parts of bpm

    tab->setBPM(bpm);

    for (size_t i = 0; i < tracksCount; ++i)
    {
        //Each track
        auto track= std::make_unique<Track>();
        track->setParent(tab);

        //Name
        std::string trackName="not yet";
        //for Track#1 could be used empty
        loadString(file,trackName);

        track->setName(trackName);
        //strings N
        std::uint8_t stringsCount = 0; //track->tuning.getStringsAmount();
        file.read((char*)&stringsCount,1);
        //tuning

        auto& tuning = track->getTuningRef();
        tuning.setStringsAmount(stringsCount);

        if (logFlag) qDebug()<<"Strings amount "<<stringsCount;
        for (std::uint8_t sI=0; sI < stringsCount; ++sI) {
            std::uint8_t tune = 0;
            file.read((char*)&tune,1);
            tuning.setTune(sI,tune);
            if (logFlag) qDebug() << "Read tune "<<sI<<" "<<tune;
        }

        //maximum frets
        std::uint8_t fretsLimit = 0; //frets
         file.read((char*)&fretsLimit,1);
        track->setMidiInfo(3,fretsLimit);
        //prepare for capo
        std::uint8_t capoSet = 0;
         file.read((char*)&capoSet,1);
        track->setMidiInfo(4,capoSet);


        if (logFlag) qDebug() << "read fret limits "<<fretsLimit<<"; capo "<<capoSet;


        bool isDrums = false; //track->isDrums();

         file.read((char*)&isDrums,1);
        track->setDrums(isDrums);

        //SET GET REFACT
        short int instr = 0;
        std::uint8_t pan = 0;
        std::uint8_t volume = 0;

         file.read((char*)
 //&(tab->GpCompMidiChannels[midiChanPortIndex].instrument),
    &instr,
                    2);

        //cover under track functions refact: getVolume getPan GeInstrument
            /// pan
         file.read((char*)
  //&(tab->GpCompMidiChannels[midiChanPortIndex].balance)
                    &pan,1);
            /// volume
         file.read((char*)
 //&(tab->GpCompMidiChannels[midiChanPortIndex].volume)
                    &volume
                    ,1);

        track->setInstrument(instr);
        track->setVolume(volume);
        track->setPan(pan);


        if (logFlag) qDebug()<<" Read instr "<<instr<<" isD "<<isDrums<<"; pan "<<
              pan<<"; volume "<<volume;

        for (size_t j = 0; j < barsCount; ++j)
        {
            //Each bar for that track
            //Bar *bar = &(track->getV(i));

            auto bar = std::make_unique<Bar>();
            bar->flush();


            if (i == 0) //first track
            {
                //work over repeats signs etc - set them only from first track
                //On POLY tab format this would be applied to each track
                ///HEADED: 1) repeats 2) signatures 3) Marker
                /// 1 - change num 2 - change denum
                /// 4 - begin repeat 8 - end repeat
                /// 16- alt     32 marker - could be stored in 3 bits (+2 reserved values)


                std::uint8_t barHead = 0;
                 file.read((char*)&barHead,1);

                if (logFlag) qDebug() << "Read bar head "<<barHead;

                std::uint8_t barNum =0;
                std::uint8_t barDen =0;

                std::uint8_t repeatTimes = 0;
                std::uint8_t altRepeat = 0;


                if (barHead & 1)
                {
                     file.read((char*)&barNum,1);
                    bar->setSignNum(barNum);
                }

                if (barHead & 2)
                {
                     file.read((char*)&barDen,1);
                    bar->setSignDenum(barDen);
                }

                if (barHead & 4)
                {
                    bar->setRepeat(1);
                }

                if (barHead & 8)
                {
                     file.read((char*)&repeatTimes,1);
                    bar->setRepeat(2,repeatTimes);
                }

                if (barHead & 16)
                {
                     file.read((char*)&altRepeat,1);
                    bar->setAltRepeat(altRepeat);
                }

                if (barHead & 32)
                {
                   std::string markerText;
                   //read TEXT function;
                   //loadString(file,markerText);
                   size_t markerColor = 0;
                    file.read((char*)&markerColor,4);
                }
            }
            else
            {
               //will be erased on time loop

               // not 0 bar
               auto& ftBar = tab->at(0)->at(j); //firstTrackBar
               bar->setRepeat(ftBar->getRepeat(),ftBar->getRepeatTimes());
               bar->setSignDenum(ftBar->getSignDenum());
               bar->setSignNum(ftBar->getSignNum());
               bar->setAltRepeat(ftBar->getAltRepeat());
               //no markers here haha
            }


            size_t barLen = 0;
             file.read((char*)&barLen,1); //more then 256 notes in beat are strange

            if (barLen == 0)
            {
                if (logFlag) qDebug() << "Bars len";
                auto emptyBeat = std::make_unique<Beat>();
                emptyBeat->setPause(true);
                emptyBeat->setDuration(3);
                bar->push_back(std::move(emptyBeat));
            }

            for (size_t k = 0; k < barLen; ++k)
            {
                //Each beat
                //Beat *beat = &(bar->getV(k));
                auto beat = std::make_unique<Beat>();
                bool isPause = false;
                // file.read((char*)&isPause,1);
                std::uint8_t dur =0;
                // file.read((char*)&dur,1);
                std::uint8_t dot = 0;
                // file.read((char*)&dot,1);
                std::uint8_t det = 0;
                // file.read((char*)&det,1);


                std::uint8_t beatHead = 0;
                 file.read((char*)&beatHead,1);

                dur = beatHead & 0x7;
                isPause = ((beatHead >> 7) & 1) > 0;
                std::uint8_t gotEff = (beatHead >> 6) & 1;

                det = (beatHead >> 5) & 1;
                dot = (beatHead >> 3) & 3;

                if (det)
                {
                     file.read((char*)&det,1);
                }

                /*
                std::string beatText;
                loadString(file,beatText);
                if (beatText.empty())
                {
                    if (logFlag) qDebug() <<"beatText "<< beatText.c_str()
                }
                */
                //EffectsPack effPackBeat = beat->getEffects();



                size_t effPackBeatValue = 0;

                if (gotEff)
                     file.read((char*)&effPackBeatValue,4);


                beat->setPause(isPause);
                beat->setDuration(dur);
                beat->setDotted(dot);
                beat->setDurationDetail(det);

                beat->getEffects().putBits(effPackBeatValue);

                if (beat->getEffects() == Effect::Changes)
                {
                    size_t amountOfChanges = 0;

                    ChangesList *newChanges = beat->getChangesPtr();

                     file.read((char*)&amountOfChanges,1);

                    for (size_t indexChange=0; indexChange != amountOfChanges; ++indexChange)
                    {
                        std::uint8_t changeType = 0;
                        size_t changeValue = 0;

                        std::uint8_t changeDur = 0;
                        //change count (apply effect on few beats later)

                         file.read((char*)&changeType,1);
                         file.read((char*)&changeValue,2); //pack it
                         file.read((char*)&changeDur,1);

                        SingleChange change;
                        change.changeCount = changeDur;
                        change.changeType = changeType;
                        change.changeValue = changeValue;

                        newChanges->push_back(change);
                    }
                    beat->getEffects().setEffectAt(Effect::Changes, true);
                }

                //tremolo, chord etc should be loaded here

                size_t beatLen = 0;
                 file.read((char*)&beatLen,1); //256 notes are even too much

                //and notes inside
                for (size_t el=0; el < beatLen; ++el)
                {
                   // Note *note = &(beat->getV(el));

                    // EffectsPack effPackNote = note->getEffects();
                    auto note = std::make_unique<Note>();


                    std::uint8_t fret = 0;
                    //merge
                    std::uint8_t stringNum = 0;

                    std::uint8_t fretPack = 0;
                    // file.read((char*)&fret,1);
                    // file.read((char*)&stringNum,1);


                     file.read((char*)&fretPack,1);

                    if (isDrums)
                    {
                        fret = 0x1F & fretPack;
                        fret += 35;
                        stringNum = (fretPack>>5) & 7;
                    }
                    else
                    {
                        fret = 0x1F & fretPack;
                        stringNum = (fretPack>>5) & 7;
                    }


                    note->setFret(fret);
                    note->setStringNumber(stringNum);

                    std::uint8_t noteSpec = 0;

                     file.read((char*)&noteSpec,1);

                    std::uint8_t vol = noteSpec & 0xF;
                    // file.read((char*)&vol,1);

                    std::uint8_t effectState = 1&(noteSpec >> 7);
                    // file.read((char*)&effectState,1);

                    //byte state = (noteSpec >>4) &7;

                    note->setVolume(vol);
                    //note.setState(state);

                    if (effectState == 1)
                    {
                        size_t noteEffValue = 0;
                         file.read((char*)&noteEffValue,4);
                        note->getEffectsRef().putBits(noteEffValue);

                        //note.effPack.set(17,false); //turn off bend
                        if (note->getEffects() == Effect::Bend)
                        {
                            BendPoints *bend = note->getBendPtr();
                            readBendGMY(file,bend);
                            beat->getEffects().setEffectAt(Effect::Bend, true);
                        }

                        note->getEffectsRef().setEffectAt(Effect::Tremolo,false); //turn off tremolo
                    }

                    beat->push_back(std::move(note));
                }
                bar->push_back(std::move(beat));
            }

            track->push_back(std::move(bar));
        }

        tab->push_back(std::move(track));
    }
    return true;
}
