#ifndef MIDIENGINE_H
#define MIDIENGINE_H

#include <QObject>
#include <QString>
#include <QThread>

#include "midifile.h"

#ifdef WIN32


    #include "windows.h" //for lib mm
#endif

typedef unsigned char byte;

class MidiEngine : public QThread
{
    Q_OBJECT
protected:
#ifdef WIN32
         static HMIDIOUT winMidi;
         static UINT wTimerRes;
#endif
         static bool opened;

       MidiTrack *_toPlay;
       bool _playNotes;
       bool _emitSignal;

static MidiEngine *inst;

public:

    static void init();
    static void freeInitials();

#ifdef WIN32
static void sendSignalShortWin(DWORD signal);
#endif

    MidiEngine();
    ~MidiEngine();

    virtual void run() override;

    static void sendSignal(MidiMessage &signal);
    static void sendSignalShort(byte status, int byte1=-1, int byte2=-1);
    static void sendSignalLong(MidiMessage &signal);

    static void sendSignalShortDelay(int msdelay, byte status, int byte1=-1, int byte2=-1);

    static void setVolume(int vol);
    static int getVolume();

    static void startFile(QString aliasName = "mwmMidi");
    static void stopFile(QString aliasName =  "mwmMidi");
    static void openFile(QString filename, QString aliasName =  "mwmMidi");
    static void closeFile(QString aliasName = "mwmMidi");

    static void playTrack(MidiTrack &track);
    void playTrackRealtime(MidiTrack &track, bool playNotes=true, bool emitSignal=false);


    //COVER WINDOWS DEFINE TOO
#ifdef WIN32
    static void printMMError(DWORD err);
#endif

    static QString lastAlias;

Q_SIGNALS:
    void messagePlaying(qint16 messageType, qint16 p1, qint16 p2);
};

#endif // MIDIENGINE_H
