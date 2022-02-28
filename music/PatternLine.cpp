#include "PatternLine.hpp"

#include <QPainter>
#include <QDebug>

using namespace aurals;

void PatternLine::paint(QPainter* painter) {

    if (_bricks.empty())
        updateBricks();

    for (size_t i = 0; i < _bricks.size(); ++i)
    {
        const auto& b = _bricks[i];
        if (b.on)
            painter->fillRect(b.x, b.y, b.w, b.h, QBrush(QColor("darkgreen")));
        painter->drawRect(b.x, b.y, b.w, b.h);
    }
}


void PatternLine::updateBricks() {
   //Обновлять умней, чтобы та часть что не меняется - сохраняла паттерн
    double total = (static_cast<double>(_num) / _denom) / (1.0 / _brickSize);
    _bricks.clear();
    for (size_t i = 0; i < total; ++i)
        _bricks.push_back({static_cast<int>(i) * 30, 0, 25, 25});
}


void PatternLine::onClick(int x, int y) {
    for (auto& b: _bricks) {
        if (b.hit(x, y)) {
            b.on = !b.on;
        }
    }
    update();
}
