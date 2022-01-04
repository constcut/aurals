#ifndef TAPPER_H
#define TAPPER_H

#include <vector>
#include <chrono>

#include <QObject>

namespace mtherapp {

    using moment = std::chrono::time_point<std::chrono::steady_clock>;

    struct TapEvent {
        int idx;
        bool pressed;
        moment time;
    };

    class Tapper : public QObject {
        Q_OBJECT

        public:
            Tapper() = default;

            Q_INVOKABLE void pressed(int idx);
            Q_INVOKABLE void released(int idx);

            Q_INVOKABLE void reset();

            Q_INVOKABLE void saveAsMidi(QString filename);

        private:

            std::vector<TapEvent> _events;
    };

}


#endif // TAPPER_H
