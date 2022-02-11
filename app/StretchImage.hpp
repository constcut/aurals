#ifndef STRETCHIMAGE_HPP
#define STRETCHIMAGE_HPP

#include <QQuickPaintedItem>
#include <QPainter>
#include <QVariant>

namespace aurals {

    class StretchImageQML: public QQuickPaintedItem
    {
            Q_OBJECT
    public:

        StretchImageQML(QQuickItem* parent = nullptr){}

        void paint(QPainter* painter) {

            if (_wasScaled == false) {
                 _scaledImage = _image.scaled(_stretchedWidth, this->height());
                 _wasScaled = true;
            }

            painter->drawImage(0, 0, _scaledImage, 0, 0, _stretchedWidth, -1);
        }

        Q_INVOKABLE void setImage(QVariant img, int newWidth) {
            _image = img.value<QImage>();
            _stretchedWidth = newWidth;
            _wasScaled = false;
        }

        Q_INVOKABLE void saveImage(QString filename) { _image.save(filename); }

    private:

        QImage _image;
        int _stretchedWidth;

        QImage _scaledImage;
        bool _wasScaled = false;
    };

}

#endif // STRETCHIMAGE_HPP
