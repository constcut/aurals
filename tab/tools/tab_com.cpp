#include "tab/Tab.hpp"

#include "midi/MidiFile.hpp"
#include "GmyFile.hpp"
#include "TabLoader.hpp"
#include "GtpFiles.hpp"

#include <fstream>
#include <QDebug>


using namespace aurals;


//TODO prepare undo operations
//They have to be for all possible commands

void Tab::setSignsTillEnd(size_t num, size_t denom) {
    _macroCommands.push_back(TwoIntCommand<TabCommand>{TabCommand::SetSignTillEnd, num, denom});

    for (size_t trackId = 0; trackId < this->size(); ++trackId)
        for (size_t i = _currentBar; i < this->at(trackId)->size(); ++i){
            this->at(trackId)->at(i)->setSignDenum(denom);
            this->at(trackId)->at(i)->setSignNum(num);

            qDebug() << "Changing bar " << i << " on " << num << " " << denom;
        }
}


void Tab::moveCursorInTrackRight() {
    if (_displayBar < at(0)->size() - 1)
        ++_displayBar;
}

void Tab::moveCursorInTrackLeft() {
    if (_displayBar > 0)
        --_displayBar;
}





void Tab::changeTrackVolume(size_t newVol) {
    _macroCommands.push_back(IntCommand<TabCommand>{TabCommand::Volume, newVol});
     at(_currentTrack)->setVolume(newVol);
}


void Tab::changeTrackName(std::string newName) {
    _macroCommands.push_back(StringCommand<TabCommand>{TabCommand::Name, newName});
    at(_currentTrack)->setName(newName);
}


void Tab::changeTrackInstrument(size_t val) {
    _macroCommands.push_back(IntCommand<TabCommand>{TabCommand::Instument, val});
    at(_currentTrack)->setInstrument(val);
}

void Tab::changeTrackPanoram(size_t val) {
    _macroCommands.push_back(IntCommand<TabCommand>{TabCommand::Panoram, val});
    at(_currentTrack)->setPan(val);
}

void Tab::createNewTrack() {
    Tab* pTab = this;
    auto track = std::make_unique<Track>();
    track->setParent(pTab);
    std::string iName("NewInstrument");
    track->setName(iName);
    track->setInstrument(25);
    track->setVolume(15);
    track->setDrums(false);
    track->setPan(8); //center now

    auto& tuning = track->getTuningRef();
    tuning.setStringsAmount(6);
    tuning.setTune(0, 64);
    tuning.setTune(1, 59);
    tuning.setTune(2, 55);
    tuning.setTune(3, 50);
    tuning.setTune(4, 45);
    tuning.setTune(5, 40);

    if (pTab->size()) {
        for (size_t barI=0; barI < pTab->at(0)->size(); ++barI) {

            auto bar = std::make_unique<Bar>();

            bar->flush();
            bar->setSignDenum(4); bar->setSignNum(4); //Подумать над механизмами, разные размеры только при полиритмии нужны
            bar->setRepeat(0);

            auto beat = std::make_unique<Beat>();

            beat->setPause(true);
            beat->setDotted(0);
            beat->setDuration(3);
            beat->setDurationDetail(0);
            bar->push_back(std::move(beat));
            track->push_back(std::move(bar));
        }
    }
    else {
        auto bar = std::make_unique<Bar>();

        bar->flush();
        bar->setSignDenum(4); bar->setSignNum(4);
        bar->setRepeat(0);

        auto beat = std::make_unique<Beat>();

        beat->setPause(true);
        beat->setDotted(0);
        beat->setDuration(3);
        beat->setDurationDetail(0);
        bar->push_back(std::move(beat));
        track->push_back(std::move(bar));
    }


    pTab->push_back(std::move(track));
    pTab->connectTracks();
}



void Tab::midiPause() {
    if (_isPlaying == false) {
        //STARTMIDI
        _isPlaying = true;
    }
    else {
        //MidiEngine::stopDefaultFile();
        _isPlaying = false;
    }
}


void Tab::setMarker(std::string text) {
    auto& fromFirstTrack = at(0)->at(_currentBar);
    fromFirstTrack->setMarker(text,0);
}


void Tab::openReprise() {
    auto& firstTrackBar = this->at(0)->at(_currentBar);
    std::uint8_t repeat = firstTrackBar->getRepeat();
    std::uint8_t repeatOpens = repeat & 1;
    std::uint8_t repeatCloses = repeat & 2;
    if (repeatOpens){
        firstTrackBar->setRepeat(0); //flush
        firstTrackBar->setRepeat(repeatCloses);
    }
    else
        firstTrackBar->setRepeat(1);
}


void Tab::closeReprise(size_t count) {
    _macroCommands.push_back(IntCommand<TabCommand>{TabCommand::CloseReprise, count});
    auto& firstTrackBar = this->at(0)->at(_currentBar);
    std::uint8_t repeat = firstTrackBar->getRepeat();
    std::uint8_t repeatOpens = repeat & 1;
    std::uint8_t repeatCloses = repeat & 2;
    if (repeatCloses) {
        firstTrackBar->setRepeat(0);
        firstTrackBar->setRepeat(repeatOpens);
    }
    else {
        if (count)
            firstTrackBar->setRepeat(2, count);
    }
}


void Tab::gotoBar(size_t pos) {
    _macroCommands.push_back(IntCommand<TabCommand>{TabCommand::GotoBar, pos});
    _currentBar = pos;
    _displayBar = pos;
}


void Tab::saveAs(std::string filename) {
    _macroCommands.push_back(StringCommand<TabCommand>{TabCommand::SaveAs, filename});
    std::ofstream file(filename);
    GmyFile gmyFile;
    gmyFile.saveToFile(file, this);
    file.close();
}


void Tab::onTabCommand(TabCommand command) {
    _macroCommands.push_back(command);
    if (_handlers.count(command))
        (this->*_handlers.at(command))();
}
