#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <vector>
#include <memory>
#include <QQuickPaintedItem>

#include "BarView.hpp"
#include "tab/tools/Commands.hpp"
#include "tab/tools/Threads.hpp"


namespace aural_sight {

    class TabView;
    using ViewPull = std::vector<BarView>;


    class TrackView : public QQuickPaintedItem
    {
        Q_OBJECT

    private:
        Track* _pTrack = nullptr;

        TabView* _tabParrent = nullptr;

        //TODO убрать в Core
        std::unique_ptr<ThreadLocal> _animationThread; //Подумать над хранением, возможно удастся спрятать?
        std::vector<std::unique_ptr<ThreadLocal>> _finishPool;

        ViewPull _barsPull;

    public:

        TrackView() = default;
        TrackView(Track *from): _pTrack(from) {}
        virtual ~TrackView();

        void setTrack(Track* track) { _pTrack = track; }
        Track* getTrack() const { return _pTrack; }

        Q_INVOKABLE void setFromTab(QObject* pa, int trackIdx);

        virtual void setUI();
        virtual bool isMovableY() { return true; } //Все что будет не использоваться, убрать

        virtual void keyevent(std::string press);

        virtual void onTabCommand(TabCommand command);
        virtual void onTrackCommand(TrackCommand command);

        Q_INVOKABLE void onclick(int x1, int y1);
        Q_INVOKABLE void ondblclick(int x1, int y1);

        virtual void ongesture(int offset, bool horizontal) ;

        int horizonMove(int offset);

        void setDisplayBar(int barPosition);

        TabView *getPa() const { return _tabParrent; }
        void setPa(TabView* tv) { _tabParrent = tv; }

        void paint(QPainter *painter);

        void prepareThread(size_t shiftTheCursor);
        void launchThread();
        void stopThread();
        //void connectThreadSignal(MasterView *masterView);
        int threadSeconds() { return _animationThread->calculateSeconds(); }


        bool gotChanges() const;

    };

}

#endif // TRACKVIEW_H
