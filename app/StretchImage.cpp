#include "StretchImage.hpp"



void StretchImageQML::paint(QPainter* painter) {
    painter->drawImage(0, 0, _image);
}
