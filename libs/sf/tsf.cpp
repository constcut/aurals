// The lower this block size is the more accurate the effects are.
// Increasing the value significantly lowers the CPU usage of the voice rendering.
// If LFO affects the low-pass filter it can be hearable even as low as 8.

#include "tsf.h"
#include "tml.h"

#if !defined(TSF_MALLOC) || !defined(TSF_FREE) || !defined(TSF_REALLOC)
#  include <stdlib.h>
#  define TSF_MALLOC  malloc
#  define TSF_FREE    free
#  define TSF_REALLOC realloc
#endif

#if !defined(TSF_MEMCPY) || !defined(TSF_MEMSET)
#  include <string.h>
#  define TSF_MEMCPY  memcpy
#  define TSF_MEMSET  memset
#endif

#if !defined(TSF_POW) || !defined(TSF_POWF) || !defined(TSF_EXPF) || !defined(TSF_LOG) || !defined(TSF_TAN) || !defined(TSF_LOG10) || !defined(TSF_SQRT)
#  include <math.h>
#  if !defined(__cplusplus) && !defined(NAN) && !defined(powf) && !defined(expf)
#    define powf (float)pow // deal with old math.h files that
#    define expf (float)exp // come without powf and expf
#  endif
#  define TSF_POW     pow
#  define TSF_POWF    powf
#  define TSF_EXPF    expf
#  define TSF_LOG     log
#  define TSF_TAN     tan
#  define TSF_LOG10   log10
#  define TSF_SQRT    sqrt
#endif

#ifndef TSF_NO_STDIO
#  include <stdio.h>
#endif

#define TSF_TRUE 1
#define TSF_FALSE 0
#define TSF_BOOL char
#define TSF_PI 3.14159265358979323846264338327950288
#define TSF_NULL 0

