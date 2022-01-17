#include "StretchImage.hpp"


void StretchImageQML::paint(QPainter* painter) {
    auto rI = _image.scaled(_stretchedWidth, this->height());
    painter->drawImage(0, 0, rI, 0, 0, _stretchedWidth, -1);
}
