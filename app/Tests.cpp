#include "Tests.hpp"

#include <iostream>

#include "midi/MidiFile.hpp"
#include "tab/Tab.hpp"
#include "tab/tools/GtpFiles.hpp"
#include "app/Config.hpp"
#include "tab/tools/GmyFile.hpp"

#include "tab/tools/TabLoader.hpp"

#include <QMediaPlayer>

#include <fstream>
#include "app/log.hpp"

#include "tab/tools/MidiExport.hpp"

#define log qDebug()


#include <QTime>
#include <QElapsedTimer>

QElapsedTimer timer;


using namespace aurals;


int aurals::getTime()
{
    //QTime time;

    static bool firstRun = true;
    if (firstRun)
    {
        timer.start();
        firstRun=false;
    }

    int result = timer.elapsed();
    if (result < 0) result *= -1;

    return result;
}


//configuration connectors

extern bool gtpLog;
extern bool midiLog;
extern bool museLog;
extern bool tabLog;


bool midiPrint(std::string fileName)
{
    std::ifstream ifile(fileName.c_str(), std::ifstream::binary);

    if (ifile.is_open())
        std::cout << "File opened" ;
    else
    {
        std::cout<<"Failed to open file" ;
        std::cout<<"Scenario done.";
        return false;
    }

    MidiFile midiFile;
    midiFile.readStream(ifile);
    std::cout << "Reading file finished";
    //midiFile.printToStream(std::cout);
    return true;
}



bool testMidi(std::string fileName)
{
    std::ifstream ifile(fileName.c_str(),std::ifstream::binary);

    if (ifile.is_open())
        log << "File opened" ;
    else
    {
        log <<"Failed to open file" ;
        log<<"Scenario done.";
        return false;
    }

    MidiFile midiFile;
    midiFile.readStream(ifile);
    log << "Reading file finished";
    //midiFile.printToStream(std::cout); //Maybe get it back?

    return true;
}

bool testGP3(std::string fileName, std::string outFileName)
{
    std::ifstream itfile;
    itfile.open(fileName.c_str(),std::ifstream::binary);  //small file - runs ok!

    if (itfile.is_open())
        ; //log << "File opened" ;
    else
    {
        log <<"Failed to open GP file" ;
        log<<"Scenario done.";
        return false;
    }

    Tab tab;
    Gp3Import importer; //(tabFile,tab);


    //swtich to different versions of importers 3\4\5

    importer.import(itfile,&tab);
    tab.connectTracks(); //new for chains refact

    //std::cout << "All information about readin is in log"<<std::endl;

    auto f = exportMidi(&tab);
    std::ofstream midiOut(outFileName, std::ios::binary);
    size_t bytesWritten = f->writeStream(midiOut);

    qDebug() << "Bytes midi written " << bytesWritten << " to " << outFileName.c_str();

    return true;
}


bool testGP4(std::string fileName, std::string outFileName, std::string gmyFile="")
{
    std::ifstream itfile;
    itfile.open(fileName.c_str(),std::ifstream::binary);  //small file - runs ok!

    if (itfile.is_open())
        ;//log << "File opened" ;
    else
    {
        log <<"Failed to open GP file" ;
        log<<"Scenario done.";
        return false;
    }

    Tab tab;
    Gp4Import importer; //(tabFile,tab);
    //swtich to different versions of importers 3\4\5

    importer.import(itfile,&tab);
    tab.postLoading();
    tab.connectTracks(); //new for chains refact

    auto f = exportMidi(&tab);
    std::ofstream midiOut(outFileName, std::ios::binary);
    size_t bytesWritten = f->writeStream(midiOut);

    qDebug() << "Bytes midi written " << bytesWritten << " to " << outFileName;
    if (bytesWritten == 0) {
        qDebug() << "ERROR empty file ";
    }

    if (gmyFile.empty() == false) {
        std::ofstream gmyOut(gmyFile, std::ios::binary);
        GmyFile g;
        g.saveToFile(gmyOut, &tab);
    }

    return true;
}

//refact later
bool testGP5(std::string fileName, std::string outFileName)
{
    std::ifstream itfile;
    itfile.open(fileName.c_str(),std::ifstream::binary);  //small file - runs ok!

    if (itfile.is_open())
        ; //log << "File opened" ;
    else
    {
        log <<"Failed to open GP file" ;
        log<<"Scenario done.";
        return false;
    }

    Tab tab;
    Gp5Import importer; //(tabFile,tab);

    //swtich to different versions of importers 3\4\5

    importer.import(itfile,&tab);
    tab.postLoading();
    tab.connectTracks(); //new for chains

    auto f = exportMidi(&tab);
    std::ofstream midiOut(outFileName, std::ios::binary);
    size_t bytesWritten = f->writeStream(midiOut);

    qDebug() << "Bytes midi written " << bytesWritten << " to " << outFileName;


    return true;
}


