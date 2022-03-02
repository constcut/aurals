#ifndef PATTERNRECEIVER_H
#define PATTERNRECEIVER_H

#include <QQuickPaintedItem>

#include <vector>

#include "PatternLine.hpp"


namespace aurals {

    class PatternReceiver : public QQuickPaintedItem
    {
        Q_OBJECT

    public:
        PatternReceiver() = default;

        void paint(QPainter* painter);

        Q_INVOKABLE void connectLine(QObject* line); //Later add remove also

        Q_INVOKABLE void generateMidi(QString filename);

        Q_INVOKABLE void setBPM(int bpm) {
            _bpm = bpm;
        }


    private:

        std::vector<PatternLine*> _lines;

        int _bpm;

    };

}

#endif // PATTERNRECEIVER_H
