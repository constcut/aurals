#ifndef TAPPER_H
#define TAPPER_H

#include <vector>
#include <chrono>

#include <QObject>

namespace mtherapp {

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

            Q_INVOKABLE void pressed(int idx);
            Q_INVOKABLE void released(int idx);
            Q_INVOKABLE void saveClicksAsMidi(QString filename) const;

            Q_INVOKABLE void tapped(int idx);
            Q_INVOKABLE void saveTapsAsMidi(QString filename) const;


            //TODO настройка разных нот для idx
            //настройка разных инструментов и каналов для разных idx
            //настройка коэфициента скорости
            //настройки громкости

        private:

            std::vector<MouseEvent> _mouseEvents;

            std::vector<TapEvent> _tapEvents;
    };

}


#endif // TAPPER_H
