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


signals:


private:

    aurals::MidiFile _mid;



};

#endif // PIANOROLL_H
