#include "Base.hpp"
#include "Threads.hpp"

#include <QDirIterator>

using namespace aurals;

auto addToMap = [](auto& container, auto value) {
    if (container.count(value))
        container[value] += 1;
    else
        container[value] = 1;
};




void BaseStatistics::reset() {
    _bpmStats.clear(); //TODO handlers
    _noteStats.clear();
    _midiNoteStats.clear();
    _drumNoteStats.clear();
    _barSizeStats.clear();
    _durStats.clear();
    _pauseDurStats.clear();
    _stringStats.clear();
    _fretStats.clear();
    _tuneStats.clear();
    _melStats.clear();
    _absMelStats.clear();
    _harmStats.clear();
    _absHarmStats.clear();
    _instrumentStats.clear();
    _notesVolumeStats.clear();
    _totalTracksStats.clear();
    _totalBarsStats.clear();
    _totalBeatsStats.clear();
    _totalNotesStats.clear();
    _noteEffectsStats.clear();
    _scalesStats.clear();
    _trackMostFreqNoteStats.clear();
    _trackSecondsLength.clear();
    _octaveStats.clear();
    _stringsFretsStats = std::vector<std::unordered_map<int16_t, size_t>>(7);
    _stringsEffectsStats = std::vector<std::unordered_map<std::string, size_t>>(7);
    _totalLength = 0;

    _barNumStats.clear();
    _barDenomStats.clear();
}



void BaseStatistics::makeBeatStats(std::unique_ptr<Beat>& beat, GuitarTuning& tune) {
    auto dur = beat->getDuration();
    if (beat->getPause()) {
        addToMap(_pauseDurStats, dur);
    }
    else {
        addToMap(_durStats, dur);

        if (beat->size() == 2) {
            auto& note1 = beat->at(0);
            auto stringNum1 = note1->getStringNumber();
            auto& note2 = beat->at(1);
            auto stringNum2 = note2->getStringNumber();
            auto midiNote1 = note1->getMidiNote(tune.getTune(stringNum1));
            auto midiNote2 = note2->getMidiNote(tune.getTune(stringNum2));
            int diff = midiNote2 - midiNote1;
            addToMap(_harmStats, diff);
            addToMap(_absHarmStats, std::abs(diff));
        }
    }
}


void BaseStatistics::makeNoteStats(std::unique_ptr<Note>& note, size_t beatSize, bool isDrums,
                   GuitarTuning& tune,int& prevNote) {

    auto stringNum = note->getStringNumber();
    if (isDrums == false) {
        addToMap(_stringStats, stringNum);
        auto midiNote = note->getMidiNote(tune.getTune(stringNum));

        if (beatSize == 1) {
            if (prevNote != -1) {
                int diff = midiNote - prevNote;
                addToMap(_melStats, diff);
                addToMap(_absMelStats, std::abs(diff));
            }
            prevNote = midiNote;
        }
        else
            prevNote = -1;

        addToMap(_midiNoteStats, midiNote);
        addToMap(_octaveStats, midiNote / 12);
        addToMap(_noteStats, midiNote % 12);
        addToMap(_trackScale, midiNote % 12); //Difference is in post processing
        addToMap(_fretStats, note->getFret());
        addToMap(_notesVolumeStats, note->getVolume());

        addToMap(_stringsFretsStats[stringNum-1], note->getFret());

        for (int16_t i = 1; i < 31; ++i)
            if (note->getEffects() == static_cast<Effect>(i)) {
                addToMap(_noteEffectsStats, _effectNames[i]);
                addToMap(_stringsEffectsStats[stringNum-1], _effectNames[i]);
            }
    }
    else {
        addToMap(_drumNoteStats, note->getFret());
    }
}


void BaseStatistics::addTuneStats(GuitarTuning& tune) {
    std::string tuneString;
    for (size_t tuneI = 0; tuneI < tune.getStringsAmount(); ++tuneI)
        tuneString += _noteNames[tune.getTune(tuneI) % 12];
    addToMap(_tuneStats, tuneString);
}


