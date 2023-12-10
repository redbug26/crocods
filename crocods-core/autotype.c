#include "autotype.h"

#include "plateform.h"
#include "vga.h"
#include "ppi.h"

#include <string.h>

// char asciiFromCPC(core_crocods_t *core, int scanCode)
// {
//     char ascii = 0;

//     int i;
//     ASCII_TO_CPCKEY_MAP *pMap;
//     int nMap;

//     if (core->keyboardLayout == 0) {
//         pMap = ASCIIToCPCMapQwerty;
//     } else if (core->keyboardLayout == 1) {
//         pMap = ASCIIToCPCMapAzerty;
//     } else {
//         pMap = ASCIIToCPCMapQwerty;
//     }
//     nMap = _countof(ASCIIToCPCMapQwerty);

//     for (i = 0; i < nMap; i++) {
//         if ((pMap->cpcKey1 == scanCode) && (pMap->cpcKey2 == CPC_NIL)) {
//             return pMap->nASCII;
//         }
//         pMap++;

//     }

//     return ascii;
// } /* asciiFromCPC */

void ASCII_to_CPC(core_crocods_t *core, int nASCII, BOOL bKeyDown)
{
    int n, i;

    for (i = 0; i < 3; i++) {
        for (n = 0; n < 80; n++) {
            if (core->ROMINF[0x1eef + n + i * 80] == nASCII) {
                if (bKeyDown) {
                    if (i > 0) {
                        CPC_SetScanCode(core, (i == 1) ? CPC_SHIFT : CPC_CONTROL);
                    }
                    CPC_SetScanCode(core, n);
                } else {
                    CPC_ClearScanCode(core, n);
                    if (i > 0) {
                        CPC_ClearScanCode(core, (i == 1) ? CPC_SHIFT : CPC_CONTROL);
                    }
                }
                return;
            }
        }
    }

} /* ASCII_to_CPC */

char asciiFromCPC(core_crocods_t *core, int scanCode)
{
    int i = 0;

    return core->ROMINF[0x1eef + scanCode + i * 80];
}


/* init the auto type functions */
void AutoType_Init(core_crocods_t *core)
{
    core->AutoType.nFlags = 0;
    core->AutoType.sString = NULL;
    core->AutoType.nPos = 0;
    core->AutoType.nFrames = 0;
    core->AutoType.nCountRemaining = 0;
}

BOOL AutoType_Active(core_crocods_t *core)
{
    /* if actively typing, or waiting for first keyboard scan
     * before typing then auto-type is active */
    return ((core->AutoType.nFlags & (AUTOTYPE_ACTIVE | AUTOTYPE_WAITING)) != 0);
}

/* set the string to auto type */
void AutoType_SetString(core_crocods_t *core, const char *sString, BOOL bWaitInput)
{
    if (core->AutoType.sString != NULL) {
        free(core->AutoType.sString);
    }
    core->AutoType.sString = (char *)malloc(strlen(sString) + 1);
    strcpy(core->AutoType.sString, sString);
    core->AutoType.ch = 0;
    core->AutoType.nPos = 0;
    core->AutoType.nFrames = 0;
    core->AutoType.nCountRemaining = (int)strlen(sString);
    if (bWaitInput) {
        SoftResetCPC(core);

        /* wait for first keyboard */
        core->AutoType.nFlags |= AUTOTYPE_WAITING;
        core->AutoType.nFlags &= ~AUTOTYPE_ACTIVE;
    } else {
        core->AutoType.nFlags |= AUTOTYPE_ACTIVE;
    }
}

/* execute this every emulated frame; even if it will be skipped */
void AutoType_Update(core_crocods_t *core)
{
    if ((core->AutoType.nFlags & AUTOTYPE_ACTIVE) == 0) {
        if ((core->AutoType.nFlags & AUTOTYPE_WAITING) != 0) {
            if (Keyboard_HasBeenScanned(core)) {
                /* auto-type is now active */
                core->AutoType.nFlags |= AUTOTYPE_ACTIVE;
                /* no longer waiting */
                core->AutoType.nFlags &= ~AUTOTYPE_WAITING;
            }
        }
    } else {
        /* auto-type is active */

        /* delay frames? */
        if (core->AutoType.nFrames != 0) {
            core->AutoType.nFrames--;
            return;
        }

        /* NOTES:
         *      - if SHIFT or CONTROL is pressed, then they must be released
         *      for at least one whole frame for the CPC operating system to recognise them
         *      as released.
         *
         *      - When the same key is pressed in sequence (e.g. press, release, press, release)
         *      then there must be at least two frames for the key to be recognised as released.
         *      The CPC operating system is trying to 'debounce' the key
         */
        if (core->AutoType.nFlags & AUTOTYPE_RELEASE) {
            if (core->AutoType.nCountRemaining == 0) {
                /* auto type is no longer active */
                core->AutoType.nFlags &= ~AUTOTYPE_ACTIVE;
            }

            core->AutoType.nFlags &= ~AUTOTYPE_RELEASE;

            if (core->AutoType.ch != 1) {
                /* release the key */
                ASCII_to_CPC(core, core->AutoType.ch, FALSE);
            }

            /* number of frames for release to be acknowledged */
            core->AutoType.nFrames = 1;
        } else {
            char ch;

            /* get the current character */
            ch = core->AutoType.sString[core->AutoType.nPos];

            /* update position in string */
            core->AutoType.nPos++;

            /* update count */
            core->AutoType.nCountRemaining--;

            core->AutoType.ch = ch;

            if (ch == 1) {
                core->AutoType.nFrames = 2;
            } else {
                /* number of frames for key to be acknowledged */
                core->AutoType.nFrames = 1;

                ASCII_to_CPC(core, ch, TRUE);
            }

            core->AutoType.nFlags |= AUTOTYPE_RELEASE;
        }
    }
} /* AutoType_Update */
