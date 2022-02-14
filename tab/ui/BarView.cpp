#include "BarView.hpp"
#include "TabViews.hpp"

#include "app/Config.hpp"

using namespace aurals;


///////////Bars------------------------------
void BarView::drawMidiNote(QPainter *painter, std::uint8_t noteDur, std::uint8_t dotted, std::uint8_t durDet, int midiNote,
                  int x1, int y1)
{
    int xPoint = x1;
    int yPoint = y1;

    int lineToStand = midiNote - 20; //40
    yPoint += (stringWidth-3)*11; //was *6
    yPoint -=2; //now set on 40

    int yNote = (stringWidth-3)/2;

    int position[] = {2,5,7,10,12,14,17,19, 22,24,26,29,31,34,36,38,41,43,46,48,
                     50,53,55,58,60,62,65,67,70,72,74,77,79,82,84,86};

    bool needSignSharp =false;
    for (size_t i = 0; i < sizeof(position)/sizeof(int); ++i)
        if (position[i]==lineToStand)
            needSignSharp = true;

    int diezCount = 0;

    for (size_t i = 0; i < sizeof(position)/sizeof(int); ++i)
    {
        int index = sizeof(position)/sizeof(int) - 1 - i;
        if (lineToStand >= position[index]) {
            --lineToStand;
            ++diezCount;
       }
    }


    lineToStand -= 12; //20 more notes were addded for bass
    yNote -= lineToStand*(stringWidth-3)/2;


    int mainRadius = 5; //all constants to config
    int radiusShift = 0;
    int note32 = 4;
    int note16 = 6;
    int note8 = 8;
    int note4 = 10;
    int noteTail = 6;
    int noteTailEnd = 10;

    if (CONF_PARAM("TrackView.largeNotes")=="1")
    {
        note4 *= 2;
        note8 *= 2;
        note16 *= 2;
        note32 *= 2;
        noteTailEnd += 3;
        yPoint += 10;
        mainRadius += 3;
        radiusShift = 2;
    }

    yNote -= radiusShift;

    if (CONF_PARAM("TrackView.largeNotes")=="1")
        drawEllipse(QColor("black"), painter, xPoint-radiusShift,yPoint-radiusShift+yNote,mainRadius,mainRadius);
    else
    {
        drawEllipse(painter, xPoint - radiusShift, yPoint - radiusShift + yNote, mainRadius, mainRadius);
        drawEllipse(painter, xPoint + 1, yPoint + 1 + yNote, 3, 3);
        drawEllipse(painter, xPoint + 2, yPoint + 2 + yNote, 2, 2);
    }

    if (needSignSharp)
    {
        painter->drawLine(xPoint - radiusShift - 5, yPoint - radiusShift + yNote,
                          xPoint - radiusShift, yPoint - radiusShift + yNote);

        painter->drawLine(xPoint - radiusShift - 5, yPoint - radiusShift + yNote - 2,
                          xPoint - radiusShift, yPoint - radiusShift + yNote - 2);

        painter->drawLine(xPoint - radiusShift - 2, yPoint - radiusShift + yNote - 3,
                          xPoint - radiusShift - 2, yPoint - radiusShift + yNote + 1);

        painter->drawLine(xPoint - radiusShift - 4, yPoint - radiusShift + yNote - 3,
                          xPoint - radiusShift - 4, yPoint - radiusShift + yNote + 1);
    }

    int noteUpsideDownCoef = 1;
    if (CONF_PARAM("upsideDownNotes") == "1")
        noteUpsideDownCoef = -1;

    if (dotted)
        painter->drawEllipse(xPoint + 7, yPoint + yNote, 2, 2); //check fine on large

    if (noteDur >= 1)
        painter->drawLine(xPoint + noteTail, yPoint + yNote, xPoint + noteTail,
                          yPoint - note4 * noteUpsideDownCoef + yNote);

    if (noteDur >= 3)
        painter->drawLine(xPoint + noteTail, yPoint - note4 * noteUpsideDownCoef + yNote,
                          xPoint + noteTailEnd, yPoint - note4 * noteUpsideDownCoef + yNote - 3);

    if (noteDur >= 4)
        painter->drawLine(xPoint + noteTail, yPoint - note8 * noteUpsideDownCoef + yNote,
                          xPoint + noteTailEnd, yPoint - note8 * noteUpsideDownCoef + yNote - 3);

    if (noteDur >= 5)
        painter->drawLine(xPoint + noteTail, yPoint - note16 * noteUpsideDownCoef + yNote,
                          xPoint + noteTailEnd, yPoint - note16 * noteUpsideDownCoef + yNote - 3);

    if (noteDur >= 6)
        painter->drawLine(xPoint+noteTail, yPoint - note32 * noteUpsideDownCoef + yNote,
                          xPoint+noteTailEnd, yPoint - note32 * noteUpsideDownCoef + yNote - 3);

    if (durDet)
        painter->drawText(xPoint + 6, yPoint + 5, std::to_string(durDet).c_str());


    int beginIndex = 0;
    int endIndex = lineToStand/2;
    beginIndex = lineToStand/2;

    int lowerestBorder =  0;

    if (lineToStand<5)
    {
        if (lineToStand < -6) {
            beginIndex = -6;
            int smoothIndex = -beginIndex - 2;
            yPoint += (stringWidth - 3)*smoothIndex;
        }
    }
    else if (lineToStand >= 16)
        beginIndex = 6;

    for (int i = lowerestBorder; i < beginIndex; ++i)
        yPoint -= stringWidth - 3;

    for (int i = beginIndex ; i <= endIndex; ++i)
    {
        int smallMove = -3;
        int decreaseElse = 0; //-3

        painter->drawLine(xPoint - radiusShift + smallMove, yPoint + radiusShift / 2,
                          xPoint + radiusShift + mainRadius / 2 + smallMove+decreaseElse, yPoint + radiusShift / 2);

        yPoint -= stringWidth-3;
    }
}