void BaseStatistics::makeTabStats(std::unique_ptr<Tab>& tab)
{
    static auto animationThr = std::make_unique<ThreadLocal>(); //TODO hide in class

    addToMap(_bpmStats, tab->getBPM());
    addToMap(_totalTracksStats, tab->size());

    for (size_t i = 0; i < tab->size(); ++i)
    {

        auto& track = tab->at(i);
        auto tune = track->getTuning();
        addToMap(_instrumentStats,track->getInstrument());

        if (track->isDrums() == false)
            addTuneStats(tune);

        if (i == 0) {
            addToMap(_totalBarsStats, track->size());

            animationThr->setupValues(tab.get(), track.get(), 0);
            int32_t tabLenSeconds = animationThr->calculateSeconds();
            addToMap(_trackSecondsLength, tabLenSeconds);
            _totalLength += tabLenSeconds;
        }

        int prevNote = -1;
        for (size_t barI = 0; barI < track->size(); ++barI) {
            auto& bar = track->at(barI);
            if (i == 0) {
                addToMap(_barSizeStats, static_cast<double>(bar->getSignNum()) /  bar->getSignDenum());

                addToMap(_barNumStats, bar->getSignNum());
                addToMap(_barDenomStats, bar->getSignDenum());
            }

            addToMap(_totalBeatsStats, bar->size());
            for (size_t beatI = 0; beatI < bar->size(); ++beatI) {
                auto& beat = bar->at(beatI);
                makeBeatStats(beat, tune);
                addToMap(_totalNotesStats, beat->size());
                for (size_t noteI = 0; noteI < beat->size(); ++noteI)
                    makeNoteStats(beat->at(noteI), beat->size(), track->isDrums(), tune, prevNote);
            }
        }
        if (track->isDrums() == false)
            addTrackScaleAndClear();
    }
}


std::string BaseStatistics::scaleStructure(int16_t freqNote) {
    auto foundIt = _trackScale.find(freqNote);
    std::string scaleString;

    int prevValue = -1;
    for (auto it = foundIt; it != _trackScale.end(); ++it) {
        if (prevValue == -1)
            prevValue = it->first;
        else {
            auto diff = it->first - prevValue;
            scaleString += std::to_string(diff);
            prevValue = it->first;
        }
    }

    if (foundIt != _trackScale.begin())
        for (auto it = _trackScale.begin(); it != foundIt; ++it) {
            auto diff = (it->first + 12) - prevValue;
            scaleString += std::to_string(diff);
            prevValue = it->first + 12;
        }

    return scaleString;
}


std::string nameDiatonicScale(std::string structure) {
    if (structure == "2212221")
        return "Ionian major";
    else if (structure == "2122212")
        return "Dorian minor";
    else if (structure == "1222122")
        return "Frigian minor";
    else if (structure == "2221221")
        return "Lidian major";
    else if (structure == "2212212")
        return "Miksolidian major";
    else if (structure == "2122122")
        return "Eolian minor";
    else if (structure == "1221222")
        return "Lokrian";
    return structure;
}


std::string namePentanotincScale(std::string structure) {
    if (structure == "22323")
        return "Major pentatonic";
    else if (structure == "32232")
        return "Minor pentatonic";
    else if (structure == "23232")
        return "Pentatonic V3";
    else if (structure == "32322")
        return "Pentatonic V4";
    else if (structure == "23223")
        return "Pentatonic V5";
    return structure;
}


