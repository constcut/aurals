#include "MidiRender.hpp"

#include <QDebug>

#include "libs/sf/tsf.h"
#include "libs/sf/tml.h"


using namespace mtherapp;


MidiRender::MidiRender()
{
    _renderFrameSize = 1024;
    _sampleRate = 44100;
    _totalMsSpent = 0;
}

bool MidiRender::openMidiFile(const QString midiFilename)
{
    QByteArray fileNameBytes = midiFilename.toLocal8Bit();
    tml_message* midiMessages = tml_load_filename(fileNameBytes.constData());

    if (midiMessages==0)
        return false;

    _midiFile = midiMessages;

    return true;
}

bool MidiRender::openSoundFont(QString sfFilename)
{
    QByteArray fileNameBytes = sfFilename.toLocal8Bit();
    tsf*  sf  = tsf_load_filename(fileNameBytes.constData());

    if (sf == nullptr)
        return false;

    tsf_channel_set_bank_preset(sf, 9, 128, 0);

    TSFOutputMode mode;
    if (_mono)
        mode = TSFOutputMode::TSF_MONO;
    else
        mode = TSFOutputMode::TSF_STEREO_INTERLEAVED;

    tsf_set_output(sf, mode, _sampleRate, _volume);

    if (_soundFont != nullptr)
        free(_soundFont); //This is C-lib, God forgive me

    _soundFont = sf;
    return true;
}

QByteArray MidiRender::renderShort(QString midiFilename)
{
    QElapsedTimer timer;
    timer.start();

    if ((openMidiFile(midiFilename)==false))
        return QByteArray();

    qint64 loadingFiles = timer.restart();

    QByteArray response;
    _msRendered = 0.0;

    while (_midiFile)
        response += renderShortNext(_renderFrameSize);

     tsf_note_off_all(_soundFont);

    qint64 renderMidi = timer.elapsed();

    qDebug() <<"Rendered "<<_msRendered<<" milisecond for " << renderMidi << " miliseconds";
    _totalMsSpent += loadingFiles + renderMidi;

    return response;
}

QByteArray MidiRender::renderFloat(QString midiFilename)
{
    if ((openMidiFile(midiFilename)==false))
        return QByteArray();

    QByteArray response;
    _msRendered = 0.0;

    while (_midiFile)
        response += renderFloatNext(_renderFrameSize);

     tsf_note_off_all(_soundFont);

    return response;
}

QByteArray MidiRender::renderShort(const QString midiFilename, const QString sfFilename)
{
    if ((openSoundFont(sfFilename)==false))
        return QByteArray();

    return renderShort(midiFilename);
}


QByteArray MidiRender::renderFloat(QString midiFilename, const QString sfFilename)
{
    if ((openSoundFont(sfFilename)==false))
        return QByteArray();

    return renderFloat(midiFilename);
}


QByteArray MidiRender::renderShortNext(const int len)
{
    tml_message* g_MidiMessage = _midiFile;

    QByteArray result;
    result.resize(len);
    char *stream = result.data();

    int SampleBlock, SampleCount = (len / (2 * sizeof(short)));
    const int sampleBlockSize = 64;

    for (SampleBlock = sampleBlockSize; SampleCount; SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(short))))
    {
        if (SampleBlock > SampleCount)
            SampleBlock = SampleCount;

        for (_msRendered += SampleBlock * (1000.0 / _sampleRate); g_MidiMessage && _msRendered>= g_MidiMessage->time;
             g_MidiMessage = g_MidiMessage->next)
        {
            switch (g_MidiMessage->type)
            {
                case TML_PROGRAM_CHANGE:
                    if (g_MidiMessage->channel != 9) {
                        _g_MidiChannelPreset[g_MidiMessage->channel] = tsf_get_presetindex(_soundFont, 0, g_MidiMessage->program);
                        if (_g_MidiChannelPreset[g_MidiMessage->channel] < 0)
                                _g_MidiChannelPreset[g_MidiMessage->channel] = 0;
                    }
                    else {
                        int preset_index;
                        preset_index = tsf_get_presetindex(_soundFont, 128, g_MidiMessage->program);
                        if (preset_index == -1) preset_index = tsf_get_presetindex(_soundFont, 128, 0);
                        _g_MidiChannelPreset[g_MidiMessage->channel] = preset_index;
                        if (_g_MidiChannelPreset[g_MidiMessage->channel] < 0)
                                _g_MidiChannelPreset[g_MidiMessage->channel] = 0;
                    }
                    tsf_channel_set_presetnumber(_soundFont, g_MidiMessage->channel, g_MidiMessage->program, (g_MidiMessage->channel == 9));
                    break;
                case TML_NOTE_ON:
                    tsf_note_on(_soundFont, _g_MidiChannelPreset[g_MidiMessage->channel], g_MidiMessage->key, g_MidiMessage->velocity / 127.0f);
                    break;
                case TML_NOTE_OFF:
                    tsf_note_off(_soundFont, _g_MidiChannelPreset[g_MidiMessage->channel], g_MidiMessage->key);
                    break;
                case TML_CONTROL_CHANGE:
                    tsf_channel_midi_control(_soundFont, g_MidiMessage->channel, g_MidiMessage->control, g_MidiMessage->control_value);
                    break;
                case TML_PITCH_BEND:
                    tsf_channel_set_pitchwheel(_soundFont, g_MidiMessage->channel, g_MidiMessage->pitch_bend);
                    break;
                case TML_SET_TEMPO:
                    //Unhandled
                break;
                default:
                    qDebug() << "EVENT NOT HANDLED: " << static_cast<int>(g_MidiMessage->type);
            }
        }
        tsf_render_short(_soundFont, (short*)stream, SampleBlock, 0);
    }

   _midiFile = g_MidiMessage;

   return result;
}


