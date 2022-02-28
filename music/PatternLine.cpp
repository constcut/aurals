#include "PatternLine.hpp"

#include <QPainter>

using namespace aurals;

void PatternLine::paint(QPainter* painter) {

    size_t elements = 4;

    for (size_t i = 0; i < elements; ++i)
    {
        painter->drawRect(i * 30, 0, 25, 25);
    }
}
