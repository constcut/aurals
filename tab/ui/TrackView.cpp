#include "TrackView.hpp"
#include "TabViews.hpp"

#include "tab/Track.hpp"

#include "app/log.hpp"
#include "app/Config.hpp"


using namespace aural_sight;


void TrackView::setFromTab(QObject* pa, int trackIdx) {
    _tabParrent = dynamic_cast<TabView*>(pa);
    auto trackPtr = _tabParrent->getTab()->operator[](trackIdx).get();

    _tabParrent->addTrackView(this);
    if (trackPtr != _pTrack) {
        _pTrack = trackPtr;
        imagePainted = false;
        update();
    }
}


void TrackView::launchThread()
{
    if (_animationThread)
        _animationThread->start();
}

void TrackView::stopThread()
{
    if (_animationThread) {
        _animationThread->requestStop();
        //localThr->wait();
    }
}

TrackView::~TrackView() {
    if (_animationThread) {
        _animationThread->requestStop();
        _animationThread->wait();
    }
    if (_tabParrent) {
        _tabParrent->removeTrackView(this);
    }
}

/*
void TrackView::connectThreadSignal(MasterView *masterView) {
    masterView->connectThread(_animationThread);
}
*/

void TrackView::ongesture(int offset, bool horizontal)
{
    if (horizontal){
        //x offset
        /* forbiden for a while
        int restOffset = offset;
        while (restOffset != 0)
        restOffset = horizonMove(restOffset);
        */
        //there could be selection for
    }
    else{
        size_t& cursor = _pTrack->cursor();
        size_t& displayIndex = _pTrack->displayIndex();
        //y offset
        if (offset < 0) {
            /*
            int quant = offset/-80;
                displayIndex = cursor += quant;
            cursorBeat=0;
            if (displayIndex > pTrack->size())
                cursor = displayIndex = pTrack->size()-1;
            */

            int absOffset = -1*offset;
            int shiftTo = 0;
            int curY = _barsPull[0].getY();

            while (absOffset>0) {
                int barY = _barsPull[shiftTo].getY();
                if (barY > curY) {
                    absOffset -= (barY-curY);
                    curY=barY;
                    if (absOffset < _barsPull[0].getH()) {
                        //--shiftTo;
                        break;
                    }
                }
                ++shiftTo;
            }


            size_t trackLen = _pTrack->size();
            displayIndex = cursor  +=shiftTo;

            if (trackLen <= displayIndex){

                displayIndex = trackLen;
                if (trackLen)
                    --displayIndex;

                cursor=displayIndex;
            }
            qDebug()<<"Shifting to "<<shiftTo;
        }
        else {
            size_t& cursorBeat = _pTrack->cursorBeat();
            size_t quant = offset/80;
            if (cursor > quant)
                displayIndex = cursor -= quant;
            else
                displayIndex = cursor = 0;
            cursorBeat=0;

        }
        _tabParrent->setCurrentBar(cursor);
        //verticalMove - same way but skips whole line - height is always the same
    }
}

int TrackView::horizonMove(int offset)
{
    //index 0 only for first iteration

    BarView *bV = &(_barsPull.at(0));

    int absOffset = offset > 0? offset: offset*-1;
    int rest = offset;

    if (absOffset > bV->getW()){

        size_t& displayIndex = _pTrack->displayIndex();
        size_t& lastSeen = _pTrack->lastSeen();

            if (offset > 0){
                if ((displayIndex+1) < (lastSeen-1))
                    ++displayIndex;
                rest -= bV->getW();
            }
            else {
                if (displayIndex > 0)
                    --displayIndex;
                rest += bV->getW();
            }
       return rest;
    }

    return 0;
}

