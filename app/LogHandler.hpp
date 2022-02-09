#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <QObject>

#include <QQuickPaintedItem>
#include <QPainter>

#include "app/log.hpp"

namespace aurals {

    class LogHandler : public QObject
    {
        Q_OBJECT
    public:
        explicit LogHandler(QObject *parent = nullptr);
        static LogHandler& getInstance();

        void setFilename(const QString filename) { _logFileName = filename; }

        void addLine(const QString anotherLine);
        QStringList getLines() const { return _logLines; }

        QtMessageHandler getOldHandler() const { return _oldHandler;}

    protected:

        QStringList _logLines;
        QString _logFileName;
        QtMessageHandler _oldHandler;
    };


    class ConsoleLogQML : public QQuickPaintedItem {
        Q_OBJECT

      public:
        ConsoleLogQML() = default;
        ~ConsoleLogQML() = default;

        void paint(QPainter* painter);
    };

}

#endif // LOGHANDLER_H
