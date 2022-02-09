#include "TrackView.hpp"
#include "TabViews.hpp"

#include "tab/Track.hpp"

#include "app/log.hpp"
#include "app/Config.hpp"


using namespace aurals;


void TrackView::setFromTab(QObject* pa, int trackIdx) {
    _tabParrent = dynamic_cast<TabView*>(pa);
    auto trackPtr = _tabParrent->getTab()->operator[](trackIdx).get();

    _tabParrent->addTrackView(this);
    if (trackPtr != _pTrack) {
        _pTrack = trackPtr;
        fillBarsPool();
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


void TrackView::onclick(int x1, int y1)
{
    if (_tabParrent == nullptr || _tabParrent->getPlaying())
        return;

    size_t& cursor = _pTrack->cursor();
    size_t& cursorBeat = _pTrack->cursorBeat();
    size_t& stringCursor = _pTrack->stringCursor();

    for (size_t i = 0; i < _barsPool.size(); ++i)
    {
        if (_barsPool.at(i).hit(x1,y1))
        {
            BarView *bV = &_barsPool.at(i);
            cursor = i;

            int beatClick = bV->getClickBeat(x1);
            const int stringClick = bV->getClickString(y1);
            const int stringUpperBarrier = _pTrack->getTuning().getStringsAmount();

            if (beatClick >= static_cast<int>(_pTrack->at(cursor)->size()))
                --beatClick;

            if (beatClick > 0)
                cursorBeat = beatClick;
            else
                cursorBeat = 0;

            if (stringClick >= 0 && stringClick < stringUpperBarrier)
                stringCursor = stringClick;

            _pTrack->digitPress() = -1;
            _tabParrent->setCurrentBar(cursor);
        }
    }
    update();
}




void TrackView::onSelection(int x1, int y1)
{
    if (_tabParrent == nullptr)
        return;

    bool wasPressed = false;
    for (size_t i = 0; i < _barsPool.size(); ++i)
    {

        if (_barsPool.at(i).hit(x1,y1))
        {
            BarView *bar = &(_barsPool.at(i));
            BarView *bV = bar;

            if (bV == 0) continue;

            int beatClick = bV->getClickBeat(x1);
            int fullBar = bV->getBar()->size();

            if (fullBar <= beatClick)
                continue;

            int& selectionBeatFirst = _pTrack->selectBeatFirst();
            int& selectionBeatLast = _pTrack->selectBeatLast();
            int& selectionBarFirst = _pTrack->selectBarFirst();
            int& selectionBarLast = _pTrack->selectBarLast();

            if (selectionBeatFirst == -1) {
                selectionBeatFirst = selectionBeatLast =  bV->getClickBeat(x1);
                selectionBarFirst = selectionBarLast = i;
            }
            else
            {
                if (i  > selectionBarLast)
                {
                    selectionBeatLast =  bV->getClickBeat(x1);
                    selectionBarLast = i;
                }
                else if (i < selectionBarFirst)
                {
                    selectionBeatFirst =   bV->getClickBeat(x1);
                    selectionBarFirst = i;
                }
                else
                {
                    if (selectionBarFirst == selectionBarLast)
                    {
                        int addBeat = bV->getClickBeat(x1);

                        if (addBeat > selectionBeatLast)
                            selectionBeatLast = addBeat;

                        if (addBeat < selectionBeatFirst)
                            selectionBeatFirst = addBeat;
                    }
                    else
                    {
                        int addBeat = bV->getClickBeat(x1);
                        //if (addBeat > selectionBeatLast)
                        if (i == selectionBarLast)
                        {
                            selectionBeatLast = addBeat;
                        }
                        else //if (addBeat < selectionBeatfirstt)
                        if (i == selectionBarFirst)
                        {
                            selectionBeatFirst = addBeat;
                        }
                        else
                        {
                            if (i == selectionBarLast - 1)
                            {
                                //pre last bar
                                if (addBeat == _pTrack->at(i)->size()-1)
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

                            if ( i == selectionBarFirst + 1)
                            {
                                //pre last bar
                                if (addBeat==0)
                                {
                                    //its last beat
                                    if (selectionBeatFirst == _pTrack->at(i - 1)->size() - 1)
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




void TrackView::fillBarsPool() {

    size_t trackLen = _pTrack->size();
    int stringsN = _pTrack->getTuning().getStringsAmount();

    std::uint8_t lastNum = 0;
    std::uint8_t lastDen = 0;

    int xPos = 0;
    int yPos = 0;

    std::vector<size_t> currentLine;

    _linesIdxs.clear();
    _barsPool.clear();

    for (size_t i = 0; i < trackLen; ++i)
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

        BarView bView(curBar.get(), stringsN,i);
        bView.setSameSign(sameSign);

        int xShNEXT = xPos + bView.getW() + 15;
        int border = width();


        if (xShNEXT > border) {
            xPos = 0;
            yPos += bView.getH();

            _linesIdxs.push_back(currentLine);
            currentLine.clear();
        }

        currentLine.push_back(i);

        bView.setShifts(xPos, yPos);

        xPos += bView.getW();
        _barsPool.push_back(bView);
    }

    if (currentLine.empty() == false)
        _linesIdxs.push_back(currentLine);

}


size_t TrackView::getLineOfBar(size_t idx) {
    size_t currentLine = 0;
    for (auto& line: _linesIdxs) {
        if (find(line.begin(), line.end(), idx) != line.end())
            break;
        ++currentLine;
    }
    return currentLine;
}


void TrackView::moveNextLine() {

    size_t& displayIndex = _pTrack->displayIndex();
    size_t& cursor = _pTrack->cursor();
    const size_t currentLine = getLineOfBar(displayIndex);

    if (currentLine < _linesIdxs.size() - 1) {
        displayIndex = _linesIdxs[currentLine + 1][0];
        if (cursor < displayIndex)
            cursor = displayIndex;
    }
}


void TrackView::movePrevLine() {

    size_t& displayIndex = _pTrack->displayIndex();
    size_t& cursor = _pTrack->cursor();

    const size_t currentLine = getLineOfBar(displayIndex);
    const size_t cursorLine = getLineOfBar(cursor);

    if (currentLine != 0)
    {
        displayIndex = _linesIdxs[currentLine - 1][0];

        const int diff = cursorLine - (currentLine - 1);
        const int possibleLines = height() / _barsPool[0].getH();

        if ( diff > possibleLines )
            cursor = displayIndex;
    }
}


void TrackView::applySelection(BarView& barView, int idx) {

    const int selectionBarFirst = _pTrack->selectBarFirst();

    if (selectionBarFirst == -1)
        return;

    const int selectionBeatFirst = _pTrack->selectBeatFirst();
    const int selectionBeatLast = _pTrack->selectBeatLast();
    const int selectionBarLast = _pTrack->selectBarLast();

    if (idx >= selectionBarFirst && idx <= selectionBarLast)
    {
        if (selectionBarLast == selectionBarFirst)
            barView.setSelectors(selectionBeatFirst,selectionBeatLast);
        else
        {
            if (selectionBarFirst == idx)
                barView.setSelectors(selectionBeatFirst, -1);
            else if (selectionBarLast == idx)
                 barView.setSelectors(0, selectionBeatLast);
            else
                barView.setSelectors(0, -1);
        }
    }

}

void TrackView::dropSelection() {
    _pTrack->selectBarFirst() = -1;
    _pTrack->selectBeatFirst() = -1;
    _pTrack->selectBeatLast() = -1;
    _pTrack->selectBarLast() = -1;
}


void TrackView::paintByLines(QPainter* painter) {

    if (_linesIdxs.empty())
        return;


    size_t& displayIndex = _pTrack->displayIndex();
    size_t& lastSeen = _pTrack->lastSeen();
    const size_t cursor = _pTrack->cursor();

    auto scrollLine = [&]() {
        for (auto& line: _linesIdxs)
            if (find(line.begin(), line.end(), cursor) != line.end()) {
                displayIndex = line[0];
                break;
            }
    };

    if (cursor < displayIndex)
        scrollLine();
    if (lastSeen && cursor > lastSeen)
        scrollLine();

    const size_t currentLine = getLineOfBar(displayIndex);

    for (size_t i = 0; i < _linesIdxs.size(); ++i)
    {
        int yPos = _barsPool[0].getH() * (i - currentLine);
        for (size_t barIdx: _linesIdxs[i])
            _barsPool[barIdx].setY(yPos + 20);
    }

    const int possibleLines = height() / _barsPool[0].getH();

    for (size_t i = currentLine; i < currentLine + possibleLines; ++i)
    {
        if (i == _linesIdxs.size())
            break;

        for (size_t barIdx: _linesIdxs[i])
        {
            if (cursor != barIdx)
            {
                auto& barView = _barsPool[barIdx];
                barView.flushCursor();
                barView.flushSelectors();

                applySelection(barView, barIdx);

                //TODO avoid recalculations считать единожды, обновлять при изменении
                std::uint8_t barCompleteStatus = _pTrack->at(barIdx)->getCompleteStatus();

                if (barCompleteStatus == 2 || barCompleteStatus == 1)
                    changeColor(CONF_PARAM("colors.exceed"), painter);

                barView.paint(painter);

                if (barCompleteStatus == 2 || barCompleteStatus == 1)
                     changeColor(CONF_PARAM("colors.default"), painter);

            }
            lastSeen = barIdx;
        }
    }
}


void TrackView::paint(QPainter *painter)
{
    if (_pTrack == nullptr)
        return;

    if (_lastWidth != width() || _lastHeight != height())
    {
        fillBarsPool();
        _lastWidth = width();
        _lastHeight = height();
    }


    auto f = painter->font();
    f.setPixelSize(14);
    painter->setFont(f);

    paintByLines(painter);

    size_t cursor = _pTrack->cursor();
    size_t cursorBeat = _pTrack->cursorBeat();
    int stringCursor = _pTrack->stringCursor();

    if (cursor < _barsPool.size())
    {
        BarView& bView = _barsPool[cursor];
        changeColor(CONF_PARAM("colors.curBar"), painter);

        if (_tabParrent->getPlaying())
            stringCursor = -1;

        bView.flushSelectors();
        applySelection(bView, cursor);

        bView.setCursor(cursorBeat, stringCursor + 1);
        bView.paint(painter);
        changeColor(CONF_PARAM("colors.default"), painter);
    }

}



int TrackView::getFullPixelHeight() {

    const size_t trackLen = _pTrack->size();
    const int stringsN = _pTrack->getTuning().getStringsAmount();
    const int widthBorder = width();

    int xPos = 0;
    int yPos = 0;
    int lastHeight = 0;

    for (size_t i = 0; i < trackLen; ++i)
    {
        auto& curBar = _pTrack->at(i);
        BarView bView(curBar.get(), stringsN,i);

        if (int nextX = xPos + bView.getW() + 15; nextX > widthBorder) { //ВСЕ КОНСТАНТЫ ОТРИСОВКИ, КАК ПАРАМЕТРЫ
            xPos = 0;
            yPos += bView.getH();
        }

        bView.setShifts(xPos, yPos);
        xPos += bView.getW();
        lastHeight = bView.getH();
    }

    return yPos + lastHeight;
}



void TrackView::prepareThread(int shiftTheCursor)
{
    if (_animationThread) {
        _animationThread->requestStop();
        _finishPool.push_back(std::move(_animationThread)); //Clear a pool
    }

    _animationThread = std::make_unique<ThreadLocal>();

    size_t& cursor = _pTrack->cursor();
    size_t& cursorBeat = _pTrack->cursorBeat();

    _animationThread->setInc(&cursor,&cursorBeat);
    _animationThread->setupValues(_tabParrent->getTab().get(), _pTrack, shiftTheCursor);

    this->connect(
        _animationThread.get(),
        SIGNAL(updateUI()),
        SLOT(update()),
        Qt::QueuedConnection);

    //TODO сигнал окончания анимации
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
