#ifndef MIDIENGINE_H
#define MIDIENGINE_H

#include <QObject>
#include <QString>
#include <QThread>

#include "midi/MidiTrack.hpp"
#include "midi/MidiMessage.hpp"

#ifdef WIN32
    #include "windows.h" //for lib mm
#endif

namespace aural_sight {

    class MidiEngine : public QThread
    {
        Q_OBJECT
    protected:
        #ifdef WIN32
         static HMIDIOUT winMidi;
         static UINT wTimerRes;
        #endif
         static bool opened;

       aural_sight::MidiTrack *_toPlay;
       bool _playNotes;
       bool _emitSignal;

       static MidiEngine& getInst() {
           static MidiEngine eng;
           return eng;
       }

    public:

        static void init();
        static void freeInitials();

    #ifdef WIN32
    static void sendSignalShortWin(DWORD signal);
    #endif

        MidiEngine();
        ~MidiEngine();

        virtual void run() override;

        static void sendSignal(const aural_sight::MidiMessage &signal);
        static void sendSignalShort(const uint8_t status, const int uint8_t1=-1, const int uint8_t2=-1);
        static void sendSignalLong(const aural_sight::MidiMessage &signal);

        static void sendSignalShortDelay(const int msdelay, const uint8_t status,
                                         const int uint8_t1=-1, const int uint8_t2=-1);

        static void setVolume(const int vol);
        static int getVolume();

        static void startFile(const QString aliasName = "mwmMidi");
        static void stopFile(const QString aliasName =  "mwmMidi");
        static void openFile(const QString filename, const QString aliasName =  "mwmMidi");
        static void closeFile(const QString aliasName = "mwmMidi");

        static void playTrack(MidiTrack &track);
        void playTrackRealtime(MidiTrack &track, const bool playNotes=true,
                               const bool emitSignal=false);


        //COVER WINDOWS DEFINE TOO
    #ifdef WIN32
        static void printMMError(DWORD err);
    #endif

        static QString lastAlias;

    Q_SIGNALS:
        void messagePlaying(qint16 messageType, qint16 p1, qint16 p2);
    };

}

#endif // MIDIENGINE_H
