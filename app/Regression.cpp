#include "Regression.hpp"

#include <string>
#include <unordered_map>
#include <fstream>
#include <iostream>

#include <QFileInfo>

#include "tab/Tab.hpp"
#include "tab/tools/GtpFiles.hpp"
#include "tab/tools/MidiExport.hpp"
#include "tab/tools/GmyFile.hpp"
#include "app/Config.hpp"


using namespace aural_sight;


bool aural_sight::checkHasRegression() {

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
    for (size_t groupIdx = 1; groupIdx <= 4; ++groupIdx) {
        size_t from = 1;
        size_t to = groupLength[groupIdx] - 1;

        if (groupIdx == 3)
            break;

        for (size_t fileIndx = from; fileIndx <= to; ++fileIndx) {
            std::string testName = std::to_string(groupIdx) + "." + std::to_string(fileIndx);
            std::string testLocation = AConfig::getInst().testsLocation;
            std::string midiFile = testLocation + std::string("regression_check/") + testName + std::string(".mid");
            std::string midiFileCheck = testLocation + std::string("regression/") + testName + std::string(".mid");
            std::string gp4File = testLocation + testName + std::string(".gp4");

            std::ifstream itfile;
            itfile.open(gp4File.c_str(),std::ifstream::binary);

            Tab tab;
            importer.import(itfile, &tab);
            tab.postLoading();
            tab.connectTracks();


            auto fSize = QFileInfo(midiFileCheck.c_str()).size();

            size_t bytesWritten = 0;
            auto f = exportMidi(&tab);
            {
                std::ofstream midiOut(midiFile, std::ios::binary);
                bytesWritten = f->writeStream(midiOut);
                std::cerr << "MidiSize: " << bytesWritten << " to " << testName
                          <<  " expected: " << fSize << std::endl;
            }

            if (fSize != bytesWritten) {
                std::cerr << "Files " << testName << " got a MIDI regression " << std::endl;
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
                std::cerr << "Files " << testName << " got a GMY regression " << std::endl;
                ++regressionCountGmy;
            }

        }
    }

    if (regressionCountMidi || regressionCountGmy) {
        std::cerr << "Total " << regressionCountMidi << " files got MIDI regression"
                  << std::endl << "Total " << regressionCountGmy << " files got GMY regression" << std::endl;
        return true;
    }
    return false;
}



std::vector<MacroCommand> writeAndReadMacro(const std::vector<MacroCommand>& commands) {
    auto tempMacroPath = AConfig::getInst().testsLocation + "macro";
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


void macroSimpleTest1() {
    Tab t;
    t.onTabCommand(TabCommand::NewTrack);
    t.onTabCommand(TabCommand::Solo);

    auto commands = writeAndReadMacro(t.getMacro());
    Tab t2;
    for (auto& c: commands)
        t2.playCommand(c);

    if (t2.at(0)->getStatus() != 2) {
        std::cerr << "ERROR: Tab commands failed!" << std::endl;
    }
    else
        std::cout << "1 Simple tab commands fine" << std::endl;
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
        std::cerr <<"ERROR: Tab commands failed!" << std::endl;
        std::cerr <<"Track name was " << t2.at(0)->getName().c_str() << std::endl;
    }
    else
        std::cout << "2 Simple tab commands fine"<< std::endl;
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
        std::cerr << "ERROR: Tab commands failed!" << std::endl;
        std::cerr << "Track instrument was " << t2.at(0)->getInstrument() << std::endl;
    }
    else
        std::cout << "3 Simple tab commands fine" << std::endl;
}


void macroSimpleTest4() {
    Tab t;
    t.onTabCommand(TabCommand::NewTrack);
    t.setSignsTillEnd(2, 2);
    auto commands = writeAndReadMacro(t.getMacro());
    Tab t2;
    for (auto& c: commands)
        t2.playCommand(c);
    if (t2.at(0)->at(0)->getSignNum() != 2 || t2.at(0)->at(0)->getSignDenum() != 2) {
        std::cerr << "ERROR: Tab commands failed!" << std::endl;
        std::cerr << "Num den were " << t2.at(0)->at(0)->getSignNum()
                 << " " <<  t2.at(0)->at(0)->getSignDenum()<< std::endl;
    }
    else
        std::cout << "4 Simple tab commands fine" << std::endl;
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
        std::cerr << "ERROR: Track commands failed!" << std::endl;
        std::cerr << "Num den were " << t2.at(0)->at(0)->getSignNum()
                 << " " <<  t2.at(0)->at(0)->getSignDenum() << std::endl;
    }
    else
        std::cout << "1 Simple track commands fine" << std::endl;
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
        std::cerr << "ERROR: Track commands failed!" << std::endl;
        std::cerr << str.c_str() << " vs some" << std::endl;
    }
    else
        std::cout << "2 Simple track commands fine" << std::endl;
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
        std::cerr << "ERROR: Track commands failed!" << std::endl;
        std::cerr << t2.at(0)->at(0)->at(0)->getDuration() << " vs X" << std::endl;
    }
    else
        std::cout << "3 Simple track commands fine" << std::endl;
}


void aural_sight::runRegressionTests() {
    //greatCheckScenarioCase(1, 1, 12, 4);
    //greatCheckScenarioCase(2, 1, 38, 4);
    if (checkHasRegression()) {
        std::cerr << "Has regression, terminating" << std::endl;
        exit(0);
        return;
    }
    else
        std::cout << "Has no regression" << std::endl; //TODO also check more from 3rd group generate on oldest version

    macroSimpleTest1(); //Tab commands plain
    macroSimpleTest2(); //String tab command
    macroSimpleTest3(); //Int tab command
    macroSimpleTest4(); //Two int command

    macroTrackTest1();
    macroTrackTest2();
    macroTrackTest3();
}



bool aural_sight::checkMidiIORegression() {

    bool fine = true;

    std::unordered_map<size_t, size_t> groupLength = {
        {1, 12},
        {2, 39}
    };

    for (size_t groupIdx = 1; groupIdx <= 2; ++groupIdx) {
        size_t from = 1;
        size_t to = groupLength[groupIdx] - 1;

        if (groupIdx == 3)
            break;

        for (size_t fileIndx = from; fileIndx <= to; ++fileIndx) {
            std::string testLocation = AConfig::getInst().testsLocation;
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
                std::cerr << "Midi IO fail " << testName << std::endl;
                fine = false;
             }
        }
    }

    if (fine)
        std::cout << "There is not MidiIO regression" << std::endl;

    return fine;
}

