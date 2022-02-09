#ifndef BEATSTRUCTS_H
#define BEATSTRUCTS_H

#include <vector>
#include <cinttypes>

namespace aurals {


    struct SingleChange {
        std::uint8_t changeType;
        size_t changeValue;
        std::uint8_t changeCount;
    };


    class ChangesList : public std::vector<SingleChange> {
        //search functions
    };


    struct ChordDiagram { //used for compatibility
        std::uint8_t header;
        std::uint8_t sharp;
        std::uint8_t blank1;
        std::uint8_t blank2;
        std::uint8_t blank3;

        std::uint8_t root;
        std::uint8_t minMaj;
        std::uint8_t steps9x;


        size_t bass;
        size_t deminush;

        std::uint8_t add;
        char name[20];

        std::uint8_t blank4;
        std::uint8_t blank5;

        std::uint8_t s5,s9,s11;

        size_t baseFrets;
        size_t frets[7];

        std::uint8_t baresNum;
        std::uint8_t fretBare[5];
        std::uint8_t startBare[5];
        std::uint8_t endBare[5];

        std::uint8_t o1,o3,o5,o7,o9,o11,o13;

        std::uint8_t blank6;

        std::uint8_t fingering[7];
        std::uint8_t showFing;
    };


    struct ChangeTable
    {
       std::uint8_t newInstr;
       std::uint8_t newVolume;
       std::uint8_t newPan;
       std::uint8_t newChorus;
       std::uint8_t newReverb;
       std::uint8_t newPhaser;
       std::uint8_t newTremolo;
       size_t newTempo;
       std::uint8_t instrDur;
       std::uint8_t volumeDur;
       std::uint8_t panDur;
       std::uint8_t chorusDur;
       std::uint8_t reverbDur;
       std::uint8_t phaserDur;
       std::uint8_t tremoloDur;
       std::uint8_t tempoDur;
       std::uint8_t changesTo; //one track or all of them
       std::uint8_t postFix;
    };
}


#endif // BEATSTRUCTS_H
