#include "Note.hpp"

#include <iostream>

using namespace aurals;


void Note::clone(Note *from)
{
    _fret = from->_fret;
    _volume = from->_volume;
    _noteState = from->_noteState;
    _stringNumber = from->_stringNumber;
    auto eff = from->getEffects();
    _effPack.mergeWith(eff);
}


Note& Note::operator=(Note *anotherNote) {
    clone(anotherNote);
    return *this;
}


void Note::printToStream(std::ostream &stream) const {
   stream << "Fret = " <<(int)this->_fret << std::endl;
}


const ABitArray& Note::getEffects() const {
    return _effPack;
}


ABitArray& Note::getEffectsRef() {
   return _effPack;
}


void Note::setEffect(Effect eff) {
    if (eff == Effect::None)
        _effPack.flush();
    else
        _effPack.setEffectAt(eff,true);
}


void Note::signStateLeeged() {
    if (_noteState <= 1)
        _noteState=4;
    if (_noteState==2)
        _noteState=6;
}
