#include "TrackView.hpp"
#include "TabViews.hpp"

#include "tab/Track.hpp"

#include "log.hpp"
#include "app/Config.hpp"


using namespace aural_sight;


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



    //log << "Press "<<x1<<" "<<y1;
}

void TrackView::ondblclick(int x1, int y1)
{
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

void TrackView::setUI()
{
    /*
    if (_tabParrent->getMaster())
    {
        int centerX=0;

        std::string trackNames="";

        for (size_t i = 0; i < _tabParrent->getTab()->size(); ++i)
        {
          trackNames += _tabParrent->getTab()->at(i)->getName();
          trackNames += std::string(";");
        }

        //now-debug:here
        _tabParrent->getMaster()->setComboBox(0,trackNames,centerX+20,5,210,30,_tabParrent->getLastOpenedTrack());
        _tabParrent->getMaster()->setComboBox(1,"instruments",240+centerX,5,200,30,_pTrack->getInstrument());
        _tabParrent->getMaster()->setComboBox(2,"volume",450+centerX,5,50,30,_pTrack->getVolume());

        int butShift = 50;
        _tabParrent->getMaster()->SetButton(3,"open tab view",640+butShift,20,90,15,"tabview");
        _tabParrent->getMaster()->SetButton(4,"play",570+butShift,20,45,15,"playMidi");

        std::uint8_t soloMute = _pTrack->getStatus();
        _tabParrent->getMaster()->setComboBox(5,"mutesolo",510+centerX,5,50,30,soloMute);

        _tabParrent->getMaster()->setComboBox(6,"pan",570+centerX,5,50,30,_pTrack->getPan());
    }
    */
    //TODO just get rid, yet as a reminder, move all to qml part, make here setters

}

void TrackView::draw(QPainter *painter)
{
    Track *track1 = _pTrack;
    size_t trackLen = track1->size();
    int stringsN = track1->getTuning().getStringsAmount();

    int pannelShift = 10; //getMaster()->getToolBarHeight();
    //double scaleValue = AConfig::getInstance().getScaleCoef();
    //pannelShift /= scaleValue;

    int xSh=0;
    int ySh=pannelShift; //shift for the pannel
    ySh += 30;

    int hLimit = (height() - 50) / 100;
    hLimit *= 100;

    size_t& cursor = _pTrack->cursor();
    size_t& cursorBeat = _pTrack->cursorBeat();
    size_t& stringCursor = _pTrack->stringCursor();
    size_t& lastSeen = _pTrack->lastSeen();
    size_t& displayIndex = _pTrack->displayIndex();
    int& selectionBeatFirst = _pTrack->selectBeatFirst();
    int& selectionBeatLast = _pTrack->selectBeatLast();
    int& selectionBarFirst = _pTrack->selectBarFirst();
    int& selectionBarLast = _pTrack->selectBarLast();

    //to automate scroll
    if (cursor < displayIndex)
        displayIndex = cursor;

    if (cursor > (lastSeen-1))
        displayIndex = cursor;

    _barsPull.clear(); //not always - to optimize

    std::uint8_t lastNum = 0;
    std::uint8_t lastDen = 0;

    if (_pTrack->isDrums())
    {
        painter->drawText(220,55,"!Drum track!");
    }


    for (size_t i = displayIndex; i < trackLen; ++i) //trackLen
    {
        auto& curBar = track1->at(i);

        std::uint8_t curNum = curBar->getSignNum();
        std::uint8_t curDen = curBar->getSignDenum();
        bool sameSign = true;

        static bool alwaysShowSign = CONF_PARAM("TrackView.alwaysShowBarSign") == "1";

        if (alwaysShowSign)
            sameSign = false;

        if ((curNum != lastNum) ||(curDen != lastDen))
        {
                sameSign = false;
                lastNum = curNum;
                lastDen = curDen;
        }

        BarView bView(curBar.get(),stringsN,i);

        if (selectionBarFirst!=-1)
        {
            if ((i >= selectionBarFirst)&&
                (i <= selectionBarLast))
            {
                //if one - then same (first)
                //if last - then second (last)
                //if middle - -1(whole)
                if (selectionBarLast == selectionBarFirst)
                    bView.setSelectors(selectionBeatFirst,selectionBeatLast);
                else
                {
                    if (selectionBarFirst == i)
                        bView.setSelectors(selectionBeatFirst, -1);
                    else if (selectionBarLast == i)
                         bView.setSelectors(0,selectionBeatLast);
                    else
                        bView.setSelectors(0,-1); //in the middle
                }

            }
        }

        bView.setSameSign(sameSign);

        if (ySh <= (hLimit))
            lastSeen = i;

        int xShNEXT = xSh + bView.getW()+15;
        int border = width();

        if (xShNEXT > border) {
            xSh = 0;
            ySh += bView.getH(); // there was 100 hardcoded

            if (ySh >= (hLimit+480)){
                return; //stop that (there was a pannel)
            }
        }

        bView.setShifts(xSh,ySh);
        std::uint8_t barCompleteStatus = curBar->getCompleteStatus(); //avoid recalculations

        if ( i == cursor ) {
            changeColor(CONF_PARAM("colors.curBar"), painter);
            if (i==cursor)
                bView.setCursor(cursorBeat,stringCursor+1);
        }
        else {
            //refact add another color
            if ((barCompleteStatus==2)||(barCompleteStatus==1))
                changeColor(CONF_PARAM("colors.exceed"), painter);
        }


        bView.draw(painter);

        if (( i == cursor ) || (barCompleteStatus==2)||(barCompleteStatus==1))
             changeColor(CONF_PARAM("colors.default"), painter);


        xSh += bView.getW();

        //if (ySh <= (hLimit))
        {
            _barsPull.push_back(bView);
        }

        if (i == cursor)
        {
            //old cursor lines
            //painter->drawLine(xSh,ySh+20*track1->tuning.getStringsAmount(),xSh+bView.getW(),ySh+20*track1->tuning.getStringsAmount());
            //painter->drawLine(xSh+cursorBeat*12,5+ySh+20*track1->tuning.getStringsAmount(),xSh+cursorBeat*12+12,5+ySh+20*track1->tuning.getStringsAmount());
        }
    }

    //if (ySh <= (hLimit))
    {
        ++lastSeen;
    }

}

void TrackView::prepareThread(size_t shiftTheCursor)
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
}

bool TrackView::gotChanges() const
{
    if (_pTrack->commandSequence.size())
        return true;
    return false;
}