QByteArray MidiRender::renderFloatNext(int len)
{
    tml_message* g_MidiMessage = _midiFile;

    QByteArray result;
    result.resize(len);

    char *stream = result.data();

    int SampleBlock, SampleCount = (len / (2 * sizeof(short)));
    const int sampleBlockSize = 64;

    for (SampleBlock = sampleBlockSize; SampleCount; SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(short))))
    {
        if (SampleBlock > SampleCount)
            SampleBlock = SampleCount;

        for (_msRendered += SampleBlock * (1000.0 / _sampleRate); g_MidiMessage && _msRendered>= g_MidiMessage->time;
             g_MidiMessage = g_MidiMessage->next)
        {
            switch (g_MidiMessage->type)
            {
                case TML_PROGRAM_CHANGE:
                    if (g_MidiMessage->channel != 9) {
                        _g_MidiChannelPreset[g_MidiMessage->channel] = tsf_get_presetindex(_soundFont, 0, g_MidiMessage->program);
                        if (_g_MidiChannelPreset[g_MidiMessage->channel] < 0)
                                _g_MidiChannelPreset[g_MidiMessage->channel] = 0;
                    }
                    else {
                        int preset_index;
                        preset_index = tsf_get_presetindex(_soundFont, 128, g_MidiMessage->program);
                        if (preset_index == -1) preset_index = tsf_get_presetindex(_soundFont, 128, 0);
                        _g_MidiChannelPreset[g_MidiMessage->channel] = preset_index;
                        if (_g_MidiChannelPreset[g_MidiMessage->channel] < 0)
                                _g_MidiChannelPreset[g_MidiMessage->channel] = 0;
                    }
                    tsf_channel_set_presetnumber(_soundFont, g_MidiMessage->channel, g_MidiMessage->program, (g_MidiMessage->channel == 9));
                    break;
                case TML_NOTE_ON:
                    tsf_note_on(_soundFont, _g_MidiChannelPreset[g_MidiMessage->channel], g_MidiMessage->key, g_MidiMessage->velocity / 127.0f);
                    break;
                case TML_NOTE_OFF:
                    tsf_note_off(_soundFont, _g_MidiChannelPreset[g_MidiMessage->channel], g_MidiMessage->key);
                    break;
                case TML_CONTROL_CHANGE:
                    tsf_channel_midi_control(_soundFont, g_MidiMessage->channel, g_MidiMessage->control, g_MidiMessage->control_value);
                    break;
                case TML_PITCH_BEND:
                    tsf_channel_set_pitchwheel(_soundFont, g_MidiMessage->channel, g_MidiMessage->pitch_bend);
                    break;
                case TML_SET_TEMPO:
                    //Unhandled
                break;
                default:
                    qDebug() << "EVENT NOT HANDLED: " << static_cast<int>(g_MidiMessage->type);
            }
        }
        tsf_render_float(_soundFont, (float*)stream, SampleBlock, 0);
    }

   _midiFile = g_MidiMessage;

   return result;
}


QByteArray MidiRender::renderFromMemoryShort(MidiTrack &track)
{
    QByteArray response;
    _msRendered = 0.0;

    _midiTrack = &track;
    _trackPosition = 0;

    while (_trackPosition < _midiTrack->size())
        response += renderMemoryShortNext(_renderFrameSize);

     tsf_note_off_all(_soundFont);

    return response;
}

QByteArray MidiRender::renderFromMemoryFloat(MidiTrack &track)
{
    QByteArray response;
    _msRendered = 0.0;

    _midiTrack = &track;
    _trackPosition = 0;

    while (_trackPosition < _midiTrack->size())
        response += renderMemoryFloatNext(_renderFrameSize);

     tsf_note_off_all(_soundFont);

    return response;
}


