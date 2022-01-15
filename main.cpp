#include "app/Init.hpp"
#include "app/LogHandler.hpp"


using namespace aural_sight;

int main(int argc, char *argv[])
{
    LogHandler::getInstance().setFilename("log.txt");
    qDebug() << "Starting application";
    return mainInit(argc,argv);
}
