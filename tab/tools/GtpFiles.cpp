#include "GtpFiles.hpp"

#include <QDebug>
#include <fstream>


using namespace aural_sight;


bool gtpLog = false;

////////////////////////////////////////////////////////////////////////////
char miniBufer[20480]; //TODO get rid




//Some of the logs disabled yet


std::string readString(std::ifstream &file, size_t stringLen)
{
   //for current max
   //refact dynamic
    if (stringLen >= 20480)
        throw std::runtime_error("GP5 read string fail");

   if (gtpLog)  qDebug() <<"Reaging string "<<stringLen<<" bytes";
   file.read((char*)miniBufer,stringLen);//changed attention


   miniBufer[stringLen] = 0;

   if (gtpLog)  qDebug() << "Readen string "<<miniBufer;

   std::string response = std::string(miniBufer);
   return response;
}

std::string readString(std::ifstream &file)
{
    size_t stringLen = 0;
    std::string response = "";

    file.read((char*)&stringLen,4);

    if (stringLen > 0)
    {
        if (gtpLog)
            qDebug()<<"String len "<<stringLen;
        response = readString(file,stringLen); //attention and check old
    }
    else
    {
        if (gtpLog)  qDebug() << "String is empty!";
    }
    return response;
}

std::string readStringShiByte(std::ifstream &file)
{
    size_t stringLen = 0;
    std::string response = "";

    file.read((char*)&stringLen,4);

    std::uint8_t whatFo = 0; file.read((char*)&whatFo,1);

    if (stringLen == 0)
        stringLen = whatFo;
    else
    {
        --stringLen;
    }


    if (stringLen > 0)
    {
        if (gtpLog)
            qDebug()<<"String len "<<stringLen;
        response = readString(file,stringLen); //attention and check old
    }
    else
    {
        if (gtpLog)  qDebug() << "String is empty!";
    }
    return response;
}


void writeBendGTPOLD(std::ofstream *file, BendPointsGPOld *bend)
{
    //saved to be somewhere in the code
    std::uint8_t bendType=bend->getType();
    file->write((const char*)&bendType,1);


    size_t bendHeight = bend->getHeight();
    file->write((const char*)&bendHeight,4); //attention again

    size_t pointsCount = bend->size();
    file->write((const char*)&pointsCount,4);

    for (size_t pointInd=0; pointInd<pointsCount; ++pointInd)
    {
        BendPointGPOld *point = &bend->at(pointInd);

        size_t absolutePosition = point->absolutePosition;
        size_t verticalPosition = point->heightPosition;
        std::uint8_t vibratoFlag = point->vibratoFlag;

        file->write((const char*)&absolutePosition,4);
        file->write((const char*)&verticalPosition,4);
        file->write((const char*)&vibratoFlag,1);
    }
}

void writeBendGTP(std::ofstream *file, BendPoints *bend)
{
    std::uint8_t bendType=bend->getType();
    if (bendType>=1) bendType += 5;

    file->write((const char*)&bendType,1);

    //need to find a hieghest point - but this is only for output
    ///ul bendHeight = bend->getHeight();
    ///file->write((const char*)&bendHeight,4); //attention again

    size_t pointsCount = bend->size();
    file->write((const char*)&pointsCount,4);

    for (size_t pointInd=0; pointInd<pointsCount; ++pointInd)
    {
        BendPoint *point = &bend->at(pointInd);

        size_t absolutePosition = point->horizontal;
        size_t verticalPosition = point->vertical*25;
        std::uint8_t vibratoFlag = point->vFlag;

        file->write((const char*)&absolutePosition,4);
        file->write((const char*)&verticalPosition,4);
        file->write((const char*)&vibratoFlag,1);
    }
}


void readBendGTP(std::ifstream *file, BendPoints *bend)
{
    std::uint8_t bendType=0;
    file->read((char*)&bendType,1);

    size_t bendHeight = 0;
    file->read((char*)&bendHeight,4); //attention again
    //25 quarter ; 75 - 3 quartes
    //50 half ; 100 - tone
    //... 300 - three tones
    size_t pointsCount = 0;
    file->read((char*)&pointsCount,4);

    if (gtpLog)  qDebug()<< "Type "<<bendType<<"; Height "<<bendHeight<<"; N= "<<pointsCount;

    if (bendType >= 6)
        bend->setType(bendType-5); //temp action, yet not handled anyway
    else
        bend->setType(0);

    if (pointsCount > 1000)
        throw std::runtime_error("bend points count exceeds");

    for (size_t pointInd=0; pointInd<pointsCount; ++pointInd)
    {
        size_t absolutePosition = 0;
        size_t verticalPosition = 0;
        std::uint8_t vibratoFlag = 0;

        file->read((char*)&absolutePosition,4);
        file->read((char*)&verticalPosition,4);
        file->read((char*)&vibratoFlag,1);

        if (gtpLog)
            qDebug() << "Point# "<< pointInd << "; absPos="<<absolutePosition<<"; vertPos="
               <<verticalPosition<<"; vibrato- "<<vibratoFlag;


        BendPoint point;
        point.vertical = verticalPosition/25; //25 - 1/12 of 3 tones
        point.horizontal = absolutePosition;
        point.vFlag = vibratoFlag;
        bend->push_back(point);
    }

    if (gtpLog)  qDebug() << "Beng if (gtpLog)  logging finished with "<<bend->size();

    //return bend;
}




void readChordDiagramGP3(std::ifstream &file)
{
       if (gtpLog)  qDebug() << "CHORD_";

    int header=0;
    file.read((char*)&header,1);

    if (header&0x01)
    {
        char skipper[40];
        file.read((char*)skipper,25);

        file.read((char*)skipper,1); //read byte ze

        file.read((char*)skipper,34);
        skipper[34] = 0;
        ///std::string chStr = std::string(skipper);
        int firstFret = 0;
        file.read((char*)&firstFret,4);

        for (size_t i = 0; i < 6; ++i)
        {
            int fret =0;
            file.read((char*)&fret,4);
        }

        file.read((char*)skipper,36);
    }
    else
    {
         std::string chStr = readStringShiByte(file);
         int firstFret = 0;
         file.read((char*)&firstFret,4);

         if (firstFret != 0)
             for (size_t i = 0; i < 6; ++i)
             {
                 int fret =0;
                 file.read((char*)&fret,4);
             }
    }
}


void readChordDiagramGP4(std::ifstream &file)
{
   if (gtpLog)  qDebug() << "CHORD";

   int header=0;
   file.read((char*)&header,1);


   char skipper[40];

    if (header&0x01)
    {

        file.read((char*)skipper,16);


        std::uint8_t singleBy = 0;
        file.read((char*)&singleBy,1);

        file.read((char*)skipper,21);
        skipper[21] = 0;
        std::string chStr = std::string(skipper);

        if (gtpLog)
            qDebug() << "Ch text "<<chStr.c_str();

        int firstFret = 0;
        file.read((char*)&firstFret,4);//skli
        file.read((char*)&firstFret,4);

        for (size_t i = 0; i < 7; ++i)
        {
            int fret =0;
            file.read((char*)&fret,4);


        }

        file.read((char*)skipper,32);
    }
    else
    {
         //readStringShiByte(file);


         int fullLen = 0;
         file.read((char*)&fullLen,4);

         std::uint8_t singleBy = 0;
         file.read((char*)&singleBy,1);

         file.read((char*)skipper,singleBy);
         skipper[singleBy] = 0;

         std::string chStr = std::string(skipper);

         if (gtpLog)
             qDebug() << "Chh text "<<chStr.c_str() ;

         int firstFret = 0;
         file.read((char*)&firstFret,4);

         if (firstFret != 0)
             for (size_t i = 0; i < 6; ++i)
             {
                 int fret =0;
                 file.read((char*)&fret,4);
             }
    }
}

void readChanges(std::ifstream &file, Beat *cursorBeat)
{

    ChangeTable changeStruct;// = {0};

    changeStruct.newTempo = 0;

    file.read((char*)&changeStruct.newInstr,1);  //1

    file.read((char*)&changeStruct.newVolume,1);
    file.read((char*)&changeStruct.newPan,1);
    file.read((char*)&changeStruct.newChorus,1);
    file.read((char*)&changeStruct.newReverb,1);
    file.read((char*)&changeStruct.newPhaser,1);
    file.read((char*)&changeStruct.newTremolo,1);
    file.read((char*)&changeStruct.newTempo,4); //8

    if (gtpLog)  qDebug() <<  "I "<<changeStruct.newInstr<<"; V "<<changeStruct.newVolume<<"; P "<<changeStruct.newPan<<
          "; C "<<changeStruct.newChorus<<"; R "<<changeStruct.newReverb<<"; Ph "<<changeStruct.newPhaser<<
          "; Tr "<<changeStruct.newTremolo<<"; T="<<changeStruct.newTempo;


    //NO INSTR IN DOCS
    if (changeStruct.newInstr != 255)
    {

        SingleChange instrCh;
        instrCh.changeCount = 0;
        instrCh.changeType = 1;
        instrCh.changeValue = changeStruct.newInstr;
        cursorBeat->getChangesPtr()->push_back(instrCh);

    }

    if (changeStruct.newVolume != 255)
    {
        file.read((char*)&changeStruct.volumeDur,1);

        SingleChange volCh;
        volCh.changeCount = 0;
        volCh.changeType = 2;
        volCh.changeValue = changeStruct.newVolume;
        cursorBeat->getChangesPtr()->push_back(volCh);
    }

    if (changeStruct.newPan != 255)
    {
        file.read((char*)&changeStruct.panDur,1);

        SingleChange panCh;
        panCh.changeCount = 0;
        panCh.changeType = 3;
        panCh.changeValue = changeStruct.newPan;
        cursorBeat->getChangesPtr()->push_back(panCh);
    }

    if (changeStruct.newChorus != 255)
    {
        file.read((char*)&changeStruct.chorusDur,1);

        SingleChange chorusCh;
        chorusCh.changeCount = 0;
        chorusCh.changeType = 4;
        cursorBeat->getChangesPtr()->push_back(chorusCh);
    }

    if (changeStruct.newReverb != 255)
    {
         file.read((char*)&changeStruct.reverbDur,1);

         SingleChange reverbCh;
         reverbCh.changeCount = 0;
         reverbCh.changeType = 5;
         cursorBeat->getChangesPtr()->push_back(reverbCh);
    }

    if (changeStruct.newPhaser != 255)
    {
        file.read((char*)&changeStruct.phaserDur,1);

        SingleChange phaserCh;
        phaserCh.changeCount = 0;
        phaserCh.changeType = 6;
        cursorBeat->getChangesPtr()->push_back(phaserCh);
    }

    if (changeStruct.newTremolo != 255)
    {
         file.read((char*)&changeStruct.tremoloDur,1);

         SingleChange tremoloCh;
         tremoloCh.changeCount = 0;
         tremoloCh.changeType = 7;
         cursorBeat->getChangesPtr()->push_back(tremoloCh);
    }

    //-1 for ul is hiegh but 10000 bpm insane

        if (changeStruct.newTempo < 10000) //some attention here
        {
             file.read((char*)&changeStruct.tempoDur,1);
             //set changes table inside
             SingleChange tempCh;
             tempCh.changeCount = 0;
             tempCh.changeType = 8;
             tempCh.changeValue = changeStruct.newTempo;

             cursorBeat->getChangesPtr()->push_back(tempCh);

        }

    cursorBeat->setEffects(Effect::Changes);

    //refact
    file.read((char*)&changeStruct.changesTo,1); //not applied! attention
}


