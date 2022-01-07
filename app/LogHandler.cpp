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
            .arg(typeLetter, msg); //arg(context.file).arg(context.function).arg(context.line)

    LogHandler::getInstance().addLine(logLine);

    LogHandler::getInstance().getOldHandler()(type, context, logLine); //msg
}

LogHandler::LogHandler(QObject *parent) : QObject(parent)
{
    _oldHandler = qInstallMessageHandler(newLogMessageHandler);
}


void LogHandler::addLine(const QString anotherLine)
{
    if (_logFileName.isEmpty() == false) {
        //DELAYED: logging in another thread + possible server loging
        QString closedLine = anotherLine + QString("\n\n");
        QFile logFile(_logFileName);
        logFile.open(QIODevice::Append);
        logFile.write(closedLine.toLocal8Bit());
        logFile.close();
    }

    _logLines << anotherLine;
    if (_logLines.size() > 200) //make configurable
        _logLines.removeAt(0);
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
