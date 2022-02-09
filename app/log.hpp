#ifndef LOGHEADERGTLIB
#define LOGHEADERGTLIB

#include <iostream>
#include <QDebug>
#include <QString>

namespace aurals {
    inline bool enableMidiLog = false;
    //DELAYED: разные варианты логирования + возможность включить их в настройках
}

QDebug& operator<<(QDebug& logger, const std::string& msg);

#endif
