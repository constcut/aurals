#include "tab/tools/Commands.hpp"

#include "TabViews.hpp"

#include "midi/MidiFile.hpp"
#include "tab/tools/TabLoader.hpp"
#include "tab/tools/GmyFile.hpp"
#include "tab/tools/MidiExport.hpp"
#include "midi/MidiRender.hpp"

#include "app/Config.hpp"

#include <QInputDialog>
#include <QMutex>
#include <QFileDialog>
#include <QApplication>
#include <QScreen>

#include <QDebug>
#include <fstream>



using namespace aural_sight;



void changeBarSignsQt(Track* pTrack, [[maybe_unused]] int&  selectionBarFirst, [[maybe_unused]] int& selectionBarLast) {
   //TODO возможно избавиться от аргументов
    bool ok=false;
    int newNum = QInputDialog::getInt(0,"Input",
                         "New Num:", QLineEdit::Normal,
                         1,128,1,&ok);
    if (!ok)
        return;
    ok=false;
    int newDen = QInputDialog::getInt(0,"Input",
                         "New Denum(1,2,4,8,16):", QLineEdit::Normal,
                         1,128,1,&ok);
    if (ok)
        pTrack->changeBarSigns(newNum,newDen);
}



void handleKeyInput(int digit, int& digitPress, Track* pTrack, size_t cursor, size_t cursorBeat, size_t stringCursor, std::vector<ReversableCommand>& commandSequence) {

    //group operations
    if (digitPress >= 0) {
        if (digitPress < 10) {
            int pre = digitPress;
            digitPress *= 10;
            digitPress += digit;
            if (digitPress > pTrack->getMidiInfo(3)) { //Destoy all GPCOMP TODO
                digitPress = digit;
                if (digit == pre)
                    return; //no changes
            }
        }
        else
            digitPress = digit;
    }
    else
        digitPress = digit;

    if ( pTrack->at(cursor)->size() > cursorBeat ) {
        std::uint8_t lastFret = pTrack->at(cursor)->at(cursorBeat)->getFret(stringCursor+1);

        ReversableCommand command(ReversableType::SetFret,lastFret);
        command.setPosition(0,cursor,cursorBeat,stringCursor+1);
        commandSequence.push_back(std::move(command));
        pTrack->at(cursor)->at(cursorBeat)->setFret(digitPress,stringCursor+1);

        Note *inputedNote =  pTrack->at(cursor)->at(cursorBeat)->getNote(stringCursor+1);
        std::uint8_t tune = pTrack->getTuning().getTune(stringCursor);
        int chan = 0;
        if (pTrack->isDrums()) {
            chan = 10; //tune to 0 attention refact error
            tune = 0;
        }
        //STARTMIDI
        [[maybe_unused]] std::uint8_t midiNote = inputedNote->getMidiNote(tune);
        //MidiEngine::sendSignalShort(0x90|chan,midiNote,120);
        ///MidiEngine::sendSignalShortDelay(250,0x80|chan,midiNote,120);
        //MidiEngine::sendSignalShortDelay(750,0x90|chan,midiNote+2,120);
    }
}



void saveAsFromTrack(TabView* tabParent) {
    auto fd = std::make_unique<QFileDialog>();

    fd->setStyleSheet("QScrollBar:horizontal {\
                        border: 2px solid grey;\
                        background: #32CC99;\
                        height: 15px;\
                        margin: 0px 20px 0 20px;\
                    }\
                    QLineEdit { height: 20px; \
                    }");

    fd->setViewMode(QFileDialog::List);

    std::string dir="";
#ifdef __ANDROID_API__
    dir="/sdcard/";
    fd->setDirectory("/sdcard/");
    QScreen *screen = QApplication::screens().at(0);
    fd->setGeometry(0,0,screen->geometry().width(),screen->geometry().height());
#endif

    QString saveFileName = fd->getSaveFileName(0,"Save tab as",dir.c_str(),"Guitarmy files (*.gmy)");
    GmyFile gmyFile;
    std::string  gfileName = saveFileName.toStdString();
    std::ofstream file(gfileName, std::ios::binary);
    gmyFile.saveToFile(file,tabParent->getTab().get());
    return;
}



