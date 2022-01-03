#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QObject>
#include <QImage>
#include <QClipboard>

#include <QGuiApplication>
#include <QMimeData>
#include <QVariant>

class Clipboard : public QObject
{
    Q_OBJECT
public:
    explicit Clipboard(QObject *parent = nullptr);

    Q_INVOKABLE bool copyImage(QString filename)
    {
        QImage image(filename);

        QMimeData *data = new QMimeData;
        data->setImageData(image);

        QGuiApplication::clipboard()->setMimeData(data); //setImage(image,QClipboard::Clipboard);
        return true;
    }

    Q_INVOKABLE bool copyImageSrc(QVariant var)
    {
        //can try send image becfor saving it to file
        QImage img = var.value<QImage>();
        QGuiApplication::clipboard()->setImage(img);
        return true;
    }

    Q_INVOKABLE bool copyText(QString text){
        QGuiApplication::clipboard()->setText(text);
    }

//signals:
//public slots:
};

#endif // CLIPBOARD_H
