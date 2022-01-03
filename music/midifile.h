#ifndef MIDIFILE_H
#define MIDIFILE_H

#include <vector>
#include <iostream>
#include <memory>

#include "miditrack.h"



class MidiFile : public std::vector<std::unique_ptr<MidiTrack>> {

public:
    struct _midiHeader {
        char chunkId[5]; //4
        int32_t chunkSize; //4b
        int16_t formatType; //2
        int16_t nTracks; //2
        int16_t timeDevision; //2
        //read from stream or string
    };

protected:

    struct _midiHeader _midiHeader;
    int _bpm;
    
public:

    MidiFile():_bpm(120) {}
    virtual ~MidiFile() = default;
    
    bool calculateHeader(bool skip=false);
    void printToStream(std::ostream &stream);

    bool readStream(std::ifstream &ifile);
    size_t writeStream(std::ofstream &ofile);

    size_t noMetricsTest(std::ofstream &ofile);

    void setBPM(int bpmNew) { _bpm = bpmNew; }
    int  getBPM() { return _bpm; }

};

#endif // MIDIFILE_H
