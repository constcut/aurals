#ifndef MIDIRENDER_H
#define MIDIRENDER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QElapsedTimer>

#include "midi/MidiFile.hpp"
#include "midi/MidiTrack.hpp"


struct tml_message;
struct tsf;

namespace mtherapp {

    class MidiRender
    {

    public:

        void setSampleFreq(double newFreq) { _sampleRate = newFreq; } //ON REFACTORING MAKE SUCH THINGS AS A PROPERTY
        double getSampleFreq() { return _sampleRate; }

        void setFrameSize(quint32 newFrameSize)  { _renderFrameSize = newFrameSize; }
        quint32 getFrameSize() { return _renderFrameSize; }

        MidiRender();

        QByteArray renderShort(QString midiFilename, QString sfFilename);
        QByteArray renderFloat(QString midiFilename, QString sfFilename);

        QByteArray renderShort(QString midiFilename);
        QByteArray renderFloat(QString midiFilename);

        QByteArray renderFromMemoryShort(MidiTrack& track);
        QByteArray renderFromMemoryFloat(MidiTrack& track);

        bool openMidiFile(QString midiFilename);
        bool openSoundFont(QString sfFilename);

        qint64 getTotaMsSpent() { return _totalMsSpent; }
        double getRenderTimer() { return _msRendered; }

        /*void setMono() { _mono = true; }
        void setStereo() { _mono = false; }
        bool isMono() { return _mono; }*/ //Mono gives issues yet

    protected:

        QByteArray renderShortNext(int len);
        QByteArray renderFloatNext(int len);

        QByteArray renderMemoryFloatNext(int len);
        QByteArray renderMemoryShortNext(int len); //refact to quint32 etc

        tml_message* _midiFile = nullptr;
        tsf* _soundFont = nullptr;

        MidiTrack* _midiTrack;
        quint32 _trackPosition;

        int _renderFrameSize;
        double _sampleRate;

        int _g_MidiChannelPreset[16];
        double _msRendered;
        qint64 _totalMsSpent;

        bool _mono = false;
    };

}

#endif // MIDIRENDER_H