void readTrack(std::ifstream &file, Track *currentTrack, int gpVersion=4, int trackIndex=0, std::uint8_t verInd=255)
{
    std::uint8_t trackHeader = 0;
    file.read((char*)&trackHeader,1);


    if (trackHeader & 1)
    {
        currentTrack->setDrums(true);
        if (gtpLog)  qDebug() <<"This is drums track!";
    }

    if (gpVersion == 5)
    {
        //ONLY FOR FIRST
        if ((trackIndex==0)||(verInd==0))
        {
            size_t byteZero = 0;
            //file.read((char*)&byteZero,4);

            file.read((char*)&byteZero,1);
        }
    }

    size_t trackNameLen = 0;
    file.read((char*)&trackNameLen,1);

    if (gtpLog)  qDebug() <<"XLENTRACK " <<trackNameLen;

    char trackName[40];
    file.read((char*)trackName,40); //39? or 40?
    std::string trackNameStr(trackName);

    currentTrack->setName(trackNameStr);

    //trackName[trackNameLen] = 0;

    if (gtpLog)  qDebug() << " Track name '" << trackName << "' ; head = " <<(int)trackHeader;


    size_t stringsAmount = 0;
    file.read((char*)&stringsAmount,4);

    if (gtpLog)  qDebug() <<"N strings " << (int) stringsAmount;


    size_t tunes[7] = {0}; //TUNEC!!!
    for (size_t ii = 0; ii < 7; ++ii)
        file.read((char*)&tunes[ii],4);


    auto& tuning = currentTrack->getTuningRef();
    tuning.setStringsAmount(stringsAmount);
    for (size_t ii = 0; ii < stringsAmount; ++ii){
        if (gtpLog)  qDebug() << "Tunes for "<<ii<<" is "<<tunes[ii];
        tuning.setTune(ii,tunes[ii]);
    }


    size_t port = 0, chan = 0, chanE = 0;
    file.read((char*)&port, 4);
    file.read((char*)&chan, 4);
    file.read((char*)&chanE, 4);

    currentTrack->setMidiInfo(0,port);
    currentTrack->setMidiInfo(1,chan);
    currentTrack->setMidiInfo(2,chanE);

    if (gtpLog)  qDebug() << "Port " << port << "; chan " <<chan <<"; chanE "<<chanE;

    size_t frets=0;
    file.read((char*)&frets,4);
    size_t capo=0;
    file.read((char*)&capo,4);

    if (gtpLog)  qDebug() << "Frets " << frets <<"; capo " << capo;

    size_t trackColor=0;
    file.read((char*)&trackColor,4);

    if (gtpLog)  qDebug() << "Color - "<<trackColor;

    currentTrack->setColor(trackColor);
    currentTrack->setMidiInfo(3,frets);
    currentTrack->setMidiInfo(4,capo);

    if (gpVersion==5)
    {
        char toSkip[256];

        if (verInd == 1)
            file.read((char*)toSkip,49);
        if (verInd == 0)
            file.read((char*)toSkip,44);

        size_t intLen = 0;
        std::uint8_t byteLen = 0;

        if (verInd == 1)
        {
            file.read((char*)&intLen,4);
            file.read((char*)&byteLen,1);
            file.read((char*)toSkip,intLen-1);
            file.read((char*)&intLen,4);
            file.read((char*)&byteLen,1);
            file.read((char*)toSkip,intLen-1);
        }

        //+after track?
    }
}

void readBeatEffects(std::ifstream &file, Beat *cursorBeat)
{
    std::uint8_t beatEffectsHead1;
    std::uint8_t beatEffectsHead2;

    file.read((char*)&beatEffectsHead1,1);
    file.read((char*)&beatEffectsHead2,1);

    //LARGE
    if (gtpLog)  qDebug() << "Beat effects flag present. H1=" << beatEffectsHead1 <<
           "; H2=" << beatEffectsHead2;

    //name as bools

    if (beatEffectsHead1 & 32)
    {   //tapping poping slaping

        std::uint8_t tapPopSlap;
        file.read((char*)&tapPopSlap,1);

        if (gtpLog)  qDebug() << "TapPopSlap byte = "<<tapPopSlap;

        if (tapPopSlap)
        {
            auto beatEffSet = static_cast<Effect>(29 + tapPopSlap);
            cursorBeat->setEffects(beatEffSet);
        }
    }

    if (beatEffectsHead1 & 0x10) //16
    {
        cursorBeat->setEffects(Effect::FadeIn);
    }

    if (beatEffectsHead2 & 4)
    {   //tremolo
       if (gtpLog)  qDebug() << " read bend tremolo";
       BendPoints *tremoloBend = cursorBeat->getTremoloPtr();
       readBendGTP(&file,tremoloBend);
       cursorBeat->setEffects(Effect::Tremolo);
    }


    if (beatEffectsHead1 & 64)
    {   //updown stroke
        std::uint8_t upStroke, downStroke;
        file.read((char*)&upStroke,1);
        file.read((char*)&downStroke,1);
        if (gtpLog)  qDebug() << "Up Stroke =" << upStroke <<" Down Stroke="<<downStroke;

        if (upStroke)
        cursorBeat->setEffects(Effect::UpStroke);

        if (downStroke)
            cursorBeat->setEffects(Effect::DownStroke);
        //NOTSET
    }

    if (beatEffectsHead2 & 2)
    {   //pick stoke
        std::uint8_t pickStoke;
        file.read((char*)&pickStoke,1);
        if (gtpLog)  qDebug() << "Pick stoke ";

        if (pickStoke)
        {
            auto beatEffSet = static_cast<Effect>(32 + pickStoke);
            cursorBeat->setEffects(beatEffSet);
            //useless features cover under anoter field not effects
        }
    }
}

void readNoteEffects(std::ifstream &file, Note *newNote, int gpVersion=4)
{
    if (gtpLog)  qDebug() <<"Bit 3 in header turned on";
    //NOT SET
    std::uint8_t noteEffectsHead1, noteEffectsHead2;
    file.read((char*)&noteEffectsHead1,1);
    file.read((char*)&noteEffectsHead2,1);

    if (gtpLog)  qDebug() << "Note effects heads. H1=" <<noteEffectsHead1<<
            "; H2=" <<noteEffectsHead2;


    if (noteEffectsHead1&1)
    {//bend
        if (gtpLog)  qDebug() << "Bend found.";

        BendPoints *bend = newNote->getBendPtr();
        readBendGTP(&file,bend);
        if (gtpLog)
            qDebug()<< " Bend h "<<"; len "<<bend->size()<<"; type"<<bend->getType();

        newNote->setEffect(Effect::Bend);//first common pattern
    }


    if (noteEffectsHead1&16)
    {   //grace note
        if (gtpLog)  qDebug() << "Grace note follows";

        std::uint8_t graceFret = 0;
        std::uint8_t graceDynamic = 0;
        std::uint8_t graceTransition = 0;
        std::uint8_t graceDuration = 0;

        file.read((char*)&graceFret,1);
        file.read((char*)&graceDynamic,1);
        file.read((char*)&graceTransition,1);
        file.read((char*)&graceDuration,1);

        if (gtpLog)  qDebug()<<"Fret "<<graceFret<<" Dyn "<<graceDynamic<<" Trans "<<graceTransition<<" Dur "<<graceDuration;

        if (gpVersion==5)
        {
            std::uint8_t flags =0;
            file.read((char*)&flags,1);
            if (gtpLog)  qDebug()<<"Gp5 grace flags "<<flags;
        }

        newNote->setGraceStats({graceFret, graceDynamic, graceTransition, graceDuration});
        newNote->setGraceNote(true);

        newNote->setEffect(Effect::GraceNote); //grace note
    }

    if (noteEffectsHead2&1)
    {
        if (gtpLog)  qDebug() << "Staccato appear";

        newNote->setEffect(Effect::Stokatto); //staccato
    }

    if (noteEffectsHead2&2)
    {//palm muting

        if (gtpLog)  qDebug() << "Palm mute appear";

        newNote->setEffect(Effect::PalmMute);
    }

    if (noteEffectsHead1&2)
    {//legato
       newNote->setEffect(Effect::Legato);
       if (gtpLog)  qDebug() << "legatto turned on";
    }

    if (noteEffectsHead1&8)
    {//let ring
       newNote->setEffect(Effect::LetRing);
       if (gtpLog)  qDebug() <<" Let ring turned on";
       if (gtpLog)  qDebug() <<" if (gtpLog)  log";
    }

    if (noteEffectsHead2&4)
    {//Tremolo picking : b
        std::uint8_t tremoloPicking;
        file.read((char*)&tremoloPicking,1);
        if (gtpLog)  qDebug() << "Tremolo byte "<<tremoloPicking;
        newNote->setEffect(Effect::TremoloPick); //tremolo picking
    }

    if (noteEffectsHead2&8)
    {//Slide : b
        std::uint8_t slide;
        file.read((char*)&slide,1);
        if (gtpLog)  qDebug() << "Slide byte " <<slide;
        std::uint8_t effect = 3;
        if (slide < 5)
        {
            effect += slide;
            //legatto slide here too
        }
        else
        {
            if (slide == 255)
                effect = 3+5;
            if (slide == 254)
                effect = 3+6;
        }
        auto eff = static_cast<Effect>(effect);
        newNote->setEffect(eff);
    }


    if (noteEffectsHead2&16)
    {//Harmonics : b
        std::uint8_t harmonics;
        file.read((char*)&harmonics,1);
        if (gtpLog)  qDebug() << "Harmonics byte "<<harmonics;

        if (gpVersion==5)
        {
            if (harmonics==1)
            {
                //nautur
            }
            if (harmonics==2) //artif
            {
                std::uint8_t skipIt;
                file.read((char*)&skipIt,1);
                file.read((char*)&skipIt,1);
                file.read((char*)&skipIt,1);
            }
            if (harmonics==3) //tapp
            {
                std::uint8_t skipIt;
                file.read((char*)&skipIt,1);

            }
            if (harmonics==4) //pinch
            {

            }
            if (harmonics==5) //semi
            {

            } //attention - not sets

            std::uint8_t effect = 11;
            auto eff = static_cast<Effect>(effect);
            newNote->setEffect(eff);

        }
        else
        {

            if (harmonics==15) harmonics=2;
            if (harmonics==17) harmonics=3;
            if (harmonics==22) harmonics=6;
            std::uint8_t effect = 10 + harmonics;
            auto eff = static_cast<Effect>(effect);
            newNote->setEffect(eff);
        }
    }

    if (noteEffectsHead2&32)
    {//Trill : 2b
        std::uint8_t trill1, trill2;
        file.read((char*)&trill1,1);
        file.read((char*)&trill2,1);
        if (gtpLog)  qDebug() << "Trill b1="<<trill1<<" trill b2="<<trill2;
    }\

    if (noteEffectsHead2&64)
    {
        //vibrato
        newNote->setEffect(Effect::Vibrato);//1 is vibrato
        if (gtpLog)  qDebug() << "Vibratto turned on";
    }
}

