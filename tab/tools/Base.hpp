#ifndef BASE_H
#define BASE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include <iostream>
#include <fstream>
#include <map>

#include "tab/tools/TabLoader.hpp"
#include "tab/Tab.hpp"



namespace aurals {


    class BaseStatistics {

    public:

        void start(std::string path, size_t count, size_t skip=0);

    private:

        std::string _path;

        std::unordered_map<int16_t, size_t> _bpmStats;
        std::unordered_map<int16_t, size_t> _noteStats;
        std::unordered_map<int16_t, size_t> _midiNoteStats;
        std::unordered_map<int16_t, size_t> _octaveStats;
        std::unordered_map<int16_t, size_t> _drumNoteStats;

        std::unordered_map<double, size_t> _barSizeStats;
        std::unordered_map<int16_t, size_t> _barNumStats;
        std::unordered_map<int16_t, size_t> _barDenomStats;

        std::unordered_map<int16_t, size_t> _durStats;
        std::unordered_map<int16_t, size_t> _pauseDurStats;
        std::unordered_map<int16_t, size_t> _stringStats;
        std::unordered_map<int16_t, size_t> _fretStats;
        std::unordered_map<std::string, size_t> _tuneStats;
        std::unordered_map<int, size_t> _melStats;
        std::unordered_map<int16_t, size_t> _absMelStats;
        std::unordered_map<int, size_t> _harmStats;
        std::unordered_map<int16_t, size_t> _absHarmStats;
        std::unordered_map<int16_t, size_t> _instrumentStats;
        std::unordered_map<int16_t, size_t> _notesVolumeStats;

        std::unordered_map<int16_t, size_t> _totalTracksStats;
        std::unordered_map<int16_t, size_t> _totalBarsStats;
        std::unordered_map<int16_t, size_t> _totalBeatsStats;
        std::unordered_map<int16_t, size_t> _totalNotesStats;

        std::unordered_map<std::string, size_t> _noteEffectsStats;
        std::unordered_map<std::string, size_t> _scalesStats;
        std::unordered_map<int16_t, size_t> _trackMostFreqNoteStats;
        std::map<int16_t, size_t> _trackScale;

        std::unordered_map<int32_t, size_t> _trackSecondsLength;
        int64_t _totalLength;

        std::vector<std::unordered_map<int16_t, size_t>> _stringsFretsStats;
        std::vector<std::unordered_map<std::string, size_t>> _stringsEffectsStats;


        const std::vector<std::string> _noteNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
        const std::vector<std::string> _durationNames = {"2/1", "1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/64", "1/128"};
        const std::vector<std::string> _effectNames = {"None", "Vibrato", "PalmMute", "Unknown", "LegatoSlide", "Slide", "SlideDown1",
                                                     "SlideUp1", "SlideDown2", "SlideUp2", "Legato", "Harmonics", "HarmoincsV2",
                                                     "HarmonicsV3", "HarmonicsV4", "HarmonicsV5", "HarmonicsV6", "Bend", "LetRing",
                                                     "Tremolo", "FadeIn", "GhostNote", "GraceNote", "Stokatto", "TremoloPick",
                                                     "UpStroke", "DownStroke", "Accented", "Changed", "Tap", "Slap", "Pop"};

        GTabLoader _loader;

        void reset();

        void makeTabStats(std::unique_ptr<Tab>& tab);
        void makeBeatStats(std::unique_ptr<Beat>& beat, GuitarTuning& tune);
        void makeNoteStats(std::unique_ptr<Note>& note, size_t beatSize, bool isDrums,
                           GuitarTuning& tune,int& prevNote);

        void addTuneStats(GuitarTuning& tune);

        void addTrackScaleAndClear();
        std::string scaleStructure(int16_t freqNote);

        void writeAllCSV();

        template<typename T>
        void saveStats(std::unordered_map<T, size_t>&container, std::string name) {
            std::cout << container.size() << " " << name << std::endl;
            std::ofstream os(_path + "/csv/" + name + ".csv");
            os << "value,count" << std::endl;
            using ValuePair = std::pair<T, size_t>;
            std::vector<ValuePair> sortedData(container.begin(), container.end());
            std::sort(sortedData.begin(), sortedData.end(), [](auto lhs, auto rhs) { return lhs.second > rhs.second; });
            for (auto& p: sortedData)
                os << p.first << "," << p.second << std::endl;
        }
    };

}

#endif // BASE_H