#ifdef __cplusplus
extern "C" {
#endif

typedef char tsf_fourcc[4];
typedef signed char tsf_s8;
typedef unsigned char tsf_u8;
typedef unsigned short tsf_u16;
typedef signed short tsf_s16;
typedef unsigned int tsf_u32;
typedef char tsf_char20[20];

#define TSF_FourCCEquals(value1, value2) (value1[0] == value2[0] && value1[1] == value2[1] && value1[2] == value2[2] && value1[3] == value2[3])

struct tsf
{
    struct tsf_preset* presets;
    float* fontSamples;
    struct tsf_voice* voices;
    float* outputSamples;

    int presetNum;
    int fontSampleCount;
    int voiceNum;
    int outputSampleSize;
    unsigned int voicePlayIndex;

    float outSampleRate;
    enum TSFOutputMode outputmode;
    float globalGainDB, globalPanFactorLeft, globalPanFactorRight;

};

#ifndef TSF_NO_STDIO
static int tsf_stream_stdio_read(FILE* f, void* ptr, unsigned int size) { return (int)fread(ptr, 1, size, f); }
static int tsf_stream_stdio_skip(FILE* f, unsigned int count) { return !fseek(f, count, SEEK_CUR); }
TSFDEF tsf* tsf_load_filename(const char* filename)
{
    tsf* res;
    struct tsf_stream stream = { TSF_NULL, (int(*)(void*,void*,unsigned int))&tsf_stream_stdio_read, (int(*)(void*,unsigned int))&tsf_stream_stdio_skip };
    #if __STDC_WANT_SECURE_LIB__
    FILE* f = TSF_NULL; fopen_s(&f, filename, "rb");
    #else
    FILE* f = fopen(filename, "rb");
    #endif
    if (!f)
    {
        //if (e) *e = TSF_FILENOTFOUND;
        return TSF_NULL;
    }
    stream.data = f;
    res = tsf_load(&stream);
    fclose(f);
    return res;
}
#endif

struct tsf_stream_memory { const char* buffer; unsigned int total, pos; };
static int tsf_stream_memory_read(struct tsf_stream_memory* m, void* ptr, unsigned int size) { if (size > m->total - m->pos) size = m->total - m->pos; TSF_MEMCPY(ptr, m->buffer+m->pos, size); m->pos += size; return size; }
static int tsf_stream_memory_skip(struct tsf_stream_memory* m, unsigned int count) { if (m->pos + count > m->total) return 0; m->pos += count; return 1; }
TSFDEF tsf* tsf_load_memory(const void* buffer, int size)
{
    struct tsf_stream stream = { TSF_NULL, (int(*)(void*,void*,unsigned int))&tsf_stream_memory_read, (int(*)(void*,unsigned int))&tsf_stream_memory_skip };
    struct tsf_stream_memory f = { 0, 0, 0 };
    f.buffer = (const char*)buffer;
    f.total = size;
    stream.data = &f;
    return tsf_load(&stream);
}

enum { TSF_LOOPMODE_NONE, TSF_LOOPMODE_CONTINUOUS, TSF_LOOPMODE_SUSTAIN };

enum { TSF_SEGMENT_NONE, TSF_SEGMENT_DELAY, TSF_SEGMENT_ATTACK, TSF_SEGMENT_HOLD, TSF_SEGMENT_DECAY, TSF_SEGMENT_SUSTAIN, TSF_SEGMENT_RELEASE, TSF_SEGMENT_DONE };

struct tsf_hydra
{
    struct tsf_hydra_phdr *phdrs; struct tsf_hydra_pbag *pbags; struct tsf_hydra_pmod *pmods;
    struct tsf_hydra_pgen *pgens; struct tsf_hydra_inst *insts; struct tsf_hydra_ibag *ibags;
    struct tsf_hydra_imod *imods; struct tsf_hydra_igen *igens; struct tsf_hydra_shdr *shdrs;
    int phdrNum, pbagNum, pmodNum, pgenNum, instNum, ibagNum, imodNum, igenNum, shdrNum;
};

union tsf_hydra_genamount { struct { tsf_u8 lo, hi; } range; tsf_s16 shortAmount; tsf_u16 wordAmount; };
struct tsf_hydra_phdr { tsf_char20 presetName; tsf_u16 preset, bank, presetBagNdx; tsf_u32 library, genre, morphology; };
struct tsf_hydra_pbag { tsf_u16 genNdx, modNdx; };
struct tsf_hydra_pmod { tsf_u16 modSrcOper, modDestOper; tsf_s16 modAmount; tsf_u16 modAmtSrcOper, modTransOper; };
struct tsf_hydra_pgen { tsf_u16 genOper; union tsf_hydra_genamount genAmount; };
struct tsf_hydra_inst { tsf_char20 instName; tsf_u16 instBagNdx; };
struct tsf_hydra_ibag { tsf_u16 instGenNdx, instModNdx; };
struct tsf_hydra_imod { tsf_u16 modSrcOper, modDestOper; tsf_s16 modAmount; tsf_u16 modAmtSrcOper, modTransOper; };
struct tsf_hydra_igen { tsf_u16 genOper; union tsf_hydra_genamount genAmount; };
struct tsf_hydra_shdr { tsf_char20 sampleName; tsf_u32 start, end, startLoop, endLoop, sampleRate; tsf_u8 originalPitch; tsf_s8 pitchCorrection; tsf_u16 sampleLink, sampleType; };

#define TSFR(FIELD) stream->read(stream->data, &i->FIELD, sizeof(i->FIELD));
static void tsf_hydra_read_phdr(struct tsf_hydra_phdr* i, struct tsf_stream* stream) { TSFR(presetName) TSFR(preset) TSFR(bank) TSFR(presetBagNdx) TSFR(library) TSFR(genre) TSFR(morphology) }
static void tsf_hydra_read_pbag(struct tsf_hydra_pbag* i, struct tsf_stream* stream) { TSFR(genNdx) TSFR(modNdx) }
static void tsf_hydra_read_pmod(struct tsf_hydra_pmod* i, struct tsf_stream* stream) { TSFR(modSrcOper) TSFR(modDestOper) TSFR(modAmount) TSFR(modAmtSrcOper) TSFR(modTransOper) }
static void tsf_hydra_read_pgen(struct tsf_hydra_pgen* i, struct tsf_stream* stream) { TSFR(genOper) TSFR(genAmount) }
static void tsf_hydra_read_inst(struct tsf_hydra_inst* i, struct tsf_stream* stream) { TSFR(instName) TSFR(instBagNdx) }
static void tsf_hydra_read_ibag(struct tsf_hydra_ibag* i, struct tsf_stream* stream) { TSFR(instGenNdx) TSFR(instModNdx) }
static void tsf_hydra_read_imod(struct tsf_hydra_imod* i, struct tsf_stream* stream) { TSFR(modSrcOper) TSFR(modDestOper) TSFR(modAmount) TSFR(modAmtSrcOper) TSFR(modTransOper) }
static void tsf_hydra_read_igen(struct tsf_hydra_igen* i, struct tsf_stream* stream) { TSFR(genOper) TSFR(genAmount) }
static void tsf_hydra_read_shdr(struct tsf_hydra_shdr* i, struct tsf_stream* stream) { TSFR(sampleName) TSFR(start) TSFR(end) TSFR(startLoop) TSFR(endLoop) TSFR(sampleRate) TSFR(originalPitch) TSFR(pitchCorrection) TSFR(sampleLink) TSFR(sampleType) }
#undef TSFR

struct tsf_riffchunk { tsf_fourcc id; tsf_u32 size; };
struct tsf_envelope { float delay, start, attack, hold, decay, sustain, release, keynumToHold, keynumToDecay; };
struct tsf_voice_envelope { float level, slope; int samplesUntilNextSegment; int segment; struct tsf_envelope parameters; TSF_BOOL segmentIsExponential, exponentialDecay; };
struct tsf_voice_lowpass { double QInv, a0, a1, b1, b2, z1, z2; TSF_BOOL active; };
struct tsf_voice_lfo { int samplesUntil; float level, delta; };

struct tsf_region
{
    int loop_mode;
    unsigned int sample_rate;
    unsigned char lokey, hikey, lovel, hivel;
    unsigned int group, offset, end, loop_start, loop_end;
    int transpose, tune, pitch_keycenter, pitch_keytrack;
    float volume, pan;
    struct tsf_envelope ampenv, modenv;
    int initialFilterQ, initialFilterFc;
    int modEnvToPitch, modEnvToFilterFc, modLfoToFilterFc, modLfoToVolume;
    float delayModLFO;
    int freqModLFO, modLfoToPitch;
    float delayVibLFO;
    int freqVibLFO, vibLfoToPitch;
};

struct tsf_preset
{
    tsf_char20 presetName;
    tsf_u16 preset, bank;
    struct tsf_region* regions;
    int regionNum;
};

struct tsf_voice
{
    int playingPreset, playingKey, curPitchWheel;
    struct tsf_region* region;
    double pitchInputTimecents, pitchOutputFactor;
    double sourceSamplePosition;
    float  noteGainDB, panFactorLeft, panFactorRight;
    unsigned int playIndex, sampleEnd, loopStart, loopEnd;
    struct tsf_voice_envelope ampenv, modenv;
    struct tsf_voice_lowpass lowpass;
    struct tsf_voice_lfo modlfo, viblfo;
};

static double tsf_timecents2Secsd(double timecents) { return TSF_POW(2.0, timecents / 1200.0); }
static float tsf_timecents2Secsf(float timecents) { return TSF_POWF(2.0f, timecents / 1200.0f); }
static float tsf_cents2Hertz(float cents) { return 8.176f * TSF_POWF(2.0f, cents / 1200.0f); }
static float tsf_decibelsToGain(float db) { return (db > -100.f ? TSF_POWF(10.0f, db * 0.05f) : 0); }

static TSF_BOOL tsf_riffchunk_read(struct tsf_riffchunk* parent, struct tsf_riffchunk* chunk, struct tsf_stream* stream)
{
    TSF_BOOL IsRiff, IsList;
    if (parent && sizeof(tsf_fourcc) + sizeof(tsf_u32) > parent->size) return TSF_FALSE;
    if (!stream->read(stream->data, &chunk->id, sizeof(tsf_fourcc)) || *chunk->id <= ' ' || *chunk->id >= 'z') return TSF_FALSE;
    if (!stream->read(stream->data, &chunk->size, sizeof(tsf_u32))) return TSF_FALSE;
    if (parent && sizeof(tsf_fourcc) + sizeof(tsf_u32) + chunk->size > parent->size) return TSF_FALSE;
    if (parent) parent->size -= sizeof(tsf_fourcc) + sizeof(tsf_u32) + chunk->size;
    IsRiff = TSF_FourCCEquals(chunk->id, "RIFF"), IsList = TSF_FourCCEquals(chunk->id, "LIST");
    if (IsRiff && parent) return TSF_FALSE; //not allowed
    if (!IsRiff && !IsList) return TSF_TRUE; //custom type without sub type
    if (!stream->read(stream->data, &chunk->id, sizeof(tsf_fourcc)) || *chunk->id <= ' ' || *chunk->id >= 'z') return TSF_FALSE;
    chunk->size -= sizeof(tsf_fourcc);
    return TSF_TRUE;
}

static void tsf_region_clear(struct tsf_region* i, TSF_BOOL for_relative)
{
    TSF_MEMSET(i, 0, sizeof(struct tsf_region));
    i->hikey = i->hivel = 127;
    i->pitch_keycenter = 60; // C4
    if (for_relative) return;

    i->pitch_keytrack = 100;

    i->pitch_keycenter = -1;

    // SF2 defaults in timecents.
    i->ampenv.delay = i->ampenv.attack = i->ampenv.hold = i->ampenv.decay = i->ampenv.release = -12000.0f;
    i->modenv.delay = i->modenv.attack = i->modenv.hold = i->modenv.decay = i->modenv.release = -12000.0f;

    i->initialFilterFc = 13500;

    i->delayModLFO = -12000.0f;
    i->delayVibLFO = -12000.0f;
}

static void tsf_region_operator(struct tsf_region* region, tsf_u16 genOper, union tsf_hydra_genamount* amount)
{
    enum
    {
        StartAddrsOffset, EndAddrsOffset, StartloopAddrsOffset, EndloopAddrsOffset, StartAddrsCoarseOffset, ModLfoToPitch, VibLfoToPitch, ModEnvToPitch,
        InitialFilterFc, InitialFilterQ, ModLfoToFilterFc, ModEnvToFilterFc, EndAddrsCoarseOffset, ModLfoToVolume, Unused1, ChorusEffectsSend,
        ReverbEffectsSend, Pan, Unused2, Unused3, Unused4, DelayModLFO, FreqModLFO, DelayVibLFO, FreqVibLFO, DelayModEnv, AttackModEnv, HoldModEnv,
        DecayModEnv, SustainModEnv, ReleaseModEnv, KeynumToModEnvHold, KeynumToModEnvDecay, DelayVolEnv, AttackVolEnv, HoldVolEnv, DecayVolEnv,
        SustainVolEnv, ReleaseVolEnv, KeynumToVolEnvHold, KeynumToVolEnvDecay, Instrument, Reserved1, KeyRange, VelRange, StartloopAddrsCoarseOffset,
        Keynum, Velocity, InitialAttenuation, Reserved2, EndloopAddrsCoarseOffset, CoarseTune, FineTune, SampleID, SampleModes, Reserved3, ScaleTuning,
        ExclusiveClass, OverridingRootKey, Unused5, EndOper
    };
    switch (genOper)
    {
        case StartAddrsOffset:           region->offset += amount->shortAmount; break;
        case EndAddrsOffset:             region->end += amount->shortAmount; break;
        case StartloopAddrsOffset:       region->loop_start += amount->shortAmount; break;
        case EndloopAddrsOffset:         region->loop_end += amount->shortAmount; break;
        case StartAddrsCoarseOffset:     region->offset += amount->shortAmount * 32768; break;
        case ModLfoToPitch:              region->modLfoToPitch = amount->shortAmount; break;
        case VibLfoToPitch:              region->vibLfoToPitch = amount->shortAmount; break;
        case ModEnvToPitch:              region->modEnvToPitch = amount->shortAmount; break;
        case InitialFilterFc:            region->initialFilterFc = amount->shortAmount; break;
        case InitialFilterQ:             region->initialFilterQ = amount->shortAmount; break;
        case ModLfoToFilterFc:           region->modLfoToFilterFc = amount->shortAmount; break;
        case ModEnvToFilterFc:           region->modEnvToFilterFc = amount->shortAmount; break;
        case EndAddrsCoarseOffset:       region->end += amount->shortAmount * 32768; break;
        case ModLfoToVolume:             region->modLfoToVolume = amount->shortAmount; break;
        case Pan:                        region->pan = amount->shortAmount * (2.0f / 10.0f); break;
        case DelayModLFO:                region->delayModLFO = amount->shortAmount; break;
        case FreqModLFO:                 region->freqModLFO = amount->shortAmount; break;
        case DelayVibLFO:                region->delayVibLFO = amount->shortAmount; break;
        case FreqVibLFO:                 region->freqVibLFO = amount->shortAmount; break;
        case DelayModEnv:                region->modenv.delay = amount->shortAmount; break;
        case AttackModEnv:               region->modenv.attack = amount->shortAmount; break;
        case HoldModEnv:                 region->modenv.hold = amount->shortAmount; break;
        case DecayModEnv:                region->modenv.decay = amount->shortAmount; break;
        case SustainModEnv:              region->modenv.sustain = amount->shortAmount; break;
        case ReleaseModEnv:              region->modenv.release = amount->shortAmount; break;
        case KeynumToModEnvHold:         region->modenv.keynumToHold = amount->shortAmount; break;
        case KeynumToModEnvDecay:        region->modenv.keynumToDecay = amount->shortAmount; break;
        case DelayVolEnv:                region->ampenv.delay = amount->shortAmount; break;
        case AttackVolEnv:               region->ampenv.attack = amount->shortAmount; break;
        case HoldVolEnv:                 region->ampenv.hold = amount->shortAmount; break;
        case DecayVolEnv:                region->ampenv.decay = amount->shortAmount; break;
        case SustainVolEnv:              region->ampenv.sustain = amount->shortAmount; break;
        case ReleaseVolEnv:              region->ampenv.release = amount->shortAmount; break;
        case KeynumToVolEnvHold:         region->ampenv.keynumToHold = amount->shortAmount; break;
        case KeynumToVolEnvDecay:        region->ampenv.keynumToDecay = amount->shortAmount; break;
        case KeyRange:                   region->lokey = amount->range.lo; region->hikey = amount->range.hi; break;
        case VelRange:                   region->lovel = amount->range.lo; region->hivel = amount->range.hi; break;
        case StartloopAddrsCoarseOffset: region->loop_start += amount->shortAmount * 32768; break;
        case InitialAttenuation:         region->volume += -amount->shortAmount / 100.0f; break;
        case EndloopAddrsCoarseOffset:   region->loop_end += amount->shortAmount * 32768; break;
        case CoarseTune:                 region->transpose += amount->shortAmount; break;
        case FineTune:                   region->tune += amount->shortAmount; break;
        case SampleModes:                region->loop_mode = ((amount->wordAmount&3) == 3 ? TSF_LOOPMODE_SUSTAIN : ((amount->wordAmount&3) == 1 ? TSF_LOOPMODE_CONTINUOUS : TSF_LOOPMODE_NONE)); break;
        case ScaleTuning:                region->pitch_keytrack = amount->shortAmount; break;
        case ExclusiveClass:             region->group = amount->wordAmount; break;
        case OverridingRootKey:          region->pitch_keycenter = amount->shortAmount; break;
        //case gen_endOper: break; // Ignore.
        //default: addUnsupportedOpcode(generator_name);
    }
}

static void tsf_region_envtosecs(struct tsf_envelope* p, TSF_BOOL sustainIsGain)
{
    // EG times need to be converted from timecents to seconds.
    // Pin very short EG segments.  Timecents don't get to zero, and our EG is
    // happier with zero values.
    p->delay   = (p->delay   < -11950.0f ? 0.0f : tsf_timecents2Secsf(p->delay));
    p->attack  = (p->attack  < -11950.0f ? 0.0f : tsf_timecents2Secsf(p->attack));
    p->release = (p->release < -11950.0f ? 0.0f : tsf_timecents2Secsf(p->release));

    // If we have dynamic hold or decay times depending on key number we need
    // to keep the values in timecents so we can calculate it during startNote
    if (!p->keynumToHold)  p->hold  = (p->hold  < -11950.0f ? 0.0f : tsf_timecents2Secsf(p->hold));
    if (!p->keynumToDecay) p->decay = (p->decay < -11950.0f ? 0.0f : tsf_timecents2Secsf(p->decay));

    if (p->sustain < 0.0f) p->sustain = 0.0f;
    else if (sustainIsGain) p->sustain = 100.0f * tsf_decibelsToGain(-p->sustain / 10.0f);
    else p->sustain = p->sustain / 10.0f;
}

static void tsf_load_presets(tsf* res, struct tsf_hydra *hydra)
{
    enum { GenInstrument = 41, GenSampleID = 53 };
    // Read each preset.
    struct tsf_hydra_phdr *pphdr, *pphdrMax;
    for (pphdr = hydra->phdrs, pphdrMax = pphdr + hydra->phdrNum - 1; pphdr != pphdrMax; pphdr++)
    {
        int sortedIndex = 0, region_index = 0;
        struct tsf_hydra_phdr *otherphdr;
        struct tsf_preset* preset;
        struct tsf_hydra_pbag *ppbag, *ppbagEnd;
        for (otherphdr = hydra->phdrs; otherphdr != pphdrMax; otherphdr++)
        {
            if (otherphdr == pphdr || otherphdr->bank > pphdr->bank) continue;
            else if (otherphdr->bank < pphdr->bank) sortedIndex++;
            else if (otherphdr->preset > pphdr->preset) continue;
            else if (otherphdr->preset < pphdr->preset) sortedIndex++;
            else if (otherphdr < pphdr) sortedIndex++;
        }

        preset = &res->presets[sortedIndex];
        TSF_MEMCPY(preset->presetName, pphdr->presetName, sizeof(preset->presetName));
        preset->presetName[sizeof(preset->presetName)-1] = '\0'; //should be zero terminated in source file but make sure
        preset->bank = pphdr->bank;
        preset->preset = pphdr->preset;
        preset->regionNum = 0;

        //count regions covered by this preset
        for (ppbag = hydra->pbags + pphdr->presetBagNdx, ppbagEnd = hydra->pbags + pphdr[1].presetBagNdx; ppbag != ppbagEnd; ppbag++)
        {
            struct tsf_hydra_pgen *ppgen, *ppgenEnd; struct tsf_hydra_inst *pinst; struct tsf_hydra_ibag *pibag, *pibagEnd; struct tsf_hydra_igen *pigen, *pigenEnd;
            for (ppgen = hydra->pgens + ppbag->genNdx, ppgenEnd = hydra->pgens + ppbag[1].genNdx; ppgen != ppgenEnd; ppgen++)
            {
                if (ppgen->genOper != GenInstrument) continue;
                if (ppgen->genAmount.wordAmount >= hydra->instNum) continue;
                pinst = hydra->insts + ppgen->genAmount.wordAmount;
                for (pibag = hydra->ibags + pinst->instBagNdx, pibagEnd = hydra->ibags + pinst[1].instBagNdx; pibag != pibagEnd; pibag++)
                    for (pigen = hydra->igens + pibag->instGenNdx, pigenEnd = hydra->igens + pibag[1].instGenNdx; pigen != pigenEnd; pigen++)
                        if (pigen->genOper == GenSampleID)
                            preset->regionNum++;
            }
        }

        preset->regions = (struct tsf_region*)TSF_MALLOC(preset->regionNum * sizeof(struct tsf_region));

        // Zones.
        //*** TODO: Handle global zone (modulators only).
        for (ppbag = hydra->pbags + pphdr->presetBagNdx, ppbagEnd = hydra->pbags + pphdr[1].presetBagNdx; ppbag != ppbagEnd; ppbag++)
        {
            struct tsf_hydra_pgen *ppgen, *ppgenEnd; struct tsf_hydra_inst *pinst; struct tsf_hydra_ibag *pibag, *pibagEnd; struct tsf_hydra_igen *pigen, *pigenEnd;
            struct tsf_region presetRegion;
            tsf_region_clear(&presetRegion, TSF_TRUE);

            // Generators.
            for (ppgen = hydra->pgens + ppbag->genNdx, ppgenEnd = hydra->pgens + ppbag[1].genNdx; ppgen != ppgenEnd; ppgen++)
            {
                // Instrument.
                if (ppgen->genOper == GenInstrument)
                {
                    struct tsf_region instRegion;
                    tsf_u16 whichInst = ppgen->genAmount.wordAmount;
                    if (whichInst >= hydra->instNum) continue;

                    tsf_region_clear(&instRegion, TSF_FALSE);
                    // Preset generators are supposed to be "relative" modifications of
                    // the instrument settings, but that makes no sense for ranges.
                    // For those, we'll have the instrument's generator take
                    // precedence, though that may not be correct.
                    instRegion.lokey = presetRegion.lokey;
                    instRegion.hikey = presetRegion.hikey;
                    instRegion.lovel = presetRegion.lovel;
                    instRegion.hivel = presetRegion.hivel;

                    pinst = &hydra->insts[whichInst];
                    for (pibag = hydra->ibags + pinst->instBagNdx, pibagEnd = hydra->ibags + pinst[1].instBagNdx; pibag != pibagEnd; pibag++)
                    {
                        // Generators.
                        struct tsf_region zoneRegion = instRegion;
                        int hadSampleID = 0;
                        for (pigen = hydra->igens + pibag->instGenNdx, pigenEnd = hydra->igens + pibag[1].instGenNdx; pigen != pigenEnd; pigen++)
                        {
                            if (pigen->genOper == GenSampleID)
                            {
                                struct tsf_hydra_shdr* pshdr = &hydra->shdrs[pigen->genAmount.wordAmount];

                                //sum regions
                                zoneRegion.offset += presetRegion.offset;
                                zoneRegion.end += presetRegion.end;
                                zoneRegion.loop_start += presetRegion.loop_start;
                                zoneRegion.loop_end += presetRegion.loop_end;
                                zoneRegion.transpose += presetRegion.transpose;
                                zoneRegion.tune += presetRegion.tune;
                                zoneRegion.pitch_keytrack += presetRegion.pitch_keytrack;
                                zoneRegion.volume += presetRegion.volume;
                                zoneRegion.pan += presetRegion.pan;
                                zoneRegion.ampenv.delay += presetRegion.ampenv.delay;
                                zoneRegion.ampenv.attack += presetRegion.ampenv.attack;
                                zoneRegion.ampenv.hold += presetRegion.ampenv.hold;
                                zoneRegion.ampenv.decay += presetRegion.ampenv.decay;
                                zoneRegion.ampenv.sustain += presetRegion.ampenv.sustain;
                                zoneRegion.ampenv.release += presetRegion.ampenv.release;
                                zoneRegion.modenv.delay += presetRegion.modenv.delay;
                                zoneRegion.modenv.attack += presetRegion.modenv.attack;
                                zoneRegion.modenv.hold += presetRegion.modenv.hold;
                                zoneRegion.modenv.decay += presetRegion.modenv.decay;
                                zoneRegion.modenv.sustain += presetRegion.modenv.sustain;
                                zoneRegion.modenv.release += presetRegion.modenv.release;
                                zoneRegion.initialFilterQ += presetRegion.initialFilterQ;
                                zoneRegion.initialFilterFc += presetRegion.initialFilterFc;
                                zoneRegion.modEnvToPitch += presetRegion.modEnvToPitch;
                                zoneRegion.modEnvToFilterFc += presetRegion.modEnvToFilterFc;
                                zoneRegion.delayModLFO += presetRegion.delayModLFO;
                                zoneRegion.freqModLFO += presetRegion.freqModLFO;
                                zoneRegion.modLfoToPitch += presetRegion.modLfoToPitch;
                                zoneRegion.modLfoToFilterFc += presetRegion.modLfoToFilterFc;
                                zoneRegion.modLfoToVolume += presetRegion.modLfoToVolume;
                                zoneRegion.delayVibLFO += presetRegion.delayVibLFO;
                                zoneRegion.freqVibLFO += presetRegion.freqVibLFO;
                                zoneRegion.vibLfoToPitch += presetRegion.vibLfoToPitch;

                                // EG times need to be converted from timecents to seconds.
                                tsf_region_envtosecs(&zoneRegion.ampenv, TSF_TRUE);
                                tsf_region_envtosecs(&zoneRegion.modenv, TSF_FALSE);

                                // LFO times need to be converted from timecents to seconds.
                                zoneRegion.delayModLFO = (zoneRegion.delayModLFO < -11950.0f ? 0.0f : tsf_timecents2Secsf(zoneRegion.delayModLFO));
                                zoneRegion.delayVibLFO = (zoneRegion.delayVibLFO < -11950.0f ? 0.0f : tsf_timecents2Secsf(zoneRegion.delayVibLFO));

                                // Pin values to their ranges.
                                if (zoneRegion.pan < -100.0f) zoneRegion.pan = -100.0f;
                                else if (zoneRegion.pan > 100.0f) zoneRegion.pan = 100.0f;
                                if (zoneRegion.initialFilterQ < 1500 || zoneRegion.initialFilterQ > 13500) zoneRegion.initialFilterQ = 0;

                                zoneRegion.offset += pshdr->start;
                                zoneRegion.end += pshdr->end;
                                zoneRegion.loop_start += pshdr->startLoop;
                                zoneRegion.loop_end += pshdr->endLoop;
                                if (pshdr->endLoop > 0) zoneRegion.loop_end -= 1;
                                if (zoneRegion.pitch_keycenter == -1) zoneRegion.pitch_keycenter = pshdr->originalPitch;
                                zoneRegion.tune += pshdr->pitchCorrection;

                                // Pin initialAttenuation to max +6dB.
                                if (zoneRegion.volume > 6.0f)
                                {
                                    zoneRegion.volume = 6.0f;
                                    //addUnsupportedOpcode("extreme gain in initialAttenuation");
                                }

                                preset->regions[region_index] = zoneRegion;
                                preset->regions[region_index].sample_rate = pshdr->sampleRate;
                                region_index++;
                                hadSampleID = 1;
                            }
                            else tsf_region_operator(&zoneRegion, pigen->genOper, &pigen->genAmount);
                        }

                        // Handle instrument's global zone.
                        if (pibag == hydra->ibags + pinst->instBagNdx && !hadSampleID)
                            instRegion = zoneRegion;

                        // Modulators (TODO)
                        //if (ibag->instModNdx < ibag[1].instModNdx) addUnsupportedOpcode("any modulator");
                    }
                }
                else tsf_region_operator(&presetRegion, ppgen->genOper, &ppgen->genAmount);
            }

            // Modulators (TODO)
            //if (pbag->modNdx < pbag[1].modNdx) addUnsupportedOpcode("any modulator");
        }
    }
}

static void tsf_load_samples(float** fontSamples, int* fontSampleCount, struct tsf_riffchunk *chunkSmpl, struct tsf_stream* stream)
{
    // Read sample data into float format buffer.
    float* out; unsigned int samplesLeft, samplesToRead, samplesToConvert;
    samplesLeft = *fontSampleCount = chunkSmpl->size / sizeof(short);
    out = *fontSamples = (float*)TSF_MALLOC(samplesLeft * sizeof(float));
    for (; samplesLeft; samplesLeft -= samplesToRead)
    {
        short sampleBuffer[1024], *in = sampleBuffer;;
        samplesToRead = (samplesLeft > 1024 ? 1024 : samplesLeft);
        stream->read(stream->data, sampleBuffer, samplesToRead * sizeof(short));

        // Convert from signed 16-bit to float.
        for (samplesToConvert = samplesToRead; samplesToConvert > 0; --samplesToConvert)
            // If we ever need to compile for big-endian platforms, we'll need to byte-swap here.
            *out++ = (float)(*in++ / 32767.0);
    }
}

static void tsf_voice_envelope_nextsegment(struct tsf_voice_envelope* e, int active_segment, float outSampleRate)
{
    switch (active_segment)
    {
        case TSF_SEGMENT_NONE:
            e->samplesUntilNextSegment = (int)(e->parameters.delay * outSampleRate);
            if (e->samplesUntilNextSegment > 0)
            {
                e->segment = TSF_SEGMENT_DELAY;
                e->segmentIsExponential = TSF_FALSE;
                e->level = 0.0;
                e->slope = 0.0;
                return;
            }
        case TSF_SEGMENT_DELAY:
            e->samplesUntilNextSegment = (int)(e->parameters.attack * outSampleRate);
            if (e->samplesUntilNextSegment > 0)
            {
                e->segment = TSF_SEGMENT_ATTACK;
                e->segmentIsExponential = TSF_FALSE;
                e->level = e->parameters.start / 100.0f;
                e->slope = 1.0f / e->samplesUntilNextSegment;
                return;
            }
        case TSF_SEGMENT_ATTACK:
            e->samplesUntilNextSegment = (int)(e->parameters.hold * outSampleRate);
            if (e->samplesUntilNextSegment > 0)
            {
                e->segment = TSF_SEGMENT_HOLD;
                e->segmentIsExponential = TSF_FALSE;
                e->level = 1.0;
                e->slope = 0.0;
                return;
            }
        case TSF_SEGMENT_HOLD:
            e->samplesUntilNextSegment = (int)(e->parameters.decay * outSampleRate);
            if (e->samplesUntilNextSegment > 0)
            {
                e->segment = TSF_SEGMENT_DECAY;
                e->level = 1.0;
                if (e->exponentialDecay)
                {
                    // I don't truly understand this; just following what LinuxSampler does.
                    float mysterySlope = -9.226f / e->samplesUntilNextSegment;
                    e->slope = TSF_EXPF(mysterySlope);
                    e->segmentIsExponential = TSF_TRUE;
                    if (e->parameters.sustain > 0.0f)
                    {
                        // Again, this is following LinuxSampler's example, which is similar to
                        // SF2-style decay, where "decay" specifies the time it would take to
                        // get to zero, not to the sustain level.  The SFZ spec is not that
                        // specific about what "decay" means, so perhaps it's really supposed
                        // to specify the time to reach the sustain level.
                        e->samplesUntilNextSegment = (int)(TSF_LOG((e->parameters.sustain / 100.0) / e->level) / mysterySlope);
                    }
                }
                else
                {
                    e->slope = (e->parameters.sustain / 100.0f - 1.0f) / e->samplesUntilNextSegment;
                    e->segmentIsExponential = TSF_FALSE;
                }
                return;
            }
        case TSF_SEGMENT_DECAY:
            e->segment = TSF_SEGMENT_SUSTAIN;
            e->level = e->parameters.sustain / 100.0f;
            e->slope = 0.0f;
            e->samplesUntilNextSegment = 0x7FFFFFFF;
            e->segmentIsExponential = TSF_FALSE;
            return;
        case TSF_SEGMENT_SUSTAIN:
            e->segment = TSF_SEGMENT_RELEASE;
            e->samplesUntilNextSegment = (int)((e->parameters.release <= 0 ? TSF_FASTRELEASETIME : e->parameters.release) * outSampleRate);
            if (e->exponentialDecay)
            {
                // I don't truly understand this; just following what LinuxSampler does.
                float mysterySlope = -9.226f / e->samplesUntilNextSegment;
                e->slope = TSF_EXPF(mysterySlope);
                e->segmentIsExponential = TSF_TRUE;
            }
            else
            {
                e->slope = -e->level / e->samplesUntilNextSegment;
                e->segmentIsExponential = TSF_FALSE;
            }
            return;
        case TSF_SEGMENT_RELEASE:
        default:
            e->segment = TSF_SEGMENT_DONE;
            e->segmentIsExponential = TSF_FALSE;
            e->level = e->slope = 0;
            e->samplesUntilNextSegment = 0x7FFFFFF;
    }
}

static void tsf_voice_envelope_setup(struct tsf_voice_envelope* e, struct tsf_envelope* new_parameters, int midiNoteNumber, TSF_BOOL setExponentialDecay, float outSampleRate)
{
    e->parameters = *new_parameters;
    if (e->parameters.keynumToHold)
    {
        e->parameters.hold += e->parameters.keynumToHold * (60.0f - midiNoteNumber);
        e->parameters.hold = (e->parameters.hold < -10000.0f ? 0.0f : tsf_timecents2Secsf(e->parameters.hold));
    }
    if (e->parameters.keynumToDecay)
    {
        e->parameters.decay += e->parameters.keynumToDecay * (60.0f - midiNoteNumber);
        e->parameters.decay = (e->parameters.decay < -10000.0f ? 0.0f : tsf_timecents2Secsf(e->parameters.decay));
    }
    e->exponentialDecay = setExponentialDecay;
    tsf_voice_envelope_nextsegment(e, TSF_SEGMENT_NONE, outSampleRate);
}

static void tsf_voice_envelope_process(struct tsf_voice_envelope* e, int numSamples, float outSampleRate)
{
    if (e->slope)
    {
        if (e->segmentIsExponential) e->level *= TSF_POWF(e->slope, (float)numSamples);
        else e->level += (e->slope * numSamples);
    }
    if ((e->samplesUntilNextSegment -= numSamples) <= 0)
        tsf_voice_envelope_nextsegment(e, e->segment, outSampleRate);
}

static void tsf_voice_lowpass_setup(struct tsf_voice_lowpass* e, float Fc)
{
    // Lowpass filter from http://www.earlevel.com/main/2012/11/26/biquad-c-source-code/
    double K = TSF_TAN(TSF_PI * Fc), KK = K * K;
    double norm = 1 / (1 + K * e->QInv + KK);
    e->a0 = KK * norm;
    e->a1 = 2 * e->a0;
    e->b1 = 2 * (KK - 1) * norm;
    e->b2 = (1 - K * e->QInv + KK) * norm;
}

static float tsf_voice_lowpass_process(struct tsf_voice_lowpass* e, double In)
{
    double Out = In * e->a0 + e->z1; e->z1 = In * e->a1 + e->z2 - e->b1 * Out; e->z2 = In * e->a0 - e->b2 * Out; return (float)Out;
}

static void tsf_voice_lfo_setup(struct tsf_voice_lfo* e, float delay, int freqCents, float outSampleRate)
{
    e->samplesUntil = (int)(delay * outSampleRate);
    e->delta = (4.0f * tsf_cents2Hertz((float)freqCents) / outSampleRate);
    e->level = 0;
}

static void tsf_voice_lfo_process(struct tsf_voice_lfo* e, int blockSamples)
{
    if (e->samplesUntil > blockSamples) { e->samplesUntil -= blockSamples; return; }
    e->level += e->delta * blockSamples;
    if      (e->level >  1.0f) { e->delta = -e->delta; e->level =  2.0f - e->level; }
    else if (e->level < -1.0f) { e->delta = -e->delta; e->level = -2.0f - e->level; }
}

static void tsf_voice_kill(struct tsf_voice* v)
{
    v->region = TSF_NULL;
    v->playingPreset = -1;
}

static void tsf_voice_end(struct tsf_voice* v, float outSampleRate)
{
    tsf_voice_envelope_nextsegment(&v->ampenv, TSF_SEGMENT_SUSTAIN, outSampleRate);
    tsf_voice_envelope_nextsegment(&v->modenv, TSF_SEGMENT_SUSTAIN, outSampleRate);
    if (v->region->loop_mode == TSF_LOOPMODE_SUSTAIN)
    {
        // Continue playing, but stop looping.
        v->loopEnd = v->loopStart;
    }
}

static void tsf_voice_endquick(struct tsf_voice* v, float outSampleRate)
{
    v->ampenv.parameters.release = 0.0f; tsf_voice_envelope_nextsegment(&v->ampenv, TSF_SEGMENT_SUSTAIN, outSampleRate);
    v->modenv.parameters.release = 0.0f; tsf_voice_envelope_nextsegment(&v->modenv, TSF_SEGMENT_SUSTAIN, outSampleRate);
}

static void tsf_voice_calcpitchratio(struct tsf_voice* v, float outSampleRate)
{
    double note = v->playingKey, adjustedPitch;
    note += v->region->transpose;
    note += v->region->tune / 100.0;

    adjustedPitch = v->region->pitch_keycenter + (note - v->region->pitch_keycenter) * (v->region->pitch_keytrack / 100.0);
    if (v->curPitchWheel != 8192) adjustedPitch += ((4.0 * v->curPitchWheel / 16383.0) - 2.0);

    v->pitchInputTimecents = adjustedPitch * 100.0;
    v->pitchOutputFactor = v->region->sample_rate / (tsf_timecents2Secsd(v->region->pitch_keycenter * 100.0) * outSampleRate);
}

static void tsf_voice_render(tsf* f, struct tsf_voice* v, float* outputBuffer, int numSamples)
{
    struct tsf_region* region = v->region;
    float* input = f->fontSamples;
    float* outL = outputBuffer;
    float* outR = (f->outputmode == TSF_STEREO_UNWEAVED ? outL + numSamples : TSF_NULL);

    // Cache some values, to give them at least some chance of ending up in registers.
    TSF_BOOL updateModEnv = (region->modEnvToPitch || region->modEnvToFilterFc);
    TSF_BOOL updateModLFO = (v->modlfo.delta && (region->modLfoToPitch || region->modLfoToFilterFc || region->modLfoToVolume));
    TSF_BOOL updateVibLFO = (v->viblfo.delta && (region->vibLfoToPitch));
    TSF_BOOL isLooping    = (v->loopStart < v->loopEnd);
    unsigned int tmpLoopStart = v->loopStart, tmpLoopEnd = v->loopEnd;
    double tmpSampleEndDbl = (double)v->sampleEnd, tmpLoopEndDbl = (double)tmpLoopEnd + 1.0;
    double tmpSourceSamplePosition = v->sourceSamplePosition;
    struct tsf_voice_lowpass tmpLowpass = v->lowpass;

    TSF_BOOL dynamicLowpass = (region->modLfoToFilterFc || region->modEnvToFilterFc);
    float tmpSampleRate, tmpInitialFilterFc, tmpModLfoToFilterFc, tmpModEnvToFilterFc;

    TSF_BOOL dynamicPitchRatio = (region->modLfoToPitch || region->modEnvToPitch || region->vibLfoToPitch);
    double pitchRatio;
    float tmpModLfoToPitch, tmpVibLfoToPitch, tmpModEnvToPitch;

    TSF_BOOL dynamicGain = (region->modLfoToVolume != 0);
    float noteGain = 0, tmpModLfoToVolume;

    if (dynamicLowpass) tmpSampleRate = f->outSampleRate, tmpInitialFilterFc = (float)region->initialFilterFc, tmpModLfoToFilterFc = (float)region->modLfoToFilterFc, tmpModEnvToFilterFc = (float)region->modEnvToFilterFc;
    else tmpSampleRate = 0, tmpInitialFilterFc = 0, tmpModLfoToFilterFc = 0, tmpModEnvToFilterFc = 0;

    if (dynamicPitchRatio) pitchRatio = 0, tmpModLfoToPitch = (float)region->modLfoToPitch, tmpVibLfoToPitch = (float)region->vibLfoToPitch, tmpModEnvToPitch = (float)region->modEnvToPitch;
    else pitchRatio = tsf_timecents2Secsd(v->pitchInputTimecents) * v->pitchOutputFactor, tmpModLfoToPitch = 0, tmpVibLfoToPitch = 0, tmpModEnvToPitch = 0;

    if (dynamicGain) tmpModLfoToVolume = (float)region->modLfoToVolume * 0.1f;
    else noteGain = tsf_decibelsToGain(v->noteGainDB), tmpModLfoToVolume = 0;

    while (numSamples)
    {
        float gainMono, gainLeft, gainRight;
        int blockSamples = (numSamples > TSF_RENDER_EFFECTSAMPLEBLOCK ? TSF_RENDER_EFFECTSAMPLEBLOCK : numSamples);
        numSamples -= blockSamples;

        if (dynamicLowpass)
        {
            float fres = tmpInitialFilterFc + v->modlfo.level * tmpModLfoToFilterFc + v->modenv.level * tmpModEnvToFilterFc;
            tmpLowpass.active = (fres <= 13500.0f);
            if (tmpLowpass.active) tsf_voice_lowpass_setup(&tmpLowpass, tsf_cents2Hertz(fres) / tmpSampleRate);
        }

        if (dynamicPitchRatio)
            pitchRatio = tsf_timecents2Secsd(v->pitchInputTimecents + (v->modlfo.level * tmpModLfoToPitch + v->viblfo.level * tmpVibLfoToPitch + v->modenv.level * tmpModEnvToPitch)) * v->pitchOutputFactor;

        if (dynamicGain)
            noteGain = tsf_decibelsToGain(v->noteGainDB + (v->modlfo.level * tmpModLfoToVolume));

        gainMono = noteGain * v->ampenv.level;

        // Update EG.
        tsf_voice_envelope_process(&v->ampenv, blockSamples, f->outSampleRate);
        if (updateModEnv) tsf_voice_envelope_process(&v->modenv, blockSamples, f->outSampleRate);

        // Update LFOs.
        if (updateModLFO) tsf_voice_lfo_process(&v->modlfo, blockSamples);
        if (updateVibLFO) tsf_voice_lfo_process(&v->viblfo, blockSamples);

        switch (f->outputmode)
        {
            case TSF_STEREO_INTERLEAVED:
                gainLeft = gainMono * f->globalPanFactorLeft * v->panFactorLeft, gainRight = gainMono * f->globalPanFactorRight * v->panFactorRight;
                while (blockSamples-- && tmpSourceSamplePosition < tmpSampleEndDbl)
                {
                    unsigned int pos = (unsigned int)tmpSourceSamplePosition, nextPos = (pos >= tmpLoopEnd && isLooping ? tmpLoopStart : pos + 1);

                    // Simple linear interpolation.
                    float alpha = (float)(tmpSourceSamplePosition - pos), val = (input[pos] * (1.0f - alpha) + input[nextPos] * alpha);

                    // Low-pass filter.
                    if (tmpLowpass.active) val = tsf_voice_lowpass_process(&tmpLowpass, val);

                    *outL++ += val * gainLeft;
                    *outL++ += val * gainRight;

                    // Next sample.
                    tmpSourceSamplePosition += pitchRatio;
                    if (tmpSourceSamplePosition >= tmpLoopEndDbl && isLooping) tmpSourceSamplePosition -= (tmpLoopEnd - tmpLoopStart + 1.0);
                }
                break;

            case TSF_STEREO_UNWEAVED:
                gainLeft = gainMono * f->globalPanFactorLeft * v->panFactorLeft, gainRight = gainMono * f->globalPanFactorRight * v->panFactorRight;
                while (blockSamples-- && tmpSourceSamplePosition < tmpSampleEndDbl)
                {
                    unsigned int pos = (unsigned int)tmpSourceSamplePosition, nextPos = (pos >= tmpLoopEnd && isLooping ? tmpLoopStart : pos + 1);

                    // Simple linear interpolation.
                    float alpha = (float)(tmpSourceSamplePosition - pos), val = (input[pos] * (1.0f - alpha) + input[nextPos] * alpha);

                    // Low-pass filter.
                    if (tmpLowpass.active) val = tsf_voice_lowpass_process(&tmpLowpass, val);

                    *outL++ += val * gainLeft;
                    *outR++ += val * gainRight;

                    // Next sample.
                    tmpSourceSamplePosition += pitchRatio;
                    if (tmpSourceSamplePosition >= tmpLoopEndDbl && isLooping) tmpSourceSamplePosition -= (tmpLoopEnd - tmpLoopStart + 1.0);
                }
                break;

            case TSF_MONO:
                gainMono *= (f->globalPanFactorLeft + f->globalPanFactorRight) * .5f;
                while (blockSamples-- && tmpSourceSamplePosition < tmpSampleEndDbl)
                {
                    unsigned int pos = (unsigned int)tmpSourceSamplePosition, nextPos = (pos >= tmpLoopEnd && isLooping ? tmpLoopStart : pos + 1);

                    // Simple linear interpolation.
                    float alpha = (float)(tmpSourceSamplePosition - pos), val = (input[pos] * (1.0f - alpha) + input[nextPos] * alpha);

                    // Low-pass filter.
                    if (tmpLowpass.active) val = tsf_voice_lowpass_process(&tmpLowpass, val);

                    *outL++ += val * gainMono;

                    // Next sample.
                    tmpSourceSamplePosition += pitchRatio;
                    if (tmpSourceSamplePosition >= tmpLoopEndDbl && isLooping) tmpSourceSamplePosition -= (tmpLoopEnd - tmpLoopStart + 1.0);
                }
                break;
        }

        if (tmpSourceSamplePosition >= tmpSampleEndDbl || v->ampenv.segment == TSF_SEGMENT_DONE)
        {
            tsf_voice_kill(v);
            return;
        }
    }

    v->sourceSamplePosition = tmpSourceSamplePosition;
    if (tmpLowpass.active || dynamicLowpass) v->lowpass = tmpLowpass;
}

TSFDEF tsf* tsf_load(struct tsf_stream* stream)
{
    tsf* res = TSF_NULL;
    struct tsf_riffchunk chunkHead;
    struct tsf_riffchunk chunkList;
    struct tsf_hydra hydra;
    float* fontSamples = TSF_NULL;
    int fontSampleCount;

    if (!tsf_riffchunk_read(TSF_NULL, &chunkHead, stream) || !TSF_FourCCEquals(chunkHead.id, "sfbk"))
    {
        //if (e) *e = TSF_INVALID_NOSF2HEADER;
        return res;
    }

    // Read hydra and locate sample data.
    TSF_MEMSET(&hydra, 0, sizeof(hydra));
    while (tsf_riffchunk_read(&chunkHead, &chunkList, stream))
    {
        struct tsf_riffchunk chunk;
        if (TSF_FourCCEquals(chunkList.id, "pdta"))
        {
            while (tsf_riffchunk_read(&chunkList, &chunk, stream))
            {
                #define HandleChunk(chunkName) (TSF_FourCCEquals(chunk.id, #chunkName) && !(chunk.size % chunkName##SizeInFile)) \
                    { \
                        int num = chunk.size / chunkName##SizeInFile, i; \
                        hydra.chunkName##Num = num; \
                        hydra.chunkName##s = (struct tsf_hydra_##chunkName*)TSF_MALLOC(num * sizeof(struct tsf_hydra_##chunkName)); \
                        for (i = 0; i < num; ++i) tsf_hydra_read_##chunkName(&hydra.chunkName##s[i], stream); \
                    }
                enum
                {
                    phdrSizeInFile = 38, pbagSizeInFile =  4, pmodSizeInFile = 10,
                    pgenSizeInFile =  4, instSizeInFile = 22, ibagSizeInFile =  4,
                    imodSizeInFile = 10, igenSizeInFile =  4, shdrSizeInFile = 46
                };
                if      HandleChunk(phdr) else if HandleChunk(pbag) else if HandleChunk(pmod)
                else if HandleChunk(pgen) else if HandleChunk(inst) else if HandleChunk(ibag)
                else if HandleChunk(imod) else if HandleChunk(igen) else if HandleChunk(shdr)
                else stream->skip(stream->data, chunk.size);
                #undef HandleChunk
            }
        }
        else if (TSF_FourCCEquals(chunkList.id, "sdta"))
        {
            while (tsf_riffchunk_read(&chunkList, &chunk, stream))
            {
                if (TSF_FourCCEquals(chunk.id, "smpl"))
                {
                    tsf_load_samples(&fontSamples, &fontSampleCount, &chunk, stream);
                }
                else stream->skip(stream->data, chunk.size);
            }
        }
        else stream->skip(stream->data, chunkList.size);
    }
    if (!hydra.phdrs || !hydra.pbags || !hydra.pmods || !hydra.pgens || !hydra.insts || !hydra.ibags || !hydra.imods || !hydra.igens || !hydra.shdrs)
    {
        //if (e) *e = TSF_INVALID_INCOMPLETE;
    }
    else if (fontSamples == TSF_NULL)
    {
        //if (e) *e = TSF_INVALID_NOSAMPLEDATA;
    }
    else
    {
        res = (tsf*)TSF_MALLOC(sizeof(tsf));
        TSF_MEMSET(res, 0, sizeof(tsf));
        res->presetNum = hydra.phdrNum - 1;
        res->presets = (struct tsf_preset*)TSF_MALLOC(res->presetNum * sizeof(struct tsf_preset));
        res->fontSamples = fontSamples;
        res->fontSampleCount = fontSampleCount;
        res->outSampleRate = 44100.0f;
        res->globalPanFactorLeft = res->globalPanFactorRight = 1.0f;
        fontSamples = TSF_NULL; //don't free below
        tsf_load_presets(res, &hydra);
    }
    TSF_FREE(hydra.phdrs); TSF_FREE(hydra.pbags); TSF_FREE(hydra.pmods);
    TSF_FREE(hydra.pgens); TSF_FREE(hydra.insts); TSF_FREE(hydra.ibags);
    TSF_FREE(hydra.imods); TSF_FREE(hydra.igens); TSF_FREE(hydra.shdrs);
    TSF_FREE(fontSamples);
    return res;
}

TSFDEF void tsf_close(tsf* f)
{
    struct tsf_preset *preset, *presetEnd;
    if (!f) return;
    for (preset = f->presets, presetEnd = preset + f->presetNum; preset != presetEnd; preset++)
        TSF_FREE(preset->regions);
    TSF_FREE(f->presets);
    TSF_FREE(f->fontSamples);
    TSF_FREE(f->voices);
    TSF_FREE(f->outputSamples);
    TSF_FREE(f);
}

TSFDEF int tsf_get_presetindex(const tsf* f, int bank, int preset_number)
{
    const struct tsf_preset *presets;
    int i, iMax;
    for (presets = f->presets, i = 0, iMax = f->presetNum; i < iMax; i++)
        if (presets[i].preset == preset_number && presets[i].bank == bank)
            return i;
    return -1;
}

TSFDEF int tsf_get_presetcount(const tsf* f)
{
    return f->presetNum;
}

TSFDEF const char* tsf_get_presetname(const tsf* f, int preset)
{
    return (preset < 0 || preset >= f->presetNum ? TSF_NULL : f->presets[preset].presetName);
}

TSFDEF const char* tsf_bank_get_presetname(const tsf* f, int bank, int preset_number)
{
    return tsf_get_presetname(f, tsf_get_presetindex(f, bank, preset_number));
}

TSFDEF void tsf_set_output(tsf* f, enum TSFOutputMode outputmode, int samplerate, float global_gain_db)
{
    f->outSampleRate = (float)(samplerate >= 1 ? samplerate : 44100.0f);
    f->outputmode = outputmode;
    f->globalGainDB = global_gain_db;
}

TSFDEF void tsf_set_panning(tsf* f, float pan_factor_left, float pan_factor_right)
{
    f->globalPanFactorLeft = pan_factor_left;
    f->globalPanFactorRight = pan_factor_right;
}

TSFDEF void tsf_note_on(tsf* f, int preset_index, int key, float vel)
{
    int midiVelocity = (int)(vel * 127), voicePlayIndex;
    TSF_BOOL haveGroupedNotesPlaying = TSF_FALSE;
    struct tsf_voice *v, *vEnd; struct tsf_region *region, *regionEnd;

    if (preset_index < 0 || preset_index >= f->presetNum) return;
    if (vel <= 0.0f) { tsf_note_off(f, preset_index, key); return; }

    // Are any grouped notes playing? (Needed for group stopping) Also stop any voices still playing this note.
    for (v = f->voices, vEnd = v + f->voiceNum; v != vEnd; v++)
    {
        if (v->playingPreset != preset_index) continue;
        if (v->region->group) haveGroupedNotesPlaying = TSF_TRUE;
    }

    // Play all matching regions.
    voicePlayIndex = f->voicePlayIndex++;
    for (region = f->presets[preset_index].regions, regionEnd = region + f->presets[preset_index].regionNum; region != regionEnd; region++)
    {
        struct tsf_voice* voice = TSF_NULL; double adjustedPan; TSF_BOOL doLoop; float filterQDB;
        if (key < region->lokey || key > region->hikey || midiVelocity < region->lovel || midiVelocity > region->hivel) continue;

        if (haveGroupedNotesPlaying && region->group)
            for (v = f->voices, vEnd = v + f->voiceNum; v != vEnd; v++)
                if (v->playingPreset == preset_index && v->region->group == region->group)
                    tsf_voice_endquick(v, f->outSampleRate);

        for (v = f->voices, vEnd = v + f->voiceNum; v != vEnd; v++) if (v->playingPreset == -1) { voice = v; break; }
        if (!voice)
        {
            f->voiceNum += 4;
            f->voices = (struct tsf_voice*)TSF_REALLOC(f->voices, f->voiceNum * sizeof(struct tsf_voice));
            voice = &f->voices[f->voiceNum - 4];
            voice[1].playingPreset = voice[2].playingPreset = voice[3].playingPreset = -1;
        }

        voice->region = region;
        voice->playingPreset = preset_index;
        voice->playingKey = key;
        voice->playIndex = voicePlayIndex;

        // Pitch.
        voice->curPitchWheel = 8192;
        tsf_voice_calcpitchratio(voice, f->outSampleRate);

        // Gain.
        voice->noteGainDB = f->globalGainDB + region->volume;
        // Thanks to <http:://www.drealm.info/sfz/plj-sfz.xhtml> for explaining the velocity curve in a way that I could understand, although they mean "log10" when they say "log".
        voice->noteGainDB += (float)(-20.0 * TSF_LOG10(1.0 / vel));
        // The SFZ spec is silent about the pan curve, but a 3dB pan law seems common. This sqrt() curve matches what Dimension LE does; Alchemy Free seems closer to sin(adjustedPan * pi/2).
        adjustedPan = (region->pan + 100.0) / 200.0;
        voice->panFactorLeft = (float)TSF_SQRT(1.0 - adjustedPan);
        voice->panFactorRight = (float)TSF_SQRT(adjustedPan);

        // Offset/end.
        voice->sourceSamplePosition = region->offset;
        voice->sampleEnd = f->fontSampleCount;
        if (region->end > 0 && region->end < voice->sampleEnd) voice->sampleEnd = region->end + 1;

        // Loop.
        doLoop = (region->loop_mode != TSF_LOOPMODE_NONE && region->loop_start < region->loop_end);
        voice->loopStart = (doLoop ? region->loop_start : 0);
        voice->loopEnd = (doLoop ? region->loop_end : 0);

        // Setup envelopes.
        tsf_voice_envelope_setup(&voice->ampenv, &region->ampenv, key, TSF_TRUE, f->outSampleRate);
        tsf_voice_envelope_setup(&voice->modenv, &region->modenv, key, TSF_FALSE, f->outSampleRate);

        // Setup lowpass filter.
        filterQDB = region->initialFilterQ / 10.0f;
        voice->lowpass.QInv = 1.0 / TSF_POW(10.0, (filterQDB / 20.0));
        voice->lowpass.z1 = voice->lowpass.z2 = 0;
        voice->lowpass.active = (region->initialFilterFc <= 13500);
        if (voice->lowpass.active) tsf_voice_lowpass_setup(&voice->lowpass, tsf_cents2Hertz((float)region->initialFilterFc) / f->outSampleRate);

        // Setup LFO filters.
        tsf_voice_lfo_setup(&voice->modlfo, region->delayModLFO, region->freqModLFO, f->outSampleRate);
        tsf_voice_lfo_setup(&voice->viblfo, region->delayVibLFO, region->freqVibLFO, f->outSampleRate);
    }
}

TSFDEF void tsf_bank_note_on(tsf* f, int bank, int preset_number, int key, float vel)
{
    tsf_note_on(f, tsf_get_presetindex(f, bank, preset_number), key, vel);
}

TSFDEF void tsf_note_off(tsf* f, int preset_index, int key)
{
    struct tsf_voice *v = f->voices, *vEnd = v + f->voiceNum, *vMatchFirst = TSF_NULL, *vMatchLast;
    for (; v != vEnd; v++)
    {
        //Find the first and last entry in the voices list with matching preset, key and look up the smallest play index
        if (v->playingPreset != preset_index || v->playingKey != key || v->ampenv.segment >= TSF_SEGMENT_RELEASE) continue;
        else if (!vMatchFirst || v->playIndex < vMatchFirst->playIndex) vMatchFirst = vMatchLast = v;
        else if (v->playIndex == vMatchFirst->playIndex) vMatchLast = v;
    }
    if (!vMatchFirst) return;
    for (v = vMatchFirst; v <= vMatchLast; v++)
    {
        //Stop all voices with matching preset, key and the smallest play index which was enumerated above
        if (v != vMatchFirst && v != vMatchLast &&
            (v->playIndex != vMatchFirst->playIndex || v->playingPreset != preset_index || v->playingKey != key || v->ampenv.segment >= TSF_SEGMENT_RELEASE)) continue;
        tsf_voice_end(v, f->outSampleRate);
    }
}

TSFDEF void tsf_bank_note_off(tsf* f, int bank, int preset_number, int key)
{
    tsf_note_off(f, tsf_get_presetindex(f, bank, preset_number), key);
}

TSFDEF void tsf_note_off_all(tsf* f)
{
    struct tsf_voice *v = f->voices, *vEnd = v + f->voiceNum;
    for (; v != vEnd; v++) if (v->playingPreset != -1 && v->ampenv.segment < TSF_SEGMENT_RELEASE)
        tsf_voice_end(v, f->outSampleRate);
}

TSFDEF void tsf_render_short(tsf* f, short* buffer, int samples, int flag_mixing)
{
    float *floatSamples;
    int channelSamples = (f->outputmode == TSF_MONO ? 1 : 2) * samples, floatBufferSize = channelSamples * sizeof(float);
    short* bufferEnd = buffer + channelSamples;
    if (floatBufferSize > f->outputSampleSize)
    {
        TSF_FREE(f->outputSamples);
        f->outputSamples = (float*)TSF_MALLOC(floatBufferSize);
        f->outputSampleSize = floatBufferSize;
    }

    tsf_render_float(f, f->outputSamples, samples, TSF_FALSE);

    floatSamples = f->outputSamples;
    if (flag_mixing)
        while (buffer != bufferEnd)
        {
            float v = *floatSamples++;
            int vi = *buffer + (v < -1.00004566f ? (int)-32768 : (v > 1.00001514f ? (int)32767 : (int)(v * 32767.5f)));
            *buffer++ = (vi < -32768 ? (short)-32768 : (vi > 32767 ? (short)32767 : (short)vi));
        }
    else
        while (buffer != bufferEnd)
        {
            float v = *floatSamples++;
            *buffer++ = (v < -1.00004566f ? (short)-32768 : (v > 1.00001514f ? (short)32767 : (short)(v * 32767.5f)));
        }
}

TSFDEF void tsf_render_float(tsf* f, float* buffer, int samples, int flag_mixing)
{
    struct tsf_voice *v = f->voices, *vEnd = v + f->voiceNum;
    if (!flag_mixing) TSF_MEMSET(buffer, 0, (f->outputmode == TSF_MONO ? 1 : 2) * sizeof(float) * samples);
    for (; v != vEnd; v++)
        if (v->playingPreset != -1)
            tsf_voice_render(f, v, buffer, samples);
}

#ifdef __cplusplus
}
#endif


