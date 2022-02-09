#ifndef TAPPER_H
#define TAPPER_H

#include <vector>
#include <chrono>

#include <QObject>

namespace aurals {

    using moment = std::chrono::time_point<std::chrono::high_resolution_clock>;

    struct MouseEvent {
        int idx;
        bool pressed;
        moment time;
    };

    struct TapEvent {
        int idx;
        moment time;
    };

    class Tapper : public QObject {
        Q_OBJECT

        public:
            Tapper() = default;

            Q_INVOKABLE void reset();

            Q_INVOKABLE void pressed(const int idx);
            Q_INVOKABLE void released(const int idx);
            Q_INVOKABLE void saveClicksAsMidi(const QString filename) const;

            Q_INVOKABLE void tapped(const int idx);
            Q_INVOKABLE void saveTapsAsMidi(const QString filename) const;

            Q_INVOKABLE void setSpeedCoef(double coef) { _speedCoef = coef; }


        private:

            std::vector<MouseEvent> _mouseEvents;
            std::vector<TapEvent> _tapEvents;

            double _speedCoef = 1.0;
    };

}


#endif // TAPPER_H
