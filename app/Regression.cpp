#include "Regression.hpp"

#include <string>
#include <unordered_map>
#include <fstream>
#include <iostream>

#include "app/log.hpp"
#include <QFileInfo>

#include "tab/Tab.hpp"
#include "tab/tools/GtpFiles.hpp"
#include "tab/tools/MidiExport.hpp"
#include "tab/tools/GmyFile.hpp"
#include "app/Config.hpp"


using namespace aurals;


bool aurals::checkHasRegression() {

    std::unordered_map<size_t, size_t> groupLength = {
        {1, 12},
        {2, 39},
        {3, 70},
        {4, 109}
    };

    Gp4Import importer;
    GmyFile exporter;
    uint16_t regressionCountMidi = 0;
    uint32_t regressionCountGmy = 0;

    size_t totalChecks = 0;

    for (size_t groupIdx = 1; groupIdx <= 4; ++groupIdx) {
        size_t from = 1;
        size_t to = groupLength[groupIdx] - 1;

        if (groupIdx == 4)
            break;


        for (size_t fileIndx = from; fileIndx <= to; ++fileIndx) {
            std::string testName = std::to_string(groupIdx) + "." + std::to_string(fileIndx);
            std::string testLocation = Config::getInst().testsLocation;
            std::string midiFile = testLocation + std::string("regression_check/") + testName + std::string(".mid");
            std::string midiFileCheck = testLocation + std::string("regression/") + testName + std::string(".mid");
            std::string gp4File = testLocation + testName + std::string(".gp4");

            std::ifstream itfile;
            itfile.open(gp4File.c_str(),std::ifstream::binary);

            Tab tab;
            importer.import(itfile, &tab);
            tab.postLoading();
            tab.connectTracks();

            ++totalChecks;


            auto fSize = QFileInfo(midiFileCheck.c_str()).size();

            size_t bytesWritten = 0;
            auto f = exportMidi(&tab);
            {
                std::ofstream midiOut(midiFile, std::ios::binary);
                bytesWritten = f->writeStream(midiOut);
                if (fSize != bytesWritten)
                    qDebug() << "MidiSize: " << bytesWritten << " to " << testName
                          <<  " expected: " << fSize;
            }

            if (fSize != bytesWritten) {
                qDebug() << "Files " << testName << " got a MIDI regression ";
                ++regressionCountMidi;
            }
            else {
                //Delayed compare binnary each byte (mtherapp)
            }

            std::string gmyFile = testLocation + std::string("regression_check/") + testName + std::string(".gmy");
            std::string gmyFileCheck = testLocation + std::string("regression/") + testName + std::string(".gmy");

            {
                std::ofstream gmyOut(gmyFile, std::ios::binary);
                exporter.saveToFile(gmyOut, &tab);
            }

            auto sizeNew = QFileInfo(gmyFile.c_str()).size();
            auto sizeOld = QFileInfo(gmyFileCheck.c_str()).size();

            if (sizeNew != sizeOld) {
                qDebug() << "Files " << testName << " got a GMY regression ";
                ++regressionCountGmy;
            }

        }
    }

    if (regressionCountMidi || regressionCountGmy) {
        qDebug() << "Total " << regressionCountMidi << " files got MIDI regression";
        qDebug() << "Total " << regressionCountGmy << " files got GMY regression";
        return true;
    }

    qDebug() << "Total checks for regression " << totalChecks;

    return false;
}



std::vector<MacroCommand> writeAndReadMacro(const std::vector<MacroCommand>& commands) {
    auto tempMacroPath = Config::getInst().testsLocation + "macro";
    {
        std::ofstream os(tempMacroPath, std::ios::binary);
        saveMacroComannds(commands, os);
        //qDebug() << commands.size() << " commands written";
    }
    std::vector<MacroCommand> readCommands;
    {
        std::ifstream is(tempMacroPath, std::ios::binary);
        readCommands = loadMacroCommands(is);
        //qDebug() << readCommands.size() << " commands read";
    }
    return readCommands;
}


void macroSimpleTest1() { //Update this test + reversable commands for this
    Tab t;
    t.onTabCommand(TabCommand::NewTrack);
    t.onTabCommand(TabCommand::Solo);

    auto commands = writeAndReadMacro(t.getMacro());
    Tab t2;
    for (auto& c: commands)
        t2.playCommand(c);

    if (t2.at(0)->getStatus() != 2) {
        qDebug() << "ERROR: Tab commands failed! 1";
    }
    else
        qDebug() << "1 Simple tab commands fine";
}




void macroSimpleTest2() {
    Tab t;
    t.onTabCommand(TabCommand::NewTrack);
    t.changeTrackName("check");
    auto commands = writeAndReadMacro(t.getMacro());
    Tab t2;
    //qDebug() << "Commands size " << commands.size();
    for (auto& c: commands)
        t2.playCommand(c);
    if (t2.at(0)->getName() != "check") {
        qDebug() <<"ERROR: Tab commands failed! 2";
        qDebug() <<"Track name was " << t2.at(0)->getName();
    }
    else
        qDebug() << "2 Simple tab commands fine";
}


