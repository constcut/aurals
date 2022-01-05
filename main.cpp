#include "app/Init.hpp"
#include "app/LogHandler.hpp"

using namespace mtherapp;

int main(int argc, char *argv[])
{
    LogHandler::getInstance().setFilename("log.txt");
    qDebug() << "Starting application";
    return mainInit(argc,argv);
}
