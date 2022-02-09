#include "GraphicMap.hpp"

#include <QPainter>

using namespace aurals;


int GraphicMap::getAssociation(const int x, const int y) const{

    for (int i = elements.size()-1; i >= 0; --i) {
        int x2 = elements[i].w + elements[i].x;
        int y2 = elements[i].h + elements[i].y;
        if ((x > elements[i].x && x < x2) && (y > elements[i].y && y < y2))
            return elements[i].association;
    }
    return -1;
}

void GraphicMap::addElement(const int x, const int y, const int w, const int h,
                            const QColor c, const QColor hl, const int association) {
    GraphicElement el;
    el.x = x;
    el.y = y;
    el.w = w;
    el.h = h;
    el.color = c;
    el.hl = hl;
    el.association = association;
    elements << el;
}

void GraphicMap::paint(QPainter *painter) {

    for (auto& el: elements) {
        if (el.selected == false)
            painter->fillRect(el.x,el.y,el.w,el.h,el.color);
        else
            painter->fillRect(el.x,el.y,el.w,el.h,el.hl);
        painter->setPen(el.hl);
        painter->drawRect(el.x,el.y,el.w,el.h);
    }
}


//================PianoQML====================================


PianoQML::PianoQML() :_whiteKeyWidth(30), _whiteKeyHeight(110), _blackKeyWidth(16), _blackKeyHeight(80) {

    addNote(0, 0, 21, true);
    addNote(_whiteKeyWidth,0,23);

    int offsetsOfFirstNotes = _whiteKeyWidth*2;
    int octaveWidth = _whiteKeyWidth*7;

    for (int i = 0; i < 7; ++i)
        addOctave(octaveWidth * i + offsetsOfFirstNotes ,0,24 + 12 * i);
    addNote(octaveWidth * 7 + offsetsOfFirstNotes, 0, 108);
}

void PianoQML::addOctave(int x, const int y, const int startingNote) {
    for (int i = 0; i < 7; ++i) {
        bool hasBlack = (i == 2 || i == 6) ? false : true;
        int subShift = (i + 1) / 4;
        int noteShift = 2 * i - subShift; //7 notes to 12
        addNote(i * _whiteKeyWidth + x, y, startingNote+noteShift,hasBlack);
    }
}

void PianoQML::addNote(int x, const int y, const int note, const bool hasBlack) {
    GraphicElement whiteKey;
    whiteKey.association = note;
    whiteKey.x = x;
    whiteKey.y = y;
    whiteKey.color = QColor(Qt::white);
    whiteKey.hl = QColor(Qt::darkGreen);
    whiteKey.w = _whiteKeyWidth;
    whiteKey.h = _whiteKeyHeight;

    elements << whiteKey;

    if (hasBlack) {
        GraphicElement blackKey;
        blackKey.association = note + 1;
        blackKey.x = x+ (_whiteKeyWidth-_blackKeyWidth);
        blackKey.y = y;
        blackKey.w = _blackKeyWidth;
        blackKey.h = _blackKeyHeight;
        elements << blackKey;
    }
}

void PianoQML::selectByAssociation(const int ass) {
    GraphicElement *el = findElementByAssociation(ass);
    if (el) {
        el->selected = true;
        update();
    }
}

void PianoQML::unSelectByAssociation(const int ass) {
    GraphicElement *el = findElementByAssociation(ass);
    if (el) {
        el->selected = false;
        update();
    }
}

GraphicElement *PianoQML::findElementByAssociation(const int ass) {
    for (auto& e: elements)
        if (e.association == ass)
            return &e;
    return nullptr;
}
