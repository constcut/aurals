#ifndef TABCLIPBOARD_H
#define TABCLIPBOARD_H



namespace aurals {


    class Bar;

    enum class ClipboardType {
        NotSet = -1,
        SingleBarCopy = 0,
        SingleBeatCopy = 1,
        BarsCopy = 2,
        BeatsCopy = 3,
        BarPointer = 4,
    };


    class AClipboard
    {
    private:

        ClipboardType _type;

        int _trackIdx1 = -1;
        int _trackIdx2 = -1;

        int _barIdx1 = -1;
        int _barIdx2 = -1;

        int _beatIdx1 = -1;
        int _beatIdx2 = -1;

        Bar* _ptr;

        static AClipboard *currentClip;

    public:

        AClipboard():
            _type(ClipboardType::NotSet), _ptr(nullptr)
        {}

        static AClipboard* current() { return currentClip; }
        static void setCurrentClip(AClipboard* newClip) { currentClip=newClip; }


        void setClipboardType(ClipboardType newType) {
            _type = newType;
            if (_type == ClipboardType::NotSet)
                flush();
        }

        void flush() {
            _trackIdx1 = _barIdx1 = _beatIdx1 = -1;
            _trackIdx2 = _barIdx2 = _beatIdx2 = -1;
        }

        ClipboardType getClipboardType() const { return _type; }

        void setPtr(Bar *newPtr) { _ptr=newPtr; }
        Bar* getPtr() const { return _ptr; }

        void setBeginIndexes(int track, int bar, int beat=-1)
        {
            _trackIdx1 = track;
            _barIdx1 = bar;
            if (beat != -1)
                _beatIdx1 = beat;
        }

        void setEndIndexes(int track, int bar, int beat=-1)
        {
            _trackIdx2 = track;
            _barIdx2 = bar;
            if (beat != -1)
                _beatIdx2 = beat;
        }

        int getTrackIndex() const { return _trackIdx1; }
        int getBarIndex() const { return _barIdx1; }
        int getBeatIndex() const { return _beatIdx1; }


        int getSecondBarIdx() const { return _trackIdx2; }
        int getSecondBeatIdx() const { return _barIdx2; }
        int getSecondTrackIdx() const { return _beatIdx2; }
    };


}

#endif // TABCLIPBOARD_H
