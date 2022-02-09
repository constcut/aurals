#ifndef BARVIEW_H
#define BARVIEW_H

#include "tab/Bar.hpp"

#include <QPainter>



namespace aurals {

    class BarView
    {
       // Q_OBJECT //Yet copy constructor issue - solve it to paint in config //#include <QQuickPaintedItem>

    private:

        static const int stringWidth = 12;
        static const int inbarWidth = 20; //ВСЕ КОНСТАНТЫ ВЫНЕСТИ В КОНФИГУРАЦИЮ, ПОЗВОЛИТЬ НАСТРАИВАТЬ refactoring

        Bar* _pBar;
        int _nStrings;
        int _cursor;
        int _stringCursor;

        int _barNumber;

        bool _sameSign;

        int _selectorBegin;
        int _selectorEnd;

        bool _repBegin;
        bool _repEnd;

        int _x = 0;
        int _y = 0;
        int _w = 0;
        int _h = 0;

    public:

        BarView() = default;
        virtual ~BarView() = default;

        int getX() const { return _x; }
        int getY() const { return _y; }
        int getW() const { return _w; }
        int getH() const { return _h; }

        void setW(int newW) { _w = newW; }
        void setH(int newH) { _h = newH; }
        void setX(int newX) { _x = newX; }
        void setY(int newY) { _y = newY; }

        bool hit(int hX, int hY) {
            if ((hX >= _x) && (hY >= _y)) {
              int xDiff = hX - _x;
              int yDiff = hY - _y;
              if ((xDiff <= _w) && (yDiff <= _h))
                  return true;
            }
            return false;
        }

        BarView(Bar *b, int nstr, int barNum = -1);

        Bar* getBar() { return _pBar; }

        void setSameSign(bool value) { _sameSign = value; }

        void setSelectors(int beg, int end){_selectorBegin = beg; _selectorEnd = end;}
        void flushSelectors() { _selectorBegin = _selectorEnd = -1; }

        void setCursor(int cur, int strCur = -1) { _cursor = cur; _stringCursor =strCur; }
        int getCursor() const { return _cursor; }
        void flushCursor() {  _cursor = -1; _stringCursor = -1; }

        int getClickString(int y1) const;
        int getClickBeat(int x1) const;

        void drawNote(QPainter *painter, std::uint8_t noteDur, std::uint8_t dotted, std::uint8_t durDet,
                      int x1, int y1);

        void drawMidiNote(QPainter *painter, std::uint8_t noteDur, std::uint8_t dotted, std::uint8_t durDet, int midiNote,
                          int x1, int y1);

        void drawEffects(QPainter *painter, int x1, int y1, int w1, int h1, const ABitArray &eff);

        void paint(QPainter *painter); //from beat to beat

        void setShifts(int x1, int y1) {
            _x = x1 + 20; //Возможно избавиться от этой странноватой логики здесь, устанавливать в TrackView x y
            _y = y1 + 20; //ВСЕ КОНСТАНТЫ ВЫНЕСТИ В КОНФИГУРАЦИЮ, ПОЗВОЛИТЬ НАСТРАИВАТЬ refactoring
        }

    };

}

#endif // BARVIEW_H
