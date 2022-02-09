#ifndef NOTE_H
#define NOTE_H

#include <vector>

#include "NoteStructs.hpp"
#include "Effects.hpp"


namespace aurals {


    class Beat;

    class Note
    {

    public:

        void printToStream(std::ostream &stream) const;

        enum FretValues {
            emptyFret = 63,
            pauseFret = 62,
            ghostFret = 61,
            leegFret = 60
        };

        enum NoteState {
            normalNote = 0,
            leegNote = 2,
            deadNote = 3,
            leegedNormal = 4,
            leegedLeeg = 6
        };

        Note(): _fret(emptyFret), _volume(0), _fingering(0), _graceIsHere(false) {}
        virtual ~Note() = default;
        Note& operator=(Note *anotherNote);

    private:

        std::uint8_t _fret; //[0-63]; some last values used for special coding {pause, empty, leege note... ghost note(x)}
        std::uint8_t _volume;//[0-64]]-1);  values less than 14 used for ppp pp p m f ff fff - before -is precents for 2 per step
        std::uint8_t _fingering; //store information about finger to play - //fingers {none, L1,L2,L3,L4,L5, R1,R2,R3,R4,R5 +(12)(13)(14)(15)(16) ) - pressure hand for another there

        std::uint8_t _noteState;

        std::uint8_t _fingering1;
        std::uint8_t _fingering2;

        std::uint8_t _stringNumber;

        Note* _prevNote;
        Note* _nextNote; //Возможно шаблон от которого наследоваться prev/next/parent? до chain

        ABitArray _effPack;

        bool _graceIsHere; //Возможно достаточно флага в эффектах
        GraceNote _graceStats;

        BendPoints _bend;

    public:

        void setNext(Note *nextOne) { _nextNote = nextOne; }
        void setPrev(Note *prevOne) { _prevNote = prevOne; }

        Note *getNext() const { return _nextNote; }
        Note *getPrev() const { return _prevNote; }


        void setStringNumber(std::uint8_t num) {_stringNumber = num;}
        std::uint8_t getStringNumber() const { return _stringNumber; }

        void setFret(std::uint8_t fValue) { _fret = fValue; }
        std::uint8_t getFret() const { return _fret; }

        int getMidiNote(int tune = 0) const { return _fret + tune; }

        void setState(std::uint8_t nState) { _noteState = nState; }
        void signStateLeeged();
        std::uint8_t getState() const { return _noteState;}

        void setVolume(std::uint8_t vValue) { _volume = vValue; }
        std::uint8_t getVolume() const { return _volume; }

        void setFingering(std::uint8_t fValue) {_fingering = fValue;}
        std::uint8_t getFingering() const { return _fingering; }

        void setEffect(Effect eff);
        const ABitArray &getEffects() const;
        ABitArray& getEffectsRef();
        void addEffects(ABitArray &append) { _effPack.mergeWith(append); }

        BendPoints* getBendPtr() { return &_bend; }

        void setFingering1(std::uint8_t fValue) { _fingering1 = fValue; } //Older versions
        void setFingering2(std::uint8_t fValue) { _fingering2 = fValue; }
        std::uint8_t getFingering1() const { return _fingering1; }
        std::uint8_t getFingering2() const { return _fingering2; }

        void setGraceStats(GraceNote stats) { _graceStats = stats; }
        GraceNote getGraceStats() const { return _graceStats; }

        bool isGraceNote() const { return _graceIsHere; }
        void setGraceNote(bool flag) { _graceIsHere = flag; }

        void clone(Note *from);
    };

}


#endif // NOTE_H
