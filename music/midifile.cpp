#include "midifile.h"


#include <iostream>
#include <fstream>
#include <algorithm>

#include <QDebug>


bool midiLog = false;

/*
void reverseEndian(void _*_ p,int s) {
    char *bytes = (char*)p;
    if (s == 4){
        char b = bytes[0];
        bytes[0] = bytes[3];
        bytes[3] = b;
        b = bytes[1];
        bytes[1] = bytes[2];
        bytes[2] = b;
    }
    if (s == 2) {
        char b = bytes[0];
        bytes[0] = bytes[1];
        bytes[1] = b;
    }
}*/ //TODO check midi file read-write stays the same - then wipe


template <class T>
void reverseEndian(T *objp) {
    unsigned char *memp = reinterpret_cast<unsigned char*>(objp);
    std::reverse(memp, memp + sizeof(T));
}


void writeReversedEndian(const char* in, size_t len, std::ofstream& file) {
    char buf[4];
    if (len == 2) {
        buf[0] = in[1];
        buf[1] = in[0];
    }
    else if (len == 4) {
        buf[0] = in[3];
        buf[1] = in[2];
        buf[2] = in[1];
        buf[3] = in[0];
    }
    file.write(buf, len);
}



bool MidiFile::readStream(std::ifstream & ifile)
{
    ifile.read((char*)_midiHeader.chunkId, 4);
    ifile.read((char*)&(_midiHeader.chunkSize), 4);
    reverseEndian(&(_midiHeader.chunkSize));
    ifile.read((char*)&(_midiHeader.formatType), 2);
    reverseEndian(&(_midiHeader.formatType));
    ifile.read((char*)&(_midiHeader.nTracks), 2);
    reverseEndian(&(_midiHeader.nTracks));
    ifile.read((char*)&(_midiHeader.timeDevision), 2);
    reverseEndian(&(_midiHeader.timeDevision));
	
	//allocate tracks:
	for (int nT=0; nT < _midiHeader.nTracks; ++nT)
	{
        auto midiTrack = std::make_unique<MidiTrack>();
        this->push_back(std::move(midiTrack));
	}

    if (midiLog)  qDebug() << "MidiHeader filled : " << _midiHeader.chunkId;
    if (midiLog)  qDebug() << " ch size: " << _midiHeader.
        chunkSize << "; tracks n:" << _midiHeader.nTracks;
    if (midiLog)  qDebug() << " time devision: " << _midiHeader.
        timeDevision << "; ftype:" << _midiHeader.formatType;

	if (_midiHeader.nTracks > 0)
        for (size_t i = 0; i < _midiHeader.nTracks; ++i)	// why was 1!!!&&
		{

            if (midiLog)  qDebug() << "Reading track " << i;

            ifile.read((char*)at(i)->trackHeader.chunkId, 4);
            at(i)->trackHeader.chunkId[4] = 0;

            if (midiLog)  qDebug() << "Track " << i << " header " << at(i)->trackHeader.chunkId;

            uint32_t trackSize = 0;
            ifile.read((char*)&trackSize, 4);
            reverseEndian(&trackSize);

            if (midiLog)  qDebug() << "Size of track " << trackSize;

            at(i)->trackHeader.trackSize = trackSize;
            size_t totalRead = 0;
			
            while (totalRead < trackSize) {
                auto singleSignal = std::make_unique<MidiSignal>(); //for those who have troubles in speach
                size_t signalBytesRead = singleSignal->readStream(ifile);
				totalRead += signalBytesRead;
                at(i)->push_back(std::move(singleSignal));
                if (midiLog)  qDebug() << "Cycle of events. Read " << totalRead << " of " << trackSize;
			}
            if (midiLog)  qDebug() <<"Track reading finished. "<<totalRead<<" from "<<trackSize;
        }

    return true;
}



void MidiFile::printToStream(std::ostream &stream)
{
	stream << "Output MidiFile.";
	stream << "chunky = " << _midiHeader.chunkId <<std::endl;
	stream << "Chunk Size = " << _midiHeader.chunkSize;
	stream << "; Format type = " << _midiHeader.formatType <<std::endl;
	stream << "Tracks amount = " << _midiHeader.nTracks;
	stream << "; TimeD = " << _midiHeader.timeDevision << std::endl;
	
	//then all the tracks
	for (short i = 0 ; i < _midiHeader.nTracks; ++i)
            at(i)->printToStream(stream);
	
	stream << "Printing finished for MidiFile" << std::endl;
}



size_t MidiFile::writeStream(std::ofstream &ofile) {

    size_t bytesWritten = 0;
    calculateHeader(); //also fills header of tracks
		
    ofile.write((const char*)_midiHeader.chunkId,4);
    writeReversedEndian((const char*)&_midiHeader.chunkSize, 4, ofile);
    writeReversedEndian((const char*)&_midiHeader.formatType,2, ofile);
    writeReversedEndian((const char*)&_midiHeader.nTracks,2, ofile);
    writeReversedEndian((const char*)&_midiHeader.timeDevision,2, ofile);

	bytesWritten += 14;
		
    for (short int i = 0; i < _midiHeader.nTracks; ++i) {

        ofile.write((const char*)at(i)->trackHeader.chunkId,4);
        writeReversedEndian((const char*)&at(i)->trackHeader.trackSize, 4, ofile);

		bytesWritten += 8;
        size_t amountOfEvents = at(i)->size();
		
		for (size_t j = 0; j < amountOfEvents; ++j)
            bytesWritten += at(i)->at(j)->writeStream(ofile);
	}
		
	return bytesWritten;		
}



size_t MidiFile::noMetricsTest(std::ofstream &ofile)
{
    size_t bytesWritten = 0;

    int tracks = 1;

    //write header
    ofile.write((const char*)_midiHeader.chunkId,4);
    writeReversedEndian((const char*)&_midiHeader.chunkSize, 4, ofile);
    writeReversedEndian((const char*)&_midiHeader.formatType,2, ofile);
    writeReversedEndian((const char*)&tracks,2, ofile);
    writeReversedEndian((const char*)&_midiHeader.timeDevision,2, ofile);

    bytesWritten += 14;

    calculateHeader(true);

    //for (short int i = 1; i < 2; ++i)
    {
        size_t i = 1;
        ofile.write((const char*)at(i)->trackHeader.chunkId,4);
        writeReversedEndian((const char*)&at(i)->trackHeader.trackSize, 2, ofile);

        bytesWritten += 8;

        size_t amountOfEvents = at(i)->size();

        for (size_t j = 0; j < amountOfEvents; ++j)
        {
            bytesWritten += at(i)->at(j)->writeStream(ofile,true);
        }
    }

    return bytesWritten;
}


bool MidiFile::calculateHeader(bool skip)
{
     size_t calculatedTracks = this->size();
     //NOTE this will work only with previusly loaded file
     if (midiLog)  qDebug() << "Calculating headers "<<calculatedTracks<<"-tracks.";
     _midiHeader.nTracks = calculatedTracks; //NOW PUSED

     //and here we fill header
     _midiHeader.chunkSize = 6;
     _midiHeader.formatType = 1;
     _midiHeader.timeDevision = 480; //LOT OF ATTENTION HERE WAS 480
            //and then  bpm*4;
     memcpy(_midiHeader.chunkId,"MThd",5);


     for (size_t i = 0; i < calculatedTracks; ++i)
     {
        at(i)->calculateHeader(skip);
     }

     return true;
}

