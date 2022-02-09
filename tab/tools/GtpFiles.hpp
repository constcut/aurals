#ifndef GTPFILES_H
#define GTPFILES_H

#include "../Tab.hpp"


namespace aurals {


    void readBend(std::ifstream *file, BendPoints *bend);
    void writeBend(std::ofstream *file, BendPoints *bend);


    class Gp4Export //: public AExport
    {
        public:

        bool exPort(std::ofstream &file, Tab &tab);
    };

    class Gp3Import
    {

    public:
        bool import(std::ifstream &file, Tab *tab, std::uint8_t knownVersion=0);
    };


    class Gp4Import
    {
    public:
        bool import(std::ifstream &file, Tab *tab, std::uint8_t knownVersion=0);

        //readString
        //readBend

        //readChordDiagram
        //readBeatEffects
        //readNoteEffects
        //readMixTable
        //readGraceNote
        //readTrack
        //readBeat
        //readNote

    };


    //GP5

    class Gp5Import
    {
    public:
        bool import(std::ifstream &file, Tab *tab, std::uint8_t knownVersion=0);

        //readString
        //readBend

        //readChordDiagram
        //readBeatEffects
        //readNoteEffects
        //readMixTable
        //readGraceNote
        //readTrack
        //readBeat
        //readNote

    };

}


#endif // GTPFILES_H
