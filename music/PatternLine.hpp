#ifndef PATTERNLINE_HPP
#define PATTERNLINE_HPP

#include <QQuickPaintedItem>


namespace aurals {

    struct PatternBrick {
        //velocity
        //other params
        bool on = false;
    };


    class PatternLine  : public QQuickPaintedItem
    {
        Q_OBJECT

    public:
        PatternLine() = default;

        //setters\getters

        //paint event

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

        void updateBricks() {
            _bricks.clear();
            for (size_t i = 0; i < _num; ++i)
                _bricks.push_back({});
        }


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