void readNote(std::ifstream &file, Note *newNote, int gpVersion=4)
{
    //unused ul beatIndex, Bar *cursorBar,

    std::uint8_t noteHeader;
    file.read((char*)&noteHeader,1);

    std::uint8_t noteType=0;
    if (gtpLog)  qDebug() << "Note header "<<(int)noteHeader;

    newNote->setEffect(Effect::None); //flush first

    if (noteHeader & 0x20)
    {
        file.read((char*)&noteType,1);
        std::uint8_t bby=0; //^IN DOCS WE HAVE SHORT INT HERE
        //file.read((char*)&bby,1);
        if (gtpLog)  qDebug() << "Note type = "<<(int)noteType<<" : "<<bby;

        //could be leag on 2
        //dead on 3 is ghost
        //normal leeg (dead?)
        //leeged normal, leeged leeg
        newNote->setState(noteType);

        //byte sNum=newNote->getStringNumber();

        if (noteType == 2)
        {
            //1:find last fret to set here
            //2:update status for last note

            /*

            Note *prevNote=0;
            if (gtpLog)  qDebug() << "Prev note for sNum="<<sNum;

            byte beatShiftBack = 1;
            byte wasInBlock = 0;
            bool notFoundInPrev=false;

                */

            /*
            if (beatIndex >= 1)
            {
                Beat *prevBeat = &cursorBar->getV(beatIndex-beatShiftBack);

                pointx1:

                for (ul strInd = 0; strInd < prevBeat->size(); ++strInd)
                    {
                        Note *prevNoteSearch = &prevBeat->getV(strInd);
                        byte prevSNum = prevNoteSearch->getStringNumber();
                        byte fretPrevValue = prevNoteSearch->getFret();
                           if (gtpLog)  qDebug()<< strInd <<"PrevN sNum "<<prevSNum<<" "<<fretPrevValue;
                        if (sNum==prevSNum)
                        {
                            notFoundInPrev = false;
                            if (prevNote)
                                if (prevNote->getFret()==63)
                                    ;
                                       //prevNote->setFret(fretPrevValue);

                            prevNote = prevNoteSearch;
                            if (fretPrevValue==63)
                            {
                                //LZPlane..
                                if (beatIndex > beatShiftBack)
                                {
                                    ++beatShiftBack;
                                    prevBeat = &cursorBar->getV(beatIndex-beatShiftBack);
                                    strInd=-1;
                                    ++wasInBlock;
                                    notFoundInPrev=true;
                                    continue;
                                }
                                else
                                    break;
                            }
                            break;
                        }
                    }

                if (notFoundInPrev)
                {
                    ++beatShiftBack;
                    prevBeat = &cursorBar->getV(beatIndex-beatShiftBack);

                    if (beatIndex >= beatShiftBack)
                    goto pointx1;
                }
            }
            else
            {
                //protection from the fool of first leeg needed
                if (cursorBar)
                {
                     Bar *prevBar = cursorBar - 1;
                     Beat *prevBeat = &prevBar->getV(prevBar->size()-beatShiftBack);

                     pointx0:

                     for (ul strInd = 0; strInd < prevBeat->size(); ++strInd)
                         {
                             Note *prevNoteSearch = &prevBeat->getV(strInd);
                             byte prevSNum = prevNoteSearch->getStringNumber();
                             byte fretPrevValue = prevNoteSearch->getFret();
                             if (gtpLog)  qDebug() << strInd <<" PrevN sNum "<<prevSNum<<" "<<fretPrevValue;
                             if (sNum==prevSNum)
                             {
                                 notFoundInPrev = false;
                                 if (prevNote)
                                     if (prevNote->getFret()==63)
                                     {
                                           // prevNote->setFret(fretPrevValue);
                                     }

                                 prevNote = prevNoteSearch;
                                 if (fretPrevValue==63)
                                 {
                                     if (beatIndex > beatShiftBack)
                                     {
                                         ++beatShiftBack;
                                         prevBeat = &cursorBar->getV(beatIndex-beatShiftBack);
                                         strInd=-1;
                                         ++wasInBlock;
                                         notFoundInPrev = true;
                                         continue;
                                     }
                                     else
                                         break;
                                 }
                                 break;
                             }
                         }

                     if (notFoundInPrev)
                     {
                         ++beatShiftBack;
                         prevBeat = &cursorBar->getV(beatIndex-beatShiftBack);

                         if (beatIndex >= beatShiftBack)
                         goto pointx0;
                     }

                }

            }


            //SECTION 2 //set same effect

            if (prevNote)
            {
                byte prevFret = prevNote->getFret();
                if (gtpLog)  qDebug() << "Prev found "<<prevNote->getStringNumber()<<
                       " "<<prevFret ;


                prevNote->signStateLeeged();

                if (prevNote->getEffects() == 1)
                {
                    newNote->setEffect(1);
                    //and other effects not to break - refact attention
                }

                newNote->setFret(prevFret);


                if (gtpLog)  qDebug() << "After leeg sign state "<<prevNote->getState()<<" wib "<<wasInBlock;


                if (prevFret==63)
                if (gtpLog)  qDebug()<<"if (gtpLog)  log";
            }
            //*/
        }
    }

    //we push it down with a
    if (noteHeader & 1)
    {
        if (gpVersion==4)
        {
        //another duration
        if (gtpLog)  qDebug()  <<"Time independent ";
        std::uint8_t t1,t2;
        file.read((char*)&t1,1);
        file.read((char*)&t2,1);
        if (gtpLog)  qDebug()<<"T: "<<t1<<";"<<t2;
        //attention?
        }
    }

    if (noteHeader & 16)
    {
        if (gtpLog)  qDebug() <<"Bit 4 in header turned on";
        std::uint8_t bByte=0;
        file.read((char*)&bByte,1);
        if (gtpLog)  qDebug()<<"velocity byte(forte) "<<bByte;
        newNote->setVolume(bByte);
    }

    if (noteHeader & 32)
    {
        if (gtpLog)  qDebug() <<"Bit 5 in header turned on";
        std::uint8_t bByte=0;
        file.read((char*)&bByte,1);
        if (gtpLog)  qDebug()<<"some byte fret "<<bByte;
        if (noteType != 2)
        {
            if (gtpLog)  qDebug()<<"not leeg setting prev fret";
            newNote->setFret(bByte);
        }
        else
            if (gtpLog)  qDebug() <<"leeg escape prev fret";
    }



    if (noteHeader & 2)
       if (gtpLog)  qDebug() <<"Bit 1 in header turned on"; //DOT NOTE //wow - where is it turned then?

    if (noteHeader & 4)
    {
        if (gtpLog)  qDebug() <<"Bit 2 in header turned on"; //GHOST NOTE
        //ghost not here
        newNote->setEffect(Effect::GhostNote); //ghost note
    }

    if (noteHeader & 64)
    {
        if (gtpLog)  qDebug() <<"Bit 6 in header turned on"; //ACCENTED
        newNote->setEffect(Effect::HeavyAccented); //there is no heavy accented note anymore (
        //in gp4

    }

    if (noteHeader & 128)
    {
        if (gtpLog)  qDebug() <<"Bit 7 in header turned on";

        std::uint8_t bByte=0;
        std::uint8_t bByte2=0;
        file.read((char*)&bByte,1);
        file.read((char*)&bByte2,1);

        if (gtpLog)  qDebug()<<"fingering byte "<<bByte<<":"<<bByte2;
    }

    if (gpVersion==5)
    {
        if (noteHeader & 2)
        {
            newNote->setEffect(Effect::HeavyAccented); //accented heavy
            //refact to use ne w value later
        }

        char toSkip[10];

        if (noteHeader & 1)
        {
            file.read((char*)toSkip,8);
        }

        file.read((char*)toSkip,1);
    }

    if (noteHeader & 8)
    {
       readNoteEffects(file,newNote,gpVersion);
    }
}

void readBeat(std::ifstream &file, Beat *cursorBeat)
{
    std::uint8_t beatHeader = 0;
    file.read((char*)&beatHeader,1);

    bool dotted = beatHeader & 0x1;
    bool precChord = beatHeader & 0x2;
    bool precText = beatHeader & 0x4;
    bool precEffects = beatHeader & 0x8;
    bool precChanges = beatHeader & 0x10;
    bool precNTrump = beatHeader & 0x20;
    bool precStatus = beatHeader & 0x40;


    if (gtpLog)  qDebug() << "Beat header " << (int)beatHeader;

    cursorBeat->setPause(false);
    if (precStatus)
    {
        std::uint8_t beatStatus;
        file.read((char*)&beatStatus,1);
        if (gtpLog)  qDebug() <<"Beat status "<<(int)beatStatus;
        if ((beatStatus == 2) || (beatStatus == 0))
         cursorBeat->setPause(true);
    }



    std::uint8_t durationGP =0;
    file.read((char*)&durationGP,1);

    if (gtpLog)  qDebug() <<"Beat duration "<<(int)durationGP;

    std::uint8_t duration=durationGP+2; //moved from -2 double to 1
    //x - double //0 - full //1 - half
    //2- forth(qua)  //3- eights  //4-16th
    //5-32th //6-64th

    //attention
    cursorBeat->setDuration(duration);
    cursorBeat->setDotted(dotted); //notify dotted
    //dotted will go now deeper

    if (precNTrump)
    {
        size_t trumpletN = 0;
        file.read((char*)&trumpletN,4);
        if (gtpLog)  qDebug() <<"Beat tump "<<trumpletN;
        cursorBeat->setDurationDetail(trumpletN);
    }
    else
    {
       cursorBeat->setDurationDetail(0);
    }

    if (precChord)
    {
        readChordDiagramGP4(file);
    }

    if (precText)
    {
        if (gtpLog)  qDebug() << "Text";

        size_t textLen = 0;
        file.read((char*)&textLen,4);

        std::uint8_t byteLen = 0;
        file.read((char*)&byteLen,1);

        char textBufer[255];
        file.read((char*)textBufer,byteLen);

        //len+1
        textBufer[byteLen]=0;

        if (gtpLog)  qDebug() <<"TextLen "<<textLen<<" value "<<textBufer<<"; bL "<<byteLen;

        std::string foundText(textBufer);
        cursorBeat->setText(foundText);
    }

    if (precEffects)
    {
       readBeatEffects(file,cursorBeat);
    }

    if (precChanges)
    {
        if (gtpLog)  qDebug() << "Changes table found";
        readChanges(file,cursorBeat);
    }

}

//readGraceNote

size_t readStringsFlag(std::ifstream &file, std::vector<int> &polyStrings)
{
    std::uint8_t stringsFlag = 0;
    file.read((char*)&stringsFlag,1);

    size_t totalCountStrings = 0;

    //attention here was from 7 to 0 now else
    if (stringsFlag&1) polyStrings.push_back(7);//++totalCountStrings;
    if (stringsFlag&2) polyStrings.push_back(6);//++totalCountStrings;
    if (stringsFlag&4) polyStrings.push_back(5);//++totalCountStrings;
    if (stringsFlag&8) polyStrings.push_back(4);//++totalCountStrings;
    if (stringsFlag&16) polyStrings.push_back(3);//++totalCountStrings;
    if (stringsFlag&32) polyStrings.push_back(2);//++totalCountStrings;
    if (stringsFlag&64) polyStrings.push_back(1);// ++totalCountStrings;
    if (stringsFlag&128) polyStrings.push_back(0);//checkcheck
    totalCountStrings  = polyStrings.size();

    if (gtpLog)  qDebug()<<"Strings flag "<<(int)stringsFlag
      <<"; total count "<<totalCountStrings;

    return totalCountStrings;
}


void readBar(std::ifstream &file, Tab *tab, size_t tracksAmount, size_t index)
{
    size_t i = index;

    std::uint8_t beatHeader = 0;
    file.read((char*)&beatHeader,1);

    std::uint8_t precNum = beatHeader & 0x1;
    std::uint8_t precDenum = beatHeader & 0x2;
    std::uint8_t precBegRepeat = beatHeader & 0x4;
    std::uint8_t precEndRepeat = beatHeader & 0x8;
    std::uint8_t precNumAltEnding = beatHeader & 0x10;
    std::uint8_t precMarker = beatHeader & 0x20;
    std::uint8_t precTonality = beatHeader & 0x40;
    std::uint8_t precDoubleBar = beatHeader & 0x80;

    if (gtpLog)  qDebug()<< i << " beat h= " << (int)beatHeader;
    if (gtpLog)  qDebug() << "[" << precNum << "][" << precDenum << "][" << precBegRepeat << "][" << precEndRepeat << "][" << precNumAltEnding <<
    "][" << precMarker << "][" << precTonality << "][" << precDoubleBar << "]";



    for (size_t iTrack = 0; iTrack < tracksAmount; ++iTrack)
    {
        auto& currentBar = tab->at(iTrack)->at(i);
        currentBar->setRepeat(0);
        currentBar->setAltRepeat(0);
    }

    if (precBegRepeat)
    {
        for (size_t iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            auto& currentBar = tab->at(iTrack)->at(i);
            currentBar->setRepeat(1); //ow shit - its broken
        }
    }

    if (precNum)
    {
        std::uint8_t signNumeration = 0;
        file.read((char*)&signNumeration,1);
        if (gtpLog)  qDebug() << "Set num to " <<(int)signNumeration;

        for (size_t iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            auto& currentBar = tab->at(iTrack)->at(i);
            currentBar->setSignNum(signNumeration);
        }
    }
    else
    {
        for (size_t iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            auto& currentBar = tab->at(iTrack)->at(i);
            currentBar->setSignNum(0);
        }
    }

    if (precDenum)
    {
        std::uint8_t signDenumeration = 0;
        file.read((char*)&signDenumeration,1);
        if (gtpLog)  qDebug() << "Set denum to "	<<(int)signDenumeration;

        for (size_t iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            auto& currentBar = tab->at(iTrack)->at(i);
            currentBar->setSignDenum(signDenumeration);
        }
    }
    else
    {
        for (size_t iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            auto& currentBar = tab->at(iTrack)->at(i);
            currentBar->setSignDenum(0);
        }
    }

    if (precEndRepeat)
    {
        std::uint8_t repeatTimes = 0;
        file.read((char*)&repeatTimes,1);
        if (gtpLog)  qDebug() << "Repeat times " <<(int)repeatTimes;
        //i'm not sure, but repeat flag appear on next bar
        //maybe its bug or how it used to be on gtp

        //attention ! looks like 4 and 5 versions a different
        ++repeatTimes;

        for (size_t iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            auto& currentBar = tab->at(iTrack)->at(i);
            currentBar->setRepeat(2,repeatTimes); //ow shit
        }
    }
    if (precNumAltEnding)
    {
        std::uint8_t altEnding = 0;
        file.read((char*)&altEnding,1);
        if (gtpLog)  qDebug() << "AltEnding " << (int)altEnding;
        for (size_t iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            auto& currentBar = tab->at(iTrack)->at(i);
            currentBar->setAltRepeat(altEnding); //ow shit
        }
    }
    if (precMarker)
    {
        size_t unknown = 0;
        file.read((char*)&unknown,4); //they say its a byte.. fuck them..

        std::uint8_t markerSize;
        file.read((char*)&markerSize,1);

        char markerBufer[255];
        file.read((char*)markerBufer,markerSize);
        markerBufer[markerSize] = 0;

        size_t markerColor;
        file.read((char*)&markerColor,4);

        if (gtpLog)  qDebug() << "Marker size "<<markerSize<<" buf "<<markerBufer;

        std::string markerBuferStr(markerBufer);
        for (size_t iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            auto& currentBar = tab->at(iTrack)->at(i);
            currentBar->setMarker(markerBuferStr,markerColor);
        }
    }
    if (precTonality)  //4?
    {
        std::uint8_t tonality = 0;
        file.read((char*)&tonality,1); //skip 1!!
        file.read((char*)&tonality,1);
        if (gtpLog)  qDebug() << "Tonality " <<(int)tonality;
        for (size_t iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            auto& currentBar = tab->at(iTrack)->at(i);
            currentBar->setTonality(tonality);
        }
    }
}


