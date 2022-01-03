#include "midirender.h"

#define TSF_IMPLEMENTATION 1
#include "libs/sf/tsf.h"
#define TML_IMPLEMENTATION 1
#include "libs/sf/tml.h"

#include <QElapsedTimer>
#include <QDebug>

#include <iostream>


MidiRender::MidiRender()
{
    _renderFrameSize = 1024;
    _sampleRate = 44100;
    _totalMsSpent = 0;
}

bool MidiRender::openMidiFile(QString midiFilename)
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
    //TODO TSF_STEREO_UNWEAVED для возможности отображения в WaveShape

    tsf_set_output(sf, mode, _sampleRate, -6.0f); //Volume -6 dB TODO set\get

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

    qDebug() <<"For "<<_msRendered<<" rendered "<<loadingFiles<<" "<<renderMidi;
    _totalMsSpent += loadingFiles + renderMidi;

    return response;
}

QByteArray MidiRender::renderFloat(QString midiFilename)
{
    if ((openMidiFile(midiFilename)==false))
        return QByteArray();

    QByteArray response;
    _msRendered = 0.0; //rename to the milisecondsRendered
    //nice to have also milisecondsTotal - to know% better

    while (_midiFile)
        response += renderFloatNext(_renderFrameSize);

     tsf_note_off_all(_soundFont);

    return response;
}

QByteArray MidiRender::renderShort(QString midiFilename, QString sfFilename)
{
    if ((openSoundFont(sfFilename)==false))
        return QByteArray();

    return renderShort(midiFilename);
}


QByteArray MidiRender::renderFloat(QString midiFilename, QString sfFilename)
{
    if ((openSoundFont(sfFilename)==false))
        return QByteArray();

    return renderFloat(midiFilename);
}


QByteArray MidiRender::renderShortNext(int len)
{
    tml_message* g_MidiMessage = _midiFile;

    QByteArray result; //(const char*)data,len); - old way
    result.resize(len);      //may have some tail in the end

    char *stream = result.data();

    int SampleBlock, SampleCount = (len / (2 * sizeof(short)));
    //TSF_RENDER_EFFECTSAMPLEBLOCK
    for (SampleBlock = 64; SampleCount; SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(short))))
    {
        if (SampleBlock > SampleCount) SampleBlock = SampleCount; //this is a moment when would have tail can cut

        for (_msRendered += SampleBlock * (1000.0 / _sampleRate); g_MidiMessage && _msRendered>= g_MidiMessage->time;
             g_MidiMessage = g_MidiMessage->next)
        {

            switch (g_MidiMessage->type)
            {

                case TML_PROGRAM_CHANGE: //channel program (preset) change

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
                case TML_NOTE_ON: //play a note
                    tsf_note_on(_soundFont, _g_MidiChannelPreset[g_MidiMessage->channel], g_MidiMessage->key, g_MidiMessage->velocity / 127.0f);
                    break;
                case TML_NOTE_OFF: //stop a note
                    tsf_note_off(_soundFont, _g_MidiChannelPreset[g_MidiMessage->channel], g_MidiMessage->key);
                    break;
                case TML_CONTROL_CHANGE:
                    tsf_channel_midi_control(_soundFont, g_MidiMessage->channel, g_MidiMessage->control, g_MidiMessage->control_value);
                    break;
                case TML_PITCH_BEND: //pitch wheel modification
                    tsf_channel_set_pitchwheel(_soundFont, g_MidiMessage->channel, g_MidiMessage->pitch_bend);
                    break;

                case TML_SET_TEMPO:
                //Unhandled
                break;

                default:
                    std::cerr << "EVENT NOT HANDLED: " << static_cast<int>(g_MidiMessage->type) << std::endl;
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

    QByteArray result; //(const char*)data,len); - old way
    result.resize(len);      //may have some tail in the end

    char *stream = result.data();

    int SampleBlock, SampleCount = (len / (2 * sizeof(float)));
    for (SampleBlock = 64; SampleCount; SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(float))))
    {
        if (SampleBlock > SampleCount) SampleBlock = SampleCount; //this is a moment when would have tail can cut

        for (_msRendered += SampleBlock * (1000.0 / _sampleRate); g_MidiMessage && _msRendered >= g_MidiMessage->time; g_MidiMessage = g_MidiMessage->next)
        {
            //qDebug() << "Render message time "<<g_MidiMessage->time;

            switch (g_MidiMessage->type)    //TODO update as above
            {
                case TML_PROGRAM_CHANGE: //channel program (preset) change
                    _g_MidiChannelPreset[g_MidiMessage->channel] = tsf_get_presetindex(_soundFont, 0, g_MidiMessage->program);
                    if (_g_MidiChannelPreset[g_MidiMessage->channel] < 0) _g_MidiChannelPreset[g_MidiMessage->channel] = 0;
                    break;
                case TML_NOTE_ON: //play a note
                    tsf_note_on(_soundFont, _g_MidiChannelPreset[g_MidiMessage->channel], g_MidiMessage->key, g_MidiMessage->velocity / 127.0f);
                    break;
                case TML_NOTE_OFF: //stop a note
                    tsf_note_off(_soundFont, _g_MidiChannelPreset[g_MidiMessage->channel], g_MidiMessage->key);
                    break;
            }
        }
        tsf_render_float(_soundFont, (float*)stream, SampleBlock, 0);
    }

   _midiFile = g_MidiMessage;

   return result;
}

