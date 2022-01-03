#ifndef LOGHEADERGTLIB
#define LOGHEADERGTLIB

#include <iostream>
#include <QDebug>
#include <QString>

namespace mtherapp {
    inline bool enableMidiLog = false;
    //TODO разные варианты логирования
}

QDebug& operator<<(QDebug& logger, const std::string& msg);

#endif
