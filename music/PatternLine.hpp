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


    struct PatternLineState { //Возможно вынести наружу и хранить внутри PL
        uint8_t num = 4;
        uint8_t denom = 4;
        uint8_t brickSize = 4;
        uint8_t midiNote = 35;
        std::vector<PatternBrick> bricks;
    };

    class PatternLine  : public QQuickPaintedItem
    {
        Q_OBJECT

    public:
        PatternLine() = default;

        void paint(QPainter* painter);

        Q_INVOKABLE void setNumerator(int num) {
            _state.num = num;
            updateBricks();
            update();
        }

        Q_INVOKABLE void setDenomenator(int denom) {
            _state.denom = denom;
            updateBricks();
            update();
        }

        Q_INVOKABLE void setBrickSize(int size) {
            _state.brickSize = size;
            updateBricks();
            update();
        }

        Q_INVOKABLE void setMidiNote(int note) {
            _state.midiNote = note;
        }

        void updateBricks();

        Q_INVOKABLE void onClick(int x, int y);

        Q_INVOKABLE int fullWidth() {
            if (_state.bricks.empty() == false) {
                return _state.bricks.back().x + _state.bricks.back().w;
            }
            return 0;
        }

        const std::vector<PatternBrick>& getBricks() { return _state.bricks; }
        uint8_t getNum() { return _state.num; }
        uint8_t getDenom() { return _state.denom; }
        uint8_t getBrickSize() { return _state.brickSize; }
        uint8_t getMidiNote() { return _state.midiNote; }

        const PatternLineState& getState() const { return _state; }
        void setState(PatternLineState& state) { _state = state; }


    private:

        PatternLineState _state;

    };

}


#endif // PATTERNLINE_HPP