void BaseStatistics::addTrackScaleAndClear() {

    if (_trackScale.empty())
        return;

    auto mostFrequent = std::max_element(_trackScale.begin(), _trackScale.end(),
    [](const std::pair<int, int>& lhs, const std::pair<int, int>& rhs) {
        return lhs.second < rhs.second; });

    std::string noteName = _noteNames[mostFrequent->first];
    addToMap(_trackMostFreqNoteStats, mostFrequent->first);

    if(_trackScale.size() == 7) {
        auto structure = scaleStructure(mostFrequent->first);
        addToMap(_scalesStats, nameDiatonicScale(structure) + " " + noteName);
    }
    else if (_trackScale.size() == 5) {
        auto structure = scaleStructure(mostFrequent->first);
        addToMap(_scalesStats, namePentanotincScale(structure) + noteName);
    }
    else if (_trackScale.size() == 12)
        addToMap(_scalesStats, "Chromatic " + noteName);
    else if (_trackScale.size() < 5)
        addToMap(_scalesStats, "Incomplete " + noteName);
    else if (_trackScale.size() == 6)
        addToMap(_scalesStats, "Pentatonic Modulation " + noteName);
    else
        addToMap(_scalesStats, "Diatonic Modulation " + noteName);
    _trackScale.clear();
}


void BaseStatistics::start(std::string path, size_t count, size_t skip) {

    _path = path;
    reset();
    const std::string basePath{path + "base"};
    size_t filesCount = 0;
    size_t fineFiles = 0;

    QDirIterator it(basePath.c_str());

    while (it.hasNext()) {
        ++filesCount;

        QString nextName = it.next();

        if (filesCount >= count)
            break;

        if (filesCount < skip)
            continue;

        std::string filePath = nextName.toStdString();
        if (_loader.open(filePath)) {
            auto tab = std::move(_loader.getTab());
            ++fineFiles;
            makeTabStats(tab);
        }
        if (filesCount % 100 == 0)
            std::cout << filesCount << " files done" << std::endl;

    }

    std::cout << "Total files processed: " << fineFiles << std::endl;
    writeAllCSV();
}


void BaseStatistics::writeAllCSV() {
    saveStats(_bpmStats, "bpm"); //TODO as hanlder uo map
    saveStats(_noteStats, "notes");
    saveStats(_midiNoteStats, "midiNotes");
    saveStats(_drumNoteStats, "drumNotes");
    saveStats(_durStats, "durSize");
    saveStats(_pauseDurStats, "pauseDurSize");
    saveStats(_stringStats, "strings");
    saveStats(_fretStats, "frets");
    saveStats(_tuneStats, "tunes");
    saveStats(_absMelStats, "absMelody");
    saveStats(_melStats, "melody");
    saveStats(_absHarmStats, "absHarmony");
    saveStats(_harmStats, "harmony");
    saveStats(_barSizeStats, "barSize");
    saveStats(_instrumentStats, "instruments");
    saveStats(_notesVolumeStats, "noteVolumes");
    saveStats(_totalTracksStats, "totalTracks");
    saveStats(_totalBarsStats, "totalBars");
    saveStats(_totalBeatsStats, "totalBeats");
    saveStats(_totalNotesStats, "totalNotes");
    saveStats(_noteEffectsStats, "noteEffects");
    saveStats(_scalesStats, "scales");
    saveStats(_trackMostFreqNoteStats, "trackMostFreqNote");
    saveStats(_trackSecondsLength, "trackSecondsLength");

    saveStats(_barNumStats, "barNums");
    saveStats(_barDenomStats, "barDenums");

    saveStats(_octaveStats, "octaves");


    for (size_t i = 0; i < _stringsFretsStats.size(); ++i) {
        saveStats(_stringsFretsStats[i], "string" + std::to_string(i+1) + "Frets");
        saveStats(_stringsEffectsStats[i], "string" + std::to_string(i+1) + "Effects");
    }

    const double minutes = _totalLength / 60.0;
    const double hours = minutes / 60.0;
    const double days = hours / 24.0;
    const double moonMonthes = days / 29.5;

    std::cout << "Total seconds of tabs processed: " << _totalLength
              << " minutes: " <<  minutes << " hours: " << hours
              << " days: " << days << std::endl;

    std::cout << "Moon monthes: " << moonMonthes << std::endl;
}
