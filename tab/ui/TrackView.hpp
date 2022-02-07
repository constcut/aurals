#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <vector>
#include <memory>
#include <QQuickPaintedItem>
#include <QImage>

#include "BarView.hpp"
#include "tab/tools/Commands.hpp"
#include "tab/tools/Threads.hpp"


namespace aural_sight {

    class TabView;
    using ViewPull = std::vector<BarView>;
    using BarLines = std::vector<std::vector<size_t>>;


    class TrackView : public QQuickPaintedItem
    {
        Q_OBJECT

    private:

        Track* _pTrack = nullptr;
        TabView* _tabParrent = nullptr;

        std::unique_ptr<ThreadLocal> _animationThread;
        std::vector<std::unique_ptr<ThreadLocal>> _finishPool;

        ViewPull _barsPool;
        BarLines _linesIdxs;

        int _lastWidth = 0;
        int _lastHeight = 0;

    public:

        TrackView() = default;
        virtual ~TrackView();

        Q_INVOKABLE void setFromTab(QObject* pa, int trackIdx);

        virtual void keyevent(std::string press);

        virtual void onTabCommand(TabCommand command);
        virtual void onTrackCommand(TrackCommand command);

        Q_INVOKABLE void onclick(int x1, int y1);

        void setDisplayBar(int barPosition);

        void paint(QPainter *painter);
        Q_INVOKABLE int getFullPixelHeight();

        Q_INVOKABLE void prepareThread(int shiftTheCursor);
        Q_INVOKABLE void launchThread();
        Q_INVOKABLE void stopThread();

        int threadSeconds() { return _animationThread->calculateSeconds(); }

        bool gotChanges() const;

    private:

        void paintMainArea(QPainter *painter);
        void paintByLines(QPainter *painter);

        void fillBarsPool();

        void moveNextLine();
        void movePrevLine();

    public:

        Q_INVOKABLE int getInstrumet();
        Q_INVOKABLE void setInstrument(int newInstr);

        Q_INVOKABLE int getVolume();
        Q_INVOKABLE void setVolume(int newVol);

        Q_INVOKABLE int getPanoram();
        Q_INVOKABLE void setPanoram(int newPan);

        Q_INVOKABLE int getStatus(); //0 - none, 1 - mute, 2 - solo
        Q_INVOKABLE void setStatus(int newStatus);

        Q_INVOKABLE QString getName();
        Q_INVOKABLE void setName(QString newName);

    };

}

#endif // TRACKVIEW_H