void macroSimpleTest3() {
    Tab t;
    t.onTabCommand(TabCommand::NewTrack);
    t.changeTrackInstrument(38);
    auto commands = writeAndReadMacro(t.getMacro());
    Tab t2;
    for (auto& c: commands)
        t2.playCommand(c);
    if (t2.at(0)->getInstrument() != 38) {
        qDebug() << "ERROR: Tab commands failed! 3";
        qDebug() << "Track instrument was " << t2.at(0)->getInstrument();
    }
    else
        qDebug() << "3 Simple tab commands fine";
}


void macroSimpleTest4() {
    Tab t;
    t.onTabCommand(TabCommand::NewTrack);
    t.setSignsTillEnd(2, 2);
    auto commands = writeAndReadMacro(t.getMacro()); //Seams there is error on replay here
    Tab t2;
    for (auto& c: commands)
        t2.playCommand(c);
    if (t2.at(0)->at(0)->getSignNum() != 2 || t2.at(0)->at(0)->getSignDenum() != 2) {
        qDebug() << "ERROR: Tab commands failed! 4";
        qDebug() << "Num den were " << t2.at(0)->at(0)->getSignNum()
                 << " " <<  t2.at(0)->at(0)->getSignDenum();
    }
    else
        qDebug() << "4 Simple tab commands fine";
}



void macroTrackTest1() {
    Tab t;
    t.onTabCommand(TabCommand::NewTrack);
    t.at(0)->setBarSign(2, 2);
    auto commands = writeAndReadMacro(t.getMacro());
    Tab t2;
    for (auto& c: commands)
        t2.playCommand(c);
    if (t2.at(0)->at(0)->getSignNum() != 2 || t2.at(0)->at(0)->getSignDenum() != 2) {
        qDebug() << "ERROR: Track commands failed! 1+";
        qDebug() << "Num den were " << t2.at(0)->at(0)->getSignNum()
                 << " " <<  t2.at(0)->at(0)->getSignDenum();
    }
    else
        qDebug() << "1 Simple track commands fine";
}



void macroTrackTest2() {
    Tab t;
    t.onTabCommand(TabCommand::NewTrack);
    t.at(0)->setTextOnBeat("some");
    auto commands = writeAndReadMacro(t.getMacro());
    Tab t2;
    for (auto& c: commands)
        t2.playCommand(c);
    std::string str;
    t2.at(0)->at(0)->at(0)->getText(str);
    if (str != "some") {
        qDebug() << "ERROR: Track commands failed! 2+";
        qDebug() << str.c_str() << " vs some";
    }
    else
        qDebug() << "2 Simple track commands fine";
}

void macroTrackTest3() {
    Tab t;
    t.onTabCommand(TabCommand::NewTrack);
    t.at(0)->onTrackCommand(TrackCommand::DecDuration);
    auto commands = writeAndReadMacro(t.getMacro());
    Tab t2;
    for (auto& c: commands)
        t2.playCommand(c);

    if (t2.at(0)->at(0)->at(0)->getDuration() != 4) {
        qDebug() << "ERROR: Track commands failed! 3+";
        qDebug() << t2.at(0)->at(0)->at(0)->getDuration() << " vs X";
    }
    else
        qDebug() << "3 Simple track commands fine";
}

#include <QtGlobal>


void aurals::runRegressionTests() {
    //greatCheckScenarioCase(1, 1, 12, 4);
    //greatCheckScenarioCase(2, 1, 38, 4);
    if (checkHasRegression()) {
        qDebug() << "Has regression, terminating";

        #ifndef Q_OS_ANDROID
        exit(0);
        #endif
        return;
    }
    else
        qDebug() << "Has no regression";

    macroSimpleTest1(); //Tab commands plain
    macroSimpleTest2(); //String tab command
    macroSimpleTest3(); //Int tab command
    macroSimpleTest4(); //Two int command

    macroTrackTest1();
    macroTrackTest2();
    macroTrackTest3();
}



bool aurals::checkMidiIORegression() {

    bool fine = true;

    std::unordered_map<size_t, size_t> groupLength = {
        {1, 12},
        {2, 39},
        {3, 70}
    };

    for (size_t groupIdx = 1; groupIdx <= 3; ++groupIdx) {
        size_t from = 1;
        size_t to = groupLength[groupIdx] - 1;

        if (groupIdx == 3)
            break;

        for (size_t fileIndx = from; fileIndx <= to; ++fileIndx) {
            std::string testLocation = Config::getInst().testsLocation;
            std::string testName = std::to_string(groupIdx) + "." + std::to_string(fileIndx);
            std::string midiFile = testLocation + std::string("regression_check/") + testName + std::string(".mid");
            std::string midiFileCheck = testLocation + std::string("regression/") + testName + std::string(".mid");

            MidiFile mid;
            {
                std::ifstream is(midiFile, std::ios::binary);
                mid.readStream(is);
            }
            {
                std::ofstream os(midiFileCheck, std::ios::binary);
                mid.writeStream(os);
            }

             auto originalSize = QFileInfo(midiFile.c_str()).size();
             auto outputSize = QFileInfo(midiFileCheck.c_str()).size();

             if (originalSize != outputSize) {
                qDebug() << "Midi IO fail " << testName;
                fine = false;
             }
        }
    }

    if (fine)
        qDebug() << "There is not MidiIO regression";

    return fine;
}

