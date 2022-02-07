#include "TabViews.hpp"

#include "tab/Tab.hpp"
#include "midi/MidiFile.hpp"
#include "app/Config.hpp"
#include "tab/tools/GmyFile.hpp"
#include "tab/tools/TabLoader.hpp"
#include "tab/tools/MidiExport.hpp"

#include "tab/tools/Threads.hpp"

#include <QInputDialog>
#include <QMessageBox>

#include <QDebug>


using namespace aural_sight;


int aural_sight::translateDefaulColor(const std::string& confParam){
    int numColor = -1;
    if (confParam == "black") numColor = 0;
    if (confParam == "red") numColor = 1;
    if (confParam == "yellow") numColor = 2;
    if (confParam == "orange") numColor = 3;
    if (confParam == "green") numColor = 4;
    if (confParam == "blue") numColor = 5;
    if (confParam == "darkblue") numColor = 6;
    if (confParam == "violet") numColor = 7;
    if (confParam == "white") numColor = 8;
    if (confParam == "gray") numColor = 9;
    if (confParam == "lightgray") numColor = 10;
    if (confParam == "darkgray") numColor = 11;
    if (confParam == "cyan") numColor = 12;
    if (confParam == "mageta") numColor = 13;
    if (confParam == "darkgreen") numColor = 14;
    if (confParam == "darkcyan") numColor = 15;
    if (confParam == "darkred") numColor = 16;
    return numColor;
}


void aural_sight::changeColor(const std::string& color, QPainter* src)
{
    int colorS = aural_sight::translateDefaulColor(color);
    switch (colorS)
    {
        case 0: src->setPen(Qt::black); break;
        case 1: src->setPen(Qt::red); break;
        case 2: src->setPen(Qt::yellow); break; //orange
        case 3: src->setPen(Qt::darkYellow); break;
        case 4: src->setPen(Qt::green); break;
        case 5: src->setPen(Qt::blue); break;
        case 6: src->setPen(Qt::darkBlue); break;
        case 7: src->setPen(Qt::magenta); break; //violet
        case 8: src->setPen(Qt::white); break;
        case 9: src->setPen(Qt::gray); break;
        case 10: src->setPen(Qt::lightGray); break;
        case 11: src->setPen(Qt::darkGray); break;
        case 12: src->setPen(Qt::cyan); break;
        case 13: src->setPen(Qt::darkMagenta); break;
        case 14: src->setPen(Qt::darkGreen); break;
        case 15: src->setPen(Qt::darkCyan); break;
        case 16: src->setPen(Qt::darkRed); break;

    }
}


void aural_sight::drawEllipse(QColor c, QPainter *painter, int x, int y, int w, int h) {
    QBrush startBra = painter->brush();
    painter->setBrush(c);
    painter->drawEllipse(x,y,w,h);
    painter->setBrush(startBra);
}



void aural_sight::drawEllipse(QPainter *painter, int x, int y, int w, int h) {
    painter->drawEllipse(x,y,w,h);
}



void TabView::addTrackView(TrackView* trackView) {
    auto it = std::find(_tracksView.begin(), _tracksView.end(), trackView);
    if (it == _tracksView.end())
        _tracksView.push_back(trackView);
}


void TabView::removeTrackView(TrackView* trackView) {
    auto it = std::find(_tracksView.begin(), _tracksView.end(), trackView);
    if (it != _tracksView.end())
        _tracksView.erase(it);
}


void TabView::loadTab(QString filename) {
    GTabLoader loader;
    loader.open(filename.toStdString());
    _pTab = std::move(loader.getTab());
    refreshTabStats();
    update();
}


void TabView::exportMidi(QString filename, int shift) {
    auto m = ::exportMidi(_pTab.get(), shift); //TODO position
    m->writeToFile(filename.toStdString());
}


void TabView::refreshTabStats() {
    stopAllThreads();
    setPlaying(false);

    _tracksView.clear();

    if (_pTab) {
        _pTab->getDisplayBar() = 0;
        _pTab->getCurrentTrack() = 0;
        _pTab->getCurrentBar() = 0;
    }
}



