#include "PatternLine.hpp"

#include <QPainter>
#include <QDebug>
#include <QJsonArray>

#include <cmath>

using namespace aurals;

void PatternLine::paint(QPainter* painter) {

    if (_state.bricks.empty())
        updateBricks();

    for (size_t i = 0; i < _state.bricks.size(); ++i) {
        const auto& b = _state.bricks[i];
        if (b.on)
            painter->fillRect(b.x, b.y, b.w, b.h, QBrush(QColor("darkgreen")));

        if (b.border)
            painter->setPen(QColor("gray"));

        painter->drawRect(b.x, b.y, b.w, b.h);

        if (b.border)
            painter->setPen(QColor("black"));
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

    int countTillBorder = (1.0 / _state.denom) / (1.0 / _state.brickSize);
    if (countTillBorder <= 0)
        countTillBorder = 1;

    for (size_t i = 0; i < total; ++i)
    {
        bool borderBrick = false;
        if (i % countTillBorder == 0)
            borderBrick = true;

        const int x = static_cast<int>(i) * (brickWidth + brickPadding);
        _state.bricks.push_back({x, 0, brickWidth, brickHeight, borderBrick});
    }
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


QJsonObject PatternBrick::serialize() const
{
   QJsonObject j;
   j["x"] = x; //Возможно сохранять эти значения лишнее и даже вредное для разных размеров экранов
   j["y"] = y;
   j["w"] = w;
   j["h"] = h;
   j["border"] = border;
   j["on"] = on;


   return j;
}


void PatternBrick::deserialize(QJsonObject object)
{
    x = object["x"].toInt();
    y = object["y"].toInt();
    w = object["w"].toInt();
    h = object["h"].toInt();
    border = object["border"].toBool();
    on = object["on"].toBool();
}


QJsonObject PatternLineState::serialize() const
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


void PatternLineState::deserialize(QJsonObject object)
{
    num = object["num"].toInt();
    denom = object["denom"].toInt();
    brickSize = object["brickSize"].toInt();
    midiNote = object["midiNote"].toInt();

    QJsonArray arr = object["bricks"].toArray();

    for (auto brickRef: arr) {
        auto brickObj = brickRef.toObject();
        PatternBrick brick;
        brick.deserialize(brickObj);
        bricks.push_back(brick);
    }
}


