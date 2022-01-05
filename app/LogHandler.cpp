#include "LogHandler.hpp"

using namespace mtherapp;

LogHandler& LogHandler::getInstance() {
    static LogHandler handler;
    return handler;
}


void newLogMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString typeLetter=" ";

    if (type == QtDebugMsg)
        typeLetter = "D";
    else
    if (type == QtInfoMsg)
        typeLetter = "I";
    else
    if (type == QtWarningMsg)
        typeLetter = "W";
    else
    if (type == QtCriticalMsg)
        typeLetter = "C";
    else
    if (type == QtFatalMsg)
        typeLetter = "F";

    QString logLine =  QString("[%1] %2") //[%2:%3:%4] -> 5
            .arg(typeLetter).arg(msg); //arg(context.file).arg(context.function).arg(context.line)

    LogHandler::getInstance().addLine(logLine);

    LogHandler::getInstance().oldHandler(type, context, logLine); //msg
}

LogHandler::LogHandler(QObject *parent) : QObject(parent)
{
    oldHandler = qInstallMessageHandler(newLogMessageHandler);
}


void LogHandler::addLine(QString anotherLine)
{
    if (logFileName.isEmpty() == false)
    {
        //TODO logging in another thread + possible server loging
        QString closedLine = anotherLine + QString("\n\n");
        QFile logFile(logFileName);
        logFile.open(QIODevice::Append);
        logFile.write(closedLine.toLocal8Bit());
        logFile.close();
    }

    logLines << anotherLine;
    if (logLines.size() > 200) //make configurable
        logLines.removeAt(0);
}


void ConsoleLogQML::paint(QPainter* painter) {

  QStringList log = LogHandler::getInstance().getLines();
  int counter = 0;
  for (int i = log.size()-1; i >= 0; i--) {
     ++counter;
    if (counter > 100)
        break; //maybe 2000, but return only 200 from them
     QString line =  log[i];
     QFont f = painter->font();
     f.setPixelSize(11);
     painter->setFont(f);
     painter->drawText(20,counter*10,line);
  }
}


QDebug& operator<<(QDebug& logger, const std::string& msg) {
    logger << QString::fromStdString(msg);
    return logger;
}