void BarView::drawNote(QPainter *painter, std::uint8_t noteDur, std::uint8_t dotted, std::uint8_t durDet,
                       int x1, int y1)
{
    int xPoint = x1;
    int yPoint = y1;

    int mainRadius = 5; //ВСЕ КОНСТАНТЫ ВЫНЕСТИ В КОНФИГУРАЦИЮ, ПОЗВОЛИТЬ НАСТРАИВАТЬ refactoring
    int radiusShift = 0;
    int note32 = 4;
    int note16 = 6;
    int note8 = 8;
    int note4 = 10;
    int noteTail = 6;
    int noteTailEnd = 10;

    if (CONF_PARAM("TrackView.largeNotes") == "1")
    {
        note4 *= 2;
        note8 *= 2;
        note16 *= 2;
        note32 *= 2;
        noteTailEnd += 3;
        yPoint += 10;
        mainRadius += 3;
        radiusShift = 2;
    }

    int notesUpsideDownCoef = 1;

    if (CONF_PARAM("upsideDownNotes") == "1")
        notesUpsideDownCoef = -1;

    if (noteDur >= 2)
    {
        if (CONF_PARAM("TrackView.largeNotes") == "1")
            drawEllipse(QColor("black"), painter, xPoint - radiusShift, yPoint - radiusShift, mainRadius, mainRadius);
        else
        {
            painter->drawEllipse(xPoint - radiusShift, yPoint - radiusShift, mainRadius, mainRadius);
            painter->drawEllipse(xPoint + 1, yPoint + 1, 3, 3);
            painter->drawEllipse(xPoint + 2, yPoint + 2, 2, 2);
        }
    }
    else
        painter->drawEllipse(xPoint - radiusShift, yPoint - radiusShift, mainRadius, mainRadius);


    if (dotted)
        painter->drawEllipse(xPoint + 7, yPoint, 2, 2);

    if (noteDur >= 1)
        painter->drawLine(xPoint + noteTail, yPoint, xPoint + noteTail, yPoint - note4 * notesUpsideDownCoef);

    if (noteDur >= 3)
        painter->drawLine(xPoint + noteTail, yPoint - note4 * notesUpsideDownCoef,
                          xPoint + noteTailEnd, yPoint - note4 * notesUpsideDownCoef);
    if (noteDur >= 4)
        painter->drawLine(xPoint + noteTail, yPoint - note8 * notesUpsideDownCoef,
                          xPoint + noteTailEnd, yPoint - note8 * notesUpsideDownCoef);
    if (noteDur >= 5)
        painter->drawLine(xPoint + noteTail, yPoint - note16 * notesUpsideDownCoef,
                          xPoint + noteTailEnd, yPoint - note16 * notesUpsideDownCoef);
    if (noteDur >= 6)
        painter->drawLine(xPoint + noteTail, yPoint - note32 * notesUpsideDownCoef,
                          xPoint + noteTailEnd, yPoint - note32 * notesUpsideDownCoef);
    if (durDet)
        painter->drawText(xPoint + 6, yPoint + 5, std::to_string(durDet).c_str());
}


