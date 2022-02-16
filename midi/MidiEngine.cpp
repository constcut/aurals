#include "MidiEngine.hpp"

#ifndef WIN32
#include <QMediaPlayer> //Linux fails :( Android should be doublechecked
#endif

#include <QDebug>

#include <QThread>

#define min_DEF(a,b) a>b ? b:a;
#define max_DEF(a,b) a>b ? a:b;

using namespace aurals;


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


void MidiEngine::startFile([[maybe_unused]] const QString aliasName)
{
    if (opened)
        freeInitials();

#ifdef WIN32

    QString commandString = "play " + aliasName;
    QByteArray stringBytes = commandString.toLocal8Bit();

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


void MidiEngine::stopFile([[maybe_unused]] const QString aliasName)
{
    if (opened)
        freeInitials();


#ifdef WIN32

   QString commandString = "stop " + aliasName;
   QByteArray stringBytes = commandString.toLocal8Bit();

   DWORD error = mciSendStringA(stringBytes.constData(),0,0,0);
   if (error != 0) {
       qDebug() <<"Stop midi error "<<error;
       printMMError(error);
   }
#else
   if (midiPlayer)
   midiPlayer->stop();
#endif
}

void MidiEngine::openFile([[maybe_unused]] const QString filename, [[maybe_unused]] const QString aliasName)
{
    if (opened)
        freeInitials();

#ifdef WIN32

    QString commandString = "close " + lastAlias;
    QByteArray stringBytes = commandString.toLocal8Bit();

    if (lastAlias.isEmpty() == false)
        mciSendStringA(stringBytes.constData(),0,0,0);
    //it can give error
#else
      if (midiPlayer==0)
        midiPlayer = new QMediaPlayer();
    //Earlier it worked on linux, later Qt broke it
    //command <<getTestsLocation()<<"midiOutput.mid";
    //QString playerPath = command.c_str();
    //midiPlayer->setMedia(QUrl::fromLocalFile(playerPath));
    return;
#endif


#ifdef WIN32

    QString openCommand = "open " + filename + " type sequencer alias " + aliasName;
    QByteArray openCommandBytes = openCommand.toLocal8Bit();
    lastAlias = aliasName;

    qDebug () << openCommand;

    DWORD error = mciSendStringA(openCommandBytes.constData(),0,0,0);
    if (error != 0) {
        qDebug() <<"Open midi error "<<error;
        printMMError(error);
    }
#endif

}

void MidiEngine::closeFile([[maybe_unused]] QString aliasName)
{
    if (opened)
        freeInitials();

    #ifdef WIN32

    QString commandString = "close " + aliasName;  //maybe will need some transform of path here in case of full path in WIN
    QByteArray stringBytes = commandString.toLocal8Bit();


    DWORD error = mciSendStringA(stringBytes.constData(),0,0,0);
    if (error != 0) {
        qDebug() <<"Close file error"<<error;
        printMMError(error);
    }
    #endif
}


void MidiEngine::init()
{
#ifdef WIN32

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
#endif
    opened = false;
}

MidiEngine::MidiEngine()
{
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

    for (size_t i = 0; i < track.size(); ++i) {
        aurals::MidiMessage& sig = track[i];

        unsigned absValue = 10;
        //ul waitTime = absValue

        if (sig.getTypeAndChannel() !=0xff)
            sendSignalShortDelay(absValue, sig.getTypeAndChannel(),
                                 sig.getParameter1(), sig.getParameter2());
    }
}

void MidiEngine::run()
{
    if (opened==false)
        init();

    QThread::sleep(1); //1 second sleep

    qDebug() <<"Starting midi real time playing "<<_toPlay->size();

    for (size_t i = 0; i < _toPlay->size(); ++i)
    {
       aurals::MidiMessage& sig = _toPlay->operator [](i);

        double toWaitSeconds = sig.getSecondsLength();
        quint32 toWaitMs = toWaitSeconds*1000.0;

        if (toWaitMs > 0)
            QThread::msleep(toWaitMs);

        //best is to check this moment

        if (_playNotes)
        if (sig.getTypeAndChannel() !=0xff)
            sendSignalShort(sig.getTypeAndChannel(),sig.getParameter1(),sig.getParameter2());

        if (_emitSignal)
            Q_EMIT messagePlaying(sig.getTypeAndChannel(),sig.getParameter1(),sig.getParameter2());

    }
    qDebug() << "Realtime play finished";
}

void MidiEngine::playTrackRealtime(MidiTrack &track, const bool playNotes, const bool emitSignal)
{
   _toPlay = &track;
   this->_playNotes = playNotes;
   this->_emitSignal = emitSignal;

   start();
}

void MidiEngine::sendSignal(const MidiMessage &signal)
{
    if (opened==false)
        init();

    if (signal.getTypeAndChannel() ==0xff)
        sendSignalLong(signal);
    else
        sendSignalShort(signal.getTypeAndChannel() ,signal.getParameter1(),signal.getParameter2());
}

void MidiEngine::sendSignalShort([[maybe_unused]] const uint8_t status, [[maybe_unused]] const int byte1, [[maybe_unused]] const int byte2)
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

void MidiEngine::sendSignalLong([[maybe_unused]] const MidiMessage &signal)
{
    if (opened==false)
        init();

#ifdef WIN32

    //MIDIHDR mH;
    //https://msdn.microsoft.com/en-us/library/windows/desktop/dd798474(v=vs.85).aspx
    //midiOutLongMsg(winMidi,mH,sizeof(mH));
    //FAILED on last windows build
#endif
}


#ifdef WIN32

//UINT uTimerID,UINT uMsg,DWORD_PTR dwUser,DWORD_PTR dw1,DWORD_PTR dw2
void Midi_Callback_Win(UINT uId, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    MidiEngine::sendSignalShortWin(dwUser);
}


void MidiEngine::sendSignalShortWin(DWORD signal)
{
    if (opened==false)
        init();

    MMRESULT fResult = midiOutShortMsg(winMidi,signal);
}

#endif

void MidiEngine::sendSignalShortDelay([[maybe_unused]] const int msdelay, [[maybe_unused]] const uint8_t status, [[maybe_unused]] const int byte1, [[maybe_unused]] const int byte2)
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



    //UINT uDelay,UINT uResolution,LPTIMECALLBACK fptc,DWORD_PTR dwUser,UINT fuEvent
    if (timeSetEvent(msdelay, wTimerRes, Midi_Callback_Win, signal, TIME_ONESHOT) ==  0)
    {
        qDebug() <<"failed to start mmtimer";
    }
    else
        qDebug() << "Timer was set for midi event";
        //*/ //FAILED on last windows build

    //yet have some issue
#endif
}

void MidiEngine::setVolume([[maybe_unused]] const int vol)
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

