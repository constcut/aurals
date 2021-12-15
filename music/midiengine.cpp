#include "midiengine.h"

#ifndef WIN32 //that did halped earlier to play something on android - but rather useless now
#include <QMediaPlayer>
#endif //the android implementation should be different (maybe Sonivox usage)

#include "log.hpp"

#include <QThread>


#define min_DEF(a,b) a>b ? b:a;
#define max_DEF(a,b) a>b ? a:b;

MidiEngine *MidiEngine::inst=0;

#ifdef WIN32
         HMIDIOUT MidiEngine::winMidi;
         UINT MidiEngine::wTimerRes;
#endif


bool MidiEngine::opened=false;
QString MidiEngine::lastAlias = QString();

#ifndef WIN32
QMediaPlayer *midiPlayer=0;
#endif

#ifdef WIN32
void MidiEngine::printMMError(DWORD err) //beter return value
{
    wchar_t textBuf[255];
    if (mciGetErrorStringW(err,textBuf,255))
    {
        QString buf =  QString::fromWCharArray(textBuf);
        qDebug() <<"Error "<<err<<" "<<buf;
    }
    else
    qDebug() <<"Failed to decrypt MM error";
}
#endif

void MidiEngine::startFile(QString aliasName)
{
    if (opened)
        freeInitials();

    //if (opened==false)
        //init();

 QString commandString = "play " + aliasName;  //maybe will need some transform of path here in case of full path in WIN
 QByteArray stringBytes = commandString.toLocal8Bit();

#ifdef WIN32
     DWORD error = mciSendStringA(stringBytes.constData(),0,0,0);
     if (error != 0)
     {
         qDebug() <<"Play midi error "<<error;
         printMMError(error);
     }
#else
    if (midiPlayer)
    midiPlayer->play();
#endif

}

void MidiEngine::stopFile(QString aliasName)
{
    if (opened)
        freeInitials();

    //if (opened==false)
        //init();

QString commandString = "stop " + aliasName;  //maybe will need some transform of path here in case of full path in WIN
QByteArray stringBytes = commandString.toLocal8Bit();

#ifdef WIN32
   DWORD error = mciSendStringA(stringBytes.constData(),0,0,0);
   if (error != 0)
   {
       qDebug() <<"Stop midi error "<<error;
       printMMError(error);
   }
#else
   if (midiPlayer)
   midiPlayer->stop();
#endif
}

void MidiEngine::openFile(QString filename, QString aliasName)
{
    if (opened)
        freeInitials();

    //if (opened==false)
        //init();

    QString commandString = "close " + lastAlias;  //maybe will need some transform of path here in case of full path in WIN
    QByteArray stringBytes = commandString.toLocal8Bit();

#ifdef WIN32
    if (lastAlias.isEmpty() == false)
        mciSendStringA(stringBytes.constData(),0,0,0);
    //it can give error
#else
      if (midiPlayer==0)
        midiPlayer = new QMediaPlayer();

      ///COMENTED TO CO<PILE
    //command <<getTestsLocation()<<"midiOutput.mid";
    //QString playerPath = command.c_str();
    //midiPlayer->setMedia(QUrl::fromLocalFile(playerPath));
    return;
#endif

    //\"
    QString openCommand = "open " + filename + " type sequencer alias " + aliasName;
    QByteArray openCommandBytes = openCommand.toLocal8Bit();
    lastAlias = aliasName;

    qDebug () << openCommand;

#ifdef WIN32
    DWORD error = mciSendStringA(openCommandBytes.constData(),0,0,0);
    if (error != 0)
    {
        qDebug() <<"Open midi error "<<error;
        printMMError(error);
    }
#endif

}

void MidiEngine::closeFile(QString aliasName)
{
    if (opened)
        freeInitials();

    //if (opened==false)
        //init();

    QString commandString = "close " + aliasName;  //maybe will need some transform of path here in case of full path in WIN
    QByteArray stringBytes = commandString.toLocal8Bit();

    #ifdef WIN32
    DWORD error = mciSendStringA(stringBytes.constData(),0,0,0);
    if (error != 0)
    {
        qDebug() <<"Close file error"<<error;
        printMMError(error);
    }
    #endif
}


