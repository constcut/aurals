#ifndef PIANOROLL_H
#define PIANOROLL_H

#include <QQuickPaintedItem>

#include "midi/MidiFile.hpp"


class PianoRoll : public QQuickPaintedItem
{
    Q_OBJECT
public:
    PianoRoll() = default;

    //Track id?

    Q_INVOKABLE void loadMidi(QString filename);
    Q_INVOKABLE int getContentWidth();
    Q_INVOKABLE int getContentHeight();

    Q_INVOKABLE void reset();

    void paint(QPainter* painter);


    Q_INVOKABLE int getTrackCount() {
        return _mid.size();
    }

    Q_INVOKABLE void setCurrentTrack(int newIdx) {
        _currentTrack = newIdx;
        update();
    }

signals:


private:

    aurals::MidiFile _mid;

    size_t _currentTrack = 1;



};

#endif // PIANOROLL_H
