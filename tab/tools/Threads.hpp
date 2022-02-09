#ifndef THREADS_HPP
#define THREADS_HPP

#include <vector>
#include <stdint.h>

#include <QThread>


namespace aurals {


    class Tab;
    class Track;
    class Bar;


    class PlayAnimationThr
    {
    protected:
        size_t* _cursorToInc1;
        size_t* _cursorToInc2;

        size_t _limit;

        int _bpm;
        std::vector<int> _waitTimes;
        std::vector<int> _waitIndexes;
        std::vector< std::vector<int> > _beatTimes;

        int _status;
        bool _pleaseStop;

    public:

       virtual void callUpdate() = 0;
       virtual void noticeFinished() = 0;

       void requestStop() { _pleaseStop = true; }

       PlayAnimationThr():_cursorToInc1(0),_cursorToInc2(0),_limit(0),_bpm(1),_status(0),_pleaseStop(false) {}

       int getStatus() const { return _status; }

       void setBPM(int newBPM) { _bpm = newBPM; _waitTimes.clear(); }
       void setLimit(size_t max) { _limit = max; }
       void setInc(size_t *ptrA, size_t *ptrB) { _cursorToInc1 = ptrA; _cursorToInc2 = ptrB; }

       void setupValues(Tab* tab_ptr, Track* track_ptr, size_t shiftTheCursor);
       void addNumDenum(uint8_t nu, uint8_t de, size_t nextIndex);
       void addBeatTimes(Bar* bar);

       void threadRun();
       double calculateSeconds();

    protected:
       virtual void sleepThread(int ms)=0;
    };


    class ThreadLocal : public QThread, public PlayAnimationThr
    {
        Q_OBJECT
    signals:
        void updateUI();
        void nowFinished();

    public:
        void run() Q_DECL_OVERRIDE {
            threadRun();
        }

        void noticeFinished() override {
            emit nowFinished();
        }

        void callUpdate() override {
            emit updateUI();
        }

        void sleepThread(int ms) override {
            msleep(ms);
        }
    };

}

#endif
