#ifndef EFFECTS_H
#define EFFECTS_H

#include "Chain.hpp"

#include <map>


namespace aurals {


    enum class Effect {
        None = 0,
        Vibrato = 1,
        PalmMute = 2,
        Hammer = 3,
        Slide = 4,
        LegatoSlide = 5,
        SlideDownV1 = 6,
        SlideUpV1 = 7,
        SlideDownV2 = 8,
        SlideUpV2 = 9,
        Legato = 10,
        Harmonics = 11,
        HarmonicsV2 = 12,
        HarmonicsV3 = 13,
        HarmonicsV4 = 14,
        HarmonicsV5 = 15,
        HarmonicsV6 = 16,
        Bend = 17,
        LetRing = 18,
        Tremolo = 19,
        FadeIn = 20,
        GhostNote = 21,
        GraceNote = 22,
        Stokatto = 23,
        TremoloPick = 24,
        UpStroke = 25,
        DownStroke = 26,
        HeavyAccented = 27,
        Changes = 28,
        Tap = 29,
        Slap = 30,
        Pop = 31,
        Picking1 = 32,
        Picking2 = 33,
        Picking3 = 34,
        Picking4 = 35
    };


    class ABitArray {

    private:

        uint64_t bits;

    public:

        ABitArray();
        bool getEffectAt(Effect id) const;
        void setEffectAt(Effect id, bool value);

        size_t takeBits() const { return bits;}
        void putBits(size_t newBits) { bits = newBits; }

        bool inRange(Effect lowId, Effect highId) const;

        bool empty() const;

        bool operator==(Effect id) const;
        bool operator!=(Effect id) const;

        void flush();

        void mergeWith(ABitArray& addition);

        //TODO добавить свитчер (ReversedCommands)

    };

}



#endif // EFFECTS_H
