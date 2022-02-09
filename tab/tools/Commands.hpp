#ifndef COMMANDS_H
#define COMMANDS_H

#include <cstdint>
#include <vector>
#include <memory>
#include <variant>
#include <string>
#include <fstream>

#include <QObject>

#include "tab/Note.hpp" //needed only because tab\track commands are not sepparated with reversable - split in 2 headers TODO
#include "tab/Beat.hpp"
#include "tab/Bar.hpp"




namespace aurals {


    enum CommandPack { //Типы комманд
        SingleTabCommand=0,
        SingleTrackCommand,
        IntTabCommand,
        IntTrackCommand,
        StringTabCommand,
        StringTrackCommand,
        TwoIntTabCommand,
        TwoIntTrackCommand
    };


    Q_NAMESPACE
    enum class TabCommand { //Make all reversable
        SetSignTillEnd,
        SaveAs, //Move to QML
        Mute, //Make reversable use from tab
        Solo, //Make reversable use from tab
        MoveLeft,
        MoveRight,
        MoveUp,
        MoveDown,
        Drums, //Make reversable use from tab
        Instument, //Make reversable use from tab
        Panoram,//Make reversable use from tab
        Volume, //Make reversable use from tab
        Name, //Make reversable use from tab
        BPM,
        NewTrack,
        DeleteTrack, //Undo?
        PauseMidi,
        AddMarker,
        OpenReprise,
        CloseReprise,
        GotoBar,
        Tune
    };
    Q_ENUM_NS(TabCommand)


    Q_NAMESPACE
    enum class TrackCommand {
        GotoStart,
        SetSignForSelected,
        SelectionExpandLeft,
        SelectionExpandRight,
        InsertBar,
        NextBar,
        PrevBar,
        NextPage,
        PrevPage,
        StringDown,
        StringUp,
        PrevBeat,
        NextBeat,
        SetPause,
        DeleteBar,
        DeleteSelectedBars,
        DeleteSelectedBeats,
        DeleteNote,
        IncDuration,
        DecDuration,
        NewBar,
        SetDot,
        SetTriole,
        Leeg,
        Dead,
        Vibrato,
        Slide,
        Hammer,
        LetRing,
        PalmMute,
        Harmonics,
        TremoloPickings,
        Trill,
        Stokatto,
        FadeIn,
        Accent,
        HeaveAccent,
        Bend, //Request must come from qml
        Chord, //But we need save reversable ^ (both)
        Text,
        Changes, //As above
        UpStroke,
        DownStroke,
        SetBarSign,
        Cut,
        Copy,
        CopyBeat,
        CopyBars,
        Paste,
        Undo,

        PrevLine,
        NextLine,

        //Next commands yet not implemented
        QuickOpen,
        Tapping,
        Slap,
        Pop,
        UpM,
        DownM,
        Figering
    };
    Q_ENUM_NS(TrackCommand)



    template<typename CommandType>
    struct StringCommand {
        CommandType type;
        std::string parameter;
    };

    template<typename CommandType>
    struct IntCommand {
        CommandType type;
        size_t parameter;
    };

    template<typename CommandType>
    struct TwoIntCommand {
        CommandType type;
        size_t parameter1;
        size_t parameter2;
    };


    using MacroCommand = std::variant<TabCommand, TrackCommand, StringCommand<TabCommand>,
        IntCommand<TabCommand>, TwoIntCommand<TabCommand>, StringCommand<TrackCommand>, TwoIntCommand<TrackCommand>>;


    void saveMacroComannds(const std::vector<MacroCommand>& commands, std::ofstream& os);
    std::vector<MacroCommand> loadMacroCommands(std::ifstream& is);


    enum class ReversableType { //Если будет конфликт имён убрать в клас хранитель
        Empty = 0,
        SwitchEffectNote = 1,
        SwitchEffectBeat = 2,
        SetFret = 3,
        SetDuration = 4,
        SetDurationDetail = 5,
        SetDot = 6,
        SetPause = 7,
        DeleteNote = 8,
        Pasty = 9,
        Cut = 10,
        Intrument = 11,
        Pan = 12,
        Volume = 13,
        Drums = 14,
        Bpm = 15,
        InsertNewBar = 16,
        ChangeNoteState = 17,
        InsertNewPause = 18,
        SetSign = 19,
        InsertBeat = 20,
        DeleteBar = 21,
        DeleteRangeOfBars = 22,
        DeleteRangeOfBeats = 23
    };




    class Note;
    class Bar;
    class Beat;


    class ReversableCommand //TODO variant
    {
        ReversableType commandType;
        std::uint8_t commandValue;
        std::uint8_t commandValue2;

        std::uint8_t track;
        std::uint8_t bar;
        std::uint8_t beat;
        std::uint8_t string;

    public:

        //Или вместо варианта общий класс от которого наследоваться ^^^
        using NotesBuffer = std::vector<std::unique_ptr<Note>>;
        NotesBuffer storedNotes;

        using BeatsBuffer = std::vector<std::unique_ptr<Beat>>;
        BeatsBuffer storedBeats;

        using BarsBuffer = std::vector<std::unique_ptr<Bar>>;
        BarsBuffer storedBars;



        ReversableCommand() : commandType(ReversableType::Empty),
            commandValue(0),track(0),bar(0),beat(0),string(0)
        {}

        ReversableCommand(ReversableType newType, std::uint8_t newValue=0):
            commandType(newType), commandValue(newValue), track(0), bar(0), beat(0), string(0)
        {}

        void setType(ReversableType newType) { commandType = newType; }
        ReversableType getType() { return commandType; }

        void setValue(std::uint8_t newValue) { commandValue = newValue; }
        std::uint8_t getValue() { return commandValue; }

        void setValue2(std::uint8_t newValue) { commandValue2 = newValue; }
        std::uint8_t getValue2() { return commandValue2; }

        void setPosition(std::uint8_t trackN, std::uint8_t barN, std::uint8_t beatN, std::uint8_t stringN=255) {
            track = trackN;
            bar = barN;
            beat = beatN;
            string = stringN;
        }

        std::uint8_t getTrackNum() { return track; }
        std::uint8_t getBarNum() { return bar; }
        std::uint8_t getBeatNum() { return beat; }
        std::uint8_t getStringNum() { return string; }
    };


}




#endif // COMMANDS_H
