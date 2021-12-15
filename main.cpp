#include "app/init.h"

#include "log.hpp"

#include <iostream>

#include "midi/MidiFile.hpp"
#include "app/loghandler.h"



using namespace std;

using namespace mtherapp;


int main(int argc, char *argv[])
{
    LogHandler logHandler;
    logHandler.setFilename("log.txt");
    qDebug() << "Starting application";
    return mainInit(argc,argv);
}
