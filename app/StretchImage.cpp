#include "StretchImage.hpp"

using namespace aural_sight;


void StretchImageQML::paint(QPainter* painter) { //TODO move into header
    auto rI = _image.scaled(_stretchedWidth, this->height()); //TODO precalculate once
    painter->drawImage(0, 0, rI, 0, 0, _stretchedWidth, -1);
}