//===================MIDI


#if !defined(TML_MALLOC) || !defined(TML_FREE) || !defined(TML_REALLOC)
#  include <stdlib.h>
#  define TML_MALLOC  malloc
#  define TML_FREE    free
#  define TML_REALLOC realloc
#endif

#if !defined(TML_MEMCPY)
#  include <string.h>
#  define TML_MEMCPY  memcpy
#endif

#define TML_NULL 0

////crash on errors and warnings to find broken midi files while debugging
//#define TML_ERROR(msg) *(int*)0 = 0xbad;
//#define TML_WARN(msg)  *(int*)0 = 0xf00d;

////print errors and warnings
//#define TML_ERROR(msg) printf("ERROR: %s\n", msg);
//#define TML_WARN(msg)  printf("WARNING: %s\n", msg);

#ifndef TML_ERROR
#define TML_ERROR(msg)
#endif

#ifndef TML_WARN
#define TML_WARN(msg)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TML_NO_STDIO
static int tml_stream_stdio_read(FILE* f, void* ptr, unsigned int size) { return (int)fread(ptr, 1, size, f); }
TMLDEF tml_message* tml_load_filename(const char* filename)
{
    struct tml_message* res;
    struct tml_stream stream = { TML_NULL, (int(*)(void*,void*,unsigned int))&tml_stream_stdio_read };
    #if __STDC_WANT_SECURE_LIB__
    FILE* f = TML_NULL; fopen_s(&f, filename, "rb");
    #else
    FILE* f = fopen(filename, "rb");
    #endif
    if (!f) { TML_ERROR("File not found"); return 0; }
    stream.data = f;
    res = tml_load(&stream);
    fclose(f);
    return res;
}
#endif

