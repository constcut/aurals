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

        void setSampleRate(const double newFreq) { _sampleRate = newFreq; } //ON REFACTORING MAKE SUCH THINGS AS A PROPERTY
        double getSampleRate() const { return _sampleRate; }

        void setFrameSize(const quint32 newFrameSize)  { _renderFrameSize = newFrameSize; }
        quint32 getFrameSize() const { return _renderFrameSize; }

        MidiRender();

        QByteArray renderShort(const QString midiFilename, const QString sfFilename);
        QByteArray renderFloat(const QString midiFilename, const QString sfFilename);

        QByteArray renderShort(const QString midiFilename);
        QByteArray renderFloat(const QString midiFilename);

        QByteArray renderFromMemoryShort(MidiTrack &track);
        QByteArray renderFromMemoryFloat(MidiTrack& track);

        bool openMidiFile(const QString midiFilename);
        bool openSoundFont(const QString sfFilename);
        bool openSoundFont();
        void setSoundFont(const QString sfFilename);
        QString getSoundFont() const { return _soundfontFile; }

        qint64 getTotaMsSpent() const { return _totalMsSpent; }
        double getRenderTimer() const { return _msRendered; }

        float getVolumeDb() const { return _volume; }
        void setVolumeDb(const float newVol) {
            _volume = newVol; }

    protected:

        QByteArray renderShortNext(const int len);
        QByteArray renderFloatNext(const int len);

        QByteArray renderMemoryFloatNext(const int len);
        QByteArray renderMemoryShortNext(const int len);

        tml_message* _midiFile = nullptr;
        tsf* _soundFont = nullptr;

        MidiTrack* _midiTrack = nullptr;
        quint32 _trackPosition;

        int _renderFrameSize;
        double _sampleRate;
        float _volume = -6.0;
        QString _soundfontFile = "epiano.sf2";

        int _g_MidiChannelPreset[16];
        double _msRendered;
        qint64 _totalMsSpent;
        bool _mono = false;
    };

}

#endif // MIDIRENDER_H
