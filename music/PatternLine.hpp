#ifndef PATTERNLINE_HPP
#define PATTERNLINE_HPP

#include <QQuickPaintedItem>
#include <QJsonObject>

#include <vector>


namespace aurals {

    struct PatternBrick {
        //velocity + other effects?

        int x;
        int y;
        int w;
        int h;

        bool border = false;
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

        QJsonObject serialize() const;
        void deserialize(QJsonObject);
    };


    struct PatternLineState { //Возможно вынести наружу и хранить внутри PL
        uint8_t num = 4;
        uint8_t denom = 4;
        uint8_t brickSize = 4;
        uint8_t midiNote = 35;
        std::vector<PatternBrick> bricks;

        QJsonObject serialize() const;
        void deserialize(QJsonObject);

    };

    class PatternLine  : public QQuickPaintedItem
    {
        Q_OBJECT

    public:
        PatternLine() = default;

        void paint(QPainter* painter);

        Q_INVOKABLE void setNumerator(int num) {
            if (num == _state.num && _state.bricks.empty() == false)
                return;
            _state.num = num;
            updateBricks();
            update();
        }

        Q_INVOKABLE void setDenomenator(int denom) {
            if (denom == _state.denom && _state.bricks.empty() == false)
                return;
            _state.denom = denom;
            updateBricks();
            update();
        }

        Q_INVOKABLE void setBrickSize(int size) {
            if (size == _state.brickSize && _state.bricks.empty() == false)
                return;
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
        Q_INVOKABLE int getNum() { return _state.num; }
        Q_INVOKABLE int getDenom() { return _state.denom; }
        Q_INVOKABLE int getBrickSize() { return _state.brickSize; }
        Q_INVOKABLE int getMidiNote() { return _state.midiNote; }

        const PatternLineState& getState() const { return _state; }
        void setState(PatternLineState& state) { _state = state; }


    private:

        PatternLineState _state;

    };

}


#endif // PATTERNLINE_HPP
