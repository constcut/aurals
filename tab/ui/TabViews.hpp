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

        std::unique_ptr<ThreadLocal> _localThr;
        std::vector<std::unique_ptr<ThreadLocal>> _finishPool;

        std::vector<TrackView*> _tracksView;

    public:

        TabView() = default;
        virtual ~TabView();

        void setCurrentBar(int curBar) {
            _pTab->getCurrentBar() = curBar; }

        Q_INVOKABLE int getTimeLineBar();

        Q_INVOKABLE int getCurTrack() { return _pTab->getCurrentTrack(); }
        Q_INVOKABLE void setTrackIdx(int idx) {
            _pTab->getCurrentTrack() = idx;
            update();
        }


        void setTab(std::unique_ptr<Tab> point2Tab);
        void refreshTabStats();

        std::unique_ptr<Tab>& getTab() { return _pTab; }
        Q_INVOKABLE int tracksCount() { return _pTab->size(); }

        Q_INVOKABLE void loadTab(QString filename);
        Q_INVOKABLE void exportMidi(QString filename, int shift);


        void paint(QPainter *painter);


        Q_INVOKABLE void keyPress(int code); //KeyRelease too?
        virtual void keyevent(std::string press);

        void onTabCommand(TabCommand command);
        void onTrackCommand(TrackCommand command);

        Q_INVOKABLE void passTabCommand(int id) { //TODO register enum in QML
            auto c = static_cast<TabCommand>(id);
            onTabCommand(c);
        }

        Q_INVOKABLE void passTrackCommand(int id) {
            auto c = static_cast<TrackCommand>(id);
            onTrackCommand(c);
        }



        void setPlaying(bool playValue) { _pTab->setPlaying(playValue); }
        bool getPlaying();

        void addTrackView(TrackView* trackView);
        void removeTrackView(TrackView* trackView);


        Q_INVOKABLE void onclick(int x1, int y1);
        Q_INVOKABLE void ondblclick(int x1, int y1);

        virtual void ongesture(int offset, bool horizontal);

        Q_INVOKABLE void prepareAllThreads(int shiftTheCursor);
        Q_INVOKABLE void launchAllThreads();
        Q_INVOKABLE void stopAllThreads();

        //TODO связывание с Core вместо
        //void connectAllThreadsSignal(MasterView *masterView);

        bool gotChanges() const;

        std::vector<TrackView*>& getTracksViewRef() { return _tracksView; }

    };

}


#endif // TABVIEWS_H
