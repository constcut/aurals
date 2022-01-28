#include "TabViews.hpp"

#include "tab/Tab.hpp"
#include "midi/MidiFile.hpp"
#include "app/Config.hpp"
#include "tab/tools/GmyFile.hpp"
#include "tab/tools/TabLoader.hpp"

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
    //saveColor();
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
    //storeValue = colorS;
}

void aural_sight::drawEllipse(QColor c, QPainter *painter, int x, int y, int w, int h) {
    QBrush startBra = painter->brush();
    painter->setBrush(c);
    painter->drawEllipse(x,y,w,h);
    painter->setBrush(startBra); //return brach back
}


void aural_sight::drawEllipse(QPainter *painter, int x, int y, int w, int h) {
    painter->drawEllipse(x,y,w,h);
}



int scaleCoef = 1;


//Tab view
TabView::TabView()
{
    //_statusLabel = std::make_unique<GLabel>(50,460,"file was loaded.");
    //_bpmLabel = std::make_unique<GLabel>(300,460,"bpm=notsetyet");
}

void TabView::addSingleTrack(Track *track)
{
    auto tV = std::make_unique<TrackView>(track);
    tV->setPa(this);

    _tracksView.push_back(std::move(tV));
}

void TabView::setUI()
{
    //if (getMaster())
        //getMaster()->SetButton(0,"back to track",630,20,100,15,"opentrack");
}

void TabView::setTab(std::unique_ptr<Tab> point2Tab) {
    _pTab = std::move(point2Tab);
    refreshTabStats();
}


void TabView::loadTab() {
    GTabLoader loader;
    loader.open("tests/1.1.gp4");
    _pTab = std::move(loader.getTab());
    update();
    qDebug() << "Tab loaded";
}


void TabView::refreshTabStats() {
    stopAllThreads();
    setPlaying(false);

    _tracksView.clear();

    if (_pTab) {
        _pTab->getDisplayBar() = 0;
        _pTab->getDisplayTrack() = 0;
        _pTab->getCurrentTrack() = 0;
        _pTab->getCurrentBar() = 0;
        //_bpmLabel->setText("bpm=" + std::to_string(_pTab->getBPM()));
        for (size_t i = 0; i < _pTab->size(); ++i)
            addSingleTrack(_pTab->at(i).get());
        std::string statusBar1,statusBar2;
        statusBar1 = "Tab Name";
        statusBar2 = "BPM = " + std::to_string(_pTab->getBPM());
        //getMaster()->setStatusBarMessage(1,statusBar1.c_str());
        //getMaster()->setStatusBarMessage(2,statusBar2.c_str());
        //getMaster()->setStatusBarMessage(0,"Tab was loaded",500);
    }
}


void TabView::onclick(int x1, int y1)
{
    if (_pTab==0)
        return;


    int awaitBar = (x1-200)/30; //TODO size_t и дополнительные рассчёты, т.к. -1 всё равно не вариант
    int toolBarHeight = 10; //getMaster()->getToolBarHeight();
    int awaitTrack = (y1-toolBarHeight)/30;
    awaitTrack-=1;

    if (awaitBar >= 0){
        awaitBar += _pTab->getDisplayBar();
        if (awaitBar == _pTab->getCurrentBar()){
            qDebug() << "Track pressed "<<awaitTrack<<"; Bar "<<awaitBar;
            if (awaitTrack >= 0){
                if (awaitTrack < _pTab->size()) {
                    auto& trackView = _tracksView[awaitTrack];
                    _pTab->getLastOpenedTrack() = awaitTrack;
                    trackView->setDisplayBar(awaitBar);

                    /*
                    MainView *mainView = (MainView*)getMaster()->getFirstChild();
                    mainView->changeCurrentView(trackView.get());
                    std::string statusBar1,statusBar2;
                    statusBar1 = _pTab->at(awaitTrack)->getName();
                    statusBar2 = "bar " + std::to_string( _pTab->getCurrentBar() );
                    getMaster()->setStatusBarMessage(1,statusBar1.c_str());
                    getMaster()->setStatusBarMessage(2,statusBar2.c_str());
                    */
                    //TODO throw signal to qml, and it can open track
                }
            }
        }
        else
            if (awaitBar < _pTab->at(0)->size()) {
                _pTab->getCurrentBar() = awaitBar;
                size_t chosenTrack = _pTab->getDisplayTrack() + awaitTrack;
                if ((awaitTrack>=0) && (chosenTrack < _pTab->size()))
                        _pTab->getCurrentTrack() = chosenTrack;
            }
    }
    else {
        ++awaitTrack;
        size_t chosenTrack = _pTab->getDisplayTrack() + awaitTrack;
        if ((awaitTrack>=0) && (chosenTrack < _pTab->size()))
                _pTab->getCurrentTrack() = chosenTrack;
    }
}


void TabView::ondblclick(int x1, int y1) {
   onclick(x1,y1);
}


void TabView::ongesture(int offset, bool horizontal) {
    int quant = offset/25;
    if (horizontal) {
        size_t nextCursor = quant + _pTab->getDisplayBar();
        if (nextCursor<0)
            nextCursor=0;
        else
            if (nextCursor > _pTab->at(0)->size())
                nextCursor = _pTab->at(0)->size()-1;
        _pTab->getDisplayBar() = nextCursor;
    }
    else {
        if (quant >= 2)
            keyevent("^^^"); //TODO with command
        else
            if (quant <= -2)
                keyevent("vvv");
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

        qDebug() << yLimit << " " << xLimit << " XY limits";

        //TrackView should be agregated
        for (size_t i = 0 ; i < _pTab->size(); ++i)
        {
           size_t trackIndex = i + _pTab->getDisplayTrack();
           if (trackIndex >= _pTab->size())
            break;
           std::string trackVal = std::to_string(trackIndex+1) +" " + _pTab->at(trackIndex)->getName();
           //int pannelShift = getMaster()->getToolBarHeight();
           int yPos = (i+1)*30; //pannelShift+(i+2)*30;
           if (yPos > (yLimit-100))
               break;
           if (trackIndex == _pTab->getCurrentTrack())
                aural_sight::changeColor(CONF_PARAM("colors.curTrack"), painter);

           painter->drawText(20,yPos,trackVal.c_str());
           //painter->drawEllipse(10,yPos,5,5);
           painter->drawRect(7,yPos-10,10,10);

           if (trackIndex == _pTab->getCurrentTrack())
               aural_sight::changeColor(CONF_PARAM("colors.default"), painter);

           std::uint8_t trackStat = _pTab->at(trackIndex)->getStatus();
           if (trackStat==1)
            painter->drawText(9,yPos+3,"m");
           else
               if (trackStat==2)
                painter->drawText(9,yPos+3,"s");


           auto& tr = _pTab->at(trackIndex);
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
    }
}


void TabView::prepareAllThreads(size_t shiftTheCursor)
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
}

void TabView::launchAllThreads()
{


    for (size_t i = 0; i <_tracksView.size(); ++i)
        _tracksView[i]->launchThread();

    //return;

    if (_localThr)
        _localThr->start();
}

void TabView::stopAllThreads()
{
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
