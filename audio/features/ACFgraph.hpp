#ifndef ACFGRAPH_H
#define ACFGRAPH_H


#include <QQuickPaintedItem>
#include <QPainter>
#include <QByteArray>

#include <unordered_set>

#include "audio/features/Yin.hpp"


namespace aural_sight {



    class ACGraphQML : public QQuickPaintedItem
    {
            Q_OBJECT
    public:
        ACGraphQML([[maybe_unused]] QQuickItem* parent = nullptr)
        : _input(4096, 0.0)
        { _yin.init(44100, 4096); }

        ~ACGraphQML() = default;

        void paint(QPainter* painter);

        Q_INVOKABLE bool loadByteArray(QByteArray analyseData);

        Q_INVOKABLE double getLastFreq() { return _lastFreq; }
        //Load from samples

        Q_INVOKABLE void setCursor(double pos) {
            _cursorPos = pos;
            update(); //TODO вначале нарисовать всю подложку
        }

    private:
        QVector<float> _input; //TODO тут и в спектральном анализаторе
        YinPP _yin;

        double _lastFreq = 0;

        void prepareBackground(QPainter &painter, const QRect &rect) const;

        double _cursorPos = -1.0;

    };

}
#endif // ACFGRAPH_H