void TabView::paint(QPainter *painter)
{
    //statusLabel->draw(painter);
    //bpmLabel->draw(painter);

   if (_pTab != 0)
    {
        if (_pTab->playing())
            _pTab->getDisplayBar()=_pTab->getCurrentBar();

        int yLimit = height();//getMaster()->getHeight();
        int xLimit = width(); //getMaster()->getWidth();

        changeColor(CONF_PARAM("colors.default"), painter);

        //TrackView should be agregated
        for (size_t i = 0 ; i < _pTab->size(); ++i)
        {
            if (i >= _pTab->size())
                break;

            std::string trackVal = std::to_string(i+1) +" " + _pTab->at(i)->getName();

            int yPos = (i+1) * 30; //pannelShift+(i+2)*30;
            if (yPos > yLimit - 100)
               break;

            if (i == _pTab->getCurrentTrack())
                aural_sight::changeColor(CONF_PARAM("colors.curTrack"), painter);

            painter->drawText(20, yPos,trackVal.c_str());
            painter->drawRect(7, yPos-10, 10, 10);

            if (i == _pTab->getCurrentTrack())
               aural_sight::changeColor(CONF_PARAM("colors.default"), painter);

            std::uint8_t trackStat = _pTab->at(i)->getStatus();
            if (trackStat==1)
            painter->drawText(9,yPos+3,"m");
            else
               if (trackStat==2)
                painter->drawText(9,yPos+3,"s");


           auto& tr = _pTab->at(i);
           for (size_t j = 0 ; j < tr->size(); ++j)
           {
               size_t barIndex = j + _pTab->getDisplayBar();
               if (barIndex >= tr->size())
                    break;

               std::string sX = std::to_string(barIndex+1);
               auto& cB= tr->at(barIndex);
               if (cB->size() == 1)
               {
                   auto& beat = cB->at(0);
                   Note *note = 0;
                   if (beat->size())
                        note = beat->at(0).get();
                   if (note == 0)
                      sX+="*";
               }

               int reprize = cB->getRepeat();
               auto [markerText, markerColor] = cB->getMarker();
               bool isMarkerHere = markerText.empty() == false;

                //hi light color bar
               if (barIndex == _pTab->getCurrentBar())
                   painter->fillRect(200+30*j,yPos,20,20,QColor(CONF_PARAM("colors.curBar").c_str()));

               painter->drawText(200+30*j,yPos+10,sX.c_str());
               if (i == 0)  {
                   if (reprize) {
                       std::string rep;
                       if (reprize == 1)
                           rep = "|:";
                       if (reprize == 2)
                       {
                           std::uint8_t repTimes = cB->getRepeatTimes();
                           if (repTimes != 2)
                               rep += std::to_string(repTimes);

                           rep += ":|";
                       }
                       if (reprize == 3)
                           rep = ":|:";

                       painter->drawText(200+30*j,80-60,rep.c_str());
                   }

                   if (isMarkerHere){
                        painter->drawText(200+30*j,70-60,markerText.c_str());
                   }
               }
               painter->drawRect(200 + 30 * j, yPos, 20, 20);
               if ((200 + j * 30) > (xLimit - 100)) //800 border
                    break;
           }

           std::string sX = std::to_string(tr->size());
           int border = width() - 20;
           painter->drawText(border,10+yPos,sX.c_str());

           sX.clear();
           std::uint8_t vol =  tr->getVolume();//pTab->GpCompMidiChannels[port*chan].volume; //tr->getVolume();
           sX = "vol " + std::to_string(vol);
           painter->drawText(70,10+yPos,sX.c_str());

           sX.clear();
           std::uint8_t pan =  tr->getPan();//pTab->GpCompMidiChannels[port*chan].balance;//tr->getPan();
           int intPan = pan - 7;
           sX = "pan " + std::to_string(intPan);
           painter->drawText(110,10+yPos,sX.c_str());

           sX.clear();
           std::uint8_t ins =  tr->getInstrument();//pTab->GpCompMidiChannels[port*chan].instrument;//tr->getPan();

           if (tr->isDrums() == false)
            sX = std::to_string(ins) + "i";
           else
               sX ="d" + std::to_string(ins);

           painter->drawText(170,10+yPos,sX.c_str()); //mute or solo
        }

        /*
        _tracksView[0]->setWidth(width());
        _tracksView[0]->setHeight(height());
        _tracksView[0]->paint(painter); */ //MODE show together - better use 2 qml components
    }
}