void TrackView::onclick(int x1, int y1)
{
    if (_tabParrent == nullptr)
        return;

    if (_tabParrent->getPlaying())
        return; //skip

    size_t& cursor = _pTrack->cursor();
    size_t& cursorBeat = _pTrack->cursorBeat();
    size_t& stringCursor = _pTrack->stringCursor();
    size_t& displayIndex = _pTrack->displayIndex();

    //touch and mouse events on first note
    for (size_t i = 0; i < _barsPull.size(); ++i)
    {
        /*
        log << "Bar "<<i<<" "<<barsPull.getV(i).getX()<<
               " "<<barsPull.getV(i).getY()<<" "<<
               " "<<barsPull.getV(i).getW()<<" "<<
               " "<<barsPull.getV(i).getH()<<"; hits-"<<
               (int)(barsPull.getV(i).hit(x1,y1));
               */

        if (_barsPull.at(i).hit(x1,y1))
        {
            BarView *bV = &(_barsPull.at(i)); //it must be that way i know it
            //may be refact to make Poly<BarView>

            size_t realIndex = i + displayIndex;
            //set cursor from press
            cursor = realIndex;

            int beatClick = bV->getClickBeat(x1);
            int stringClick = bV->getClickString(y1);

            int stringUpperBarrier = _pTrack->getTuning().getStringsAmount();

            //++beatClick;
            if (beatClick >= _pTrack->at(cursor)->size())
                --beatClick;

            if (beatClick>0)
                cursorBeat = beatClick;
            else
                cursorBeat = 0;

            if ((stringClick >= 0) && (stringClick < stringUpperBarrier))
                stringCursor = stringClick;

            _pTrack->digitPress() = -1;

            _tabParrent->setCurrentBar(cursor);
            //getMaster()->pleaseRepaint();
        }
    }


    update();
    //log << "Press "<<x1<<" "<<y1;
}