bool Gp4Import::import(std::ifstream &file, Tab *tab, std::uint8_t knownVersion)
{
    if (gtpLog)  qDebug() << "Starting GP4 import";


    if (knownVersion==0)
    {
        std::uint8_t preVersion;///???
        file.read((char*)&preVersion,1);
        std::string formatVersion = readString(file,29);
        std::uint8_t postVersion;///???
        file.read((char*)&postVersion,1);
    }

    std::string title,subtitle,interpret,albumn,author,copyright,tabAuthor,instructions;

    title = readString(file);
    //std::cerr << "title " << title << std::endl;
    subtitle = readString(file);
    interpret = readString(file);
    albumn = readString(file);
    author = readString(file);
    copyright = readString(file);
    tabAuthor = readString(file);
    instructions = readString(file);

    //notice
    size_t noticeLen = 0;
    file.read((char*)&noticeLen,4);
    if (gtpLog)  qDebug() << "Notice len is " << (int)noticeLen;

    if (noticeLen > 0)
    {
        if (gtpLog)  qDebug() << "Read notices ";
        for (size_t i =0 ; i < noticeLen; ++i)
            std::string noticeOne = readString(file);
    }

    std::uint8_t tripletFeel = 0;
    file.read((char*)&tripletFeel,1);

    int tripletFeelInt = (int)tripletFeel; //hate this - if (gtpLog)  log should fix it
    //refact
    if (gtpLog)  qDebug() << "Triplet feel = " << tripletFeelInt;

    size_t lyTrack = 0;
    file.read((char*)&lyTrack,4);
    if (gtpLog)  qDebug() << "Lyrics track " <<(int)lyTrack;

    for (size_t i = 0; i < 5; ++i)
    {
        size_t emWo = 0;
        file.read((char*)&emWo,4);
        std::string lyricsOne = readString(file);
    }

    size_t bpm = 0;
    int signKey = 0;
    std::uint8_t octave = 0;

    file.read((char*)&bpm,4);
    file.read((char*)&signKey,4);
    file.read((char*)&octave,1);

    tab->setBPM(bpm);

    if (gtpLog)  qDebug() <<"Bpm rate is " << bpm;
    if (gtpLog)  qDebug() <<"Sign Key = " <<signKey << " ; octave " <<octave ;

    //4 8 - 12
    char midiChannelsData[768];
    file.read((char*)midiChannelsData,768);

    //debug
    if (gtpLog)
        qDebug() << "Midi Channels data read. size of structure: "<<(int)sizeof(MidiChannelInfo)<<
           "; full size = "<<(int)(sizeof(MidiChannelInfo)*64)
               <<" and ul "<<(int)(sizeof(unsigned int));

    tab->copyMidiChannelInfo(midiChannelsData);

    size_t beatsAmount = 0;
    size_t tracksAmount = 0;

    file.read((char*)&beatsAmount,4);
    file.read((char*)&tracksAmount,4);

    if (gtpLog)  qDebug() << "Beats count " <<beatsAmount<<"; tracks count " <<tracksAmount;


    for (size_t i = 0;i < tracksAmount; ++i)
    {
        auto newTrack = std::make_unique<Track>();
        newTrack->setParent(tab);

        for (size_t j = 0; j < beatsAmount; ++j)
        {
            auto newBeatBar = std::make_unique<Bar>(); //RESERVATION
            newTrack->push_back(std::move(newBeatBar));
        }

        tab->push_back(std::move(newTrack));
    }


    for (size_t i = 0; i < beatsAmount; ++i)
        readBar(file,tab,tracksAmount,i);


    for (size_t i = 0; i < tracksAmount; ++i)
    {
        Track *currentTrack = tab->at(i).get();
        readTrack(file, currentTrack);
    }

    auto& t1 =  tab->at(0);
    Bar* cursorBar = t1->at(0).get();

    Beat *cursorBeat = nullptr;
    if (cursorBar->size())
        cursorBeat =cursorBar->at(0).get();

    if (gtpLog)  qDebug() <<"Begining beats amounts "<<beatsAmount;

    size_t globalBeatsAmount = beatsAmount*tracksAmount;
    for (size_t i = 0; i < globalBeatsAmount; ++i)
    {

        size_t beatsInPair = 0;
        file.read((char*)&beatsInPair,4);

        if (gtpLog)  qDebug() <<i <<" Beats in pair " <<beatsInPair;

        //refact - over here was critical error its not usefull code
        if (beatsInPair > 1000)
        {
            if (i != 0)
            {
                if (gtpLog)  qDebug() << "DEBUG OUT";
                for (int iii = 0; iii < 10; ++iii)
                {
                    std::uint8_t singleB;
                    file.read((char*)&singleB,1);
                    if (gtpLog)  qDebug() << "[" << iii << "] = " << singleB;
                }
                if (gtpLog)  qDebug() << "DEBUG OUT";
            }

            if (gtpLog)  qDebug() << "Seams to be critical error";
        }

        size_t indexOfTrack = i % tracksAmount;
        Track *updatingTrack = tab->at(indexOfTrack).get();
        size_t indexOfBar = i / tracksAmount;
        Bar *updatingBar = updatingTrack->at(indexOfBar).get();

        cursorBar = updatingBar;
        cursorBeat = 0;

        if (cursorBar->size())
            cursorBeat = cursorBar->at(0).get();
        //++cursorBar;
        //cursorBeat = &cursorBar->getV(0);

        for (size_t ind = 0; ind < beatsInPair; ++ind)
        {
            auto newOne = std::make_unique<Beat>();
            cursorBar->push_back(std::move(newOne));
        }

        for (size_t j = 0; j < beatsInPair; ++j)
        {
            cursorBeat = cursorBar->at(j).get();
            //Reading beat main
            readBeat(file,cursorBeat);

            std::vector<int> polyStrings;
            size_t totalCountStrings = readStringsFlag(file,polyStrings);


            for (size_t noteInd = 0; noteInd < totalCountStrings; ++noteInd)
            {
                auto newNote = std::make_unique<Note>();

                std::uint8_t sNum=polyStrings[totalCountStrings-noteInd-1];
                newNote->setStringNumber(sNum);

                //then note follows
                readNote(file,newNote.get());
                cursorBeat->push_back(std::move(newNote));
            }

            ++cursorBeat;

        }
    }

    if (gtpLog)  qDebug() << "Importing finished!";
    if (gtpLog)  qDebug() << "fine.";

    return true;
}

///////////////////////WRITE OPERATIONS////////////////////////////
/// \brief writeString
/// \param file
/// \param value
/// \param stringLen
///
//  ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ


void writeString(std::ofstream &file, std::string value, size_t stringLen=0)
{
    if (stringLen==0)
        stringLen = value.length();

    file.write((const char*)&stringLen,4);
    file.write((const char*)value.c_str(),stringLen);
}


void writeTrack(std::ofstream &file, [[maybe_unused]] Track *currentTrack)
{
    std::uint8_t trackHeader = 0;
    file.write((const char*)&trackHeader,1);

    size_t trackNameLen = 0;
    file.write((const char*)&trackNameLen,1);

    char trackName[40];
    file.write((const char*)trackName,40);

    size_t stringsAmount = 0;
    file.write((const char*)&stringsAmount,4);


    size_t tunes[7] = {0}; //TUNEC!!!
    for (size_t ii = 0; ii < 7; ++ii)
        file.write((const char*)&tunes[ii],4);


    size_t port=0, chan=0, chanE=0;
    file.write((const char*)&port,4);
    file.write((const char*)&chan,4);
    file.write((const char*)&chanE,4);

    size_t frets=0;
    file.write((const char*)&frets,4);
    size_t capo=0;
    file.write((const char*)&capo,4);


    size_t trackColor=0;
    file.write((const char*)&trackColor,4);

}


void writeBeatEffects(std::ofstream &file, [[maybe_unused]] Beat *cursorBeat)
{
    //Заглушка
    std::uint8_t beatEffectsHead1 = 0; //prepare function!
    std::uint8_t beatEffectsHead2 = 0;

    file.write((const char*)&beatEffectsHead1,1);
    file.write((const char*)&beatEffectsHead2,1);

    if (beatEffectsHead1 & 32)
    {   //tapping poping slaping
        std::uint8_t tapPopSlap;
        file.write((const char*)&tapPopSlap,1);
    }

    if (beatEffectsHead2 & 4)
    {   //tremolo
       BendPoints tremoloBend;
       writeBendGTP(&file, &tremoloBend);
    }

    if (beatEffectsHead1 & 64)
    {   //updown stroke
        std::uint8_t upStroke, downStroke;
        file.write((const char*)&upStroke,1);
        file.write((const char*)&downStroke,1);
    }

    if (beatEffectsHead2 & 2)
    {   //pick stoke
        std::uint8_t pickStoke;
        file.write((const char*)&pickStoke,1);
    }
}

void writeBeat(std::ofstream &file, Beat *cursorBeat)
{   //template

    std::uint8_t beatHeader = 0; //prepare function
    file.write((char*)&beatHeader,1);

    bool precChord = beatHeader & 0x2;
    bool precText = beatHeader & 0x4;
    bool precEffects = beatHeader & 0x8;
    bool precChanges = beatHeader & 0x10;
    bool precNTrump = beatHeader & 0x20;
    bool precStatus = beatHeader & 0x40;

    if (precStatus)
    {
        std::uint8_t beatStatus;
        file.write((const char*)&beatStatus,1);
    }

    std::uint8_t durationGP =0;
    file.write((const char*)&durationGP,1);

    if (precNTrump)
    {
        size_t trumpletN = 0;
        file.write((const char*)&trumpletN,4);
    }

    if (precChord)
    {
        //readChordDiagram(file);
    }

    if (precText)
    {
        if (gtpLog)  qDebug() << "ATTENTION missing TEXT"; //!!!
    }

    if (precEffects)
    {
       writeBeatEffects(file,cursorBeat);
    }

    if (precChanges)
    {
        //readChanges(file,cursorBeat);
    }

}


