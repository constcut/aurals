#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <QObject>

#include <QQuickPaintedItem>
#include <QPainter>

#include "log.hpp"

class LogHandler : public QObject
{
    Q_OBJECT
public:
    explicit LogHandler(QObject *parent = nullptr);

    static LogHandler *instance;

    void addLine(QString anotherLine);

    QtMessageHandler oldHandler;

    QStringList getLines() { return logLines; }

    void setFilename(QString filename) { logFileName = filename; }

protected:
    QStringList logLines;

    QString logFileName;

};


class ConsoleLogQML : public QQuickPaintedItem
{
    Q_OBJECT
  public:
    ConsoleLogQML() {}
    ~ConsoleLogQML() {}

    void paint(QPainter* painter);
};

#endif // LOGHANDLER_H