void setTextOnBeat(Track *track) {
    auto& beat = track->at(track->cursor())->at(track->cursorBeat());
    std::string beatText;
    beat->getText(beatText);

    bool ok=false;
    QString newText = QInputDialog::getText(0,"Input",
                     "Input text:", QLineEdit::Normal,beatText.c_str(),&ok);
    if (ok) {
        beatText = newText.toStdString();
        track->setTextOnBeat(beatText);
    }
}


void setBarSign(Track* pTrack) {
    bool ok=false;
    int newNum = QInputDialog::getInt(0,"Input","New Num:",
                        QLineEdit::Normal,1,128,1,&ok);

    bool thereWasChange = false;
    if (ok)
        thereWasChange = true;
    ok=false;
    int newDen = QInputDialog::getInt(0,"Input","New Denum(1,2,4,8,16):",
                 QLineEdit::Normal,1,128,1,&ok);
    if (ok)
        thereWasChange = true;

    if (thereWasChange) { //also could set to all here if turned on such flag
        pTrack->setBarSign(newNum, newDen);
    }
}




void TabView::onTrackCommand([[maybe_unused]] TrackCommand command) {

    if (_tracksView.empty())
        return; //TODO возможно всю обработку можно сделать в TabView, а потом вовсе вывести
     //На текущий момент поддерживаем только 1 отображение
    _tracksView[0]->onTrackCommand(command);
}


void TrackView::onTrackCommand(TrackCommand command) {

    size_t& cursor = _pTrack->cursor(); //TODO get rid slowly
    size_t& cursorBeat = _pTrack->cursorBeat();
    size_t& stringCursor = _pTrack->stringCursor();
    int& selectionBarFirst = _pTrack->selectBarFirst();
    int& selectionBarLast = _pTrack->selectBarLast();

    if (command == TrackCommand::PlayFromStart) {
        _pTrack->gotoTrackStart();
        onTabCommand(TabCommand::PlayMidi);
    }
    else if (command == TrackCommand::SetSignForSelected)
      changeBarSignsQt(_pTrack, selectionBarFirst, selectionBarLast);
    else if (command == TrackCommand::SaveAsFromTrack)
        saveAsFromTrack(_tabParrent);
    else if (command == TrackCommand::Text)
        setTextOnBeat(_pTrack);
    else if (command == TrackCommand::SetBarSign)
        setBarSign(_pTrack);
    else if (command == TrackCommand::PrevLine)
        movePrevLine();
    else if (command == TrackCommand::NextLine)
        moveNextLine();
    else
        _pTrack->onTrackCommand(command);

    update(); //TODO only on change
}



void TrackView::keyevent(std::string press) //TODO масштабные макротесты, чтобы покрывать все сценарии
{
    size_t& cursor = _pTrack->cursor();
    size_t& cursorBeat = _pTrack->cursorBeat();
    size_t& stringCursor = _pTrack->stringCursor();

    if (isdigit(press[0]))
        handleKeyInput(press[0]-48, _pTrack->digitPress(), _pTrack, cursor, cursorBeat, stringCursor, _pTrack->commandSequence);
    else
        qDebug() << "Key event not handled in TrackView: " << press.c_str();
}



void TrackView::onTabCommand(TabCommand command) {
    _tabParrent->onTabCommand(command);
}



