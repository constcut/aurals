#ifndef PATTERNLINE_HPP
#define PATTERNLINE_HPP

#include <QQuickPaintedItem>


class PatternLine  : public QQuickPaintedItem
{
    Q_OBJECT

public:
    PatternLine() = default;

    //setters\getters

    //paint event

    void paint(QPainter* painter);

private:

    //fields sizes

    //vector of elements struct

};

#endif // PATTERNLINE_HPP