bool greatCheck()
{

    //phase 1
    size_t from = 1; //1
    size_t to = 11; //70

    int scen = 1;


    qDebug() << "Starting big check";

    for (size_t i = from; i <= to; ++i)
    {
        try
        {
            std::string newLine = std::to_string(scen) + "." + std::to_string(i);

            std::string testLocation = Config::getInst().testsLocation;

            //std::string gp5File = testLocation + std::string("g5/") +std::string(newLine.c_str()) + std::string(".gp5");
            std::string gp4File = testLocation  +std::string(newLine.c_str()) + std::string(".gp4");
            //std::string gp3File = testLocation + std::string("g3/") +std::string(newLine.c_str()) + std::string(".gp3");
            std::string outGp3 =  testLocation + std::string("all_out/") + std::string(newLine.c_str()) + std::string("_gen3.mid");
            std::string outGp4 =  testLocation + std::string("all_out/") + std::string(newLine.c_str()) + std::string("_gen4.mid");
            std::string outGp5 =  testLocation + std::string("all_out/") + std::string(newLine.c_str()) + std::string("_gen5.mid");


            std::string midiFile = testLocation + std::string("m/") +std::string(newLine.c_str()) + std::string(".mid");
            std::string outMid = std::string("all_out/") + std::string(newLine.c_str()) + std::string("_amid.mid");

            //log<<"TestFile "<<gp3File.c_str();
            //if ( testMidi(midiFile,outMid,log) == false ) return 0;
            //if ( testGP3 (gp3File,outGp3,doTheLogs)  == false ) return 0; //last true - no out
            if ( testGP4 (gp4File,outGp4)  == false ) return 0; //last true - no out
            //if ( testGP5 (gp5File,outGp5,doTheLogs) == false ) return 0;

            std::cout << "done";
        }
        catch(...)
        {
            qDebug() <<"Exception";
        }
    }

    //2.1_midi 2.1_gen

    return true;
}



bool aurals::greatCheckScenarioCase(uint32_t scen, uint32_t from, uint32_t to, uint32_t v)
{
    if (scen == 4)
        return false; //yet not in resourses

    for (uint32_t i = from; i <= to; ++i)
    {
        try
        {
            std::string newLine = std::to_string(scen) + "." + std::to_string(i);

            std::string testLocation = Config::getInst().testsLocation;
            //std::string gp5File = testLocation + std::string("g5/") +std::string(newLine.c_str()) + std::string(".gp5");
            std::string gp4File = testLocation +std::string(newLine.c_str()) + std::string(".gp4");
            //std::string gp3File = testLocation + std::string("g3/") +std::string(newLine.c_str()) + std::string(".gp3");

            //std::string outGp5 =  std::string("all_out/") + std::string(newLine.c_str()) + std::string("_gen5.mid");
            std::string outGp4 =  testLocation + std::string("all_out/") + std::string(newLine.c_str()) + std::string(".mid");
            //std::string outGp4plus = testLocation +  std::string("regression/") + std::string(newLine.c_str()) + std::string(".mid");
            //std::string outGp3 =  std::string("all_out/") + std::string(newLine.c_str()) + std::string("_gen3.mid");
            std::string gmyOut = testLocation +  std::string("all_out/") + std::string(newLine.c_str()) + std::string(".gmy");



            //log<<"TestFile "<<newLine.c_str();
            //resourses yet contain only gp4, later solve it
            /*if (v==5)
            if ( testGP5 (gp5File,outGp5,doTheLogs) == false )
                log <<"Test gp5 "<<newLine.c_str()<<" failed";*/

            if (v==4)
            {
                if ( testGP4 (gp4File, outGp4, gmyOut) == false )
                  log <<"Test gp4 "<<newLine.c_str()<<" failed";
                //Add tabloader test
            }

            /*
            if (v==3)
                if ( testGP3 (gp3File,outGp3,doTheLogs) == false )
                    log <<"Test gp3 "<<newLine.c_str()<<" failed";*/

            //std::cout << "test file done"<<std::endl;
        }
        catch(...)
        {
            std::cout <<"Exception";
        }
    }

    //2.1_midi 2.1_gen

    return true;
}

void aurals::connectConfigs(Config& config)
{
    config.connectLog(&tabLog,0,"tab");
    config.connectLog(&midiLog,2,"midi");
    config.connectLog(&gtpLog,3,"gtp");
}


bool aurals::testScenario() {
    return greatCheck();
}
