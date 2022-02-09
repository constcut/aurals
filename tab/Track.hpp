#ifndef TRACK_H
#define TRACK_H

#include <unordered_map>

#include "Chain.hpp"
#include "TrackStructs.hpp"
#include "tools/Commands.hpp" //move into g0
#include "Bar.hpp"


namespace aurals {


    class Tab;

    enum class NoteStates {
        Leeg = 2,
        Dead = 3,
    };

    class Track : public ChainContainer<Bar, Tab>
    {
    public:

        Track();
        virtual ~Track() = default;

        void printToStream(std::ostream &stream) const;

        Track &operator=([[maybe_unused]]Track another);

        virtual void push_back(std::unique_ptr<Bar> val) override;
        virtual void insertBefore(std::unique_ptr<Bar> val, int index=0) override;

    private:

        std::string _name;
        size_t _instrument; //Enum
        size_t _color; //Здесь другой тип
        std::uint8_t _pan;
        std::uint8_t _volume;
        bool _drums;

        size_t _midiInfo[4]; //Port,Channel,ChannelE,Capo структуру? или пропускать?
        size_t _beatsAmount;
        std::uint8_t _status; //0 - none 1 - mute 2 - solo - enum

        GuitarTuning _tuning;

        std::vector<Bar*> _timeLoop;
        std::vector<size_t> _timeLoopIndexStore;

    public:

        size_t connectBars();
        size_t connectBeats();
        size_t connectNotes(); //for let ring
        size_t connectTimeLoop();
        void connectAll();

        const std::vector<Bar*>& getTimeLoop() const { return _timeLoop; }
        const std::vector<size_t>& getTimeLoopIndexes() const { return _timeLoopIndexStore; }

        void pushReprise(Bar *beginRepeat, Bar *endRepeat,
                         Bar *preTail, Bar *tailBegin, Bar *tailEnd, size_t beginIndex, size_t endIndex,
                         size_t preTailIndex=0, size_t tailBeginIndex=0, size_t tailEndIndex=0);

        void setName( std::string &nValue) { _name = nValue; }
        std::string getName() const { return _name; }

        void setInstrument(size_t iValue) { _instrument = iValue;}
        size_t getInstrument() const { return _instrument; }

        void setColor(size_t cValue) { _color = cValue; }
        size_t getColor() const { return _color; }

        void setPan(std::uint8_t pValue) { _pan = pValue; }
        std::uint8_t getPan() const { return _pan; }

        void setVolume(std::uint8_t vValue) { _volume = vValue; }
        std::uint8_t getVolume() const { return _volume; }


        void setMidiInfo(size_t index, size_t value) { _midiInfo[index] = value; }
        size_t getMidiInfo(size_t index) { return _midiInfo[index]; } //Возможно просто избавиться

        void setDrums(bool newDrums) {
            _drums = newDrums;
            if (_drums) _midiInfo[3]=99;
        }

        bool isDrums() const { return _drums; }

        std::uint8_t getStatus() const { return _status; }
        void setStatus(std::uint8_t newStat) { _status = newStat; }

        const GuitarTuning& getTuning() const { return _tuning; }
        GuitarTuning& getTuningRef() { return _tuning; }

    private: //всё перенести в TrackCommandHandler или TrackView и сделать friend
        size_t _cursor;
        size_t _cursorBeat;
        size_t _stringCursor;
        size_t _displayIndex;
        size_t _lastSeen;
        int _selectCursor; //Решить проблемы int может быть достаточно просто bool флага?
        int _selectionBarFirst;
        int _selectionBarLast;
        int _selectionBeatFirst;
        int _selectionBeatLast;
        int _digitPress;


    public:
        size_t& cursor() { return _cursor; } //TODO после всего рефакторинга обязаны быть const (нужны для комманд)
        size_t& cursorBeat() { return _cursorBeat; }
        size_t& stringCursor() { return _stringCursor; }
        size_t& displayIndex() { return _displayIndex; }
        size_t& lastSeen() { return _lastSeen; }
        int& selectCursor() { return _selectCursor;}
        int& selectBarFirst() { return _selectionBarFirst;}
        int& selectBarLast() { return _selectionBarLast;}
        int& selectBeatFirst() { return _selectionBeatFirst;}
        int& selectBeatLast() { return _selectionBeatLast;}
        int& digitPress() { return _digitPress; }


        void switchEffect(Effect effect);
        void switchBeatEffect(Effect effect);
        void switchNoteState(NoteStates changeState);
        void reverseCommand(ReversableCommand command);

        std::vector<ReversableCommand> commandSequence;

        void onTrackCommand(TrackCommand command);

        void gotoTrackStart();

        void moveSelectionLeft();
        void moveSelectionRight();
        void insertBar();
        void insertNewPause();
        void moveToNextBar();
        void moveToPrevBar();
        void moveToPrevPage();
        void moveToNextPage();

        void moveToStringUp();
        void moveToStringDown();
        void moveToPrevBeat();
        void moveToNextBeat();
        void setTrackPause();
        void deleteBar();
        void deleteSelectedBars();
        void deleteSelectedBeats();
        void deleteNote();
        void incDuration();
        void decDuration();
        void saveFromTrack();
        void newBar();
        void setDotOnBeat();
        void setTriolOnBeat();
        //void setBendOnNote(); //TODO + changes + chord

