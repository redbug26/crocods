#include "sound.h"
#include "plateform.h"

#include "emu2149.h"


#ifdef SOUNDV2

PSG psg;

#define CPC_CLK 1000000

GB_sample_t *sndbuf;
int sndbufend, sndbufbeg;

u8 PlaySound(core_crocods_t *gb)
{
    return 1;
}

void PauseSound(core_crocods_t *gb)
{
}

void Reset8912(core_crocods_t *gb)
{
    PSG_reset(&psg);
}

void Write8912(core_crocods_t *gb, int reg, int val)
{
    PSG_writeReg(&psg, reg, val);

    return;
}

int Read8912(core_crocods_t *gb, int r)
{
    return (u8)(psg.reg[r & 0x1f]);
}


void disableSound(core_crocods_t *core)
{
    core->soundEnabled = 0;
}

void enableSound(core_crocods_t *core)
{
    sndbufbeg = 0;
    sndbufend = 0;
    core->soundEnabled = 1;
}


void initSound(core_crocods_t *gb, int r)
{
    ddlog(gb, 2, "initSound\n");

    // PSG initialize

    sndbuf = malloc(sizeof(GB_sample_t) * SNDBUFSIZE);
    sndbufbeg = 0;
    sndbufend = 0;

    gb->snd_cycle_count_init.both = (int64_t)(4000000.0 / 44100 * 4294967296.0); // number of Z80 cycles per sample

    PSG *_psg = PSG_new(CPC_CLK, r);

    memcpy(&psg, _psg, sizeof(PSG));

//    PSG_set_quality(&psg, 1);

    PSG_reset(&psg);
}

// snd_bufsize is number of sample (not bytes)

char useProcSound = 0;


void crocods_copy_sound_buffer(core_crocods_t *gb, GB_sample_t *dest, unsigned int snd_bufsize)
{
    return;

    ddlog(gb, 2, "crocods_copy_sound_buffer: want to play: %d have %d (%d %d)\n", snd_bufsize, (sndbufend - sndbufbeg + SNDBUFSIZE) % SNDBUFSIZE, sndbufbeg, sndbufend);

    if (gb->soundEnabled == 0) {
        return;
    }

    if (useProcSound == 0) {     // procsound never used
        int i;

        for (i = 0; i < snd_bufsize; i++) {
            s32 left, right;

            PSG_calc_stereo(&psg, &left, &right);

            dest[i].left = (s16)left;
            dest[i].right = (s16)right;
        }
    } else {
        int i;

        int sndbufpos = sndbufbeg;

        for (i = 0; i < snd_bufsize; i++) {
            dest[i].left = sndbuf[sndbufpos].left;
            dest[i].right =  sndbuf[sndbufpos].right;

            sndbufpos++;
            if (sndbufpos == SNDBUFSIZE) {
                sndbufpos = 0;
            }
            if (sndbufpos == sndbufend) {  // Hope that never occurs
                if (sndbufpos > 0) {
                    sndbufpos--;
                }
            }
        }

        sndbufbeg = sndbufpos;
    }
} /* crocods_copy_sound_buffer */

char waitSound(core_crocods_t *gb)
{
    u32 ws = (sndbufend + SNDBUFSIZE - sndbufbeg) % SNDBUFSIZE;

    if (ws > SNDBUFSIZE / 2) {
        ddlog(gb, 2, "waitSound: %d b:%d e:%d enabled:%d\n", ws, sndbufbeg, sndbufend, gb->soundEnabled);
        return 1;
    }

    return 0;
}

uint16_t swapbyte(uint16_t number)
{
    return (number << 8) | (number >> 8);
}

void procSound(core_crocods_t *gb)
{
    useProcSound = 1;
    sndbufend++;

    if (sndbufend == SNDBUFSIZE) {
        sndbufend = 0;
    }

    s32 left, right;

    PSG_calc_stereo(&psg, &left, &right);

    // sndbuf[sndbufend].left = (s16)left;;
    // sndbuf[sndbufend].right = (s16)right;

    switch ( gb->audio_format) {
        case CROCODS_AUDIO_S16MSB:
            sndbuf[sndbufend].left = (s16)swapbyte(left);
            sndbuf[sndbufend].right = (s16)swapbyte(right);
            break;
        case CROCODS_AUDIO_S16LSB:
            sndbuf[sndbufend].left = (s16)left;
            sndbuf[sndbufend].right = (s16)right;
            break;
        case CROCODS_AUDIO_F32LSB:
            // todo
            break;
    }

} /* procSound */

#endif /* ifdef SOUNDV2 */
