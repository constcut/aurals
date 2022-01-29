#ifndef TABVIEWS_H
#define TABVIEWS_H

#include <vector>
#include <memory>
#include <QQuickPaintedItem>

#include "tab/tools/TabClipboard.hpp"
#include "tab/tools/Commands.hpp"

#include "TrackView.hpp"
#include "tab/Tab.hpp"


namespace aural_sight {


    //TODO функции в отдельный хэдер
    void changeColor(const std::string& color, QPainter* src);
    void drawEllipse(QColor c, QPainter *painter, int x, int y, int w, int h);
    void drawEllipse(QPainter *painter, int x, int y, int w, int h);
    int translateDefaulColor(const std::string& confParam);


    class TabView : public QQuickPaintedItem
    {
        Q_OBJECT

    private:
        std::unique_ptr<Tab> _pTab;


        //TODO убрать в Core
        std::unique_ptr<ThreadLocal> _localThr;
        std::vector<std::unique_ptr<ThreadLocal>> _finishPool;

        std::vector<std::unique_ptr<TrackView>> _tracksView; //Maybe add here only when we create QML
        //And remove when it destroyed

    public:


        TabView();
        virtual ~TabView();

        virtual void setUI();

        void setCurrentBar(int curBar) {
            _pTab->getCurrentBar() = curBar; }

        virtual bool isMovableX() { return true; }
        virtual bool isMovableY() { return true; }


        int getCurTrack() { return _pTab->getCurrentTrack(); }
        int getLastOpenedTrack() { return _pTab->getLastOpenedTrack(); }

        void setTab(std::unique_ptr<Tab> point2Tab);
        void refreshTabStats();

        std::unique_ptr<Tab>& getTab() { return _pTab; }
        Q_INVOKABLE int tracksCount() { return _pTab->size(); }


        Q_INVOKABLE void loadTab(QString filename);

        void paint(QPainter *painter);

        virtual void keyevent(std::string press);

        virtual void onTabCommand(TabCommand command);
        virtual void onTrackCommand(TrackCommand command);

        void setPlaying(bool playValue) { _pTab->setPlaying(playValue); }
        bool getPlaying();

        void addSingleTrack(Track *track);

        Q_INVOKABLE void onclick(int x1, int y1);
        Q_INVOKABLE void ondblclick(int x1, int y1);

        virtual void ongesture(int offset, bool horizontal);

        void prepareAllThreads(size_t shiftTheCursor);
        void launchAllThreads();
        void stopAllThreads();

        //TODO связывание с Core вместо
        //void connectAllThreadsSignal(MasterView *masterView);

        bool gotChanges() const;

        std::vector<std::unique_ptr<TrackView>>& getTracksViewRef() { return _tracksView; }

    };

}


#endif // TABVIEWS_H