void writeBar(std::ofstream &file, Bar *cursorBar)
{   //TEMPLATE

    std::uint8_t beatHeader = 0; //PREPARE
    file.write((const char*)&beatHeader,1);

    std::uint8_t precNum = beatHeader & 0x1;
    std::uint8_t precDenum = beatHeader & 0x2;
    std::uint8_t precEndRepeat = beatHeader & 0x8;
    std::uint8_t precNumAltEnding = beatHeader & 0x10;
    std::uint8_t precMarker = beatHeader & 0x20;
    std::uint8_t precTonality = beatHeader & 0x40;

    if (precNum)
    {
        std::uint8_t signNumeration = 0;
        file.write((const char*)&signNumeration,1);
    }

    if (precDenum)
    {
        std::uint8_t signDenumeration = 0;
        file.write((const char*)&signDenumeration,1);
    }

    if (precEndRepeat)
    {
        std::uint8_t repeatTimes = 0;
        file.write((const char*)&repeatTimes,1);
    }

    if (precNumAltEnding)
    {
        std::uint8_t altEnding = 0;
        file.write((const char*)&altEnding,1);
    }

    if (precMarker)
    {
        std::uint8_t markerSize = 0;
        file.write((const char*)&markerSize,1);
        char markerBufer[255];
        file.write((const char*)markerBufer, markerSize);
        size_t markerColor;
        file.write((const char*)&markerColor,4);
    }
    if (precTonality)  //4?
    {
        std::uint8_t tonality = 0;
        file.write((const char*)&tonality,1); //skip 1!! ???
        file.write((const char*)&tonality,1);
    }
}


void writeNoteEffects(std::ofstream &file, Note *newNote)
{ //TEMPLATE

    std::uint8_t noteEffectsHead1 = 0, noteEffectsHead2 = 0; //prepare
    file.write((const char*)&noteEffectsHead1,1);
    file.write((const char*)&noteEffectsHead2,1);

    if (noteEffectsHead1&1)
        writeBendGTP(&file, newNote->getBendPtr());

    if (noteEffectsHead1&16)
    {   //grace note
        std::uint8_t graceFret = 0;
        std::uint8_t graceDynamic = 0;
        std::uint8_t graceTransition = 0;
        std::uint8_t graceDuration = 0;

        file.write((const char*)&graceFret,1);
        file.write((const char*)&graceDynamic,1);
        file.write((const char*)&graceTransition,1);
        file.write((const char*)&graceDuration,1);
    }

    if (noteEffectsHead2&4)
    {//Tremolo picking : b
        std::uint8_t tremoloPicking;
        file.write((const char*)&tremoloPicking,1);
    }

    if (noteEffectsHead2&8)
    {//Slide : b
        std::uint8_t slide;
        file.write((const char*)&slide,1);
    }


    if (noteEffectsHead2&16)
    {//Harmonics : b
        std::uint8_t harmonics;
        file.write((const char*)&harmonics,1);
    }

    if (noteEffectsHead2&32)
    {//Trill : 2b
        std::uint8_t trill1, trill2;
        file.write((const char*)&trill1,1);
        file.write((const char*)&trill2,1);
    }\
}

void writeStringsFlag([[maybe_unused]] std::ofstream &file, [[maybe_unused]] Beat *cursorBeat)
{ //TEMPLATE
} //UNDONE


void writeNote(std::ofstream &file, Note *newNote)
{ //TEMPLATE

    std::uint8_t noteHeader = 0; //prepare
    file.write((const char*)&noteHeader,1);

    if (noteHeader & 0x20)
    {
        std::uint8_t noteType;
        file.write((const char*)&noteType,1);
    }

    if (noteHeader & 1)
    {
        if (gtpLog)  qDebug()  <<"Time independent ";
        std::uint8_t t1,t2;
        file.write((const char*)&t1,1);
        file.write((const char*)&t2,1);
    }

    if (noteHeader & 16)
    {
        if (gtpLog)  qDebug() <<"Bit 4 in header turned on";
        std::uint8_t bByte=0;
        file.write((const char*)&bByte,1);
    }

    if (noteHeader & 32)
    {
        if (gtpLog)  qDebug() <<"Bit 5 in header turned on";
        std::uint8_t bByte=0;
        file.write((const char*)&bByte,1);
    }

    if (noteHeader & 128)
    {
        std::uint8_t bByte=0;
        std::uint8_t bByte2=0;
        file.write((const char*)&bByte,1);
        file.write((const char*)&bByte2,1);
    }

    if (noteHeader & 8)
    {
       writeNoteEffects(file,newNote);
    }
}





//END of WRITE OPERATIONS




//////////////////////FORMAT GTP5////////////////////////////////
//--////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void readChangesGP5(std::ifstream &file, Beat *cursorBeat, std::uint8_t verInd)
{

    ChangeTable changeStruct;// = {0};

    changeStruct.newTempo = 0;

    file.read((char*)&changeStruct.newInstr,1);  //1

    char toSkip[60];

    file.read((char*)toSkip,16);


    file.read((char*)&changeStruct.newVolume,1);
    file.read((char*)&changeStruct.newPan,1);
    file.read((char*)&changeStruct.newChorus,1);
    file.read((char*)&changeStruct.newReverb,1);
    file.read((char*)&changeStruct.newPhaser,1);
    file.read((char*)&changeStruct.newTremolo,1);

    size_t strLen  = 0;
    file.read((char*)&strLen,4);
    if (strLen > 1)
    {
        //byte readByte;
        //file.read((char*)&readByte,1);

        std::string tempoName = readString(file,strLen); //not -1
        if (gtpLog)
            qDebug()<<"tempo name "<<tempoName.c_str();
    }
    else
    {
        if (gtpLog) qDebug()<<"tempo name is empty";
        std::uint8_t readByte;
        file.read((char*)&readByte,1);
    }


    file.read((char*)&changeStruct.newTempo,4); //8

    if (gtpLog)  qDebug() <<  "I "<<changeStruct.newInstr<<"; V "<<changeStruct.newVolume<<"; P "<<changeStruct.newPan<<
          "; C "<<changeStruct.newChorus<<"; R "<<changeStruct.newReverb<<"; Ph "<<changeStruct.newPhaser<<
          "; Tr "<<changeStruct.newTremolo<<"; T="<<changeStruct.newTempo;


    //NO INSTR IN DOCS
    if (changeStruct.newInstr != 255)
    {

        SingleChange instrCh;
        instrCh.changeCount = 0;
        instrCh.changeType = 1;
        instrCh.changeValue = changeStruct.newInstr;
        cursorBeat->getChangesPtr()->push_back(instrCh);

    }

    if (changeStruct.newVolume != 255)
    {
        file.read((char*)&changeStruct.volumeDur,1);

        SingleChange volCh;
        volCh.changeCount = 0;
        volCh.changeType = 2;
        volCh.changeValue = changeStruct.newVolume;
        cursorBeat->getChangesPtr()->push_back(volCh);
    }

    if (changeStruct.newPan != 255)
    {
        file.read((char*)&changeStruct.panDur,1);

        SingleChange panCh;
        panCh.changeCount = 0;
        panCh.changeType = 3;
        panCh.changeValue = changeStruct.newPan;
        cursorBeat->getChangesPtr()->push_back(panCh);
    }

    if (changeStruct.newChorus != 255)
    {
        file.read((char*)&changeStruct.chorusDur,1);

        SingleChange chorusCh;
        chorusCh.changeCount = 0;
        chorusCh.changeType = 4;
        cursorBeat->getChangesPtr()->push_back(chorusCh);
    }

    if (changeStruct.newReverb != 255)
    {
         file.read((char*)&changeStruct.reverbDur,1);

         SingleChange reverbCh;
         reverbCh.changeCount = 0;
         reverbCh.changeType = 5;
         cursorBeat->getChangesPtr()->push_back(reverbCh);
    }

    if (changeStruct.newPhaser != 255)
    {
        file.read((char*)&changeStruct.phaserDur,1);

        SingleChange phaserCh;
        phaserCh.changeCount = 0;
        phaserCh.changeType = 6;
        cursorBeat->getChangesPtr()->push_back(phaserCh);
    }

    if (changeStruct.newTremolo != 255)
    {
         file.read((char*)&changeStruct.tremoloDur,1);

         SingleChange tremoloCh;
         tremoloCh.changeCount = 0;
         tremoloCh.changeType = 7;
         cursorBeat->getChangesPtr()->push_back(tremoloCh);
    }

    //-1 for ul is hiegh but 10000 bpm insane

   if (changeStruct.newTempo < 100000) //some attention here
    {
         file.read((char*)&changeStruct.tempoDur,1);
         //set changes table inside
         SingleChange tempCh;
         tempCh.changeCount = 0;
         tempCh.changeType = 8;
         tempCh.changeValue = changeStruct.newTempo;

         cursorBeat->getChangesPtr()->push_back(tempCh);

         if (verInd==1)
         {
            std::uint8_t someSkip;
            file.read((char*)&someSkip,1);

            if (someSkip)
            {
                if (gtpLog)
                    qDebug()<<"Skip byte = "<<someSkip;
            }
         }

    }

    cursorBeat->setEffects(Effect::Changes);

    //refact
    file.read((char*)&changeStruct.changesTo,1); //not applied! attention

    //if (1)
    {
        //char toSkip[2];
        //file.read((char*)toSkip,49); //or 44

        //ul intLen = 0;
        std::uint8_t byteLen = 0;


        file.read((char*)&byteLen,1); //skipperd

        //file.read((char*)&intLen,4);

        if (verInd==1)
        {
            /* looks like 70 is missing 1byte
            for (int z=0; z<5; ++z)
            {
                byte readOne = 0;
                file.read((char*)&readOne,1);
                qDebug() <<"R n# "<<z<<" "<<readOne;
            }
            */

            //another function?


            std::string rS1 = readString(file);
            std::string rS2 = readString(file);

            if (gtpLog)
            {
                qDebug() <<"R1 "<<rS1.c_str()<<" : R1";
                qDebug() <<"R2 "<<rS2.c_str()<<" : R2";
            }

        }
    }
}

void readChordDiagramGP5(std::ifstream &file)
{
    //on version 4 it could be not always the same!!!
    //readChordDiagram(file);



    //return;
    char chordBufer[64];
    if (gtpLog)  qDebug() << "Chord";
    file.read((char*)chordBufer,17);


    //file.read((char*)chordBufer,21);
    //chordBufer[21]=0;

    char fByte = 0; file.read((char*)&fByte,1);
    std::string chStr = readString(file,21);

    if (gtpLog)
        qDebug() <<fByte<< " Ch str "<<chStr.c_str();


    file.read((char*)chordBufer,4);

    int firstFret = 0;
    file.read((char*)&firstFret,4); //first fret

    for (size_t i =0; i < 7; ++i)
    {
        file.read((char*)chordBufer,4);//string fret?
    }

    file.read((char*)chordBufer,32);

    //on version 4 it could be not always the same!!!
}


void readBeatGP5(std::ifstream &file, Beat *cursorBeat, std::uint8_t verInd=255)
{
    std::uint8_t beatHeader = 0;
    file.read((char*)&beatHeader,1);

    bool dotted = beatHeader & 0x1;
    bool precChord = beatHeader & 0x2;
    bool precText = beatHeader & 0x4;
    bool precEffects = beatHeader & 0x8;
    bool precChanges = beatHeader & 0x10;
    bool precNTrump = beatHeader & 0x20;
    bool precStatus = beatHeader & 0x40;


    if (gtpLog)  qDebug() << "Beat header " << (int)beatHeader;

    cursorBeat->setPause(false);
    if (precStatus)
    {
        std::uint8_t beatStatus;
        file.read((char*)&beatStatus,1);
        if (gtpLog)  qDebug() <<"Beat status "<<(int)beatStatus;
        if ((beatStatus == 2) || (beatStatus == 0))
         cursorBeat->setPause(true);
    }



    std::uint8_t durationGP =0;
    file.read((char*)&durationGP,1);

    if (gtpLog)  qDebug() <<"Beat duration "<<(int)durationGP;

    std::uint8_t duration=durationGP+2; //moved from -2 double to 1
    //x - double //0 - full //1 - half
    //2- forth(qua)  //3- eights  //4-16th
    //5-32th //6-64th

    //attention
    cursorBeat->setDuration(duration);
    cursorBeat->setDotted(dotted); //notify dotted
    //dotted will go now deeper

    if (precNTrump)
    {
        size_t trumpletN = 0;
        file.read((char*)&trumpletN,4);
        if (gtpLog)  qDebug() <<"Beat tump "<<trumpletN;
        cursorBeat->setDurationDetail(trumpletN);
    }
    else
    {
       cursorBeat->setDurationDetail(0);
    }

    if (precChord)
    {
        readChordDiagramGP5(file);
    }

    if (precText)
    {
        if (gtpLog)  qDebug() << "TEXT";

        size_t textLen = 0;
        file.read((char*)&textLen,4);

        std::uint8_t byteLen = 0;
        file.read((char*)&byteLen,1);

        char textBufer[255];
        file.read((char*)textBufer,byteLen);

        //len+1
        textBufer[byteLen]=0;

        if (gtpLog)  qDebug() <<"TextLen "<<textLen<<" value "<<textBufer<<"; bL "<<byteLen;

        std::string foundText(textBufer);
        cursorBeat->setText(foundText);
    }

    if (precEffects)
    {
       readBeatEffects(file,cursorBeat);
    }

    if (precChanges)
    {
        if (gtpLog)  qDebug() << "Changes table found";
        readChangesGP5(file,cursorBeat,verInd);
    }


}