BarView::BarView(Bar *b,int nstr, int barNum):
    _pBar(b), _nStrings(nstr), _cursor(-1), _stringCursor(-1),
    _barNumber(barNum), _sameSign(false)
{
    _selectorBegin = -1;
    _selectorEnd = -1;
    _repBegin = false;
    _repEnd = false;

    const size_t barLen = b->size();
    _h = stringWidth * (nstr + 1);

    if (CONF_PARAM("TrackView.largeNotes") == "1")
        _h = stringWidth*(nstr + 2);

    _h += 10; //mini shift from last
    _w = (barLen + 1) * inbarWidth;

    if (CONF_PARAM("upsideDownNotes") == "1")
        _h+= 10;

    if (CONF_PARAM("showNotesView") == "1") {
       _h += (stringWidth-3)*(5);
       _h += 75;
       _h += 20; //to ensure bass would be ok
       if (CONF_PARAM("upsideDownNotes")=="0")
           _h+=10;
    }

    if (b->getRepeat() & 1) {
        _repBegin = true;
        _w += 15;
    }
    if (b->getRepeat() & 2) {
        _repEnd = true;
        _w += 15;
    }
}


void BarView::paint(QPainter *painter)
{
    Bar *bar1 = _pBar;
    int cX = _x;
    int cY = _y;

    Track *track = bar1->getParent();
    Tab* tab = track->getParent();
    bool isSelected = false;

    if (_selectorBegin!=-1)
    {
        int skipFromStart = (_selectorBegin)*inbarWidth;
        int skipFromEnd = (bar1->size() - _selectorEnd)*inbarWidth;

        if (_selectorEnd == -1)
            painter->fillRect(getX()+skipFromStart,getY(),
                              getW()-skipFromStart,getH(),QColor(CONF_PARAM("colors.selection").c_str()));
        else
            painter->fillRect(getX()+skipFromStart,getY(),
                              getW()-skipFromStart-skipFromEnd,getH(),QColor(CONF_PARAM("colors.selection").c_str()));
        isSelected = true;
    }

    if (_sameSign == false)
    {
        std::string numVal,denVal;
        numVal = std::to_string( bar1->getSignNum() );
        denVal = std::to_string( bar1->getSignDenum() );

        auto f = painter->font();
        auto fontSize = f.pixelSize();
        if (fontSize == -1)
            fontSize = 14;
        f.setPixelSize(18);
        painter->setFont(f);
        painter->drawText(cX-15,cY-15+50+3,numVal.c_str());
        painter->drawText(cX-15,cY+15+50+3,denVal.c_str());
        f.setPixelSize(fontSize);
        painter->setFont(f);
    }

    if (_repBegin)
        cX += 5;
    else
        cX -=10;

    if (_barNumber!=-1)
    {
        std::string numberLabel = std::to_string(_barNumber + 1);
        int xMiniSHift = 0;

        if ((_barNumber + 1) >= 10)
            xMiniSHift -= 5;

        if ((_barNumber + 1) >= 100)
            xMiniSHift -= 5;

        if (_repBegin) xMiniSHift -= 15;

        painter->drawText(cX+xMiniSHift, cY + 15 + 3, numberLabel.c_str());
    }

    size_t barLen = bar1->size();
    int amountStr = _nStrings;

    if (_repBegin)
    {
        int toBegin = -3;

        for (int l=0; l<2; ++l)
            painter->drawLine(toBegin + cX + l, cY + stringWidth / 2, toBegin + cX + l,
                              cY + amountStr * stringWidth - stringWidth / 2);

        painter->drawEllipse(toBegin + cX + 5, cY + stringWidth * 2, 3, 3);
        painter->drawEllipse(toBegin + cX + 5, cY + stringWidth * 5, 3, 3);
    }

    if (_repEnd)
    {
        int toBegin = _w - 10;
        if (_repBegin)
            toBegin -= 15;

        for (int l = 0; l < 2; ++l)
            painter->drawLine(toBegin + cX + l, cY + stringWidth / 2,
                              toBegin + cX + l, cY + amountStr * stringWidth - stringWidth / 2);

        std::string repCount =  std::to_string(bar1->getRepeatTimes());
        painter->drawText(toBegin + cX- 9, cY + stringWidth * 2 - 3, repCount.c_str());
        painter->drawEllipse(toBegin + cX - 9, cY + stringWidth * 2, 3, 3);
        painter->drawEllipse(toBegin + cX - 9, cY + stringWidth * 5, 3, 3);
    }


    for (int i = 1 ; i < (amountStr+1); ++i)
    {
        if (i == _stringCursor)
            changeColor(CONF_PARAM("colors.curString"), painter);

        painter->drawLine(cX + 10, cY + i * stringWidth - stringWidth / 2,
                          cX + 10 + barLen * inbarWidth, cY + i * stringWidth - stringWidth / 2);


        if (i == _stringCursor)
            changeColor(CONF_PARAM("colors.curBar"), painter);
    }

    //start and end lines
    painter->drawLine(cX + 10, cY + stringWidth / 2,
                      cX + 10, cY + stringWidth * amountStr - stringWidth / 2);

    painter->drawLine(cX + 10 + barLen * inbarWidth, cY + stringWidth / 2,
                      cX + 10 + barLen * inbarWidth, cY + stringWidth * amountStr - stringWidth / 2);

    auto [barMarker, colorMark] = bar1->getMarker();
    bool markerPrec = barMarker.empty() == false;

    bool wasNoBeatEffects = true;
    for (size_t i = 0; i < barLen; ++i) {


        auto& curBeat = bar1->at(i);
        auto tuning = track->getTuning();


        if (i == _cursor) {
             changeColor(CONF_PARAM("colors.curBeat"), painter);

             if (tab->playing())
             {

                painter->drawLine(cX + 19 + i * inbarWidth + inbarWidth / 4, cY,
                                  cX + 19 + i * inbarWidth + inbarWidth / 4,
                                  cY + stringWidth * tuning.getStringsAmount());

                /*
                painter->drawLine(cX + 19 + i * inbarWidth + inbarWidth / 4, cY,
                                  cX + 19 + i * inbarWidth + inbarWidth / 4,
                                  cY + stringWidth);

                painter->drawLine(cX + 19 + i * inbarWidth + inbarWidth / 4, cY + stringWidth * (tuning.getStringsAmount() - 1),
                                  cX + 19 + i * inbarWidth + inbarWidth / 4,
                                  cY + stringWidth * tuning.getStringsAmount());*/
             }

        }



        for (size_t j = 0; j < curBeat->size(); ++j)
        {
            auto& curNote = curBeat->at(j);
            if (curNote)
            {
                int noteTune = 0;
                int midiNote = 0;

                if (track)
                   noteTune = tuning.getTune(curNote->getStringNumber() - 1);

                midiNote = curNote->getMidiNote(noteTune);

                if (CONF_PARAM("showNotesView")=="1")
                {
                    drawMidiNote(painter,curBeat->getDuration(),
                                 curBeat->getDotted(),curBeat->getDurationDetail(),
                                 midiNote,
                                 cX + 10 + i * inbarWidth + inbarWidth / 2,
                                 cY + (stringWidth - 3) * (amountStr+5));
                }






                std::string noteVal;

                std::uint8_t noteState = curNote->getState();

                if (noteState != 3) //not x note
                {
                    if (noteState == 2) {
                        if (curNote->getFret() != 63)
                            noteVal = "," + std::to_string( curNote->getFret() );
                        else
                            noteVal = ",<";
                    }
                    else
                        noteVal = std::to_string( curNote->getFret() );
                }
                else
                    noteVal = "x"; //x note

                bool ghostNote = curNote->getEffects().getEffectAt(Effect::GhostNote);
                if (ghostNote) {
                    std::string sX ="(" +  noteVal + ")";
                    noteVal.clear();
                    noteVal = sX.c_str();
                }

                int miniShift = inbarWidth/2;

                if (curNote->getFret() >= 10)
                    miniShift /= 2;

                int currentString = curNote->getStringNumber();
                std::string currentBackground = CONF_PARAM("colors.background");

                if (isSelected)
                {
                    if (_selectorEnd == -1)//till end
                    {
                        if (i >= _selectorBegin)
                            currentBackground = CONF_PARAM("colors.selection");
                    }
                    else
                    {
                        if (_selectorBegin == 0)
                        {
                            if (i<=_selectorEnd)
                                currentBackground = CONF_PARAM("colors.selection");
                        }
                        else
                        {
                            //not zero start not -1 end
                            if ((i<=_selectorEnd) && (i >= _selectorBegin))
                                    currentBackground = CONF_PARAM("colors.selection");
                        }

                    }
                }

                painter->fillRect(cX + 10 + i * inbarWidth + inbarWidth / 4,
                                  cY + stringWidth * currentString - stringWidth,
                                  inbarWidth - inbarWidth / 4, stringWidth, QColor(currentBackground.c_str())); //BG color
                //DRAW effects

                bool palmMute = curNote->getEffects().getEffectAt(Effect::PalmMute);
                if (palmMute)
                    painter->drawRect(cX + 10 + i * inbarWidth + inbarWidth / 4,
                                      cY + stringWidth * currentString - stringWidth,
                                      inbarWidth - inbarWidth / 4, stringWidth);

                bool letRing = curNote->getEffects().getEffectAt(Effect::LetRing);
                if (letRing)
                    painter->drawEllipse(cX + 10 + i * inbarWidth + inbarWidth / 4,
                                         cY + stringWidth * currentString - stringWidth,
                                         inbarWidth - inbarWidth / 4, stringWidth);

                bool harmonics = curNote->getEffects().getEffectAt(Effect::HarmonicsV4);
                if (harmonics)
                {
                    //change midi note?
                    int XBegin = cX + 10 + i * inbarWidth + inbarWidth / 4;
                    int XEnd = inbarWidth - inbarWidth / 4 + XBegin;

                    int YBegin = cY+stringWidth * currentString - stringWidth;
                    int YEnd = stringWidth + YBegin;

                    XBegin -= 2;
                    YBegin -=2;
                    XEnd += 2;
                    YEnd += 2;

                    painter->drawLine(XBegin, (YBegin+YEnd) / 2, (XBegin+XEnd) / 2, YBegin);
                    painter->drawLine((XBegin+XEnd) / 2,YBegin, XEnd, (YBegin+YEnd) / 2);
                    painter->drawLine(XEnd, (YBegin+YEnd) / 2, (XBegin+XEnd) / 2, YEnd);
                    painter->drawLine((XBegin+XEnd) / 2, YEnd, XBegin, (YBegin + YEnd) / 2);
                }


                drawEffects(painter, cX + 10 + i * inbarWidth, cY + stringWidth * currentString,
                            inbarWidth - inbarWidth / 4, stringWidth, curNote->getEffects());

                int centerBoeder = 1;
                if (ghostNote)
                    miniShift -= 3;

                painter->drawText(cX + 10 + i * inbarWidth + miniShift,
                                  cY + stringWidth * currentString - centerBoeder, noteVal.c_str());

                if (ghostNote)
                    miniShift += 3;
            }
        }

        if (curBeat->getPause())
        {
            painter->drawText(cX+10+i*inbarWidth+ inbarWidth/2,cY + stringWidth * 3,"z");
            painter->drawText(cX+10+i*inbarWidth+ inbarWidth/2,cY + stringWidth * 4,"z");
        }


        std::string durVal = std::to_string(curBeat->getDuration());

        if (curBeat->getDotted())
            durVal += ".";

        std::uint8_t trump = curBeat->getDurationDetail();
        if (trump)
            durVal += "[" + std::to_string( trump );


        bool downStroke = curBeat->getEffects().getEffectAt(Effect::DownStroke);
        if (downStroke) {
            painter->drawLine(cX + 10 + i * inbarWidth + inbarWidth / 2, cY,
                              cX + 10 + i * inbarWidth + inbarWidth / 2, cY + stringWidth * (amountStr));

            painter->drawLine(cX + 10 + i * inbarWidth + inbarWidth / 2, cY + stringWidth * (amountStr),
                              cX + 10 + i * inbarWidth + inbarWidth / 2-2, cY + stringWidth * (amountStr)-3);

            painter->drawLine(cX + 10 + i * inbarWidth + inbarWidth / 2, cY + stringWidth * (amountStr),
                              cX + 10 + i * inbarWidth + inbarWidth / 2+2, cY + stringWidth * (amountStr)-3);
        }

        bool upStroke = curBeat->getEffects().getEffectAt(Effect::UpStroke);
        if (upStroke)
        {
            painter->drawLine(cX + 10 + i * inbarWidth + inbarWidth / 2, cY ,
                              cX + 10 + i * inbarWidth + inbarWidth / 2, cY + stringWidth * (amountStr));

            painter->drawLine(cX + 10 + i * inbarWidth + inbarWidth / 2, cY,
                              cX + 10 + i * inbarWidth + inbarWidth / 2-2, cY + 3);

            painter->drawLine(cX + 10 + i * inbarWidth + inbarWidth / 2, cY,
                              cX + 10 + i * inbarWidth + inbarWidth / 2 + 2, cY + 3);
        }

        bool fadeIn = curBeat->getEffects().getEffectAt(Effect::FadeIn);
        if (fadeIn)
        {
            painter->drawLine(cX + 10 + i * inbarWidth + inbarWidth / 2,cY + stringWidth * (amountStr)+stringWidth/2,
                              cX + 10 + i * inbarWidth + inbarWidth / 2+5,cY + stringWidth * (amountStr)+stringWidth/2-3);

            painter->drawLine(cX + 10 + i * inbarWidth + inbarWidth / 2,cY + stringWidth * (amountStr)+stringWidth/2,
                              cX + 10 + i * inbarWidth + inbarWidth / 2+5,cY + stringWidth * (amountStr)+stringWidth/2+3);

            wasNoBeatEffects = false;
        }

        bool changes = curBeat->getEffects().getEffectAt(Effect::Changes);
        if (changes)
        {
            painter->drawRect(cX + 10 + i * inbarWidth + inbarWidth / 2,cY + stringWidth * (amountStr),inbarWidth,stringWidth);
            painter->drawText(cX + 10 + i * inbarWidth + inbarWidth / 2,cY + stringWidth * (amountStr+1),"ch");//could replace with bpm value

            wasNoBeatEffects = false;
        }

        std::string textBeat; curBeat->getText(textBeat);
        bool textPrec = textBeat.empty()==false;
        if (textPrec)
        {
            auto f = painter->font();
            auto fontSize = f.pixelSize(); //refact fix
            if (fontSize == -1)
                fontSize = 14;
            f.setPixelSize(9);
            painter->setFont(f);
            painter->drawText(cX + 10 + i * inbarWidth + inbarWidth / 2, cY + stringWidth * (amountStr + 1), textBeat.c_str());
            f.setPixelSize(fontSize);
            painter->setFont(f);

        }


        if (CONF_PARAM("showNotesView") != "1")
            drawNote(painter, curBeat->getDuration(),curBeat->getDotted(),curBeat->getDurationDetail(),
                     cX + 10 + i * inbarWidth + inbarWidth / 2,
                     cY + stringWidth * (amountStr));

        if (i == _cursor)
             changeColor(CONF_PARAM("colors.curBar"), painter);
    }


    if (wasNoBeatEffects && markerPrec)
        painter->drawText(cX + 10 + inbarWidth / 2,
                          cY + stringWidth * (amountStr + 1), barMarker.c_str());


    if (CONF_PARAM("showNotesView") == "1")
    {
        int prefLines = 10;

        for (int lines = 0; lines < 11; ++lines)
        {
            if (lines==5)
                continue;

            painter->drawLine(cX + 10, cY + (stringWidth - 3) * (amountStr+prefLines + lines),
                              cX + 10 + barLen * inbarWidth, cY + (stringWidth - 3) * (amountStr + prefLines + lines));
        }

        painter->drawLine(cX + 10, cY + (stringWidth - 3) * (amountStr + prefLines + 0),
                cX + 10, cY + (stringWidth - 3) * (amountStr + prefLines + 4));

        painter->drawLine(cX + 10 + barLen * inbarWidth, cY + (stringWidth - 3) * (amountStr + prefLines + 0),
                cX + 10 + barLen * inbarWidth, cY + (stringWidth - 3) * (amountStr + prefLines + 4));

        painter->drawLine(cX + 10, cY + (stringWidth - 3) * (amountStr + prefLines + 6),
                cX + 10, cY + (stringWidth - 3) * (amountStr + prefLines + 10));

        painter->drawLine(cX + 10 + barLen * inbarWidth, cY + (stringWidth - 3) * (amountStr + prefLines + 6),
                cX + 10 + barLen * inbarWidth, cY + (stringWidth - 3) * (amountStr + prefLines + 10));
    }

    flushSelectors();
}


