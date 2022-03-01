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


    private:

        std::vector<PatternLine*> _lines;

    };

}

#endif // PATTERNRECEIVER_H