void readBarGP5(std::ifstream &file, Tab *tab, size_t tracksAmount, size_t index)
{
    size_t i = index;

    std::uint8_t beatHeader = 0;
    file.read((char*)&beatHeader,1);

    std::uint8_t precNum = beatHeader & 0x1;
    std::uint8_t precDenum = beatHeader & 0x2;
    std::uint8_t precBegRepeat = beatHeader & 0x4;
    std::uint8_t precEndRepeat = beatHeader & 0x8;
    std::uint8_t precNumAltEnding = beatHeader & 0x10;
    std::uint8_t precMarker = beatHeader & 0x20;
    std::uint8_t precTonality = beatHeader & 0x40;
    std::uint8_t precDoubleBar = beatHeader & 0x80;

    if (gtpLog)  qDebug()<< i << " beat h= " << (int)beatHeader;
    if (gtpLog)  qDebug() << "[" << precNum << "][" << precDenum << "]["
        << precBegRepeat << "][" << precEndRepeat << "][" << precNumAltEnding <<
        "][" << precMarker << "][" << precTonality << "][" << precDoubleBar << "]";



    for (size_t iTrack = 0; iTrack < tracksAmount; ++iTrack)
    {
        auto& currentBar = tab->at(iTrack)->at(i);
        currentBar->setRepeat(0);
        currentBar->setAltRepeat(0);
    }

    if (precBegRepeat)
    {
        for (size_t iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            auto& currentBar = tab->at(iTrack)->at(i);
            currentBar->setRepeat(1); //ow shit - its broken
        }
    }

    if (precNum)
    {
        std::uint8_t signNumeration = 0;
        file.read((char*)&signNumeration,1);
        if (gtpLog)  qDebug() << "Set num to " <<(int)signNumeration;

        for (size_t iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            auto& currentBar = tab->at(iTrack)->at(i);
            currentBar->setSignNum(signNumeration);
        }
    }
    else
    {
        for (size_t iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            auto& currentBar = tab->at(iTrack)->at(i);
            currentBar->setSignNum(0);
        }
    }

    if (precDenum)
    {
        std::uint8_t signDenumeration = 0;
        file.read((char*)&signDenumeration,1);
        if (gtpLog)  qDebug() << "Set denum to "	<<(int)signDenumeration;

        for (size_t iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            auto& currentBar = tab->at(iTrack)->at(i);
            currentBar->setSignDenum(signDenumeration);
        }
    }
    else
    {
        for (size_t iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            auto& currentBar = tab->at(iTrack)->at(i);
            currentBar->setSignDenum(0);
        }
    }

    if (precEndRepeat)
    {
        std::uint8_t repeatTimes = 0;
        file.read((char*)&repeatTimes,1);
        if (gtpLog)  qDebug() << "Repeat times " <<(int)repeatTimes;
        //i'm not sure, but repeat flag appear on next bar
        //maybe its bug or how it used to be on gtp
        for (size_t iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            auto& currentBar = tab->at(iTrack)->at(i);
            currentBar->setRepeat(2,repeatTimes); //ow shit
        }
    }

    if (precMarker)
    {
        size_t unknown = 0;
        file.read((char*)&unknown,4); //they say its a byte.. fuck them..

        size_t markerSize=0;
        file.read((char*)&markerSize,1);

        //file.read((char*)&markerSize,4);

        char markerBufer[255];
        file.read((char*)markerBufer,markerSize);
        markerBufer[markerSize] = 0;

        size_t markerColor;
        file.read((char*)&markerColor,4);

        if (gtpLog)  qDebug() << "Marker size "<<markerSize<<" buf "<<markerBufer;

        std::string markerBuferStr(markerBufer);
        for (size_t iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            auto& currentBar = tab->at(iTrack)->at(i);
            currentBar->setMarker(markerBuferStr,markerColor);
        }
    }

    if (precNumAltEnding)
    {
        std::uint8_t altEnding = 0;
        file.read((char*)&altEnding,1);
        if (gtpLog)  qDebug() << "AltEnding " << (int)altEnding;
        for (size_t iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            auto& currentBar = tab->at(iTrack)->at(i);
            currentBar->setAltRepeat(altEnding); //ow shit
        }
    }

    if (precTonality)  //4?
    {
        std::uint8_t tonality = 0;
        file.read((char*)&tonality,1); //skip 1!!
        file.read((char*)&tonality,1);
        if (gtpLog)  qDebug() << "Tonality " <<(int)tonality;
        for (size_t iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            auto& currentBar = tab->at(iTrack)->at(i);
            currentBar->setTonality(tonality);
        }
    }

    char toSkip[5];
    if (precNum || precDenum)
    {
       file.read((char*)toSkip,4);
    }

    if (!precNumAltEnding)
    {
        file.read((char*)toSkip,1);
    }

    //not fromhere
    std::uint8_t tripletFeel=0;
    file.read((char*)&tripletFeel,1);

   //byte skipByteOh = 0;
    //file.read((char*)&skipByteOh,1);

}


bool Gp5Import::import(std::ifstream& file, Tab *tab, std::uint8_t knownVersion)
{
    if (gtpLog)  qDebug() << "Starting GP5 import";

    char placeToSkip[255];
    std::uint8_t versionIndex = 255;

    if (knownVersion)
    {
        if (knownVersion == 5)
            versionIndex = 0;
        if (knownVersion == 51)
            versionIndex = 1;
    }
    else
    {
        std::uint8_t preVersion;///???
        file.read((char*)&preVersion,1);
        std::string formatVersion = readString(file,29);
        std::uint8_t postVersion;///???
        file.read((char*)&postVersion,1);


        std::string version0 = "FICHIER GUITAR PRO v5.00";
        std::string version1 = "FICHIER GUITAR PRO v5.10";

        std::string versionCut = formatVersion.substr(0,version0.size());



        if (versionCut == version0)
            versionIndex = 0;
        if (versionCut == version1)
            versionIndex = 1;
    }

    std::string title,subtitle,interpret,albumn,author,copyright,tabAuthor,instructions,words;

    title = readStringShiByte(file);
    subtitle = readStringShiByte(file);
    interpret = readStringShiByte(file);
    albumn = readStringShiByte(file);
    words  = readStringShiByte(file);
    author = readStringShiByte(file);
    copyright = readStringShiByte(file);
    tabAuthor = readStringShiByte(file);
    instructions = readStringShiByte(file);

    //notice
    size_t noticeLen = 0;
    file.read((char*)&noticeLen,4);
    if (gtpLog)  qDebug() << "Notice len is " << (int)noticeLen;

    if (noticeLen > 0)
    {
        if (gtpLog)  qDebug() << "Read notices ";
        for (size_t i =0 ; i < noticeLen; ++i)
        {
            /*
            ul intLen = 0;
            file.read((char*)&intLen,4);
            byte noteLen = 0;
            file.read((char*)&noteLen,1);
            qDebug() <<"Note strlens "<<noteLen<<" "<<intLen;
            */
            std::string noticeOne = readStringShiByte(file); //,noteLe
            if (gtpLog)
                qDebug()<<"Notice#"<<i<<" "<<noticeOne.c_str();
        }
    }

    //byte tripletFeel = 0; //not here in gp5
    //file.read((char*)&tripletFeel,1);


    size_t lyTrack = 0;
    file.read((char*)&lyTrack,4);
    if (gtpLog)  qDebug() << "Lyrics track " <<(int)lyTrack ;

    for (size_t i = 0; i < 5; ++i)
    {
        size_t emWo = 0;
        file.read((char*)&emWo,4);
        std::string lyricsOne = readString(file);
    }


    if (versionIndex == 1)
        file.read((char*)placeToSkip,49);
    else
        if (versionIndex == 0)
            file.read((char*)placeToSkip,30);

    for (size_t i = 0; i < 11; ++i)
    {
         size_t pageInt = 0;
         file.read((char*)&pageInt,4);
         std::uint8_t strLenIn = 0;
         file.read((char*)&strLenIn,1);

         if (gtpLog)
         qDebug()<<"Page int "<<pageInt<<" strLen "<<strLenIn;
         std::string whatIsThat = readString(file,strLenIn);
    }



    size_t bpm = 0;
    int signKey = 0;
    std::uint8_t octave = 0;

    file.read((char*)&bpm,4);


    if (versionIndex == 1)
    {
        std::uint8_t skipByte=0;
        file.read((char*)&skipByte,1);
    }

    file.read((char*)&signKey,4);
    file.read((char*)&octave,1);

    tab->setBPM(bpm);

    if (gtpLog)  qDebug() <<"Bpm rate is " << bpm ;
    if (gtpLog)  qDebug() <<"Sign Key = " <<signKey << " ; octave " <<octave ;


    //4 8 - 12
    char midiChannelsData[768];
    file.read((char*)midiChannelsData,768);

    if (gtpLog)  qDebug() << "Midi Channels data read. size of structure: "<<(int)sizeof(MidiChannelInfo)<<
           "; full size = "<<(int)(sizeof(MidiChannelInfo)*64);

    tab->copyMidiChannelInfo(midiChannelsData);

    //Unknonwn skip
    file.read((char*)placeToSkip,42);


    size_t beatsAmount = 0;
    size_t tracksAmount = 0;

    file.read((char*)&beatsAmount,4);
    file.read((char*)&tracksAmount,4);

    if (gtpLog)  qDebug() << "Beats count " <<beatsAmount<<"; tracks count " <<tracksAmount;


    for (size_t i = 0;i < tracksAmount; ++i) {
        auto newTrack = std::make_unique<Track>();
        newTrack->setParent(tab);

        for (size_t j = 0; j < beatsAmount; ++j) {
            auto newBeatBar = std::make_unique<Bar>();
            newTrack->push_back(std::move(newBeatBar));
        }
        tab->push_back(std::move(newTrack));
    }

    for (size_t i = 0; i < beatsAmount; ++i) {
        std::uint8_t skipOne=0;
        if (i > 0) {
            file.read((char*)&skipOne,1);
        }
        readBarGP5(file,tab,tracksAmount,i);
    }

    for (size_t i = 0; i < tracksAmount; ++i)
    {
        Track *currentTrack = tab->at(i).get();
        readTrack(file,currentTrack,5,i,versionIndex);//5 is the version
    }

    /*
    qDebug() <<"Tracks "<<(int)tab->size();
    qDebug() <<"-";
    qDebug()<<" in cur "<<(int)tab->getV(0)->size();
    qDebug() <<"+";
    */

    Bar *cursorBar = tab->at(0)->at(0).get();
    Beat *cursorBeat = 0; //cursorBar->getV(0);

    if (cursorBar)
    if (cursorBar->size())
     cursorBeat =cursorBar->at(0).get();

    if (gtpLog)  qDebug() <<"Begining beats amounts "<<beatsAmount ;
    std::uint8_t oneSkip = 0;

    if (versionIndex == 1)
        file.read((char*)&oneSkip,1);
    else
        if (versionIndex == 0)
        {
            file.read((char*)&oneSkip,1);
            file.read((char*)&oneSkip,1);
        }



    size_t globalBeatsAmount = beatsAmount*tracksAmount;
    for (size_t i = 0; i < globalBeatsAmount; ++i) //
    {
            //SOMWHERE OVER HERE IN GTP5 lays voices
            //for (int voices=0; voices <2; ++voices)
            {
            size_t beatsInPair = 0;
            file.read((char*)&beatsInPair,4);

            if (gtpLog)  qDebug() <<i <<" Beats in pair " <<beatsInPair  ;

            //refact - over here was critical error its not usefull code
            if (beatsInPair > 1000)
            {
                if (i != 0)
                {
                    if (gtpLog)  qDebug() << "DEBUG OUT";
                    for (int iii = 0; iii < 10; ++iii)
                    {
                        std::uint8_t singleB;
                        file.read((char*)&singleB,1);
                        if (gtpLog)  qDebug() << "[" << iii << "] = " << singleB;
                    }
                    if (gtpLog)  qDebug() << "DEBUG OUT";
                }

                if (gtpLog)  qDebug() << "Seams to be critical error";
            }

            size_t indexOfTrack = i % tracksAmount;
            auto& updatingTrack = tab->at(indexOfTrack);
            size_t indexOfBar = i / tracksAmount;
            Bar *updatingBar = updatingTrack->at(indexOfBar).get();

            cursorBar = updatingBar;
            cursorBeat = 0;

            if (cursorBar->size())
                cursorBeat = cursorBar->at(0).get();
            //++cursorBar;
            //cursorBeat = &cursorBar->getV(0);

            for (size_t ind = 0; ind < beatsInPair; ++ind)
            {
                auto newOne = std::make_unique<Beat>();
                cursorBar->push_back(std::move(newOne));
            }

            for (size_t j = 0; j < beatsInPair; ++j)
            {
                cursorBeat = cursorBar->at(j).get();
                //Reading beat main
                readBeatGP5(file,cursorBeat,versionIndex);

                std::vector<int> polyStrings;
                size_t totalCountStrings = readStringsFlag(file,polyStrings);


                //first here is fine
                for (size_t noteInd = 0; noteInd < totalCountStrings; ++noteInd)
                {
                    auto newNote = std::make_unique<Note>();

                    std::uint8_t sNum=polyStrings[totalCountStrings-noteInd-1];
                    newNote->setStringNumber(sNum);

                    //then note follows
                    readNote(file,newNote.get(),5); //format 5
                    cursorBeat->push_back(std::move(newNote));
                }

                ++cursorBeat;

                //additions
                std::uint8_t readByte=0;
                file.read((char*)&readByte,1);
                file.read((char*)&readByte,1);

                if (readByte & 0x8)
                {
                    std::uint8_t oneMore;
                    file.read((char*)&oneMore,1);
                }
            }
        }
        //SOMWHERE OVER HERE IN GTP5 lays voices


        //OF FUCKS OF ATTENTION GROW HERE PLEASE(SORRY GREAT ONE)(YOU ARE THE ONLY who helps always)
        ///ITS A NIGHTMARE
        //God damn skip

      for (size_t subvoice=0; subvoice < 1; ++subvoice)
      {
          size_t preReader = 0;

          file.read((char*)&preReader,4);

          if (gtpLog)
            qDebug()<<" PRE-Reader "<<preReader;

          if (preReader > 10000)
              throw std::runtime_error("pre reader exceeds");

          Beat readerBeat;
          //Bar readerBar;
          for (size_t i = 0; i < preReader; ++i)
          {
              readBeatGP5(file,&readerBeat,versionIndex);

              std::vector<int> polyStrings;
              size_t totalCountStrings = readStringsFlag(file,polyStrings);

              //first here is fine
              for (size_t noteInd = 0; noteInd < totalCountStrings; ++noteInd)
              {
                  Note newNote;
                  std::uint8_t sNum=polyStrings[totalCountStrings-noteInd-1];
                  newNote.setStringNumber(sNum);
                  readNote(file,&newNote,5); //format 5
                  //cursorBeat->add(newNote);
              }

              //additions
              std::uint8_t readByte0,readByte=0;
              file.read((char*)&readByte0,1);
              file.read((char*)&readByte,1);

              if (readByte & 0x8)
              {
                  std::uint8_t oneMore;
                  file.read((char*)&oneMore,1);
              }
          }

          std::uint8_t skipperByte=0;
          file.read((char*)&skipperByte,1);
      }

      /*
      for (size_t i = 0; i < 11; ++i)
      {
          byte reader = 0;
          file.read((char*)&reader,1);

          if (gtpLog)
            qDebug()<<" Reader "<<reader;
      } //*/

      if (gtpLog)
        qDebug()<<"reaad";

    }

    if (gtpLog)  qDebug() << "Importing finished!";
    if (gtpLog)  qDebug() << "fine.";

    return true;
}

