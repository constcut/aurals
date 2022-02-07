#ifndef BARVIEW_H
#define BARVIEW_H

#include "tab/Bar.hpp"

#include <QPainter>
//#include <QQuickPaintedItem>


namespace aural_sight {

    class BarView /*: public QQuickPaintedItem*/
    {
       // Q_OBJECT //Yet copy constructor issue - solve it

    private:

        static const int stringWidth=12;
        static const int inbarWidth=20;

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

        int x = 0;
        int y = 0;
        int w = 0;
        int h = 0;

    public:

        BarView() = default;
        virtual ~BarView() = default;

        int getX() const { return x; }
        int getY() const { return y; }
        int getW() const { return w; }
        int getH() const { return h; }

        void setW(int newW) { w = newW; }
        void setH(int newH) { h = newH; }
        void setX(int newX) { x = newX; }
        void setY(int newY) { y = newY; }

        bool hit(int hX, int hY) {
            if ((hX >= x) && (hY >= y)) {
              int xDiff = hX - x;
              int yDiff = hY - y;
              if ((xDiff <= w) && (yDiff <= h))
                  return true;
            }
            return false;
        }

        BarView(Bar *b, int nstr, int barNum = -1);

        void setBar(Bar *newBar) {_pBar = newBar;}
        Bar *getBar() const { return _pBar; }

        int getBarsize() const { return _pBar->size(); }

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
            x = x1 + 20;
            y = y1 + 20; //TODO move this logic outside?
        }

        void setNStrings(int ns) { _nStrings = ns; }
    };

}

#endif // BARVIEW_H
