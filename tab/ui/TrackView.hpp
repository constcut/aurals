#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <vector>
#include <memory>

#include "BarView.hpp"
#include "tab/tools/Commands.hpp"
#include "tab/tools/Threads.hpp"


namespace aural_sight {

    class TabView;
    using ViewPull = std::vector<BarView>;


    class TrackView
    {
    private:
        Track* _pTrack;

        TabView* _tabParrent;
        std::unique_ptr<ThreadLocal> _animationThread; //Подумать над хранением, возможно удастся спрятать?
        std::vector<std::unique_ptr<ThreadLocal>> _finishPool;

        ViewPull _barsPull;

    public:

        //TODO!!! WHEN APPLY QML REPLACE
        int width() { return 0; }
        int height() { return 0; }
        //TODO!!! WHEN APPLY QML REPLACE

        Track* getTrack() const { return _pTrack; }

        virtual void setUI();
        virtual bool isMovableY() { return true; } //Emm

        TrackView(Track *from): _pTrack(from) {}

        virtual ~TrackView();

        virtual void keyevent(std::string press);

        virtual void onTabCommand(TabCommand command);
        virtual void onTrackCommand(TrackCommand command);

        void onclick(int x1, int y1);
        void ondblclick(int x1, int y1);

        virtual void ongesture(int offset, bool horizontal) ;

        int horizonMove(int offset);

        void setDisplayBar(int barPosition);

        TabView *getPa() const { return _tabParrent; }
        void setPa(TabView* tv) { _tabParrent = tv; }

        void draw(QPainter *painter);

        void prepareThread(size_t shiftTheCursor);
        void launchThread();
        void stopThread();
        //void connectThreadSignal(MasterView *masterView);
        int threadSeconds() { return _animationThread->calculateSeconds(); }


        bool gotChanges() const;

    };

}

#endif // TRACKVIEW_H
