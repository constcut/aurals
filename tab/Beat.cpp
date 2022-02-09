#include "Beat.hpp"
#include <iostream>


using namespace aurals;


void BendPoints::insertNewPoint(BendPoint bendPoint)
{
    for (size_t i = 0; i < size()-1; ++i)
    {
        if (at(i).horizontal < bendPoint.horizontal)
            if (at(i+1).horizontal > bendPoint.horizontal) {
                insert(begin() + i + 1, bendPoint);
                return;
            }
        if (at(i).horizontal == bendPoint.horizontal) {
            at(i).vertical = bendPoint.vertical;
            return;
        }
    }
    push_back(bendPoint);
}


void Beat::clone(Beat *from)
{
   auto eff = from->getEffects();
   _effPack.mergeWith(eff);
   _duration = from->_duration;
   _durationDetail = from->_durationDetail;
   _dotted = from->_dotted;
   _isPaused = from->_isPaused;
   _noticeText = from->_noticeText;

   _bookmarkName = from->_bookmarkName;

   for (size_t i = 0; i < from->size(); ++i) {
       Note *note = from->at(i).get();
       auto newNote = std::make_unique<Note>();
       newNote->clone(note);
       push_back(std::move(newNote));
   }
}


Beat& Beat::operator=(Beat *another) {
    clone(another);
    return *this;
}


void Beat::printToStream(std::ostream &stream) const
{
    stream << "Outputing #"<<size()<<" Notes."<<std::endl;
    for (size_t ind = 0; ind < size(); ++ind)
            at(ind)->printToStream(stream);
}


ABitArray Beat::getEffects() {
    return _effPack;
}


void Beat::setEffects(Effect eValue) {
    if (eValue == Effect::None)
         _effPack.flush();
    else
        _effPack.setEffectAt(eValue,true);
}


std::unique_ptr<Note> Beat::deleteNote(int string) {
    for (size_t i = 0; i < size(); ++i) {
        if (string == at(i)->getStringNumber()) {
            auto note = std::move(at(i));
            remove(i);
            if (size() == 0)
                setPause(true);
            return note;
        }
    }
    return {};
}


Note* Beat::getNote(int string) const {
    for (size_t i = 0; i < size(); ++i)
        if (at(i)->getStringNumber()==string)
            return at(i).get();
    return nullptr;
}


void Beat::setFret(std::uint8_t fret, int string)
{
    if (size() == 0) {
        auto newNote = std::make_unique<Note>();
        newNote->setFret(fret);
        newNote->setStringNumber(string);
        newNote->setState(0);
        push_back(std::move(newNote));
        setPause(false);
        return;
    }

    for (size_t i = 0; i < size(); ++i) {
        if (at(i)->getStringNumber()==string) {
            at(i)->setFret(fret);
            return; //function done
        }
        if (at(i)->getStringNumber() > string) {
            auto newNote = std::make_unique<Note>();
            newNote->setFret(fret);
            newNote->setStringNumber(string);
            newNote->setState(0);
            insertBefore(std::move(newNote),i);
            return;
        }
    }

    int lastStringN = at(size()-1)->getStringNumber();
    if (lastStringN < string) {
        auto newNote = std::make_unique<Note>();
        newNote->setFret(fret);
        newNote->setStringNumber(string);
        newNote->setState(0);
        push_back(std::move(newNote));
        return;
    }
}


std::uint8_t Beat::getFret(int string) const {
    if (size() == 0)
        return 255;
    for (size_t i = 0; i < size(); ++i)
        if (at(i)->getStringNumber()==string) {
            std::uint8_t fretValue = at(i)->getFret();
            return fretValue;
        }
    return 255;
}
