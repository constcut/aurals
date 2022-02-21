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

    Q_INVOKABLE void setHeightStretch(bool value) {
        _fillHeight = value;
    }

signals:


private:

    aurals::MidiFile _mid;

    size_t _currentTrack = 1;

    bool _fillHeight = false; //Height fill option (если размер не меньше)

    //double _xZoomCoef = 1.0;
    //double _yZoomCoef = 1.0;



};

#endif // PIANOROLL_H
