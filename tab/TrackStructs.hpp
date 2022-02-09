#ifndef TRACKSTRUCTS_HPP
#define TRACKSTRUCTS_HPP

#include <cstdint>
#include "Bar.hpp"

namespace aurals {

    class GuitarTuning {
        std::uint8_t stringsAmount;
        std::uint8_t tunes[32]; //32 strings max
    public:

        void setStringsAmount(std::uint8_t amount) { stringsAmount = amount; }
        std::uint8_t getStringsAmount() const { return stringsAmount; }

        void setTune(std::uint8_t index, std::uint8_t value) { if (index <= 32) tunes[index] = value; } //(index >= 0) &&
        std::uint8_t getTune(std::uint8_t index) const { if (index <= 32) return tunes[index]; return 0; }
    };


    class ChainedBars : public std::vector<Bar*> {
      public:
        ChainedBars() = default;
        virtual ~ChainedBars() = default;
    };

}


#endif // TRACKSTRUCTS_HPP