QByteArray MidiRender::renderMemoryFloatNext(const int len)
{
    QByteArray result;
    result.resize(len);
    char *stream = result.data();

    int SampleBlock, SampleCount = (len / (2 * sizeof(float)));
    const int sampleBlockSize = 64;

    for (SampleBlock = sampleBlockSize; SampleCount;
         SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(float))))
    {
        if (SampleBlock > SampleCount) SampleBlock = SampleCount;

        for (_msRendered += SampleBlock * (1000.0 / _sampleRate);
             (_trackPosition < _midiTrack->size()) && (_msRendered >= _midiTrack->at(_trackPosition).absoluteTime());
             ++_trackPosition)
        {
            auto& signal = _midiTrack->at(_trackPosition);

            switch (signal.getTypeAndChannel() & 0xf0)
            {
                case TML_PROGRAM_CHANGE:
                    if (signal.getChannel() != 9) {
                        _g_MidiChannelPreset[signal.getChannel()] = tsf_get_presetindex(_soundFont, 0, signal.getParameter1());
                        if (_g_MidiChannelPreset[signal.getChannel()] < 0)
                                _g_MidiChannelPreset[signal.getChannel()] = 0;
                    }
                    else {
                        int preset_index;
                        preset_index = tsf_get_presetindex(_soundFont, 128, signal.getChannel());
                        if (preset_index == -1) preset_index = tsf_get_presetindex(_soundFont, 128, 0);
                        _g_MidiChannelPreset[signal.getChannel()] = preset_index;
                        if (_g_MidiChannelPreset[signal.getChannel()] < 0)
                                _g_MidiChannelPreset[signal.getChannel()] = 0;
                    }
                    tsf_channel_set_presetnumber(_soundFont, signal.getChannel(), signal.getParameter1(), (signal.getChannel() == 9));
                    break;
                case TML_NOTE_ON:
                    tsf_note_on(_soundFont, _g_MidiChannelPreset[signal.getChannel()], signal.getParameter1(), signal.getParameter2() / 127.0f);
                    break;
                case TML_NOTE_OFF:
                    tsf_note_off(_soundFont, _g_MidiChannelPreset[signal.getChannel()], signal.getParameter1());
                    break;
                case TML_CONTROL_CHANGE:
                    tsf_channel_midi_control(_soundFont, signal.getChannel(), signal.getParameter1(), signal.getParameter2());
                    break;
                case TML_PITCH_BEND:
                    tsf_channel_set_pitchwheel(_soundFont, signal.getChannel(), signal.getParameter1());
                    break;
                case TML_SET_TEMPO:
                    //Unhandled
                break;
                default:
                    qDebug() << "EVENT NOT HANDLED: " << static_cast<int>(signal.getEventType());
            }

        }
        tsf_render_float(_soundFont, (float*)stream, SampleBlock, 0);

        if (_trackPosition >= _midiTrack->size())
            break;
    }

   return result;
}


QByteArray MidiRender::renderMemoryShortNext(const int len)
{
    QByteArray result;
    result.resize(len);
    char *stream = result.data();

    int SampleBlock, SampleCount = (len / (2 * sizeof(float)));
    const int sampleBlockSize = 64;

    for (SampleBlock = sampleBlockSize; SampleCount;
         SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(short))))
    {
        if (SampleBlock > SampleCount) SampleBlock = SampleCount;

        for (_msRendered += SampleBlock * (1000.0 / _sampleRate);
             (_trackPosition < _midiTrack->size()) && (_msRendered >= _midiTrack->at(_trackPosition).absoluteTime());
             ++_trackPosition)
        {
            auto& signal = _midiTrack->at(_trackPosition);

            switch (signal.getTypeAndChannel() & 0xf0)
            {
                case TML_PROGRAM_CHANGE:
                    if (signal.getChannel() != 9) {
                        _g_MidiChannelPreset[signal.getChannel()] = tsf_get_presetindex(_soundFont, 0, signal.getParameter1());
                        if (_g_MidiChannelPreset[signal.getChannel()] < 0)
                                _g_MidiChannelPreset[signal.getChannel()] = 0;
                    }
                    else {
                        int preset_index;
                        preset_index = tsf_get_presetindex(_soundFont, 128, signal.getChannel());
                        if (preset_index == -1) preset_index = tsf_get_presetindex(_soundFont, 128, 0);
                        _g_MidiChannelPreset[signal.getChannel()] = preset_index;
                        if (_g_MidiChannelPreset[signal.getChannel()] < 0)
                                _g_MidiChannelPreset[signal.getChannel()] = 0;
                    }
                    tsf_channel_set_presetnumber(_soundFont, signal.getChannel(), signal.getParameter1(), (signal.getChannel() == 9));
                    break;
                case TML_NOTE_ON:
                    tsf_note_on(_soundFont, _g_MidiChannelPreset[signal.getChannel()], signal.getParameter1(), signal.getParameter2() / 127.0f);
                    break;
                case TML_NOTE_OFF:
                    tsf_note_off(_soundFont, _g_MidiChannelPreset[signal.getChannel()], signal.getParameter1());
                    break;
                case TML_CONTROL_CHANGE:
                    tsf_channel_midi_control(_soundFont, signal.getChannel(), signal.getParameter1(), signal.getParameter2());
                    break;
                case TML_PITCH_BEND:
                    tsf_channel_set_pitchwheel(_soundFont, signal.getChannel(), signal.getParameter1());
                    break;
                case TML_SET_TEMPO:
                    //Unhandled
                break;
                default:
                    qDebug() << "EVENT NOT HANDLED: " << static_cast<int>(signal.getEventType());
            }
        }

        tsf_render_short(_soundFont, (short*)stream, SampleBlock, 0);

        if (_trackPosition >= _midiTrack->size())
            break;
    }

   return result;
}
