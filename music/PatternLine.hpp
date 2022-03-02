#ifndef PATTERNLINE_HPP
#define PATTERNLINE_HPP

#include <QQuickPaintedItem>

#include <vector>


namespace aurals {

    struct PatternBrick {
        //velocity + other effects?

        int x;
        int y;
        int w;
        int h;

        bool on = false;

        bool hit(int xHit, int yHit) { //Delayed abstraction for BarView\PianoRoll\PatternBrick
            if ((xHit >= x) && (yHit >= y)) {
              int xDiff = xHit - x;
              int yDiff = yHit - y;
              if (xDiff <= w && yDiff <= h)
                  return true;
            }
            return false;
        }
    };


    class PatternLine  : public QQuickPaintedItem
    {
        Q_OBJECT

    public:
        PatternLine() = default;

        void paint(QPainter* painter);

        Q_INVOKABLE void setNumerator(int num) {
            _num = num;
            updateBricks();
            update();
        }

        Q_INVOKABLE void setDenomenator(int denom) {
            _denom = denom;
            updateBricks();
            update();
        }

        Q_INVOKABLE void setBrickSize(int size) {
            _brickSize = size;
            updateBricks();
            update();
        }

        void updateBricks();

        Q_INVOKABLE void onClick(int x, int y);

        Q_INVOKABLE int fullWidth() {
            if (_bricks.empty() == false) {
                return _bricks.back().x + _bricks.back().w;
            }
            return 0;
        }

        const std::vector<PatternBrick>& getBricks() { return _bricks; }
        uint8_t getNum() { return _num; }
        uint8_t getDenom() { return _denom; }
        uint8_t getBrickSize() { return _brickSize; }


    private:

        uint8_t _num = 4;
        uint8_t _denom = 4;

        uint8_t _brickSize = 4;

        uint8_t _midiNote; //basicly drums

        std::vector<PatternBrick> _bricks;

        //Sepparated bpm

    };

}


#endif // PATTERNLINE_HPP
