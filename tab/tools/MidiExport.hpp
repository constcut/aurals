#ifndef MIDIEXPORT
#define MIDIEXPORT
#include "midi/MidiFile.hpp"
#include "tab/Tab.hpp"

namespace aurals {

    std::unique_ptr<MidiFile> exportMidi(Tab* tab, size_t shiftTheCursor=0);
    void exportTrack(Track* track, MidiTrack* midiTrack, size_t channel, size_t shiftCursorBar);
    void exportBeat(Beat* beat, MidiTrack* midiTrack, size_t channel, short specialRhy=0);
    bool exportSingalsFromNoteOn(Note* note, MidiTrack* midiTrack, std::uint8_t channel);
    bool exportSingalsFromNoteOff(Note* note, MidiTrack* midiTrack, std::uint8_t channel);
    void exportPostEffect(Beat* beat, MidiTrack* midiTrack, std::uint8_t channel);
    void pushBendToTrack(BendPoints* bend, MidiTrack* midiTrack, std::uint8_t channel);

}

#endif