void MidiEngine::init()
{
#ifdef WIN32

    //WE CAN HAVE CALLBACK!

     unsigned int err = midiOutOpen(&winMidi, 0, 0, 0, CALLBACK_NULL);
       if (err != MMSYSERR_NOERROR)
          qDebug()<<"error opening default MIDI device: "<<err;
       else
           qDebug()<<"successfully opened default MIDI device";


#define TimerResolution 1

       TIMECAPS tc;

       if (timeGetDevCaps(&tc,sizeof(TIMECAPS)) != TIMERR_NOERROR)
           qDebug() << "MM timer critical error";


       UINT timeRes = max_DEF(tc.wPeriodMin,TimerResolution);
       wTimerRes = min_DEF(timeRes ,tc.wPeriodMax);

       timeBeginPeriod(wTimerRes);
#endif
       opened = true;
}

void MidiEngine::freeInitials()
{
#ifdef WIN32
    midiOutClose(winMidi);
    //logger<<"closed default MIDI device";
#endif
    opened = false;
}

MidiEngine::MidiEngine()
{
    if (inst!=0) return;
    inst = this; //last one
    init();
}

MidiEngine::~MidiEngine()
{
    freeInitials();
}

void MidiEngine::playTrack(MidiTrack &track)
{
    if (opened==false)
        init();

    for (int i = 0; i < track.size(); ++i)
    {
        MidiMessage sig = track[i];

        unsigned absValue = 10;
        //ul waitTime = absValue

        if (sig.byte0!=0xff)
            sendSignalShortDelay(absValue,
                        sig.byte0,sig.p1,
                            sig.p2);
    }
}

void MidiEngine::run()
{
    if (opened==false)
        init();

    //maybe nice to make there some semafor like thing to let know we are ready to play
    //but be prepared at this point
    QThread::sleep(1); //1 second sleep

    qDebug() <<"Starting midi real time playing "<<toPlay->size();

    for (auto i = 0; i < toPlay->size(); ++i)
    {
        MidiMessage sig = toPlay->operator [](i);

        double toWaitSeconds = sig.getSecondsLength();
        quint32 toWaitMs = toWaitSeconds*1000.0;

        if (toWaitMs > 0)
            QThread::msleep(toWaitMs);

        //best is to check this moment

        if (playNotes)
        if (sig.byte0!=0xff)
            sendSignalShort(sig.byte0,sig.p1,sig.p2);

        if (emitSignal)
            Q_EMIT messagePlaying(sig.byte0,sig.p1,sig.p2);

    }
    qDebug() << "Realtime play finished";
}

void MidiEngine::playTrackRealtime(MidiTrack &track, bool playNotes, bool emitSignal)
{
   toPlay = &track;
   this->playNotes = playNotes;
   this->emitSignal = emitSignal;

   //Then starting thread
   start();
}

void MidiEngine::sendSignal(MidiMessage &signal)
{
    if (opened==false)
        init();

    if (signal.byte0==0xff)
        sendSignalLong(signal);
    else
        sendSignalShort(signal.byte0,signal.p1,signal.p2);
}

void MidiEngine::sendSignalShort(byte status, int byte1, int byte2)
{
    if (opened==false)
        init();

#ifdef WIN32

    DWORD signal = status;

    if (byte1!=-1)
        signal += byte1<<8;
    if (byte2!=-1)
        signal +=byte2<<16;

    MMRESULT fResult = midiOutShortMsg(winMidi,signal);

#endif
}

void MidiEngine::sendSignalLong(MidiMessage &signal)
{
    if (opened==false)
        init();

#ifdef WIN32

    MIDIHDR mH;
    //https://msdn.microsoft.com/en-us/library/windows/desktop/dd798474(v=vs.85).aspx

    //midiOutLongMsg(winMidi,mH,sizeof(mH));

#endif
}