struct tml_stream_memory { const char* buffer; unsigned int total, pos; };
static int tml_stream_memory_read(struct tml_stream_memory* m, void* ptr, unsigned int size) { if (size > m->total - m->pos) size = m->total - m->pos; TML_MEMCPY(ptr, m->buffer+m->pos, size); m->pos += size; return size; }
TMLDEF struct tml_message* tml_load_memory(const void* buffer, int size)
{
    struct tml_stream stream = { TML_NULL, (int(*)(void*,void*,unsigned int))&tml_stream_memory_read };
    struct tml_stream_memory f = { 0, 0, 0 };
    f.buffer = (const char*)buffer;
    f.total = size;
    stream.data = &f;
    return tml_load(&stream);
}

struct tml_track
{
    unsigned int Idx, End, Ticks;
};

struct tml_tempomsg
{
    unsigned int time;
    unsigned char type, Tempo[3];
    tml_message* next;
};

struct tml_parser
{
    struct tml_stream *stream;
    unsigned char *buf, *buf_end;
    int last_status, message_array_size, message_count;
};

enum TMLSystemType
{
    TML_TEXT  = 0x01, TML_COPYRIGHT = 0x02, TML_TRACK_NAME    = 0x03, TML_INST_NAME      = 0x04, TML_LYRIC         = 0x05, TML_MARKER          = 0x06, TML_CUE_POINT    = 0x07,
    TML_EOT   = 0x2f, TML_SET_TEMPO = 0x51, TML_SMPTE_OFFSET  = 0x54, TML_TIME_SIGNATURE = 0x58, TML_KEY_SIGNATURE = 0x59, TML_SEQUENCER_EVENT = 0x7f,
    TML_SYSEX = 0xf0, TML_TIME_CODE = 0xf1, TML_SONG_POSITION = 0xf2, TML_SONG_SELECT    = 0xf3, TML_TUNE_REQUEST  = 0xf6, TML_EOX             = 0xf7,
    TML_SYNC  = 0xf8, TML_TICK      = 0xf9, TML_START         = 0xfa, TML_CONTINUE       = 0xfb, TML_STOP          = 0xfc, TML_ACTIVE_SENSING  = 0xfe, TML_SYSTEM_RESET = 0xff
};

