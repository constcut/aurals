#ifndef TAB_H
#define TAB_H

#include <unordered_map>
#include <list>
#include <cassert>

#include "TabStructs.hpp"
#include "Chain.hpp"
#include "tab/tools/Commands.hpp"
#include "Track.hpp"


namespace aurals {

    int updateDurationWithDetail(std::uint8_t detail, int base);

    class Track;

    class Tab : public ChainContainer<Track, void>
    {
    public:

        Tab() :_isPlaying(false), _currentTrack(0),
            _currentBar(0), _displayBar(0) {}

        virtual ~Tab() = default;
        Tab &operator=(Tab another);

        void printToStream(std::ostream &stream) const;

        void createTimeLine(size_t shiftTheCursor = 0);
        std::uint8_t getBPMStatusOnBar(size_t barN) const;
        int getBpmOnBar(size_t barN) const;

        void connectTracks();
        void postLoading();

        const std::vector<TimeLineKnot>& getTimeLine() { return _timeLine; }


     private:
        int _bpmTemp = 120;
        bool _isPlaying = false;

        std::string _origin;
        std::uint8_t _signKey;
        std::uint8_t _signOctave;
        std::uint8_t _tripletFeel;

        std::vector<TimeLineKnot> _timeLine;

        MidiChannelInfo _midiChannels[64];
        VariableStrings _variableInforation; //Возможно загружать и давать хранить

    public:

        void copyMidiChannelInfo(char* src) { assert(sizeof(_midiChannels) == 768); memcpy(_midiChannels, src, 768); }

        int getBPM() const { return _bpmTemp; }
        void setBPM(int newBPM) { _bpmTemp = newBPM; }

        bool playing() const { return _isPlaying; }
        void setPlaying(bool v) { _isPlaying = v; }

    private: //TODO всё перенести в TabCommandHandler или TabView и сделать friend

        size_t _currentTrack;
        size_t _currentBar;
        size_t _displayBar;

    public:

        size_t& getCurrentTrack() { return _currentTrack; }
        size_t& getCurrentBar() { return _currentBar; }
        size_t& getDisplayBar() { return _displayBar; }

        void onTabCommand(TabCommand command);

        void createNewTrack();

        void midiPause();
        void openReprise();
        void closeReprise();

        void moveCursorInTrackRight();
        void moveCursorInTrackLeft();

        void setSignsTillEnd(size_t num, size_t denom);
        void changeTrackVolume(size_t newVol);
        void changeTrackName(std::string newName);
        void changeTrackInstrument(size_t val);
        void changeTrackPanoram(size_t val);
        void setMarker(std::string text);

        void gotoBar(size_t pos);
        void saveAs(std::string filename);
        void closeReprise(size_t count);


        void addMacro(TrackCommand command) {
            _macroCommands.push_back(command);
        }
        void addMacro(StringCommand<TrackCommand> command) {
            _macroCommands.push_back(command);
        }
        void addMacro(TwoIntCommand<TrackCommand> command) {
            _macroCommands.push_back(command);
        }

        void playCommand(MacroCommand& command) {
            if (std::holds_alternative<TabCommand>(command)) {
                onTabCommand(std::get<TabCommand>(command));
            }
            else if (std::holds_alternative<TrackCommand>(command)) {
                at(_currentTrack)->onTrackCommand(std::get<TrackCommand>(command));
            }
            else if (std::holds_alternative<IntCommand<TabCommand>>(command)) {
                auto paramCommand = std::get<IntCommand<TabCommand>>(command);
                if (_intHandlers.count(paramCommand.type))
                    (this->*_intHandlers.at(paramCommand.type))(paramCommand.parameter);
            }
            else if (std::holds_alternative<TwoIntCommand<TabCommand>>(command)) {
                auto paramCommand = std::get<TwoIntCommand<TabCommand>>(command);
                if (_twoIntHandlers.count(paramCommand.type))
                    (this->*_twoIntHandlers.at(paramCommand.type))(paramCommand.parameter1, paramCommand.parameter2);
            } else if (std::holds_alternative<StringCommand<TabCommand>>(command)) {
                auto paramCommand = std::get<StringCommand<TabCommand>>(command);
                if (_stringHandlers.count(paramCommand.type))
                    (this->*_stringHandlers.at(paramCommand.type))(paramCommand.parameter);
            }
            else {
                at(_currentTrack)->playCommand(command);
            }

        }

        const std::vector<MacroCommand>& getMacro() const { return _macroCommands; }

    private:
        std::unordered_map<TabCommand, void (Tab::*)()> _handlers =  {
            {TabCommand::MoveRight, &Tab::moveCursorInTrackRight},
            {TabCommand::MoveLeft, &Tab::moveCursorInTrackLeft},
            {TabCommand::PauseMidi, &Tab::midiPause},
            {TabCommand::OpenReprise, &Tab::openReprise},
            {TabCommand::NewTrack, &Tab::createNewTrack}};


        std::unordered_map<TabCommand, void (Tab::*)(size_t)> _intHandlers =  {
            {TabCommand::Instument, &Tab::changeTrackInstrument},
            {TabCommand::GotoBar, &Tab::gotoBar},
            {TabCommand::CloseReprise, &Tab::closeReprise},
            {TabCommand::Panoram, &Tab::changeTrackPanoram},
            {TabCommand::Volume, &Tab::changeTrackVolume},
        };

        std::unordered_map<TabCommand, void (Tab::*)(size_t, size_t)> _twoIntHandlers =  {
            {TabCommand::SetSignTillEnd, &Tab::setSignsTillEnd},
        };

        std::unordered_map<TabCommand, void (Tab::*)(std::string)> _stringHandlers =  {
            {TabCommand::Name, &Tab::changeTrackName},
            {TabCommand::Name, &Tab::saveAs}
        };

        std::vector<MacroCommand> _macroCommands;
    };



    int translateDenum(std::uint8_t den);
    int translaeDuration(std::uint8_t dur);

}

#endif // TAB_H