//=================================Memory functions - what means midi file class usage=======================

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


//Memory functions means usage of our MidiFile class - not loading file from hard disk
QByteArray MidiRender::renderMemoryFloatNext(int len)
{
    QByteArray result;
    result.resize(len);
    char *stream = result.data();


    int SampleBlock, SampleCount = (len / (2 * sizeof(float)));
    for (SampleBlock = 64; SampleCount;
         SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(float))))
    {
        if (SampleBlock > SampleCount) SampleBlock = SampleCount;

        for (_msRendered += SampleBlock * (1000.0 / _sampleRate);
             (_trackPosition < _midiTrack->size()) && (_msRendered >= _midiTrack->at(_trackPosition)->_absoluteTime); //REPLACE 0 with time from midi signals
             ++_trackPosition)
        {
            auto& signal = _midiTrack->at(_trackPosition); //later replace such place with ->at(i)

            switch (signal->_byte0 & 0xf0)
            {
                case TML_PROGRAM_CHANGE:
                    _g_MidiChannelPreset[signal->getChannel()] = tsf_get_presetindex(_soundFont, 0, signal->_param1);
                    if (_g_MidiChannelPreset[signal->getChannel()] < 0) _g_MidiChannelPreset[signal->getChannel()] = 0;
                    break;
                case TML_NOTE_ON: //play a note
                    tsf_note_on(_soundFont, _g_MidiChannelPreset[signal->getChannel()], signal->_param1, signal->_param2 / 127.0f);
                    break;
                case TML_NOTE_OFF: //stop a note
                    tsf_note_off(_soundFont, _g_MidiChannelPreset[signal->getChannel()], signal->_param1);
                    break;
            }

        }
        tsf_render_float(_soundFont, (float*)stream, SampleBlock, 0);

        if (_trackPosition >= _midiTrack->size())
            break;
    }

   return result;
}


QByteArray MidiRender::renderMemoryShortNext(int len)
{
    QByteArray result;
    result.resize(len);
    char *stream = result.data();

    int SampleBlock, SampleCount = (len / (2 * sizeof(short)));
    for (SampleBlock = 64; SampleCount;
         SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(short))))
    {
        if (SampleBlock > SampleCount) SampleBlock = SampleCount;

        for (_msRendered += SampleBlock * (1000.0 / _sampleRate);
             (_trackPosition < _midiTrack->size()) && (_msRendered >= _midiTrack->at(_trackPosition)->_absoluteTime); //REPLACE 0 with time from midi signals
             ++_trackPosition)
        {
            auto& signal = _midiTrack->at(_trackPosition); //later replace such place with ->at(i)

            qDebug() << "Render "<<signal->_byte0<<" "<<signal->_absoluteTime;

            switch (signal->_byte0 & 0xf0)
            {
                case TML_PROGRAM_CHANGE:
                    _g_MidiChannelPreset[signal->getChannel()] = tsf_get_presetindex(_soundFont, 0, signal->_param1);
                    if (_g_MidiChannelPreset[signal->getChannel()] < 0) _g_MidiChannelPreset[signal->getChannel()] = 0;
                    break;
                case TML_NOTE_ON: //play a note
                    tsf_note_on(_soundFont, _g_MidiChannelPreset[signal->getChannel()], signal->_param1, signal->_param2 / 127.0f);
                    break;
                case TML_NOTE_OFF: //stop a note
                    tsf_note_off(_soundFont, _g_MidiChannelPreset[signal->getChannel()], signal->_param1);
                    break;
            }
        }

        tsf_render_short(_soundFont, (short*)stream, SampleBlock, 0);

        if (_trackPosition >= _midiTrack->size())
            break;
    }

   return result;
}
