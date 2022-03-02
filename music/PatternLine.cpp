#include "PatternLine.hpp"

#include <QPainter>
#include <QDebug>

using namespace aurals;

void PatternLine::paint(QPainter* painter) {

    if (_bricks.empty())
        updateBricks();

    for (size_t i = 0; i < _bricks.size(); ++i) {
        const auto& b = _bricks[i];
        if (b.on)
            painter->fillRect(b.x, b.y, b.w, b.h, QBrush(QColor("darkgreen")));
        painter->drawRect(b.x, b.y, b.w, b.h);
    }
}


void PatternLine::updateBricks()
{
   //Обновлять умней, чтобы та часть что не меняется - сохраняла паттерн
    double total = (static_cast<double>(_num) / _denom) / (1.0 / _brickSize);
    _bricks.clear();

    qDebug() << "New bricks size: " << total;

    const int brickHeight = 25;
    const int brickWidth = 64 * 4 * (1.0 / _brickSize);
    const int brickPadding = 8  * 4 * (1.0 / _brickSize);

    for (size_t i = 0; i < total; ++i) //TODO correct distance in different sizes
        _bricks.push_back({static_cast<int>(i) * (brickWidth + brickPadding),
                           0, brickWidth, brickHeight});
}


void PatternLine::onClick(int x, int y)
{
    for (auto& b: _bricks)
        if (b.hit(x, y)) {
            b.on = !b.on;
            break;
        }
    update();
}