#ifdef WIN32


//LPTIMECALLBACK Midi_Callback_Win(UINT uId, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
LPTIMECALLBACK Midi_Callback_Win(UINT uId, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    qDebug() << "Midi cb begin "<<dwUser;
    MidiEngine::sendSignalShortWin(dwUser);
    qDebug() << "Midi cb end "<<dwUser;
}


void MidiEngine::sendSignalShortWin(DWORD signal)
{
    if (opened==false)
        init();

    MMRESULT fResult = midiOutShortMsg(winMidi,signal);
}

#endif

void MidiEngine::sendSignalShortDelay( int msdelay, byte status, int byte1, int byte2)
{
    if (opened==false)
        init();

#ifdef WIN32

    DWORD signal = status;

    if (byte1!=-1)
        signal += byte1<<8;
    if (byte2!=-1)
        signal +=byte2<<16;

    qDebug()<<"Pushing signal "<<signal<<"for ms delay "<<msdelay;
    qDebug()<<"Sinal parts "<<byte1<<" "<<byte2<<"; "<<status;

    /*
    if (timeSetEvent(msdelay, wTimerRes, Midi_Callback_Win,signal,TIME_ONESHOT) ==  0)
    {
        qDebug() <<"failed to start mmtimer";
    }
    else
        qDebug() << "Timer was set for midi event";
        */

    //yet have some issue
#endif
}

void MidiEngine::setVolume(int vol)
{
    if (opened==false)
        init();

#ifdef WIN32
    DWORD volToSet = vol;
    midiOutSetVolume(winMidi,volToSet); //&
#endif
}

int MidiEngine::getVolume()
{
    if (opened==false)
        init();

    int response = 0;
#ifdef WIN32
    DWORD vol = 0;
    midiOutGetVolume(winMidi,&vol);
    response=vol;
#endif
    return response;
}

bool midiAbsSortFunction(MidiMessage &a, MidiMessage &b)
{
    /*
    ul timeA = a->absValue;
    ul timeB = b->absValue;
    return timeA>timeB;
    */
    return false; //used for next function
}

MidiTrack MidiEngine::uniteFileToTrack(MidiFile &midiFile)
{
    //TODO

    //1 makes all signals global counters apended ul(eats mem but helps alot)
    //2 put all together
    //3 sort by global
    //4 trace - group by same time
    ///ITS a good idea - should maybe check it once
    /*


    std::vector<MidiSignal*> allSignals;

    for (int trackI = 0; trackI < midiFile->len(); ++trackI)
    {
        MidiTrack *track = midiFile->getV(trackI);
        ul absTimeShift =0;
        for (int sigI = 0; sigI < track->len(); ++sigI)
        {
            MidiSignal *sig = track->getV(sigI);

            ul signalTimeShift = sig->time.getValue();
            absTimeShift += signalTimeShift;

            sig->absValue = absTimeShift;

            allSignals.push_back(sig);
        }
    }

    std::sort(allSignals.begin(),allSignals.end(),midiAbsSortFunction);

    //repair local position from abs

    MidiTrack *result = new MidiTrack();

    ul lastGlobalAbs = 0;
    for (int sigI = allSignals.size()-1; sigI >= 0; --sigI)
    {
            MidiSignal *sig = allSignals[sigI];


            ul currentAbs = sig->absValue;
            ul shift = currentAbs - lastGlobalAbs;


            MidiSignal *signalCopy = new MidiSignal(sig->byte0,
                                                    sig->param1,
                                                    sig->param2,shift);

            signalCopy->absValue = sig->absValue;

            if (sig->byte0==0xff)
                signalCopy->metaStore = sig->metaStore; //attention


            lastGlobalAbs = currentAbs;

            result->add(signalCopy);

    }

    qDebug() << "Produced midi track with "<<(int)result->len()<<" elements";
    */

    return MidiTrack(); //instead of result
}