void TabView::keyPress(int code) {
    qDebug() << "TABVIEW key event " << code;

    bool updated = false;

    if (code == Qt::Key_Left) {
        onTrackCommand(TrackCommand::PrevBeat); //onTabCommand(TabCommand::MoveLeft); + updated
    }
    if (code == Qt::Key_Right) {
        onTrackCommand(TrackCommand::NextBeat); //onTabCommand(TabCommand::MoveRight); + updated
    }
    if (code == Qt::Key_Up) {
        onTrackCommand(TrackCommand::StringDown);
    }
    if (code == Qt::Key_Down) {
        onTrackCommand(TrackCommand::StringUp);
    }

    if (isdigit(code)) {
        _tracksView[0]->keyevent(std::to_string(code - 48));
        _tracksView[0]->update();
    }

    //TODO other keys mapping if they set for any function
    //2 big handlers for Tab and Track commands

    if (updated)
        update();
}

//Tab commands area


void changeTrackName(Tab* pTab) { //TODOM
    bool ok=false;
    //refact inputs to gview
    QString newName = QInputDialog::getText(0,"Input",
                         "New Instrument name:", QLineEdit::Normal,"untitled",&ok);
    std::string stdName = newName.toStdString();
    if (ok)
       pTab->changeTrackName(stdName);
}

void changeTrackVolume(Tab* pTab)  { //TODOM
    bool ok=false;
    int newVol = QInputDialog::getInt(0,"Input",
                         "Vol Instrument:", QLineEdit::Normal,
                         0,16,1, &ok);
    if (ok)
        pTab->changeTrackVolume(newVol);

}

void setSignTillEnd(Tab* pTab) { //TODOM
    bool ok=false;
    int newNum = QInputDialog::getInt(0,"Input",
                         "New Num:", QLineEdit::Normal,
                         1,128,1,&ok);
    if (!ok)
        return;
    ok=  false;
    int newDen = QInputDialog::getInt(0,"Input",
                         "New Denum(1,2,4,8,16):", QLineEdit::Normal,
                         1,128,1,&ok);
    if (ok)
        pTab->setSignsTillEnd(newNum, newDen);

}

void setMarker(Tab* pTab) {
    bool ok=false;
    QString markerText= QInputDialog::getText(0,"Input",
                         "Marker:", QLineEdit::Normal,"untitled",&ok);
    if (ok) {
        std::string stdMarkerText = markerText.toStdString();
        pTab->setMarker(stdMarkerText);
    }
}

void goToBar(Tab* pTab) {
    size_t trackLen = pTab->at(0)->size();
    bool ok=false; //TODO позже разделить Qt запросы и установку параметров
    int newTimes = QInputDialog::getInt(0,"Input",
                         "Bar to jump:", QLineEdit::Normal, 1, trackLen, 1, &ok);
    if (ok) {
        --newTimes;
        pTab->gotoBar(newTimes);
    }
}


void setTune(Track* pTrack) {

    bool ok=false;
    QStringList items;
    char iBuf[10];

    double fTable []=
    {16.3515978313,
     17.3239144361,
     18.3540479948,
     19.4454364826,
     20.6017223071,
     21.8267644646,
     23.1246514195,
     24.4997147489,
     25.9565435987,
     27.5000000000,
     29.1352350949,
     30.8677063285
    };

    for (size_t i = 0; i < 8; ++i) {
        //octaves
        iBuf[0] = i +49;
        iBuf[1] = 0;
        std::string octave = iBuf;
        std::string note;
        int midiNote = 0;
        for (size_t j = 0; j < 12; ++j) {
            //Notes
            switch (j) {
                case 0: note = "C";  break;
                case 1: note = "C#"; break;
                case 2: note = "D"; break;
                case 3: note = "D#"; break;
                case 4: note = "E"; break;
                case 5: note = "F"; break;
                case 6: note = "F#"; break;
                case 7: note = "G"; break;
                case 8: note = "G#"; break;
                case 9: note = "A"; break;
                case 10: note = "A#"; break;
                case 11: note = "B"; break;
                default: note="noteX";
            }

            int coefOctave = 1;
            for (size_t z = 0; z < i; ++z)
                coefOctave *= 2;

            double theFreq = fTable[j]*coefOctave;
            midiNote = 12 + j + 12 * i;
            std::string fullLine = note + octave +" - " +
                std::to_string(midiNote) +" - " + std::to_string(theFreq);
            items.push_back(fullLine.c_str());
        }
    }


    auto& tuning = pTrack->getTuningRef();
    for (size_t i = 0; i < tuning.getStringsAmount(); ++i) {
        int preValue = tuning.getTune(i)-12;
        QString resp = QInputDialog::getItem(0,"Input tune",
                                        ("String #" + std::to_string(i+1)).c_str(),items,preValue,false,&ok);
        int respIndex = -1;
        for (int j = 0; j < items.size(); ++j)
            if (items.at(j)==resp) {
                respIndex = j;
                break;
            }
        if (ok && respIndex >= 0)
            tuning.setTune(i, respIndex + 12);
    }
}