/////////////////////   GP 3  /////////////////////////////////

void readChangesGP3(std::ifstream &file, Beat *cursorBeat)
{

    //gtpLog = true; //autolog!

    ChangeTable changeStruct;// = {0};

    changeStruct.newTempo = 0;

    file.read((char*)&changeStruct.newInstr,1);  //1

    file.read((char*)&changeStruct.newVolume,1);
    file.read((char*)&changeStruct.newPan,1);
    file.read((char*)&changeStruct.newChorus,1);
    file.read((char*)&changeStruct.newReverb,1);
    file.read((char*)&changeStruct.newPhaser,1);
    file.read((char*)&changeStruct.newTremolo,1);
    file.read((char*)&changeStruct.newTempo,4); //8

    if (gtpLog)  qDebug() <<  "I "<<changeStruct.newInstr<<"; V "<<changeStruct.newVolume<<"; P "<<changeStruct.newPan<<
          "; C "<<changeStruct.newChorus<<"; R "<<changeStruct.newReverb<<"; Ph "<<changeStruct.newPhaser<<
          "; Tr "<<changeStruct.newTremolo<<"; T="<<changeStruct.newTempo;


    //NO INSTR IN DOCS
    if (changeStruct.newInstr != 255)
    {
        //file.read((char*)&changeStruct.instrDur,1);

        SingleChange instrCh;
        instrCh.changeCount = 0;
        instrCh.changeType = 1;
        instrCh.changeValue = changeStruct.newInstr;
        cursorBeat->getChangesPtr()->push_back(instrCh);

    }

    if (changeStruct.newVolume != 255)
    {
        file.read((char*)&changeStruct.volumeDur,1);

        SingleChange volCh;
        volCh.changeCount = 0;
        volCh.changeType = 2;
        volCh.changeValue = changeStruct.newVolume;
        cursorBeat->getChangesPtr()->push_back(volCh);
    }

    if (changeStruct.newPan != 255)
    {
        file.read((char*)&changeStruct.panDur,1);

        SingleChange panCh;
        panCh.changeCount = 0;
        panCh.changeType = 3;
        panCh.changeValue = changeStruct.newPan;
        cursorBeat->getChangesPtr()->push_back(panCh);
    }

    if (changeStruct.newChorus != 255)
    {
        file.read((char*)&changeStruct.chorusDur,1);

        SingleChange chorusCh;
        chorusCh.changeCount = 0;
        chorusCh.changeType = 4;
        cursorBeat->getChangesPtr()->push_back(chorusCh);
    }

    if (changeStruct.newReverb != 255)
    {
         file.read((char*)&changeStruct.reverbDur,1);

         SingleChange reverbCh;
         reverbCh.changeCount = 0;
         reverbCh.changeType = 5;
         cursorBeat->getChangesPtr()->push_back(reverbCh);
    }

    if (changeStruct.newPhaser != 255)
    {
        file.read((char*)&changeStruct.phaserDur,1);

        SingleChange phaserCh;
        phaserCh.changeCount = 0;
        phaserCh.changeType = 6;
        cursorBeat->getChangesPtr()->push_back(phaserCh);
    }

    if (changeStruct.newTremolo != 255)
    {
         file.read((char*)&changeStruct.tremoloDur,1);

         SingleChange tremoloCh;
         tremoloCh.changeCount = 0;
         tremoloCh.changeType = 7;
         cursorBeat->getChangesPtr()->push_back(tremoloCh);
    }

    //-1 for ul is hiegh but 10000 bpm insane

        if (changeStruct.newTempo < 10000) //some attention here
        {
             file.read((char*)&changeStruct.tempoDur,1);
             //set changes table inside
             SingleChange tempCh;
             tempCh.changeCount = 0;
             tempCh.changeType = 8;
             tempCh.changeValue = changeStruct.newTempo;

             cursorBeat->getChangesPtr()->push_back(tempCh);

        }

    cursorBeat->setEffects(Effect::Changes);
}

void readBeatEffectsGP3(std::ifstream &file, Beat *cursorBeat)
{
    std::uint8_t beatEffectsHead;
    file.read((char*)&beatEffectsHead,1);

    if (gtpLog)  qDebug() << "Beat effects flag present. H1=" << beatEffectsHead;

    if ((beatEffectsHead&1)||(beatEffectsHead&2))
    {
        //vibrato
        cursorBeat->setEffects(Effect::PalmMute);
    }

    if (beatEffectsHead&10)
    {
        //fade in
        cursorBeat->setEffects(Effect::FadeIn); //would be fade in
    }

    if (beatEffectsHead & 32)
    {   //tapping poping slaping

        std::uint8_t tapPopSlap;
        file.read((char*)&tapPopSlap,1);

        if (gtpLog)  qDebug() << "TapPopSlap byte = "<<tapPopSlap;

        if (tapPopSlap)
        {
            auto beatEffSet = static_cast<Effect>(29 + tapPopSlap);
            cursorBeat->setEffects(beatEffSet);
            int skipInt;
            file.read((char*)&skipInt,4);
        }
        else
        {
            if (gtpLog)  qDebug() << " read bend tremolo";
            BendPoints tremoloBend;
            int tremoloValue = 0;
            file.read((char*)&tremoloValue,4);
            //readBend(file,tremoloBend);
            cursorBeat->setEffects(Effect::Tremolo); //would be tremolo
        }
    }

    if (beatEffectsHead & 64)
    {   //updown stroke
        std::uint8_t upStroke, downStroke;
        file.read((char*)&upStroke,1);
        file.read((char*)&downStroke,1);
        if (gtpLog)  qDebug() << "Up Stroke =" << upStroke <<" Down Stroke="<<downStroke;

        if (upStroke)
        cursorBeat->setEffects(Effect::UpStroke); //upstroke

        if (downStroke)
            cursorBeat->setEffects(Effect::DownStroke);

        if (gtpLog)  qDebug() << "if (gtpLog)  log";
        //NOTSET
    }

    if (beatEffectsHead & 4)
    {
        //harm natur
        cursorBeat->setEffects(Effect::Harmonics);
        /*
         TGEffectHarmonic harmonic = getFactory().newEffectHarmonic();
            harmonic.setType(TGEffectHarmonic.TYPE_NATURAL);
            effect.setHarmonic(harmonic);
         */
    }

    if (beatEffectsHead & 4)
    {
        //harm artif
        cursorBeat->setEffects(Effect::HarmonicsV2);
        /*
           TGEffectHarmonic harmonic = getFactory().newEffectHarmonic();
            harmonic.setType(TGEffectHarmonic.TYPE_ARTIFICIAL);
            harmonic.setData(0);
            effect.setHarmonic(harmonic);
         */
    }
}

void readNoteEffectsGP3(std::ifstream &file, Note *newNote)
{
    std::uint8_t noteEffectsHead;
    file.read((char*)&noteEffectsHead,1);

    if (gtpLog)  qDebug() << "Note effects heads. H1=" <<noteEffectsHead;


    if (noteEffectsHead&1)
    {//bend
        if (gtpLog)  qDebug() << "Bend found.";
        readBendGTP(&file, newNote->getBendPtr());
        newNote->setEffect(Effect::Bend);//first common pattern
    }


    if (noteEffectsHead&16)
    {   //grace note
        if (gtpLog)  qDebug() << "Grace note follows";

        std::uint8_t graceFret = 0;
        std::uint8_t graceDynamic = 0;
        std::uint8_t graceTransition = 0;
        std::uint8_t graceDuration = 0;

        file.read((char*)&graceFret,1);
        file.read((char*)&graceDynamic,1);
        file.read((char*)&graceTransition,1);
        file.read((char*)&graceDuration,1);

        if (gtpLog)  qDebug()<<"Fret "<<graceFret<<" Dyn "
            <<graceDynamic<<" Trans "<<graceTransition<<" Dur "<<graceDuration;


        newNote->setGraceStats({graceFret, graceDynamic, graceTransition, graceDuration});
        newNote->setGraceNote(true);
        newNote->setEffect(Effect::GraceNote); //grace note
    }


    if (noteEffectsHead&2)
    {//legato
       newNote->setEffect(Effect::Legato);
       if (gtpLog)  qDebug() << "legatto turned on";
    }

    if (noteEffectsHead&4)
    {//Slide : b

        if (gtpLog)  qDebug() << "Slide ";
        newNote->setEffect(Effect::Slide); //Review is it slide or hammer?
    }

    if (noteEffectsHead&8)
    {//let ring
       newNote->setEffect(Effect::LetRing);
       if (gtpLog)  qDebug() <<" Let ring turned on";
       if (gtpLog)  qDebug() <<" if (gtpLog)  log";
    }

}