int tml_readbyte(struct tml_parser* p)
{
    return (p->buf == p->buf_end ? -1 : *(p->buf++));
}

int tml_readvariablelength(struct tml_parser* p)
{
    unsigned int res = 0, i = 0;
    unsigned char c;
    for (; i != 4; i++)
    {
        if (p->buf == p->buf_end) { TML_WARN("Unexpected end of file"); return -1; }
        c = *(p->buf++);
        if (c & 0x80) res = ((res | (c & 0x7F)) << 7);
        else return (int)(res | c);
    }
    TML_WARN("Invalid variable length byte count"); return -1;
}

static int tml_parsemessage(tml_message** f, struct tml_parser* p)
{
    int deltatime = tml_readvariablelength(p), status = tml_readbyte(p);
    tml_message* evt;

    if (deltatime & 0xFFF00000) deltatime = 0; //throw away delays that are insanely high for malformatted midis
    if (status < 0) { TML_WARN("Unexpected end of file"); return -1; }
    if ((status & 0x80) == 0)
    {
        // Invalid, use same status as before
        if ((p->last_status & 0x80) == 0) { TML_WARN("Undefined status and invalid running status"); return -1; }
        p->buf--;
        status = p->last_status;
    }
    else p->last_status = status;

    if (p->message_array_size == p->message_count)
    {
        //start allocated memory size of message array at 64, double each time until 8192, then add 1024 entries until done
        p->message_array_size += (!p->message_array_size ? 64 : (p->message_array_size > 4096 ? 1024 : p->message_array_size));
        *f = (tml_message*)TML_REALLOC(*f, p->message_array_size * sizeof(tml_message));
        if (!*f) { TML_ERROR("Out of memory"); return -1; }
    }
    evt = *f + p->message_count;

    //check what message we have
    if ((status == TML_SYSEX) || (status == TML_EOX)) //sysex
    {
        //sysex messages are not handled
        p->buf += tml_readvariablelength(p);
        if (p->buf > p->buf_end) { TML_WARN("Unexpected end of file"); p->buf = p->buf_end; return -1; }
        evt->type = 0;
    }
    else if (status == 0xFF) //meta events
    {
        int meta_type = tml_readbyte(p), buflen = tml_readvariablelength(p);
        unsigned char* metadata = p->buf;
        if (meta_type < 0) { TML_WARN("Unexpected end of file"); return -1; }
        if (buflen > 0 && (p->buf += buflen) > p->buf_end) { TML_WARN("Unexpected end of file"); p->buf = p->buf_end; return -1; }

        switch (meta_type)
        {
            case TML_EOT:
                if (buflen != 0) { TML_WARN("Invalid length for EndOfTrack event"); return -1; }
                if (!deltatime) return TML_EOT; //no need to store this message
                evt->type = TML_EOT;
                break;

            case TML_SET_TEMPO:
                if (buflen != 3) { TML_WARN("Invalid length for SetTempo meta event"); return -1; }
                evt->type = TML_SET_TEMPO;
                ((struct tml_tempomsg*)evt)->Tempo[0] = metadata[0];
                ((struct tml_tempomsg*)evt)->Tempo[1] = metadata[1];
                ((struct tml_tempomsg*)evt)->Tempo[2] = metadata[2];
                break;

            default:
                evt->type = 0;
        }
    }
    else //channel message
    {
        int param;
        if ((param = tml_readbyte(p)) < 0) { TML_WARN("Unexpected end of file"); return -1; }
        evt->key = (param & 0x7f);
        evt->channel = (status & 0x0f);
        switch (evt->type = (status & 0xf0))
        {
            case TML_NOTE_OFF:
            case TML_NOTE_ON:
            case TML_KEY_PRESSURE:
            case TML_CONTROL_CHANGE:
                if ((param = tml_readbyte(p)) < 0) { TML_WARN("Unexpected end of file"); return -1; }
                evt->velocity = (param & 0x7f);
                break;

            case TML_PITCH_BEND:
                if ((param = tml_readbyte(p)) < 0) { TML_WARN("Unexpected end of file"); return -1; }
                evt->pitch_bend = ((evt->key & 0x7f) << 7) | (param & 0x7f);
                break;

            case TML_PROGRAM_CHANGE:
            case TML_CHANNEL_PRESSURE:
                evt->velocity = 0;
                break;

            default: //ignore system/manufacture messages
                evt->type = 0;
                break;
        }
    }

    if (deltatime || evt->type)
    {
        evt->time = deltatime;
        p->message_count++;
    }
    return evt->type;
}

