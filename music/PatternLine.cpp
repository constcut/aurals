#include "PatternLine.hpp"

#include <QPainter>
#include <QDebug>
#include <QJsonArray>

using namespace aurals;

void PatternLine::paint(QPainter* painter) {

    if (_state.bricks.empty())
        updateBricks();

    for (size_t i = 0; i < _state.bricks.size(); ++i) {
        const auto& b = _state.bricks[i];
        if (b.on)
            painter->fillRect(b.x, b.y, b.w, b.h, QBrush(QColor("darkgreen")));
        painter->drawRect(b.x, b.y, b.w, b.h);
    }
}


void PatternLine::updateBricks()
{
   //Обновлять умней, чтобы та часть что не меняется - сохраняла паттерн
    double total = (static_cast<double>(_state.num) / _state.denom) / (1.0 / _state.brickSize);
    _state.bricks.clear();

    const int brickHeight = 25;
    const int brickWidth = 64 * 4 * (1.0 / _state.brickSize);
    const int brickPadding = 8  * 4 * (1.0 / _state.brickSize);

    for (size_t i = 0; i < total; ++i)
        _state.bricks.push_back({static_cast<int>(i) * (brickWidth + brickPadding),
                           0, brickWidth, brickHeight});
}


void PatternLine::onClick(int x, int y)
{
    for (auto& b: _state.bricks)
        if (b.hit(x, y)) {
            b.on = !b.on;
            break;
        }
    update();
}


QJsonObject PatternBrick::serialize()
{
   QJsonObject j;
   j["x"] = x;
   j["y"] = y;
   j["w"] = w;
   j["h"] = h;
   j["on"] = on;

   return j;
}


QJsonObject PatternLineState::serialize()
{
    QJsonObject j;
    j["num"] = num;
    j["denom"] = denom;
    j["brickSize"] = brickSize;
    j["midiNote"] = midiNote;

    QJsonArray arr;
    for (auto& b: bricks)
        arr.push_back(b.serialize());

    j["bricks"] = arr;

    return j;
}