        void setTextOnBeat(std::string newText);
        void setBarSign(size_t num, size_t denom);
        void changeBarSigns(size_t num, size_t denom);

        void clipboardCopyBar(); //refact name
        void clipboarCopyBeat();
        void clipboardCopyBars();
        void clipboardCutBar();
        void clipboardPaste();
        void undoOnTrack();


        void LeegNote();
        void DeadNote();
        void Vibratto();
        void Slide();
        void Hammer();
        void LetRing();
        void PalmMute();
        void Harmonics();
        void TremoloPicking();
        void Trill();
        void Stokatto();
        void FadeIn(); //Todo fade out
        void Accent();
        void HeavyAccent();
        void UpStroke();
        void DownStroke();


        void playCommand(MacroCommand& command) {
            if (std::holds_alternative<TwoIntCommand<TrackCommand>>(command)) {
                auto paramCommand = std::get<TwoIntCommand<TrackCommand>>(command);
                if (twoIntHandlers.count(paramCommand.type))
                    (this->*twoIntHandlers.at(paramCommand.type))(paramCommand.parameter1, paramCommand.parameter2);
            } else if (std::holds_alternative<StringCommand<TrackCommand>>(command)) {
                auto paramCommand = std::get<StringCommand<TrackCommand>>(command);
                if (stringHandlers.count(paramCommand.type))
                    (this->*stringHandlers.at(paramCommand.type))(paramCommand.parameter);
            }
        }


    private:

        std::unordered_map<TrackCommand, void (Track::*)()> handlers =  {
            {TrackCommand::GotoStart, &Track::gotoTrackStart},
            {TrackCommand::SelectionExpandLeft, &Track::moveSelectionLeft},
            {TrackCommand::SelectionExpandRight, &Track::moveSelectionRight},
            {TrackCommand::InsertBar, &Track::insertBar},
            {TrackCommand::NextBar, &Track::moveToNextBar},
            {TrackCommand::PrevBar, &Track::moveToPrevBar},
            {TrackCommand::NextPage, &Track::moveToNextPage},
            {TrackCommand::PrevPage, &Track::moveToPrevPage},
            {TrackCommand::StringDown, &Track::moveToStringDown},
            {TrackCommand::StringUp, &Track::moveToStringUp},
            {TrackCommand::PrevBeat, &Track::moveToPrevBeat},
            {TrackCommand::NextBeat, &Track::moveToNextBeat},
            {TrackCommand::SetPause, &Track::setTrackPause},
            {TrackCommand::DeleteBar, &Track::deleteBar},
            {TrackCommand::DeleteNote, &Track::deleteNote},
            {TrackCommand::DeleteSelectedBars, &Track::deleteSelectedBars},
            {TrackCommand::DeleteSelectedBeats, &Track::deleteSelectedBeats},
            {TrackCommand::IncDuration, &Track::incDuration},
            {TrackCommand::DecDuration, &Track::decDuration},
            {TrackCommand::NewBar, &Track::newBar},
            {TrackCommand::SetDot, &Track::setDotOnBeat},
            {TrackCommand::SetTriole, &Track::setTriolOnBeat},
            {TrackCommand::Leeg, &Track::LeegNote},
            {TrackCommand::Dead, &Track::DeadNote},
            {TrackCommand::Vibrato, &Track::Vibratto},
            {TrackCommand::Slide, &Track::Slide},
            {TrackCommand::Hammer, &Track::Hammer},
            {TrackCommand::LetRing, &Track::LetRing},
            {TrackCommand::PalmMute, &Track::PalmMute},
            {TrackCommand::Harmonics, &Track::Harmonics},
            {TrackCommand::TremoloPickings, &Track::TremoloPicking},
            {TrackCommand::Trill, &Track::Trill},
            {TrackCommand::Stokatto, &Track::Stokatto},
            {TrackCommand::FadeIn, &Track::FadeIn},
            {TrackCommand::Accent, &Track::Accent},
            {TrackCommand::HeaveAccent, &Track::HeavyAccent},
            {TrackCommand::UpStroke, &Track::UpStroke},
            {TrackCommand::DownStroke, &Track::DownStroke},
            {TrackCommand::Cut, &Track::clipboardCutBar},
            {TrackCommand::Copy, &Track::clipboardCopyBar},
            {TrackCommand::CopyBeat, &Track::clipboarCopyBeat},
            {TrackCommand::CopyBars, &Track::clipboardCopyBars},
            {TrackCommand::Paste, &Track::clipboardPaste},
            {TrackCommand::Undo, &Track::undoOnTrack},
            };


        std::unordered_map<TrackCommand, void (Tab::*)(size_t)> intHandlers =  {
        };

        std::unordered_map<TrackCommand, void (Track::*)(size_t, size_t)> twoIntHandlers =  {
            {TrackCommand::SetSignForSelected , &Track::changeBarSigns},
            {TrackCommand::SetBarSign , &Track::setBarSign}
        };

        std::unordered_map<TrackCommand, void (Track::*)(std::string)> stringHandlers =  {
            {TrackCommand::Text, &Track::setTextOnBeat},
        };

    };

}

#endif // TRACK_H
