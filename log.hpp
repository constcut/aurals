#ifndef LOGHEADERGTLIB
#define LOGHEADERGTLIB

#include <iostream>
#include <QDebug>
#include <QString>

namespace mtherapp {
    inline bool enableMidiLog = false;
}

QDebug& operator<<(QDebug& logger, const std::string& msg);

#endif