int TabView::getTimeLineBar() {

    size_t current = _pTab->getCurrentBar();
    auto& pTrack = _pTab->at(0);

    if (current != 0)
    {
        Bar *barPtr = pTrack->at(current).get();
        const auto& timeLoop = pTrack->getTimeLoop();
        for (size_t i = 0; i < timeLoop.size();++i) {
             if (timeLoop.at(i) == barPtr)
                 return i;
        }
    }

    return 0;
}


void TabView::prepareAllThreads(int shiftTheCursor)
{
    for (size_t i = 0; i <_tracksView.size(); ++i)
    {
        _tracksView[i]->prepareThread(shiftTheCursor);
        int thrLenSeconds = _tracksView[i]->threadSeconds();
        qDebug() << "Thread "<<i<<" seconds "<<thrLenSeconds;
    }

    auto& pTrack = _pTab->at(0);

    if (_localThr) {
        _localThr->requestStop();
        //localThr->wait();
        _finishPool.push_back(std::move(_localThr)); //Consumes memory TODO checker to delete finished
    }

    _localThr = std::make_unique<ThreadLocal>();
    _localThr->setInc(&_pTab->getCurrentBar(), nullptr); //oh shhhi 2nd arg
    _localThr->setBPM(_pTab->getBPM());

    const auto& timeLoop = pTrack->getTimeLoop();
    size_t timeLoopLen = timeLoop.size();
    for (size_t i = shiftTheCursor; i < timeLoopLen;++i) {
        _localThr->addNumDenum(timeLoop.at(i)->getSignNum(),
        timeLoop.at(i)->getSignDenum(), pTrack->getTimeLoopIndexes()[i]);
    }
    _localThr->setLimit(timeLoop.size());

    qDebug() << "All threads prepared";

    this->connect(
        _localThr.get(),
        SIGNAL(updateUI()), //+Now finished
        SLOT(update()),
        Qt::QueuedConnection);
}

void TabView::launchAllThreads()
{

    for (size_t i = 0; i <_tracksView.size(); ++i)
        _tracksView[i]->launchThread();

    //return;

    if (_localThr)
        _localThr->start();

    setPlaying(true);
}

void TabView::stopAllThreads()
{
    setPlaying(false);

    for (size_t i = 0; i <_tracksView.size(); ++i)
        _tracksView[i]->stopThread();

    if (_localThr) {
        _localThr->requestStop();
        //localThr->wait();
    }
}


TabView::~TabView() {
    if (_localThr) {
        _localThr->requestStop();
        _localThr->wait();
    }
}


/*
void TabView::connectAllThreadsSignal(MasterView *masterView)
{
    for (size_t i = 0; i <_tracksView.size(); ++i)
        _tracksView[i]->connectThreadSignal(masterView);

   // masterView->connectThread(localThr);

   masterView->connectMainThread(_localThr);
}*/



// ------------OVER the changes--------------------


bool TabView::gotChanges() const
{
    for (size_t i = 0; i < _tracksView.size(); ++i)
        if (_tracksView[i]->gotChanges())
        {
            //REQUEST DIALOG - if no then return true

            QMessageBox dialog("Escape changes?","Leave modification not saved?",
                               QMessageBox::Information,
                               QMessageBox::Yes,
                               QMessageBox::No,
                               QMessageBox::Cancel |QMessageBox::Escape);
            //dialog.set
            if (dialog.exec() == QMessageBox::Yes)
                return false; //escape

            return true;
        }

    return false;
}


bool TabView::getPlaying()
{
    if (_pTab->playing())
        if (_localThr)
            if (_localThr->getStatus())
                setPlaying(false);

    return _pTab->playing();
}
