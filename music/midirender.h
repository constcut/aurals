#ifndef MIDIRENDER_H
#define MIDIRENDER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QElapsedTimer>

#include "music/midifile.h"
#include "music/miditrack.h"


struct tml_message;
struct tsf;


class MidiRender
{

public:

    void setSampleFreq(double newFreq) { freq = newFreq; } //ON REFACTORING MAKE SUCH THINGS AS A PROPERTY
    double getSampleFreq() { return freq; }

    void setFrameSize(quint32 newFrameSize)  { renderFrameSize = newFrameSize; }
    quint32 getFrameSize() { return renderFrameSize; }

    MidiRender();

    QByteArray renderShort(QString midiFilename, QString sfFilename);
    QByteArray renderFloat(QString midiFilename, QString sfFilename);

    QByteArray renderShort(QString midiFilename);
    QByteArray renderFloat(QString midiFilename);

    QByteArray renderFromMemoryShort(MidiTrack &track); //yet trying to make per track generation
    QByteArray renderFromMemoryFloat(MidiTrack &track); //at least it could be mixed and also generated in append mode

    bool openMidiFile(QString midiFilename);
    bool openSoundFont(QString sfFilename);

    qint64 getTotaMsSpent() { return totalMsSpent; }
    double getRenderTimer() { return msRendered; }
    //there is also a mixing flag in sf lib - so we can make experiments :)

protected:

    QByteArray renderShortNext(int len);
    QByteArray renderFloatNext(int len);

    QByteArray renderMemoryFloatNext(int len);
    QByteArray renderMemoryShortNext(int len); //refact to quint32 etc - everywhere

    tml_message* midiFile = nullptr;
    tsf* soundFont = nullptr;

    MidiTrack *midiTrack;
    quint32 trackPosition;

    int renderFrameSize;
    double freq;

    //migrated:
    int g_MidiChannelPreset[16];
    double msRendered;

    qint64 totalMsSpent;
};



#endif // MIDIRENDER_H
