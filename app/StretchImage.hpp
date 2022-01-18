#ifndef STRETCHIMAGE_HPP
#define STRETCHIMAGE_HPP

#include <QQuickPaintedItem>
#include <QPainter>
#include <QVariant>


class StretchImageQML: public QQuickPaintedItem
{
        Q_OBJECT
public:
    StretchImageQML([[maybe_unused]] QQuickItem* parent = nullptr){}

    void paint(QPainter* painter);

    Q_INVOKABLE void setImage(QVariant img, int newWidth) {
        _image = img.value<QImage>();
        _stretchedWidth = newWidth;
    }

    Q_INVOKABLE void saveImage(QString filename) { _image.save(filename); }

private:

    QImage _image;
    int _stretchedWidth;

};

#endif // STRETCHIMAGE_HPP