void saveAs(Tab* pTab) { //Move into Tab (но на этапе уже получения имени файла)

    auto fd = std::make_unique<QFileDialog>();
    fd->setStyleSheet("QScrollBar:horizontal {\
                      border: 2px solid grey;\
                      background: #32CC99;\
                      height: 15px;\
                      margin: 0px 20px 0 20px;\
                  }\
                  QLineEdit { height: 20px; \
                  }");

    fd->setViewMode(QFileDialog::List);

    std::string dir="";
#ifdef __ANDROID_API__
    dir="/sdcard/";
    fd->setDirectory("/sdcard/");
    QScreen *screen = QApplication::screens().at(0);
    fd->setGeometry(0,0,screen->geometry().width(),screen->geometry().height());
#endif

    QString saveFileName = fd->getSaveFileName(0,"Save tab as",dir.c_str(),"Guitarmy files (*.gmy)");
    std::string  gfileName = saveFileName.toStdString();
    pTab->saveAs(gfileName);
}


void closeReprise(Tab* pTab) { //TODO argument repeat times
    auto& firstTrackBar = pTab->at(0)->at(pTab->getCurrentBar());
    std::uint8_t repeat = firstTrackBar->getRepeat();
    std::uint8_t repeatCloses = repeat & 2;
    if (repeatCloses) {
        pTab->closeReprise(0);
    }
    else {
        bool ok=false;
        int newTimes = QInputDialog::getInt(0,"Input", "Repeat times:",
                            QLineEdit::Normal,2,99,1,&ok);
        if ((ok)&&(newTimes))
            pTab->closeReprise(newTimes);
    }
}

int changeTrackBpm(Tab* pTab) {
    bool ok=false;
    int newBpm = QInputDialog::getInt(0,"Input",
                         "New Bpm:", QLineEdit::Normal,
                         1,999,1,&ok);
    if (ok) {
        pTab->setBPM(newBpm);
        return newBpm;
    }
    else
        return pTab->getBPM();
}


void TabView::onTabCommand(TabCommand command) {
    //TODO undo для команд таблатуры так же
    if (command == TabCommand::SaveAs)
        saveAs(_pTab.get());
    else if (command == TabCommand::SetSignTillEnd)  //TODO хэндлеры для более простого вызова
        setSignTillEnd(_pTab.get());
    else if (command == TabCommand::Volume)
        ::changeTrackVolume(_pTab.get());
    else if (command == TabCommand::Name)
        ::changeTrackName(_pTab.get());
    else if (command == TabCommand::AddMarker)
        setMarker(_pTab.get());
    else if (command == TabCommand::BPM)
        changeTrackBpm(_pTab.get());
    else if (command == TabCommand::NewTrack) {
       _pTab->createNewTrack(); refreshTabStats(); } //Второе нужно для обновления
    else if (command == TabCommand::GotoBar)
        goToBar(_pTab.get());
    //if (press == "alt");//TODO
    else if (command == TabCommand::Tune)
        setTune(_pTab->at(_pTab->getCurrentTrack()).get());
    else if (command == TabCommand::CloseReprise)
        closeReprise(_pTab.get());
    else
        _pTab->onTabCommand(command);
}