TMLDEF tml_message* tml_load(struct tml_stream* stream)
{
    int num_tracks, division, trackbufsize = 0;
    unsigned char midi_header[15], *trackbuf = TML_NULL;
    struct tml_message* messages = TML_NULL;
    struct tml_track *tracks, *t, *tracksEnd;
    struct tml_parser p = { stream };

    // Parse MIDI header
    if (stream->read(stream->data, midi_header, 14) != 14) { TML_ERROR("Unexpected end of file"); return messages; }
    if (midi_header[0] != 'M' || midi_header[1] != 'T' || midi_header[2] != 'h' || midi_header[3] != 'd' ||
        midi_header[7] != 6   || midi_header[9] >  2) { TML_ERROR("Doesn't look like a MIDI file: invalid MThd header"); return messages; }
    if (midi_header[12] & 0x80) { TML_ERROR("File uses unsupported SMPTE timing"); return messages; }
    num_tracks = (int)(midi_header[10] << 8) | midi_header[11];
    division = (int)(midi_header[12] << 8) | midi_header[13]; //division is ticks per beat (quarter-note)
    if (num_tracks <= 0 && division <= 0) { TML_ERROR("Doesn't look like a MIDI file: invalid track or division values"); return messages; }

    // Allocate temporary tracks array for parsing
    tracks = (struct tml_track*)TML_MALLOC(sizeof(struct tml_track) * num_tracks);
    tracksEnd = &tracks[num_tracks];
    for (t = tracks; t != tracksEnd; t++) t->Idx = t->End = t->Ticks = 0;

    // Read all messages for all tracks
    for (t = tracks; t != tracksEnd; t++)
    {
        unsigned char track_header[8];
        int track_length;
        if (stream->read(stream->data, track_header, 8) != 8) { TML_WARN("Unexpected end of file"); break; }
        if (track_header[0] != 'M' || track_header[1] != 'T' || track_header[2] != 'r' || track_header[3] != 'k')
            { TML_WARN("Invalid MTrk header"); break; }

        // Get size of track data and read into buffer (allocate bigger buffer if needed)
        track_length = track_header[7] | (track_header[6] << 8) | (track_header[5] << 16) | (track_header[4] << 24);
        if (track_length < 0) { TML_WARN("Invalid MTrk header"); break; }
        if (trackbufsize < track_length) { TML_FREE(trackbuf); trackbuf = (unsigned char*)TML_MALLOC(trackbufsize = track_length); }
        if (stream->read(stream->data, trackbuf, track_length) != track_length) { TML_WARN("Unexpected end of file"); break; }

        t->Idx = p.message_count;
        for (p.buf_end = (p.buf = trackbuf) + track_length; p.buf != p.buf_end;)
        {
            int type = tml_parsemessage(&messages, &p);
            if (type == TML_EOT || type < 0) break; //file end or illegal data encountered
        }
        if (p.buf != p.buf_end) { TML_WARN( "Track length did not match data length"); }
        t->End = p.message_count;
    }
    TML_FREE(trackbuf);

    // Change message time signature from delta ticks to actual msec values and link messages ordered by time
    if (p.message_count)
    {
        tml_message *PrevMessage = TML_NULL, *Msg, *MsgEnd, Swap;
        unsigned int ticks = 0, tempo_ticks = 0; //tick counter and value at last tempo change
        int step_smallest, msec, tempo_msec = 0; //msec value at last tempo change
        double ticks2time = 480000 / (1000.0 * division); //milliseconds per tick

        // Loop through all messages over all tracks ordered by time
        for (step_smallest = 0; step_smallest != 0x7fffffff; ticks += step_smallest)
        {
            step_smallest = 0x7fffffff;
            msec = tempo_msec + (int)((ticks - tempo_ticks) * ticks2time);
            for (t = tracks; t != tracksEnd; t++)
            {
                if (t->Idx == t->End) continue;
                for (Msg = &messages[t->Idx], MsgEnd = &messages[t->End]; Msg != MsgEnd && t->Ticks + Msg->time == ticks; Msg++, t->Idx++)
                {
                    t->Ticks += Msg->time;
                    if (Msg->type == TML_SET_TEMPO)
                    {
                        unsigned char* Tempo = ((struct tml_tempomsg*)Msg)->Tempo;
                        ticks2time = ((Tempo[0]<<16)|(Tempo[1]<<8)|Tempo[2])/(1000.0 * division);
                        tempo_msec = msec;
                        tempo_ticks = ticks;
                    }
                    else if (Msg->type)
                    {
                        Msg->time = msec;
                        if (PrevMessage) { PrevMessage->next = Msg; PrevMessage = Msg; }
                        else { Swap = *Msg; *Msg = *messages; *messages = Swap; PrevMessage = messages; }
                    }
                }
                if (Msg != MsgEnd && t->Ticks + Msg->time > ticks)
                {
                    int step = (int)(t->Ticks + Msg->time - ticks);
                    if (step < step_smallest) step_smallest = step;
                }
            }
        }
        if (PrevMessage) PrevMessage->next = TML_NULL;
        else p.message_count = 0;
    }
    TML_FREE(tracks);

    if (p.message_count == 0)
    {
        TML_FREE(messages);
        messages = TML_NULL;
    }

    return messages;
}

