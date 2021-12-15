#ifndef MIDIRENDER_H
#define MIDIRENDER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QElapsedTimer>

//#include "mit/sf/tsf.h"
//#include "mit/sf/tml.h"

#include "music/midifile.h"

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

    ///QByteArray renderShort(MidiFile *mid);
    ///QByteArray renderFloat(MidiFile *mid);

    QByteArray renderFromClass(void *midiClass){} //TO DO LATER - sepparate Midi implementation from renderer

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

    void *midiFile;
    void *soundFont;

    MidiTrack *midiTrack;
    quint32 trackPosition;

    int renderFrameSize;
    double freq;

    //migrated:
    int g_MidiChannelPreset[16];
    double msRendered;

    qint64 totalMsSpent;
};


class MidiSaver : public QObject
{
    Q_OBJECT
public:
    MidiSaver();
    ~MidiSaver(){}

    void dumpToFile(QString filename);

public Q_SLOTS:
    Q_INVOKABLE void pushMessage(qint16 type, qint16 p1, qint16 p2);


protected:
    QElapsedTimer lastMessagetTimer;

    MidiFile mid;
};

#endif // MIDIRENDER_H