void readBeatGP3(std::ifstream &file, Beat *cursorBeat)
{
    std::uint8_t beatHeader = 0;
    file.read((char*)&beatHeader,1);

    bool dotted = beatHeader & 0x1;
    bool precChord = beatHeader & 0x2;
    bool precText = beatHeader & 0x4;
    bool precEffects = beatHeader & 0x8;
    bool precChanges = beatHeader & 0x10;
    bool precNTrump = beatHeader & 0x20;
    bool precStatus = beatHeader & 0x40;


    if (gtpLog)  qDebug() << "Beat header " << (int)beatHeader;

    cursorBeat->setPause(false);
    if (precStatus)
    {
        std::uint8_t beatStatus;
        file.read((char*)&beatStatus,1);
        if (gtpLog)  qDebug() <<"Beat status "<<(int)beatStatus;
        if ((beatStatus == 2) || (beatStatus == 0))
         cursorBeat->setPause(true);
    }



    std::uint8_t durationGP =0;
    file.read((char*)&durationGP,1);

    if (gtpLog)  qDebug() <<"Beat duration "<<(int)durationGP;

    std::uint8_t duration=durationGP+2; //moved from -2 double to 1
    //x - double //0 - full //1 - half
    //2- forth(qua)  //3- eights  //4-16th
    //5-32th //6-64th

    //attention
    cursorBeat->setDuration(duration);
    cursorBeat->setDotted(dotted); //notify dotted
    //dotted will go now deeper

    if (precNTrump)
    {
        size_t trumpletN = 0;
        file.read((char*)&trumpletN,4);
        if (gtpLog)  qDebug() <<"Beat tump "<<trumpletN;
        cursorBeat->setDurationDetail(trumpletN);
    }
    else
    {
       cursorBeat->setDurationDetail(0);
    }

    if (precChord)
    {
        readChordDiagramGP3(file);
    }

    if (precText)
    {
        if (gtpLog)  qDebug() << "TEXT";

        size_t textLen = 0;
        file.read((char*)&textLen,4);

        std::uint8_t byteLen = 0;
        file.read((char*)&byteLen,1);

        char textBufer[255];
        file.read((char*)textBufer,byteLen);

        //len+1
        textBufer[byteLen]=0;

        if (gtpLog)  qDebug() <<"TextLen "<<textLen<<" value "<<textBufer<<"; bL "<<byteLen;

        std::string foundText(textBufer);
        cursorBeat->setText(foundText);
    }

    if (precEffects)
    {
       readBeatEffectsGP3(file,cursorBeat);
    }

    if (precChanges)
    {
        if (gtpLog)  qDebug() << "Changes table found";
        readChangesGP3(file,cursorBeat);
    }

}

void readNoteGP3(std::ifstream &file, Note *newNote, [[maybe_unused]] size_t beatIndex, [[maybe_unused]] Bar *cursorBar)
{
    std::uint8_t noteHeader = 0;
    file.read((char*)&noteHeader,1);

    std::uint8_t noteType=0;
    if (gtpLog)  qDebug() << "Note header "<<(int)noteHeader;

    newNote->setEffect(Effect::None); //flush first

    if (noteHeader & 0x20) {
        file.read((char*)&noteType,1);
        std::uint8_t bby=0; //^IN DOCS WE HAVE SHORT INT HERE
        //file.read((char*)&bby,1);
        if (gtpLog)
            qDebug() << "Note type = "<<(int)noteType<<" : "<<bby;

        //could be leag on 2
        //dead on 3 is ghost
        //normal leeg (dead?)
        //leeged normal, leeged leeg
        newNote->setState(noteType);
        //Long commented code removed her
    }

    //we push it down with a
    if (noteHeader & 1) {
        //another duration
        if (gtpLog)
            qDebug()  <<"Time independent ";
        std::uint8_t t1,t2;
        file.read((char*)&t1,1);
        file.read((char*)&t2,1);
        if (gtpLog)
            qDebug()<<"T: "<<t1<<";"<<t2;
        //attention?

    }

    if (noteHeader & 16) {
        if (gtpLog)
            qDebug() <<"Bit 4 in header turned on";
        std::uint8_t bByte=0;
        file.read((char*)&bByte,1);
        if (gtpLog)
            qDebug()<<"velocity byte(forte) "<<bByte;
        newNote->setVolume(bByte);
    }

    if (noteHeader & 32) {
        if (gtpLog)
            qDebug() <<"Bit 5 in header turned on";
        std::uint8_t bByte=0;
        file.read((char*)&bByte,1);
        if (gtpLog)
            qDebug()<<"some byte fret "<<bByte;
        if (noteType != 2) {
            if (gtpLog)
                qDebug()<<"not leeg setting prev fret";
            newNote->setFret(bByte);
        }
        else
            if (gtpLog)  qDebug() <<"leeg escape prev fret";
    }


    if (noteHeader & 2)
       if (gtpLog)  qDebug() <<"Bit 1 in header turned on"; //DOT NOTE //wow - where is it turned then?

    if (noteHeader & 4) {
        if (gtpLog)
            qDebug() <<"Bit 2 in header turned on"; //GHOST NOTE
        newNote->setEffect(Effect::GhostNote); //ghost note
    }

    if (noteHeader & 64) {
        if (gtpLog)
            qDebug() <<"Bit 6 in header turned on"; //ACCENTED
        newNote->setEffect(Effect::HeavyAccented); //there is no heavy accented note anymore (
    }

    if (noteHeader & 128) {
        if (gtpLog)
            qDebug() <<"Bit 7 in header turned on";

        std::uint8_t bByte=0;
        std::uint8_t bByte2=0;
        file.read((char*)&bByte,1);
        file.read((char*)&bByte2,1);
        if (gtpLog)
            qDebug()<<"fingering byte "<<bByte<<":"<<bByte2;
    }

    if (noteHeader & 8)
       readNoteEffectsGP3(file,newNote);
}

/////////////////////////////////////////////////////////////


bool Gp3Import::import(std::ifstream &file, Tab *tab, std::uint8_t knownVersion)
{
    if (gtpLog)  qDebug() << "Starting GP3 import";

    if (knownVersion==0)
    {
        std::uint8_t preVersion;
        file.read((char*)&preVersion,1);
        std::string formatVersion = readString(file,29);
        std::uint8_t postVersion;
        file.read((char*)&postVersion,1);
    }

    std::string title,subtitle,interpret,albumn,author,copyright,tabAuthor,instructions;

    title = readString(file);
    subtitle = readString(file);
    interpret = readString(file);
    albumn = readString(file);
    author = readString(file);
    copyright = readString(file);
    tabAuthor = readString(file);
    instructions = readString(file);

    //notice
    size_t noticeLen = 0;
    file.read((char*)&noticeLen,4);
    if (gtpLog)  qDebug() << "Notice len is " << (int)noticeLen;

    if (noticeLen > 0)
    {
        if (gtpLog)  qDebug() << "Read notices ";
        for (size_t i =0 ; i < noticeLen; ++i)
            std::string noticeOne = readString(file);
    }

    std::uint8_t tripletFeel = 0;
    file.read((char*)&tripletFeel,1);

    int tripletFeelInt = (int)tripletFeel; //hate this - if (gtpLog)  log should fix it
    if (gtpLog)  qDebug() << "Triplet feel = " << tripletFeelInt ;



    size_t bpm = 0;
    int signKey = 0;


    file.read((char*)&bpm,4);
    file.read((char*)&signKey,4);

    tab->setBPM(bpm);

    if (gtpLog)  qDebug() <<"Bpm rate is " << bpm ;
    if (gtpLog)  qDebug() <<"Sign Key = " <<signKey ; // << " ; octave " <<octave ;

    //4 8 - 12
    char midiChannelsData[768];
    file.read((char*)midiChannelsData,768);

    if (gtpLog)  qDebug() << "Midi Channels data read. size of structure: "<<(int)sizeof(MidiChannelInfo)<<
           "; full size = "<<(int)(sizeof(MidiChannelInfo)*64 );

    tab->copyMidiChannelInfo(midiChannelsData);

    size_t beatsAmount = 0;
    size_t tracksAmount = 0;

    file.read((char*)&beatsAmount,4);
    file.read((char*)&tracksAmount,4);

    if (gtpLog)  qDebug() << "Beats count " <<beatsAmount<<"; tracks count " <<tracksAmount ;


    for (size_t i = 0;i < tracksAmount; ++i) {
        auto newTrack = std::make_unique<Track>();
        newTrack->setParent(tab);
        for (size_t j = 0; j < beatsAmount; ++j) {
            auto newBeatBar = std::make_unique<Bar>(); //RESERVATION
            newTrack->push_back(std::move(newBeatBar));
        }
        tab->push_back(std::move(newTrack));
    }

    for (size_t i = 0; i < beatsAmount; ++i)
        readBar(file,tab,tracksAmount,i);

    for (size_t i = 0; i < tracksAmount; ++i){
        Track *currentTrack = tab->at(i).get();
        readTrack(file,currentTrack);
    }

    Bar *cursorBar = tab->at(0)->at(0).get();
    Beat *cursorBeat = 0;

    if (cursorBar)
    if (cursorBar->size())
        cursorBeat =cursorBar->at(0).get();


    if (gtpLog)  qDebug() <<"Begining beats amounts "<<beatsAmount;


    size_t globalBeatsAmount = beatsAmount*tracksAmount;
    for (size_t i = 0; i < globalBeatsAmount; ++i)
    {

        size_t beatsInPair = 0;
        file.read((char*)&beatsInPair,4);

        if (gtpLog)  qDebug() <<i <<" Beats in pair " <<beatsInPair;

        //refact - over here was critical error its not usefull code
        if (beatsInPair > 1000)
        {
            if (i != 0)
            {
                if (gtpLog)  qDebug() << "DEBUG OUT";
                for (int iii = 0; iii < 10; ++iii)
                {
                    std::uint8_t singleB;
                    file.read((char*)&singleB,1);
                    if (gtpLog)  qDebug() << "[" << iii << "] = " << singleB;
                }
                if (gtpLog)  qDebug() << "DEBUG OUT";
            }

            if (gtpLog)  qDebug() << "Seams to be critical error";
        }

        size_t indexOfTrack = i % tracksAmount;
        auto& updatingTrack = tab->at(indexOfTrack);
        size_t indexOfBar = i / tracksAmount;
        Bar *updatingBar = updatingTrack->at(indexOfBar).get();

        cursorBar = updatingBar;

        if (cursorBar)
        {
            if (cursorBar->size())
                cursorBeat = cursorBar->at(0).get();
            else
                cursorBeat = 0;
        }
        //++cursorBar;
        //cursorBeat = &cursorBar->getV(0);

        if (beatsInPair > 10000)
            throw std::runtime_error("gp3 beats in pair issue");

        for (size_t ind = 0; ind < beatsInPair; ++ind)
        {
            auto newOne = std::make_unique<Beat>();
            cursorBar->push_back(std::move(newOne));
        }

        for (size_t j = 0; j < beatsInPair; ++j)
        {
            cursorBeat = cursorBar->at(j).get();
            //Reading beat main
            readBeatGP3(file,cursorBeat);
            //!!! THIS IS SHIT had to be CHANGED

            std::vector<int> polyStrings;
            size_t totalCountStrings = readStringsFlag(file,polyStrings);


            for (size_t noteInd = 0; noteInd < totalCountStrings; ++noteInd)
            {
                auto newNote = std::make_unique<Note>();

                std::uint8_t sNum=polyStrings[totalCountStrings-noteInd-1];
                newNote->setStringNumber(sNum);

                //then note follows
                readNoteGP3(file, newNote.get(), j, cursorBar);
                //!!! THIS IS SHIT had to be CHANGED
                cursorBeat->push_back(std::move(newNote));
            }

            ++cursorBeat;

        }
    }

    if (gtpLog)  qDebug() << "Importing finished!";
    if (gtpLog)  qDebug() << "fine.";

    return true;
}
