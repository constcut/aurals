#include "Effects.hpp"
//.....................Effects.........................................


using namespace aurals;

ABitArray::ABitArray():bits(0) {}


bool ABitArray::getEffectAt(Effect id) const {
    uint8_t index = static_cast<uint8_t>(id);
    size_t maskValue = 1;
    maskValue <<= (index-1); //-1
    return bits & maskValue;
}


bool ABitArray::operator==(Effect id) const {
    return getEffectAt(id);
}


bool ABitArray::operator!=(Effect id) const {
    return !getEffectAt(id);
}


void ABitArray::flush() {
    bits = 0;
}


void ABitArray::setEffectAt(Effect id, bool value)
{
    uint8_t index = static_cast<uint8_t>(id);
    size_t maskValue = 1;
    maskValue <<= (index - 1); //-1

    if (value == true) { //turn on
        bits |= maskValue;
    }
    else { //turn off
        size_t antiMask = 0;
        --antiMask;
        antiMask -= maskValue;
        bits &= antiMask;
    }
}


void ABitArray::mergeWith(ABitArray& addition) {
    bits |= addition.bits;
}


bool ABitArray::inRange(Effect lowId, Effect highId) const
{
    bool wasThere = false;
    uint8_t lowIndex = static_cast<uint8_t>(lowId);
    uint8_t highIndex = static_cast<uint8_t>(highId);
    for (std::uint8_t index=lowIndex; index != (highIndex+1); ++index) {
        Effect id = static_cast<Effect>(index);
        wasThere |= getEffectAt(id);
    }
    return wasThere;
}


bool ABitArray::empty() const {
    return bits == 0;
}


