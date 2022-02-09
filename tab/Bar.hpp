#ifndef BAR_H
#define BAR_H

#include "Chain.hpp"
#include "Beat.hpp"


namespace aurals {


    int translateDenum(std::uint8_t den);
    int translaeDuration(std::uint8_t dur);
    int updateDurationWithDetail(std::uint8_t detail, int base);

    class Track;

    class Bar : public ChainContainer<Beat, Track> {

    public:
        Bar() { flush(); }
        virtual ~Bar() = default;
        void flush();
        Bar &operator=(Bar *another);

        void printToStream(std::ostream& stream) const;

        virtual void push_back(std::unique_ptr<Beat> val) override;
        virtual void insertBefore(std::unique_ptr<Beat> val, int index=0) override;

    private:

        std::uint8_t _signatureNum = 4;
        std::uint8_t _signatureDenum = 4;

        std::uint8_t _repeat;
        std::uint8_t _repeatTimes;
        std::uint8_t _altRepeat;

        std::uint8_t _tonality;
        std::string _markerText;
        size_t _markerColor; //white byte == 1 if empty

        std::uint8_t _completeStatus;
        short _completeAbs;
        size_t _completeIndex;

    public:

        void setSignNum(std::uint8_t num) { _signatureNum = num; }
        void setSignDenum(std::uint8_t denum) { _signatureDenum = denum; }

        std::uint8_t getSignNum() const { return _signatureNum; }
        std::uint8_t getSignDenum() const { return _signatureDenum; }

        std::pair<uint8_t, uint8_t> countUsedSigns() const;

        std::uint8_t getCompleteStatus();
        double getCompleteAbs() const;
        size_t getCompleteIndex() const;

        void setRepeat(std::uint8_t rValue, std::uint8_t times = 0);
        std::uint8_t getRepeat() const { return _repeat; }
        std::uint8_t getRepeatTimes() const { return _repeatTimes; }

        void setAltRepeat(std::uint8_t number) { _altRepeat = number; }
        std::uint8_t getAltRepeat() const { return _altRepeat; }

        void setTonality(std::uint8_t tValue) { _tonality = tValue; }
        std::uint8_t getTonality() const { return _tonality; }

        void setMarker(const std::string& text, size_t color) { _markerText = text; _markerColor = color; }
        std::pair<std::string, size_t> getMarker() const { return {_markerText,  _markerColor}; }

        void clone(Bar *from);
    };


}

#endif // BAR_H
