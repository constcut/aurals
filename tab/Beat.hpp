#ifndef BEAT_H
#define BEAT_H

#include <memory.h>
#include <vector>
#include <string>

#include "BeatStructs.hpp"
#include "Chain.hpp"
#include "Note.hpp"


namespace aurals {


    class Bar;

    class Beat : public ChainContainer<Note, Bar> {

    public:

        Beat() = default;
        virtual ~Beat() = default;
        Beat &operator=(Beat *another);

        void printToStream(std::ostream &stream) const;

    private:

        std::uint8_t _duration = 0; // 2 1 . 2 4 8 16 32 64 [8 values] - 3 bits
        std::uint8_t _durationDetail = 0; // none, dot, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 [4 bits] //one of 2-15 means empty(together with pause only!)
        bool _isPaused;

        std::uint8_t _dotted = 0; //0 1 2

        std::string _bookmarkName;
        std::string _noticeText;

        ABitArray _effPack;
        BendPoints _tremolo;
        ChordDiagram _chordDiagram;
        ChangesList _changes;

    public:

        void setPause(bool pause) { _isPaused = pause; }
        bool getPause() const { return _isPaused; }

        std::unique_ptr<Note> deleteNote(int string);
        Note* getNote(int string) const;

        void setFret(std::uint8_t fret, int string);
        std::uint8_t getFret(int string) const;

        void setDuration(std::uint8_t dValue) { _duration = dValue; }
        void setDurationDetail(std::uint8_t dValue) {	_durationDetail = dValue; }
        void setDotted(std::uint8_t dottedValue) { _dotted = dottedValue; }

        std::uint8_t getDuration() const { return _duration;}
        std::uint8_t getDurationDetail() const { return _durationDetail; }
        std::uint8_t getDotted() const { return _dotted; }

        void setEffects(Effect eValue);
        ABitArray getEffects(); //TODO TREMOLO data missing in beat

        BendPoints* getTremoloPtr() { return &_tremolo; }
        ChangesList* getChangesPtr() { return &_changes; }

        //Реализовать более аккуратное хранение и отображение\редактирование
        void setChordDiagram(char* area) { memcpy(&_chordDiagram, area, sizeof(_chordDiagram)); }
        bool getChordDiagram(char* to) const { memcpy(to, &_chordDiagram, sizeof(_chordDiagram)); return true; }

        void setText(std::string& value) { _noticeText = value; }
        void getText(std::string& value) const { value = _noticeText; }

        void clone(Beat *from);
    };


}

#endif // BEAT_H
