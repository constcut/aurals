#ifndef PIANOROLL_H
#define PIANOROLL_H

#include <QQuickPaintedItem>

#include <QDebug>

#include "midi/MidiFile.hpp"


namespace aurals {


    struct RollNote {
        int x;
        int y;
        int w;
        int h;

        uint8_t midiNote;

        //fret, string, other data unordered_map<string, int>
    };


    class PianoRoll : public QQuickPaintedItem
    {
        Q_OBJECT
    public:
        PianoRoll() = default;

        Q_INVOKABLE void loadMidi(QString filename);
        Q_INVOKABLE int getContentWidth();
        Q_INVOKABLE int getContentHeight();

        Q_INVOKABLE void reset();

        void paint(QPainter* painter);


        Q_INVOKABLE int getTrackCount() {
            return _mid.size();
        }

        Q_INVOKABLE void setCurrentTrack(int newIdx);

        Q_INVOKABLE void setHeightStretch(bool value) {
            _fillHeight = value;
        }

        Q_INVOKABLE void zoomIn() {
            _xZoomCoef *= 2;
            _notes.clear();
            qDebug() << "Cleaning from zoomIn";
            update();
        }
        Q_INVOKABLE void zoomOut() {
            _xZoomCoef /= 2;
            _notes.clear();
            qDebug() << "Cleaning from zoomOut";
            update();
        }

        Q_INVOKABLE void ondblclick(int x, int y);

        Q_INVOKABLE void onPressAndHold(int x, int y);

        Q_INVOKABLE void onMoveVertical(int newY);
        Q_INVOKABLE void onMoveHorizontal(int newX);

        Q_INVOKABLE void saveAs(QString filename);

    signals:


    private:


        void findMinMaxMidi();
        int noteHeight();
        int midiNoteToPosition(int midiNote);
        int positionToMidiNote(int pos);

        void fillNotes();

        int _minMidi;
        int _maxMidi;

        int _bpm = 120;


        aurals::MidiFile _mid;

        size_t _currentTrack = 1;

        bool _fillHeight = false;

        double _xZoomCoef = 1.0;
        //double _yZoomCoef = 1.0;

        std::vector<RollNote> _notes;
        int _noteCursor = -1;

    };



}



#endif // PIANOROLL_H