void TrackView::ondblclick(int x1, int y1)
{
    if (_tabParrent == nullptr)
        return;

    bool wasPressed = false;
    for (size_t i = 0; i < _barsPull.size(); ++i)
    {
        /*
        log << "Bar "<<i<<" "<<barsPull.getV(i).getX()<<
               " "<<barsPull.getV(i).getY()<<" "<<
               " "<<barsPull.getV(i).getW()<<" "<<
               " "<<barsPull.getV(i).getH()<<"; hits-"<<
               (int)(barsPull.getV(i).hit(x1,y1));
               */

        if (_barsPull.at(i).hit(x1,y1))
        {
            BarView *bar = &(_barsPull.at(i));
            BarView *bV = bar; //(dynamic_cast<BarView*>(bar)); //it must be that way i know it
            //may be refact to make Poly<BarView>

            if (bV == 0) continue;

            int beatClick = bV->getClickBeat(x1);
            //Bar *hitBar = bV->getBar();
            int fullBar = bV->getBarsize();


            if (fullBar <= beatClick)
                continue;

            qDebug() << "Bar hits "<<beatClick<<" of "<<fullBar;

            size_t& displayIndex = _pTrack->displayIndex();
            int& selectionBeatFirst = _pTrack->selectBeatFirst();
            int& selectionBeatLast = _pTrack->selectBeatLast();
            int& selectionBarFirst = _pTrack->selectBarFirst();
            int& selectionBarLast = _pTrack->selectBarLast();

            if (selectionBeatFirst == -1) {
                selectionBeatFirst = selectionBeatLast =  bV->getClickBeat(x1);
                selectionBarFirst = selectionBarLast = i+displayIndex;
            }
            else
            {
                if (i + displayIndex > selectionBarLast)
                {
                    selectionBeatLast =  bV->getClickBeat(x1);
                    selectionBarLast = i + displayIndex;
                }
                else
                if (i + displayIndex < selectionBarFirst)
                {
                    selectionBeatFirst =   bV->getClickBeat(x1);
                    selectionBarFirst = i + displayIndex;
                }
                else
                {
                    if (selectionBarFirst == selectionBarLast)
                    {
                        int addBeat = bV->getClickBeat(x1);
                        if (addBeat > selectionBeatLast)
                            selectionBeatLast = addBeat;
                        if (addBeat < selectionBarFirst)
                            selectionBeatFirst = addBeat;
                    }
                    else
                    {
                        int addBeat = bV->getClickBeat(x1);
                        //if (addBeat > selectionBeatLast)
                        if (i + displayIndex == selectionBarLast)
                        {
                            selectionBeatLast = addBeat;
                        }
                        else //if (addBeat < selectionBeatfirstt)
                        if (i + displayIndex == selectionBarFirst)
                        {
                            selectionBeatFirst = addBeat;
                        }
                        else
                        {
                            if (i + displayIndex ==selectionBarLast-1)
                            {
                                //pre last bar
                                if (addBeat==_pTrack->at(i+displayIndex)->size()-1)
                                {
                                    //its last beat
                                    if (selectionBeatLast == 0)
                                    {
                                        //and current beat is irst in last bar
                                        selectionBeatLast = addBeat;
                                        --selectionBarLast;
                                    }
                                }
                            }

                            if ( i+ displayIndex == selectionBarFirst + 1)
                            {
                                //pre last bar
                                if (addBeat==0)
                                {
                                    //its last beat
                                    if (selectionBeatFirst == _pTrack->at(i + displayIndex-1)->size() - 1)
                                    {
                                        //and current beat is irst in last bar
                                        selectionBeatFirst = 0;
                                        ++selectionBarFirst;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            wasPressed = true;
        }
    }

    if (wasPressed == false) {
        int& selectionBeatFirst = _pTrack->selectBeatFirst();
        int& selectionBeatLast = _pTrack->selectBeatLast();
        int& selectionBarFirst = _pTrack->selectBarFirst();
        int& selectionBarLast = _pTrack->selectBarLast();
        selectionBeatFirst = selectionBeatLast =  -1;
        selectionBarFirst = selectionBarLast = -1;

    }
    update();
}

void TrackView::setDisplayBar(int barPosition)
{
    size_t& cursor = _pTrack->cursor();
    size_t& cursorBeat = _pTrack->cursorBeat();
    size_t& displayIndex = _pTrack->displayIndex();

    displayIndex = barPosition;
    cursor = displayIndex;
    _tabParrent->setCurrentBar(cursor);
    cursorBeat = 0;
}


void TrackView::paint(QPainter *painter)
{
    if (_pTrack == nullptr)
        return;

    if (lastWidth != width() || lastHeight != height())
        imagePainted = false;

    lastWidth = width();
    lastHeight = height();

    size_t trackLen = _pTrack->size(); //Earlier there was only track 1
    int stringsN = _pTrack->getTuning().getStringsAmount();

    int xSh = 0;
    int ySh = 0;
    BarView forH(_pTrack->at(0).get(), stringsN, 0); //Opt
    int hLimit = height() - forH.getH(); //Limit to avoid painting bar in bottom

    size_t& cursor = _pTrack->cursor();
    size_t& cursorBeat = _pTrack->cursorBeat();
    size_t& stringCursor = _pTrack->stringCursor();

    size_t& lastSeen = _pTrack->lastSeen(); //Probably remove if QImage doesn't lag
    size_t& displayIndex = _pTrack->displayIndex(); //Probably remove if QImage doesn't lag TODO

    size_t displayIdxOnStart = displayIndex;

    if (cursor < displayIndex)
        displayIndex = cursor;
    if (cursor > (lastSeen - 1))
        displayIndex = cursor;

    if (displayIdxOnStart != displayIndex)
        imagePainted = false;

    qDebug() << "Painted " << imagePainted << " " << displayIndex << " " << lastSeen;
    qDebug() << width() << " " << height();

    if (imagePainted == false) {

        _prepared = QImage(width(), height(), QImage::Format_ARGB32);

        QPainter imgPainter(&_prepared);
        imgPainter.fillRect(0, 0, width(), height(), QBrush(QColor(Qt::white)));

        //TODO move into subfunction:
        _barsPull.clear(); //not always - to optimize

        std::uint8_t lastNum = 0;
        std::uint8_t lastDen = 0;

        changeColor(CONF_PARAM("colors.default"), &imgPainter);

        for (size_t i = displayIndex; i < trackLen; ++i)
        {
            auto& curBar = _pTrack->at(i);

            std::uint8_t curNum = curBar->getSignNum();
            std::uint8_t curDen = curBar->getSignDenum();
            bool sameSign = true;

            static bool alwaysShowSign = CONF_PARAM("TrackView.alwaysShowBarSign") == "1";

            if (alwaysShowSign)
                sameSign = false;

            if ((curNum != lastNum) ||(curDen != lastDen)) {
                sameSign = false;
                lastNum = curNum;
                lastDen = curDen;
            }

            BarView bView(curBar.get(),stringsN,i);
            bView.setSameSign(sameSign);

            if (ySh <= (hLimit))
                lastSeen = i;

            int xShNEXT = xSh + bView.getW()+15;
            int border = width();

            if (xShNEXT > border) {
                xSh = 0;
                ySh += bView.getH();

                if (ySh >= (hLimit + 480)) //Check constant
                    break;; //stop that (there was a pannel)
            }

            bView.setShifts(xSh,ySh);
            std::uint8_t barCompleteStatus = curBar->getCompleteStatus(); //TODO avoid recalculations

            if (barCompleteStatus == 2 || barCompleteStatus == 1)
                changeColor(CONF_PARAM("colors.exceed"), &imgPainter);

            bView.paint(&imgPainter);

            if (barCompleteStatus == 2 || barCompleteStatus == 1)
                 changeColor(CONF_PARAM("colors.default"), &imgPainter);

            xSh += bView.getW();
            _barsPull.push_back(bView);
            //TODO maybe we have to use some limit, for supper long tabs
        }

        ++lastSeen;
        //TODO move into subfunction ^
    }

    painter->drawImage(QPoint{0,0}, _prepared);

    BarView& bView = _barsPull[cursor];

    changeColor(CONF_PARAM("colors.curBar"), painter);
    bView.setCursor(cursorBeat, stringCursor + 1);
    bView.paint(painter);
    changeColor(CONF_PARAM("colors.default"), painter);

}



int TrackView::getPixelHeight() {

    size_t trackLen = _pTrack->size();
    int stringsN = _pTrack->getTuning().getStringsAmount();

    int xSh = 0;
    int ySh = 0;
    int border = width();
    int lastHeight = 0;

    for (size_t i = 0; i < trackLen; ++i) //trackLen
    {
        auto& curBar = _pTrack->at(i);
        BarView bView(curBar.get(), stringsN,i);

        int xShNEXT = xSh + bView.getW() + 15; //TODO all constants configurable

        if (xShNEXT > border) {
            xSh = 0;
            ySh += bView.getH();
        }

        bView.setShifts(xSh, ySh);
        xSh += bView.getW();
        lastHeight = bView.getH();
    }

    return ySh + lastHeight;
}



void TrackView::prepareThread(int shiftTheCursor)
{
    //prepare for the animation

    if (_animationThread) {
        _animationThread->requestStop();
        //localThr->wait();
        _finishPool.push_back(std::move(_animationThread)); //Clear a pool
    }

    _animationThread = std::make_unique<ThreadLocal>();

    size_t& cursor = _pTrack->cursor();
    size_t& cursorBeat = _pTrack->cursorBeat();

    _animationThread->setInc(&cursor,&cursorBeat);
    _animationThread->setupValues(_tabParrent->getTab().get(),_pTrack,shiftTheCursor);

    this->connect(
        _animationThread.get(),
        SIGNAL(updateUI()), //+Now finished
        SLOT(update()),
        Qt::QueuedConnection);
}

bool TrackView::gotChanges() const
{
    if (_pTrack->commandSequence.size())
        return true;
    return false;
}




int TrackView::getInstrumet() {
    if (_pTrack != nullptr)
        return _pTrack->getInstrument();
    return 0;
}

void TrackView::setInstrument(int newInstr) {
    if (_pTrack != nullptr) {
        _pTrack->setInstrument(newInstr);
        _tabParrent->update();
    }
}

int TrackView::getVolume() {
    if (_pTrack != nullptr)
        return _pTrack->getVolume();
    return 0;
}

void TrackView::setVolume(int newVol) {
    if (_pTrack != nullptr) {
        _pTrack->setVolume(newVol);
        _tabParrent->update();
    }
}

int TrackView::getPanoram() {
    if (_pTrack != nullptr)
        return _pTrack->getPan();
    return 0;
}

void TrackView::setPanoram(int newPan) {
    if (_pTrack != nullptr) {
        _pTrack->setPan(newPan);
        _tabParrent->update();
    }

}

int TrackView::getStatus() {
    if (_pTrack != nullptr)
        return _pTrack->getStatus();
    return 0;
}

void TrackView::setStatus(int newStatus) { //0 - none, 1 - mute, 2 - solo
    if (_pTrack != nullptr) {
        _pTrack->setStatus(newStatus);
        _tabParrent->update();
    }
}

QString TrackView::getName() {
    if (_pTrack != nullptr)
        return _pTrack->getName().c_str();
    return "";
}

void TrackView::setName(QString newName) {
     if (_pTrack != nullptr) {
         auto name = newName.toStdString();
         _pTrack->setName(name);
         _tabParrent->update();
     }
}
