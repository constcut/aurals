#include "PatternLine.hpp"

#include <QPainter>
#include <QDebug>

using namespace aurals;

void PatternLine::paint(QPainter* painter) {

    if (_bricks.empty())
        updateBricks();

    qDebug() << _bricks.size() << " total bricks";
    for (size_t i = 0; i < _bricks.size(); ++i)
    {
        painter->drawRect(i * 30, 0, 25, 25);
    }
}


void PatternLine::updateBricks() {
   //Обновлять умней, чтобы та часть что не меняется - сохраняла паттерн

    double total = (static_cast<double>(_num) / _denom) / (1.0 / _brickSize);

    _bricks.clear();
    for (size_t i = 0; i < total; ++i)
        _bricks.push_back({});
}