TMLDEF int tml_get_info(tml_message* Msg, int* out_used_channels, int* out_used_programs, int* out_total_notes, unsigned int* out_time_first_note, unsigned int* out_time_length)
{
    int used_programs = 0, used_channels = 0, total_notes = 0;
    unsigned int time_first_note = 0xffffffff, time_length = 0;
    unsigned char channels[16] = { 0 }, programs[128] = { 0 };
    for (;Msg; Msg = Msg->next)
    {
        time_length = Msg->time;
        if (Msg->type == TML_PROGRAM_CHANGE && !programs[(int)Msg->program]) { programs[(int)Msg->program] = 1; used_programs++; }
        if (Msg->type != TML_NOTE_ON) continue;
        if (time_first_note == 0xffffffff) time_first_note = time_length;
        if (!channels[Msg->channel]) { channels[Msg->channel] = 1; used_channels++; }
        total_notes++;
    }
    if (time_first_note == 0xffffffff) time_first_note = 0;
    if (out_used_channels  ) *out_used_channels   = used_channels;
    if (out_used_programs  ) *out_used_programs   = used_programs;
    if (out_total_notes    ) *out_total_notes     = total_notes;
    if (out_time_first_note) *out_time_first_note = time_first_note;
    if (out_time_length    ) *out_time_length     = time_length;
    return total_notes;
}

TMLDEF void tml_free(tml_message* f)
{
    TML_FREE(f);
}

#ifdef __cplusplus
}

TMLDEF tml_message* tml_load_tsf_stream(struct tsf_stream* stream) { return tml_load((struct tml_stream*)stream); }
#endif