void BarView::drawEffects(QPainter *painter, int x1, int y1, int w1, int h1, const ABitArray& eff)
{
    if (eff.getEffectAt(Effect::HeavyAccented))
    {
        painter->drawLine(x1+w1/2,y1-h1,x1+2+w1/2,y1-3-h1);
        painter->drawLine(x1+2+w1/2,y1-3-h1,x1+4+w1/2,y1-h1);
    }

    if (eff.getEffectAt(Effect::Bend))
        painter->drawLine(x1+w1, y1-h1/2, x1+w1+6, y1-h1-6);

    if (eff.getEffectAt(Effect::TremoloPick))
    {
        painter->drawLine(x1+w1/2,y1-h1,
                          x1+w1/2+3,y1-h1-3);

        painter->drawLine(x1+w1/2+2,y1-h1,
                          x1+w1/2+5,y1-h1-3);

        painter->drawLine(x1+w1/2+4,y1-h1,
                          x1+w1/2+7,y1-h1-3);
    }

    if (eff.getEffectAt(Effect::Vibrato))
    {
        x1 += inbarWidth / 2;
        y1 -= 3 * stringWidth / 4;
        painter->drawLine(x1, y1, x1 + 2, y1 - 2);
        painter->drawLine(x1 + 2, y1 - 2, x1 + 4, y1);
        painter->drawLine(x1 + 4, y1, x1 + 6, y1 - 2);
        painter->drawLine(x1 + 6, y1 - 2, x1 + 8, y1);
        painter->drawLine(x1 + 8, y1, x1 + 10, y1 -2);
    }

    if (eff.getEffectAt(Effect::Legato))
    {
        x1 += inbarWidth / 2;
        y1 -= 3 * stringWidth / 4;
        painter->drawLine(x1, y1, x1 + 7, y1 - 3);
        painter->drawLine(x1 + 7, y1 - 3, x1 + 14, y1);
    }

    if (eff.inRange(Effect::Hammer,Effect::SlideUpV2))
    {
        if (eff.getEffectAt(Effect::Slide) || eff.getEffectAt(Effect::LegatoSlide))
        {
            x1 += 3 * inbarWidth / 4;
            y1 -= stringWidth / 2;

            painter->drawLine(x1, y1 - 2, x1 + 14, y1 + 4);

            x1 -= inbarWidth / 4;
            y1 -= stringWidth / 4;

            if (eff.getEffectAt(Effect::LegatoSlide))
            {
                painter->drawLine(x1, y1, x1 + 7, y1 - 3);
                painter->drawLine(x1 + 7, y1 - 3, x1 + 14, y1);
            }
        }

        if (eff.getEffectAt(Effect::SlideDownV2) || eff.getEffectAt(Effect::SlideDownV1))
        {
            x1 += 3 * inbarWidth / 4;
            y1 -= stringWidth / 2;
            painter->drawLine(x1, y1 - 2, x1 + 10, y1 + 7);
        }

        if (eff.getEffectAt(Effect::SlideUpV2)||eff.getEffectAt(Effect::SlideUpV1))
        {
            x1 +=  3 *inbarWidth / 4;
            y1 -= stringWidth / 2;

            painter->drawLine(x1, y1 + 7, x1 + 10, y1 - 2);
        }
        ///2.19 is only guidence
        //1 normal 2 from -1 to 12
        //3 normal 4 from 99 to 1
        //5 from 1 to 99
    }
}


int BarView::getClickString(int y1) const
{
    int yOffset = y1 - _y;
    int stringIndex = yOffset / stringWidth;
    return stringIndex;
}


int BarView::getClickBeat(int x1) const
{
    if (_repBegin) x1 -= 15;

    int xOffset = x1 - _x;
    int beatIndex = xOffset / inbarWidth;

    return beatIndex;
}

