#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QObject>
#include <QImage>
#include <QClipboard>

#include <QGuiApplication>
#include <QMimeData>
#include <QVariant>


namespace aurals {

    class Clipboard : public QObject
    {
        Q_OBJECT
    public:

        explicit Clipboard(QObject *parent = nullptr) : QObject(parent)
        {}

        Q_INVOKABLE bool copyImage(const QString filename) const {
            QImage image(filename);
            QMimeData *data = new QMimeData;
            data->setImageData(image);
            QGuiApplication::clipboard()->setMimeData(data);
            //setImage(image,QClipboard::Clipboard);
            return true;
        }

        Q_INVOKABLE bool copyImageSrc(const QVariant var) const {
            QImage img = var.value<QImage>();
            QGuiApplication::clipboard()->setImage(img);
            return true;
        }

        Q_INVOKABLE bool copyText(const QString text) const {
            QGuiApplication::clipboard()->setText(text);
            return true;
        }
    };

}

#endif // CLIPBOARD_H
