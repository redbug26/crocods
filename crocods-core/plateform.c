#include "plateform.h"

#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#ifndef _WIN32
#include <sys/time.h>
#endif

#include "autotype.h"
#include "snapshot.h"
#include "sound.h"

#include "z80.h"
#include "z80_cap32.h"

#include "os.h"

#include "ppi.h"
#include "vga.h"
#include "upd.h"
#include "crtc.h"

#include "gif.h"
#include "monitor.h"

#include "cpcfont.h"

#include "gifenc.h"

#include "iniparser.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

extern const char icons_gif[];
extern const int icons_gif_length;

extern const char icons8_gif[];
extern const int icons8_gif_length;

extern const char keyboard_gif[];
extern const int keyboard_gif_length;

extern const char tape_gif[];
extern const int tape_gif_length;

extern const char select_gif[];
extern const int select_gif_length;

extern const char menu_gif[];
extern const int menu_gif_length;

extern const char shortcuts_gif[];
extern const int shortcuts_gif_length;

#define timers2ms(tlow, thigh)           (tlow | (thigh << 16)) >> 5

#define PG_LBMASK565 0xF7DE
#define PG_LBMASK555 0x7BDE

#define AlphaBlendFast(pixel, backpixel) (((((pixel)&PG_LBMASK565) >> 1) | (((backpixel)&PG_LBMASK565) >> 1)))

// static u16 AlphaBlend(u16 pixel, u16 backpixel, u16 opacity)
// {
//  // Blend image with background, based on opacity
//  // Code optimization from http://www.gamedev.net/reference/articles/article817.asp
//  // result = destPixel + ((srcPixel - destPixel) * ALPHA) / 256
//
//  u16 dwAlphaRBtemp = (backpixel & 0x7c1f);
//  u16 dwAlphaGtemp = (backpixel & 0x03e0);
//  u16 dw5bitOpacity = (opacity >> 3);
//
//  return (
//      ((dwAlphaRBtemp + ((((pixel & 0x7c1f) - dwAlphaRBtemp) * dw5bitOpacity) >> 5)) & 0x7c1f) |
//      ((dwAlphaGtemp + ((((pixel & 0x03e0) - dwAlphaGtemp) * dw5bitOpacity) >> 5)) & 0x03e0) | 0x8000
//  );
// }

#define MAX_ROM_MODS 3
#include "rom_mods.h"

// Modules



// Z80

kFctU16Void croco_cpu_doFrame;
kFctU16Void ExecInstZ80;
kFctVoidVoid ResetZ80;
kFctVoidU8 SetIRQZ80;

// CRTC

kFctU8Void ReadCRTC;
kFctVoidU8 WriteCRTC;
kFctVoidU8 RegisterSelectCRTC;
kFctVoidU32 CRTC_DoCycles;
kFctU8Void CRTC_DoLine;
kFctVoidVoid ResetCRTC;

// VGA

kFctVoidVoid GateArray_Cycle;
kFctVoidVoid ResetVGA;

void guestExit(void); // In guest.h
void guestFullscreen(core_crocods_t *core, char on); // In guest.h
char guestIsFullscreen(core_crocods_t *core); // In guest.h

void filenameWithoutExtension(char *out, char *filename);
/*
 * inline char toupper(const char toLower)
 * {
 * if ((toLower >= 'a') && (toLower <= 'z'))
 * return char(toLower - 0x20);
 * return toLower;
 * }
 */

int emulator_patch_ROM(core_crocods_t *core, u8 *pbROMlo)
{
    u8 *pbPtr;

    int CPCkeyboard = core->keyboardLayout;

    // pbPtr = pbROMlo + 0x1d69; // location of the keyboard translation table on 664 or 6128
    pbPtr = pbROMlo + 0x1eef; // location of the keyboard translation table on 6128
    memcpy(pbPtr, cpc_keytrans[CPCkeyboard], 240); // patch the CPC OS ROM with the chosen keyboard layout

    pbPtr = pbROMlo + 0x3800;
    memcpy(pbPtr, cpc_charset[CPCkeyboard], 2048); // add the corresponding character set

    return 0;
}

#define MAXFILE 1024

void myconsoleClear(core_crocods_t *core);

void SetRect(RECT *R, int left, int top, int right, int bottom);
void FillRect(RECT *R, u16 color);
void DrawRect(RECT *R, u16 color);

void calcSize(core_crocods_t *core);

void UpdateKeyMenu(void);

// 384

static int frame = 0, msgframe = 0;
static char msgbuf[33] = {0};

u16 *kbdBuffer;

#ifdef USE_DEBUG
int DebugMode = 0;
int Cont = 1;
#endif

char *keyname0[] = {
    "CURSOR_UP", // = 0
    "CURSOR_RIGHT",
    "CURSOR_DOWN",
    "F9",
    "F6",
    "F3",
    "SMALL_ENTER",
    "FDOT",
    /* line 1", bit 0..bit 7 */
    "CURSOR_LEFT",
    "COPY",
    "F7",
    "F8",
    "F5",
    "F1",
    "F2",
    "F0",
    /* line 2", bit 0..bit 7 */
    "CLR",
    "OPEN_SQUARE_BRACKET",
    "RETURN",
    "CLOSE_SQUARE_BRACKET",
    "F4",
    "SHIFT",
    "FORWARD_SLASH",
    "CONTROL",
    /* line 3", bit 0.. bit 7 */
    "HAT",
    "MINUS",
    "AT",
    "P",
    "SEMICOLON",
    "COLON",
    "BACKSLASH",
    "DOT",
    /* line 4", bit 0..bit 7 */
    "ZERO",
    "9",
    "O",
    "I",
    "L",
    "K",
    "M",
    "COMMA",
    /* line 5", bit 0..bit 7 */
    "8",
    "7",
    "U",
    "Y",
    "H",
    "J",
    "N",
    "SPACE",
    /* line 6", bit 0..bit 7 */
    "6",
    "5",
    "R",
    "T",
    "G",
    "F",
    "B",
    "V",
    /* line 7", bit 0.. bit 7 */
    "4",
    "3",
    "E",
    "W",
    "S",
    "D",
    "C",
    "X",
    /* line 8", bit 0.. bit 7 */
    "1",
    "2",
    "ESC",
    "Q",
    "TAB",
    "A",
    "CAPS_LOCK",
    "Z",
    /* line 9", bit 7..bit 0 */
    "JOY_UP",
    "JOY_DOWN",
    "JOY_LEFT",
    "JOY_RIGHT",
    "JOY_FIRE1",
    "JOY_FIRE2",
    "SPARE",
    "DEL",

    /* no key press */
    "NIL"
};

int RgbCPCdef[ 32 ] =  {
    0x7F7F7F, // Blanc            (13)
    0x7F7F7F, // Blanc            (13)
    0x00FF7F, // Vert Marin       (19)
    0xFFFF7F, // Jaune Pastel     (25)
    0x00007F, // Bleu              (1)
    0xFF007F, // Pourpre           (7)
    0x007F7F, // Turquoise        (10)
    0xFF7F7F, // Rose             (16)
    0xFF007F, // Pourpre           (7)
    0xFFFF00, // Jaune vif        (24)
    0xFFFF00, // Jaune vif        (24)
    0xFFFFFF, // Blanc Brillant   (26)
    0xFF0000, // Rouge vif         (6)
    0xFF00FF, // Magenta vif       (8)
    0xFF7F00, // Orange           (15)
    0xFF7FFF, // Magenta pastel   (17)
    0x00007F, // Bleu              (1)
    0x00FF7F, // Vert Marin       (19)
    0x00FF00, // Vert vif         (18)
    0x00FFFF, // Turquoise vif    (20)
    0x000000, // Noir              (0)
    0x0000FF, // Bleu vif          (2)
    0x007F00, // Vert              (9)
    0x007FFF, // Bleu ciel        (11)
    0x7F007F, // Magenta           (4)
    0x7FFF7F, // Vert pastel      (22)
    0x7FFF00, // Vert citron      (21)
    0x7FFFFF, // Turquoise pastel (23)
    0x7F0000, // Rouge             (3)
    0x7F00FF, // Mauve             (5)
    0x7F7F00, // Jaune            (12)
    0x7F7FFF // Bleu pastel      (14)
};

void TraceLigne8B512(core_crocods_t *core, int y, signed int AdrLo, int AdrHi);

typedef struct {
    int normal;
} CPC_MAP;

/*
 * RECT keypos[NBCPCKEY] = {
 *      {2,116,15,130},    // (0)
 *      {16,116,29,130},   //    0x80 | MOD_CPC_SHIFT,   // CPC_1
 *      {30,116,43,130},   //    0x81 | MOD_CPC_SHIFT,   // CPC_2
 *      {44,116,57,130}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
 *      {58,116,71,130}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
 *      {72,116,85,130}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
 *      {86,116,99,130}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
 *      {100,116,113,130}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
 *      {114,116,127,130}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
 *      {128,116,141,130}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
 *      {142,116,155,130}, // (10)  0x40 | MOD_CPC_SHIFT,   // CPC_0
 *      {156,116,169,130}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
 *      {170,116,183,130}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
 *      {184,116,197,130}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
 *      {198,116,211,130}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
 *      {213,116,226,130}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
 *      {227,116,240,130}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
 *      {241,116,254,130}, // 17    0x40 | MOD_CPC_SHIFT,   // CPC_0
 *
 *      {2,131,19,145},
 *      {20,131,33,145}, //    0x83,                   // CPC_a
 *      {34,131,47,145}, //    0x73,                   // CPC_z
 *      {48,131,61,145}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
 *      {62,131,75,145}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
 *      {76,131,89,145}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
 *      {90,131,103,145}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
 *      {104,131,117,145}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
 *      {118,131,131,145}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
 *      {132,131,145,145}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
 *      {146,131,159,145}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
 *      {160,131,173,145}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
 *      {174,131,187,145}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
 *      {191,131,211,160},//    0x22,                   // CPC_RETURN
 *      {213,131,226,145}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
 *      {227,131,240,145}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
 *      {241,131,254,145}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
 *
 *      {2,146,22,160},
 *      {23,146,36,160}, //    0x83,                   // CPC_a
 *      {37,146,50,160}, //    0x73,                   // CPC_z
 *      {51,146,64,160}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
 *      {65,146,78,160}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
 *      {79,146,92,160}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
 *      {93,146,106,160}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
 *      {107,146,120,160}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
 *      {121,146,134,160}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
 *      {135,146,148,160}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
 *      {149,146,162,160}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
 *      {163,146,176,160}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
 *      {177,146,190,160}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
 *      {213,146,226,160}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
 *      {227,146,240,160}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
 *      {241,146,254,160}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
 *
 *      {2,161,29,175},
 *      {30,161,43,175}, //    0x81 | MOD_CPC_SHIFT,   // CPC_2
 *      {44,161,57,175}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
 *      {58,161,71,175}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
 *      {72,161,85,175}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
 *      {86,161,99,175}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
 *      {100,161,113,175}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
 *      {114,161,127,175}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
 *      {128,161,141,175}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
 *      {142,161,155,175}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
 *      {156,161,169,175}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
 *      {170,161,183,175}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
 *      {184,161,211,175}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
 *      {213,161,227,175}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
 *      {228,161,240,175}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
 *      {241,161,254,175}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
 *
 *      {1,176,34,190}, //    0x55,                   // CPC_j
 *      {35,176,55,190}, //    0x55,                   // CPC_j
 *      {56,176,167,190}, //    0x55,                   // CPC_j
 *      {168,176,211,190}, //    0x55,                   // CPC_j
 *      {213,176,227,190}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
 *      {228,176,240,190}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
 *      {241,176,254,190}
 * };
 */

CPC_SCANCODE keyown[13];
int keymenu[13];
u8 keyIsSet[13];


void UseResources(void *core0, const void *bytes, int len)
{
    core_crocods_t *core = (core_crocods_t *)core0;

    ddlog(core, 2, "use Resources\n");

    core->resources = (char *)malloc(len); // +1); // Crash (heap buffer overflow) when omitting +1. Why ???
    memcpy(core->resources, bytes, len);
    core->resources_len = len;

    core->standalone = 1;
}

void UseDatafile(void *core0, const void *bytes, int len, const char *filename)
{
    core_crocods_t *core = (core_crocods_t *)core0;

    ddlog(core, 2, "use Datafile\n");

    core->dataFile = (char *)malloc(len); // +1); // Crash (heap buffer overflow) when omitting +1. Why ???
    memcpy(core->dataFile, bytes, len);
    core->dataFile_len = len;

    strcpy(core->openFilename, filename);

    char *basename = strrchr(filename, '/');

    if (basename == NULL) {
        basename = strrchr(filename, '\\');
    }
    if (basename != NULL) {
        strcpy(core->filename, basename + 1);
    } else {
        strcpy(core->filename, filename);
    }
} /* UseDatafile */

void SetRect(RECT *R, int left, int top, int right, int bottom)
{
    R->left = left;
    R->top = top;
    R->right = right;
    R->bottom = bottom;
}

#ifndef USE_CONSOLE
void FillRect(RECT *R, u16 color)
{
    int x, y;

    for (y = R->top; y < R->bottom; y++) {
        for (x = R->left; x < R->right; x++) {
            backBuffer[x + y * 256] = color;
        }
    }
}

void DrawRect(RECT *R, u16 color)
{
    int x, y;

    for (y = R->top; y < R->bottom; y++) {
        backBuffer[R->left + y * 256] = color;
        backBuffer[(R->right - 1) + y * 256] = color;
    }
    for (x = R->left; x < R->right; x++) {
        backBuffer[x + R->top * 256] = color;
        backBuffer[x + (R->bottom - 1) * 256] = color;
    }
}

void DrawLift(RECT *max, RECT *dest, u16 coloron, u16 coloroff)
{
    int sizelift = 8;
    RECT *r;
    RECT r0;

    /*
     * if ( (max->right-max->left) > (dest->right-dest->left) ) {
     * int x1,x2;
     *
     * int dr0,dr,dl,mr,ml;
     *
     * dr=dest->right;
     * if ( (max->bottom-max->top) > (dest->bottom-dest->top) ) {
     * dr0=dest->right-sizelift;
     * } else {
     * dr0=dr;
     * }
     * dl=dest->left;
     * mr=max->right;
     * ml=max->left;
     *
     * x1=dl+((dl-ml)*(dr0-dl))/(mr-ml);
     * x2=dr0-((mr-dr)*(dr0-dl))/(mr-ml);
     *
     * r=&r0;
     *
     * SetRect(r, x1, dest->top, x2, dest->top+sizelift);
     * FillRect(r, coloroff);
     *
     * SetRect(r, x1, dest->top, x2, dest->top+sizelift);
     * FillRect(r, coloron);
     *
     *             SetRect(r, x1, dest->top, x2, dest->top+sizelift);
     *             FillRect(r, coloroff);
     *             }
     */

    if ( (max->bottom - max->top) > (dest->bottom - dest->top) ) {
        int y1, y2;

        int dt0, dt, db, mt, mb;

        db = dest->bottom;
        dt = dest->top;

        if ( (max->right - max->left) > (dest->right - dest->left) ) {
            dt0 = dest->top + sizelift;
        } else {
            dt0 = dt;
        }
        mb = max->bottom;
        mt = max->top;

        y1 = dt0 + ((dt - mt) * (db - dt0)) / (mb - mt);
        y2 = db - ((mb - db) * (db - dt)) / (mb - mt);

        //  y2=y1+((db-dt)*(db-dt))/(mb-mt);

        if (y2 > mb) {
            y2 = mb; // pas normal... mais ca arrive :(
        }

        r = &r0;

        if (dest->top < y1) {
            SetRect(r, dest->right - sizelift, dest->top, dest->right, y1);
            FillRect(r, coloroff);
        }

        SetRect(r, dest->right - sizelift, y1, dest->right, y2);
        FillRect(r, coloron);

        if (dest->bottom > y2) {
            SetRect(r, dest->right - sizelift, y2, dest->right, dest->bottom);
            FillRect(r, coloroff);
        }
    }

    return;
} /* DrawLift */

#endif /* ifndef USE_CONSOLE */

// -1: dernier couleur
// 0: vert
// 1: couleur
// 3: inactif

#define RGB565(R, G, B) ((((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))

void SetPalette(core_crocods_t *core, int color)
{
    ddlog(core, 2, "SetPalette %d\n", color);

    int i;

    //    if ( (color==0) || (color==1) ) {
    //        core->lastcolour=color;
    //        return;
    //    }

    if (color == -1) {
        color = core->lastcolour;
    }

    if (color == 0) {           // Green monitor
        for (i = 0; i < 32; i++) {
            core->BG_PALETTE[i] = RGB565(0, i * 8, 0);
        }
        core->lastcolour = color;
    }


    if (color == 1) { // Color monitor
        for (i = 0; i < 32; i++) {
            int r = (RgbCPCdef[ i ] >> 16) & 0xFF;
            int g = (RgbCPCdef[ i ] >> 8) & 0xFF;
            int b = (RgbCPCdef[ i ] >> 0) & 0xFF;

            core->BG_PALETTE[i] = RGB565(r, g, b);
        }
        core->lastcolour = color;
    }


    if (color == 2) { // Fast
        for (i = 0; i < 32; i++) {
            core->BG_PALETTE[i] = RGB565(255 - i * 8, 255 - i * 8, 0);
        }
    }
    if (color == 3) {   // Pause
        for (i = 0; i < 32; i++) {
            core->BG_PALETTE[i] = RGB565(i * 8, i * 8, i * 8);
        }
    }

    core->UpdateInk = 1;
} /* SetPalette */

void RedefineKey(core_crocods_t *core, int key)
{
    core->redefineKey = 1;
    core->redefineKeyKey = key;

#ifndef USE_CONSOLE
    int x, y, n;
    dmaCopy(kbdBuffer, backBuffer, SCREEN_WIDTH * SCREEN_HEIGHT * 2);

    keyEmul = 3;

    while (((~IPC->buttons) & (1 << 6)) == 0);

    x = IPC->touchXpx;
    y = IPC->touchYpx;

    for (n = 0; n < NBCPCKEY; n++) {
        if ( (x >= keypos[n].left) && (x <= keypos[n].right) && (y >= keypos[n].top) && (y <= keypos[n].bottom) ) {
            keyown[key] = keymap[n];
            break;
        }
    }
    UpdateKeyMenu();
#endif
} /* RedefineKey */

void UpdateTitlePalette(struct kmenu *current)
{
    /*
     * if (lastcolour==1) {
     * sprintf(current->title,"Monitor: [COLOR] - Green");
     * } else {
     * sprintf(current->title,"Monitor: Color - [GREEN]");
     * }
     */
}

// Retour: 1 -> return emulator  (Default)
//         0 -> return to parent
//         2 -> return to item (switch)

int ExecuteMenu(core_crocods_t *core, int n, struct kmenu *current)
{
    if (n == ID_MENU_ENTER) {
        ddlog(core, 2, "ExecuteMenu: ID_MENU_ENTER\n");
    } else if (n == ID_ENABLE_TURBO) {
        ddlog(core, 2, "ExecuteMenu: ID_ENABLE_TURBO\n");
    } else if (n == ID_DISABLE_TURBO) {
        ddlog(core, 2, "ExecuteMenu: ID_DISABLE_TURBO\n");
    } else {
        ddlog(core, 2, "ExecuteMenu: %d\n", n);
    }
    switch (n) {
        case ID_SWITCH_MONITOR:
            return 0;
            break;
        case ID_COLOR_MONITOR:
            SetPalette(core, 1);
            return 0;
            break;
        case ID_GREEN_MONITOR:
            SetPalette(core, 0);
            return 0;
            break;
        case ID_FULLSCREEN_SWITCH:
            guestFullscreen(core, !(guestIsFullscreen(core)));
            break;
        case ID_FULLSCREEN_ON:
            guestFullscreen(core, 1);
            break;
        case ID_FULLSCREEN_OFF:
            guestFullscreen(core, 0);
            break;
        case ID_SCREEN_AUTO:
            ddlog(core, 2, "ID_SCREEN_AUTO\n");

            calcSize(core);

            core->MemBitmap_width = 320;

            core->resize = 1;
            core->DrawFct = TraceLigne8B512;
            core->Regs1 = 0;
            core->Regs2 = 0;
            core->Regs6 = 0;
            core->Regs7 = 0;
            core->changeFilter = 1; // Flag to ask to the display to change the resolution

            return 1;
            break;
        case ID_SCREEN_320:
            ddlog(core, 2, "ID_SCREEN_320\n");

            core->screenBufferWidth = 320;
            core->screenBufferHeight = 200;

            core->MemBitmap_width = 320;

            core->resize = 2;
            core->DrawFct = TraceLigne8B512;
            //     BG3_XDX = 320; // 256; // 360 - 54;  // 360 = DISPLAY_X // Taille d'affichage
            //     BG3_CX = (XStart*4) << 8;
            core->x0 = (core->XStart * 4);
            core->y0 = 40;
            core->maxy = 64;
            core->changeFilter = 1; // Flag to ask to the display to change the resolution

            return 0;
            break;
        case ID_SCREEN_NORESIZE:
            ddlog(core, 2, "ID_SCREEN_NORESIZE\n");

            core->resize = 3;
            core->DrawFct = TraceLigne8B512;
            //     BG3_XDX = 256; // 256; // 360 - 54;  // 360 = DISPLAY_X // Taille d'affichage
            //     BG3_CX = (XStart*4) << 8;
            core->x0 = (core->XStart * 4);
            core->y0 = 40;
            core->maxy = 80;
            core->changeFilter = 1; // Flag to ask to the display to change the resolution

            return 0;
            break;
        case ID_SCREEN_OVERSCAN:
            ddlog(core, 2, "ID_SCREEN_OVERSCAN\n");

            core->screenBufferWidth = 384;
            core->screenBufferHeight = 272;

            core->resize = 4;
            core->DrawFct = TraceLigne8B512;
            //   BG3_XDX = 384; // 256; // 360 - 54;  // 360 = DISPLAY_X // Taille d'affichage
            //   BG3_CX = 0;
            core->x0 = (core->XStart * 4);
            core->y0 = 0;
            core->changeFilter = 1; // Flag to ask to the display to change the resolution

            return 0;
            break;

        case ID_SCREEN_NEXT:
            if (core->resize == 1) {
                appendIcon(core, 2, 3, 60);

                ExecuteMenu(core, ID_SCREEN_320, NULL);
            } else if (core->resize == 2) {
                appendIcon(core, 2, 0, 60);

                ExecuteMenu(core, ID_SCREEN_OVERSCAN, NULL);
            } else if (core->resize == 4) {
                appendIcon(core, 2, 2, 60);

                ExecuteMenu(core, ID_SCREEN_AUTO, NULL);
            }
            break;

        case ID_SHOW_VIRTUALKEYBOARD:
            core->last_keys_pressed = core->ipc.keys_pressed;

            ExecuteMenu(core, ID_MENU_EXIT, NULL);

            appendIcon(core, 3, 0, 60);

            core->inKeyboard = 1;
            core->runApplication = DispKeyboard;
            return 0;
            break;

        case ID_KEY_KEYBOARD:

#ifdef EMSCRIPTEN
            EM_ASM({
            $ ('#crocods-bar-switchkey').css ("background-position", "-237px -76px");
            $ ('#crocods-bar-switchkey').data ("menuid", 12);
        });
#endif /* ifdef EMSCRIPTEN */

//            core->last_keys_pressed = core->ipc.keys_pressed;

            core->keyEmul = 2; //  Emul du clavier
            return 0;
            break;

        case ID_PLAY_TAPE:
            core->last_keys_pressed = core->ipc.keys_pressed;

            ExecuteMenu(core, ID_MENU_EXIT, NULL);

            core->inKeyboard = 1;
            core->runApplication = DispTapePlayer;
            return 0;
            break;

        case ID_KEY_KEYPAD:
            keyown[0] = CPC_CURSOR_UP;
            keyown[1] = CPC_CURSOR_DOWN;
            keyown[2] = CPC_CURSOR_LEFT;
            keyown[3] = CPC_CURSOR_RIGHT;
            keyown[4] = CPC_RETURN;
            keyown[5] = CPC_SPACE;
            keyown[6] = CPC_1;
            keyown[7] = CPC_2;
            keyown[8] = CPC_3;
            keyown[9] = CPC_NIL;  // KEY_L
            keyown[10] = CPC_NIL; // KEY_R
            keyown[11] = CPC_6;
            keyown[12] = CPC_7;

            core->keyEmul = 3; //  Emul du clavier fleche
            return 0;
            break;
        case ID_KEY_JOYSTICK:

#ifdef EMSCRIPTEN
            EM_ASM({
            $ ('#crocods-bar-switchkey').css ("background-position", "-213px -76px");
            $ ('#crocods-bar-switchkey').data ("menuid", 10);
        });
#endif /* ifdef EMSCRIPTEN */

            keyown[0] = CPC_JOY_UP;
            keyown[1] = CPC_JOY_DOWN;
            keyown[2] = CPC_JOY_LEFT;
            keyown[3] = CPC_JOY_RIGHT;
            keyown[4] = CPC_RETURN;
            keyown[5] = CPC_JOY_FIRE1;
            keyown[6] = CPC_JOY_FIRE2;
            keyown[7] = CPC_2;
            keyown[8] = CPC_3;
            keyown[9] = CPC_NIL;  // KEY_L
            keyown[10] = CPC_NIL; // KEY_R
            keyown[11] = CPC_6;
            keyown[12] = CPC_7;

            core->keyEmul = 3; //  Emul du joystick
            return 0;
            break;
        case ID_DISPFRAMERATE:
            core->dispframerate = 1;
            return 0;
            break;
        case ID_NODISPFRAMERATE:
            core->dispframerate = 0;
            return 0;
            break;
        case ID_RESET:
            // myddlog(core, 2, "Reset CPC");
            ExecuteMenu(core, ID_MENU_EXIT, NULL);
            HardResetCPC(core);
            ExecuteMenu(core, ID_AUTORUN, NULL);

            return 0;
            break;
        case ID_LOADSNAP:
            LoadSlotSnap(core, 0);
            break;
        case ID_SAVESNAP: {
            char filename[MAX_PATH + 1];
            char snap[MAX_PATH + 1];
            char snapFile[MAX_PATH + 1];

            int i = 0;

            do {
                strcpy(snap, core->snap_dir);

                filenameWithoutExtension(filename, core->filename);

                if (i == 0) {
                    sprintf(snapFile, "%s.sna", filename);
                } else {
                    sprintf(snapFile, "%s_%03d.sna", filename, i);
                }
                apps_disk_path2Abs(snap, snapFile);

                FILE *fic = os_fopen(snap, "rb");
                if (fic == NULL) {
                    break;
                }
                fclose(fic);

                i++;

            } while (1);

            ddlog(core, 0, "Save snapfile to %s\n", snap);
            SauveSnap(core, snap);

            return 1;
            break;
        }

        case ID_EXIT:
            ExecuteMenu(core, ID_SAVE_SETTINGS, NULL);
            guestExit();
            exit(EXIT_SUCCESS);
            break;

        case ID_REDEFINE_UP:
            RedefineKey(core, 0);
            return 2;
            break;
        case ID_REDEFINE_DOWN:
            RedefineKey(core, 1);
            return 2;
            break;
        case ID_REDEFINE_LEFT:
            RedefineKey(core, 2);
            return 2;
            break;
        case ID_REDEFINE_RIGHT:
            RedefineKey(core, 3);
            return 2;
            break;
        case ID_REDEFINE_START:
            RedefineKey(core, 4);
            return 2;
            break;
        case ID_REDEFINE_A:
            RedefineKey(core, 5);
            return 2;
            break;
        case ID_REDEFINE_B:
            RedefineKey(core, 6);
            return 2;
            break;
        case ID_REDEFINE_X:
            RedefineKey(core, 7);
            return 2;
            break;
        case ID_REDEFINE_Y:
            RedefineKey(core, 8);
            return 2;
            break;
        case ID_REDEFINE_L:
            RedefineKey(core, 9);
            return 2;
            break;
        case ID_REDEFINE_R:
            RedefineKey(core, 10);
            return 2;
            break;
        case ID_REDEFINE_L2:
            RedefineKey(core, 11);
            return 2;
            break;
        case ID_REDEFINE_R2:
            RedefineKey(core, 12);
            return 2;
            break;
        case ID_HACK_TABCOUL:
            core->hack_tabcoul = 1;
            return 2;
            break;
        case ID_NOHACK_TABCOUL:
            core->hack_tabcoul =  0;
            return 2;
            break;
        case ID_ACTIVE_MAGNUM:
            break;
        case ID_MENU_ENTER:
            printf("ID_MENU_ENTER\n");
            ExecuteMenu(core, ID_PAUSE_ENTER, NULL);
            core->inMenu = 1;
            apps_menu_init(core);
            break;
        case ID_MENU_EXIT:
            ExecuteMenu(core, ID_SAVE_SETTINGS, NULL);
            ExecuteMenu(core, ID_PAUSE_EXIT, NULL);
            core->inMenu = 0;
            core->runApplication = NULL;
            core->wait_key_released = 1;
            break;
        case ID_PAUSE_ENTER:
            core->isPaused = 1;
            SetPalette(core, 3);
            break;
        case ID_PAUSE_EXIT:
//            if (core->runApplication==DispAutorun) {
//                break;
//            }
            core->isPaused = 0;
            core->inMenu = 0;
//            core->runApplication = NULL;
            SetPalette(core, -1);
            break;
        case ID_DEBUG_ENTER:
            ExecInstZ80 = ExecInstZ80_debug;
            core->debug = 1;
            break;
        case ID_DEBUG_EXIT:
            ExecInstZ80 = ExecInstZ80_orig;
            core->debug = 0;
            break;
        case ID_SAVE_SETTINGS:
            saveIni(core, 0);
            break;
        case ID_SAVE_LOCALSETTINGS:
            saveIni(core, 1);
            break;
        case ID_USE_CRTC_WINCPC:
            ReadCRTC = wincpc_ReadCRTC;
            WriteCRTC = wincpc_WriteCRTC;
            RegisterSelectCRTC = wincpc_RegisterSelectCRTC;
            CRTC_DoCycles = NULL;
            CRTC_DoLine = wincpc_CRTC_DoLine;
            ResetCRTC = wincpc_ResetCRTC;
            croco_cpu_doFrame = wincpc_cpu_doFrame;

            GateArray_Cycle = NULL;
            ResetVGA = wincpc_ResetVGA;

            ExecInstZ80 = ExecInstZ80_orig;
            ResetZ80 = ResetZ80_orig;
            SetIRQZ80 = SetIRQZ80_orig;

            break;
        case ID_USE_CRTC_ARNOLD:
            ReadCRTC = arn_ReadCRTC;
            WriteCRTC = arn_WriteCRTC;
            RegisterSelectCRTC = arn_RegisterSelectCRTC;
            CRTC_DoCycles = arn_CRTC_DoCycles;
            CRTC_DoLine = NULL;
            ResetCRTC = arn_ResetCRTC;
            croco_cpu_doFrame = arn_cpu_doFrame;

            GateArray_Cycle = arn_GateArray_Cycle;
            ResetVGA = arn_ResetVGA;

            ExecInstZ80 = ExecInstZ80_orig;
            ResetZ80 = ResetZ80_orig;
            SetIRQZ80 = SetIRQZ80_orig;

            break;

        case ID_USE_CRTC_CAP32:
            ddlog(core, 2, "Use cap32");
            ReadCRTC = cap32_ReadCRTC;
            WriteCRTC = cap32_WriteCRTC;
            RegisterSelectCRTC = cap32_RegisterSelectCRTC;
            CRTC_DoCycles = NULL; // cap32_crtc_cycle is colled by the z80 loop
            CRTC_DoLine = NULL;
            ResetCRTC = cap32_crtc_reset;
            croco_cpu_doFrame = cap32_cpu_doFrame;

            GateArray_Cycle = NULL;
            ResetVGA = cap32_ResetVGA;

            ExecInstZ80 = ExecInstZ80_orig;
            ResetZ80 = ResetZ80_cap32;
            SetIRQZ80 = SetIRQZ80_cap32;

            break;

        case ID_SOUND_ENABLE:
            core->soundEnabled = 1;
            break;

        case ID_SOUND_DISABLE:
            core->soundEnabled = 0;
            break;

        case ID_AUTORUN:
            apps_autorun_init(core, 1);
            return 2;
            break;

        case ID_INSERTDISK:
            apps_autorun_init(core, 0);
            break;

        case ID_AUTODISK:
            apps_disk_init(core, 1);
            return 2;
            break;
        case ID_DISK:
            apps_disk_init(core, 0);
            return 2;
            break;

        case ID_SHOW_DEBUGGER:
            ExecuteMenu(core, ID_MENU_EXIT, NULL);  // Un-pause
            apps_debugger_init(core, 0);
            return 2;
            break;

        case ID_SHOW_INFOPANEL:
            apps_infopanel_init(core, 0);
            return 2;
            break;

        case ID_SHOW_INFOPANEL_SHORTCUTS:
            apps_infopanel_init(core, 1);
            return 2;
            break;

        case ID_SHOW_GUESTINFO:
            apps_guestinfo_init(core, 0);
            return 2;
            break;

        case ID_SHOW_BROWSER:
            apps_browser_init(core, 0);
            return 2;
            break;

        case ID_NO_SCANLINE:
            core->scanline = 0;
            return 2;
            break;

        case ID_SCANLINE_5:
            core->scanline = 1;
            return 2;
            break;

        case ID_SCANLINE_10:
            core->scanline = 2;
            return 2;
            break;
        case ID_SCANLINE_15:
            core->scanline = 3;
            return 2;
            break;
        case ID_SCANLINE_20:
            core->scanline = 4;
            return 2;
            break;

        case ID_ENABLE_TURBO:
            disableSound(core);
            SetPalette(core, 2);
            core->turbo = 1;
            return 1;

        case ID_DISABLE_TURBO:
            enableSound(core);
            SetPalette(core, -1);
            core->turbo = 0;
            return 1;

        case ID_SWITCH_TURBO:
            if (core->turbo) {
                ExecuteMenu(core, ID_DISABLE_TURBO, NULL);
            } else {
                ExecuteMenu(core, ID_ENABLE_TURBO, NULL);
            }
            break;

        case ID_SAVESCREEN:
            saveScreen(core);
            break;

        case ID_EXPORTDISK:
            exportDisk(core);
            break;

#ifndef CLI
#ifndef OLDFARK
        case ID_CONSOLE:
            if (core->inConsole) {
                core->inConsole = 0;
            } else {
                apps_console_init(core, 1);
            }
            return 2;
            break;
#endif
#endif

        default:
            break;
    } /* switch */
    return 1;
} /* ExecuteMenu */

u16 MyReadKey(void)
{
    /*
     * u16 keys_pressed, my_keys_pressed;
     *
     * do {
     * keys_pressed = ~(REG_KEYINPUT);
     * } while ((keys_pressed & (KEY_LEFT | KEY_RIGHT | KEY_DOWN | KEY_UP | KEY_A | KEY_B | KEY_L | KEY_R))==0);
     *
     * my_keys_pressed = keys_pressed;
     *
     * do {
     * keys_pressed = ~(REG_KEYINPUT);
     * } while ((keys_pressed & (KEY_LEFT | KEY_RIGHT | KEY_DOWN | KEY_UP | KEY_A | KEY_B | KEY_L | KEY_R))!=0);
     */
    return 0; // my_keys_pressed;
}

void InitCalcPoints(core_crocods_t *core)
{
    int a, b, c, d, i;

    // Pour le mode 0
    for (i = 0; i < 256; i++) {
        a = (i >> 7)
            + ( (i & 0x20) >> 3)
            + ( (i & 0x08) >> 2)
            + ( (i & 0x02) << 2);
        b = ( (i & 0x40) >> 6)
            + ( (i & 0x10) >> 2)
            + ( (i & 0x04) >> 1)
            + ( (i & 0x01) << 3);
        core->TabPointsDef[ 0 ][ i ][ 0 ] = (u8)a;
        core->TabPointsDef[ 0 ][ i ][ 1 ] = (u8)a;
        core->TabPointsDef[ 0 ][ i ][ 2 ] = (u8)b;
        core->TabPointsDef[ 0 ][ i ][ 3 ] = (u8)b;
    }

    // Pour le mode 1
    for (i = 0; i < 256; i++) {
        a = (i >> 7) + ( (i & 0x08) >> 2);
        b = ( (i & 0x40) >> 6) + ( (i & 0x04) >> 1);
        c = ( (i & 0x20) >> 5) + (i & 0x02);
        d = ( (i & 0x10) >> 4) + ( (i & 0x01) << 1);
        core->TabPointsDef[ 1 ][ i ][ 0 ] = (u8)a;
        core->TabPointsDef[ 1 ][ i ][ 1 ] = (u8)b;
        core->TabPointsDef[ 1 ][ i ][ 2 ] = (u8)c;
        core->TabPointsDef[ 1 ][ i ][ 3 ] = (u8)d;
    }

    // Pour le mode 2
    for (i = 0; i < 256; i++) {
        core->TabPointsDef[ 2 ][ i ][ 0 ] = i >> 7;
        core->TabPointsDef[ 2 ][ i ][ 1 ] = (i & 0x20) >> 5;
        core->TabPointsDef[ 2 ][ i ][ 2 ] = (i & 0x08) >> 3;
        core->TabPointsDef[ 2 ][ i ][ 3 ] = (i & 0x02) >> 1;
    }

    // Mode 3 = Mode 0 ???
    for (i = 0; i < 256; i++) {
        for (a = 0; a < 4; a++) {
            core->TabPointsDef[ 3 ][ i ][ a ] = core->TabPointsDef[ 0 ][ i ][ a ];
        }
    }
} /* InitCalcPoints */

void CalcPoints(core_crocods_t *core)
{
    int i, j;

    if ((core->lastMode >= 0) && (core->lastMode <= 3)) {
        for (i = 0; i < 256; i++) {
            for (j = 0; j < 4; j++) {
                core->TabPoints[core->lastMode][i][j] = core->BG_PALETTE[core->TabCoul[ core->TabPointsDef[core->lastMode][i][j]]];
            }
            /* *(u32*)(&core->TabPoints[lastMode][i][0]) = (TabCoul[ core->TabPointsDef[lastMode][i][0] ] << 0) + (TabCoul[ core->TabPointsDef[lastMode][i][1] ] << 8) + (TabCoul[ core->TabPointsDef[lastMode][i][2] ] << 16) + (TabCoul[ core->TabPointsDef[lastMode][i][3] ] << 24);
             */
        }
    }
    core->UpdateInk = 0;
}

void checkVersioAbout(core_crocods_t *core)
{
    char dispAbout = 0;
    char tmp_directory[256];

    FILE *verFile;

    strcpy(tmp_directory, core->home_dir);
    apps_disk_path2Abs(tmp_directory, "version");
    verFile = os_fopen(tmp_directory, "rb");
    if (verFile == NULL) {
        dispAbout = 1;
    } else {
        char ver[256];
        fread(ver, strlen(CROCOVERSION), 1, verFile);
        if (strcmp(ver, CROCOVERSION) != 0) {
            fclose(verFile);
            dispAbout = 1;
        }
    }

    if (dispAbout == 1) {
        verFile = os_fopen(tmp_directory, "wb");
        fwrite(CROCOVERSION, strlen(CROCOVERSION), 1, verFile);
        fclose(verFile);
        ExecuteMenu(core, ID_SHOW_INFOPANEL, NULL);
    }
} /* checkVersioAbout */

/********************************************************* !NAME! **************
* Nom : InitPlateforme
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Initialisation diverses
*
* R�sultat    : /
*
* Variables globales modifi�es : JoyKey, clav
*
********************************************************** !0! ****************/
void InitPlateforme(core_crocods_t *core,  u16 screen_width)
{
    core->MemBitmap_width = screen_width;

    InitCalcPoints(core);
    CalcPoints(core);
    memset(core->clav, 0xFF, sizeof(core->clav));

    core->overlayBitmap = (u16 *)malloc(320 * 240 * 2);
    core->overlayBitmap_width = 0;

    core->ipc.keys_pressed = 0;

    core->inMenu = 0;
    core->isPaused = 0;
    core->inKeyboard = 0;

    core->runApplication = NULL;

    SoftResetCPC(core);

//    checkVersionAbout(core);      // Display about ?
} /* InitPlateforme */

void SoftResetCPC(core_crocods_t *core)
{
    ResetZ80(core);

    Keyboard_Reset(core);

    ResetVGA(core);
    WriteVGA(core, 0, 0x89);

    ResetCRTC(core);
    Monitor_Reset(core);        // Only in Arnold

    ResetPPI(core);
    ResetUPD(core);
    Reset8912(core);
}

extern const unsigned char cpc6128_bin[];
extern const unsigned char romdisc_bin[];

void HardResetCPC(core_crocods_t *core)
{
    if (InitMemCPC(core, (char *)&(cpc6128_bin[0]), (char *)&(romdisc_bin[0]))) {
        ResetZ80(core);
        ResetUPD(core);
        ResetCRTC(core);

        InitPlateforme(core, 320);

        SetPalette(core, -1);
    }
}

void updateScreenBuffer(core_crocods_t *core, unsigned short *screen, u16 screen_width)
{
    core->MemBitmap_width = screen_width;
}

/********************************************************* !NAME! **************
* Nom : Erreur
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Affichage d'un message d'erreur
*
* R�sultat    : /
*
* Variables globales modifi�es : /
*
********************************************************** !0! ****************/
// void Erreur(char *Msg)
// {
//     myddlog(core, 2, "Error: %s", Msg);
// }

/********************************************************* !NAME! **************
* Nom : Info
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Affichage d'un message d'information
*
* R�sultat    : /
*
* Variables globales modifi�es : /
*
********************************************************** !0! ****************/
// void Info(char *Msg)
// {
//     myddlog(core, 2, "Info: %s", Msg);
// }

// Draw line (used by the old WINCPC CRTC core)

void TraceLigne8B512(core_crocods_t *core, int y, signed int AdrLo, int AdrHi)
{
    // if (y>yMax) yMax=y;
    // if (y<yMin) yMin=y;

    y -= core->y0;

    if ((y < 0) || (y >= TAILLE_Y_LOW)) {
        return;
    }

    if ((!core->hack_tabcoul) && (core->UpdateInk == 1)) { // It's would be beter to put before each lines
        CalcPoints(core);
    }

    core->crtc_updated = 1;

    u16 *p;

    p = (u16 *)core->MemBitmap;
    p += (y * core->MemBitmap_width);

    //    for (int i=0;i<384;i++) {
    //        p[i]=rand()&0xFFFF;
    //    }
    //    return;

    if (core->lastMode != 2) {
        if (AdrLo < 0) {
            if ((core->resize != 1) && (core->resize != 2)) {
                int x;
                for (x = 0; x < 384; x++) {
                    p[x] = core->BG_PALETTE[core->TabCoul[ 16 ]];
                }
            }
        } else {
            int x;

            if (core->resize == 4) {
                for (x = 0; x < core->XStart * 4; x++) {
                    *p = core->BG_PALETTE[core->TabCoul[ 16 ]];
                    p++;
                }
            }
            if (core->resize == 2) {
                for (x = 0; x < (core->XStart - 8) * 4; x++) {
                    *p = core->BG_PALETTE[core->TabCoul[ 16 ]];
                    p++;
                }
            } else {
                // p+=XStart*4;
            }

            for (x = core->XStart; x < core->XEnd; x++) {
                u16 *tab = &(core->TabPoints[ core->lastMode ][ core->MemCPC[ (AdrLo & 0x7FF) | AdrHi ] ][0]);
                memcpy(p, tab, 4 * sizeof(u16));
                p += 4;
                AdrLo++;
            }

            if (core->resize == 4) {
                for (x = 0; x < (96 - core->XEnd) * 4; x++) {
                    *p = core->BG_PALETTE[core->TabCoul[ 16 ]];
                    p++;
                }
            } else if (core->resize == 2) {
                for (x = 0; x < (88 - core->XEnd) * 4; x++) {
                    *p = core->BG_PALETTE[core->TabCoul[ 16 ]];
                    p++;
                }
            }
        }
    } else { // If mode 2
        //        core->BG_PALETTE[0]=0;

        p += (y * core->MemBitmap_width);

        if (AdrLo < 0) {
            if ((core->resize != 1) && (core->resize != 2)) {
                int x;
                for (x = 0; x < 384 * 2; x++) {
                    p[x] = core->BG_PALETTE[core->TabCoul[ 16 ]];
                }
            }
        } else {
            int x;

            if (core->resize == 4) {
                for (x = 0; x < core->XStart * 8; x++) {
                    *p = core->BG_PALETTE[core->TabCoul[ 16 ]];
                    p++;
                }
            }
            if (core->resize == 2) {
                for (x = 0; x < (core->XStart - 8) * 8; x++) {
                    *p = core->BG_PALETTE[core->TabCoul[ 16 ]];
                    p++;
                }
            } else {
                // p+=XStart*4;
            }

            for (x = core->XStart; x < core->XEnd; x++) {
                u8 car = core->MemCPC[ (AdrLo & 0x7FF) | AdrHi ];
                int i;

                for (i = 0; i < 8; i++) {
                    *(p + 7 - i) = core->BG_PALETTE[core->TabCoul[car & 1]];
                    car = (car >> 1);
                }
                p += 8;
                AdrLo++;
            }

            if (core->resize == 4) {
                for (x = 0; x < (96 - core->XEnd) * 8; x++) {
                    *p = core->BG_PALETTE[core->TabCoul[ 16 ]];
                    p++;
                }
            } else if (core->resize == 2) {
                for (x = 0; x < (88 - core->XEnd) * 8; x++) {
                    *p = core->BG_PALETTE[core->TabCoul[ 16 ]];
                    p++;
                }
            }
        }
    }
} /* TraceLigne8B512 */

void calcSize(core_crocods_t *core)
{
    int x1, x2, y1, y2;

    //            int height;

    x1 = max( (50 - core->RegsCRTC[2]) << 3, 0);
    x2 = min(x1 + (core->RegsCRTC[1] << 3), 384);

    y1 = max( (35 - core->RegsCRTC[7]) << 3, 0);
    y2 = min(y1 + (core->RegsCRTC[6] << 3), 272);

    core->DrawFct = TraceLigne8B512;

    //            height = (y2 - y1);
    //            if (height < 192) height = 192;

    ddlog(core, 2, "Change top to %d\n", y1);

    core->x0 = x1;
    core->y0 = y1;             // Redbug
    core->maxy = 0;

    (*core->borderX) = (TAILLE_X_LOW - (x2 - x1)) / 2;
    (*core->borderY) = (TAILLE_Y_LOW - (y2 - y1)) / 2;

    core->Regs1 = core->RegsCRTC[1];
    core->Regs2 = core->RegsCRTC[2];
    core->Regs6 = core->RegsCRTC[6];
    core->Regs7 = core->RegsCRTC[7];

    core->screenBufferWidth = x2 - x1;
    core->screenBufferHeight = y2 - y1;

    core->MemBitmap_width = x2 - x1;

    core->changeFilter = 1;
} /* calcSize */

void UpdateScreen(core_crocods_t *core)
{
    frame++;

    if (core->resize == 1) { // Auto resize ?
        if ((core->RegsCRTC[2] != core->Regs2) || (core->RegsCRTC[6] != core->Regs6) || (core->RegsCRTC[1] != core->Regs1) || (core->RegsCRTC[7] != core->Regs7)) {
            calcSize(core);
        }
    }

    if (msgframe > frame - 50 * 3) {
        cpcprint(core, 0, 40, msgbuf, 1);
    }

    if (core->crtc_updated) {
        core->crtc_updated = 0;

        if (core->UpdateInk == 1) { // It's would be beter to put before each lines
            CalcPoints(core);
        }
    }
} /* UpdateScreen */

void DispDisk(core_crocods_t *core, int reading)
{
    dispIcon(core, 0, 0, 0, 0, 0);

#ifndef USE_CONSOLE
    int x, y;
    RECT r;
    u16 color;

    // SetRect(&r, 222,88,254,113);
    SetRect(&r, 230, 1, 254, 33);

    switch (reading) {
        case 0:
            for (y = r.top; y < r.bottom; y++) {
                for (x = r.left; x < r.right; x++) {
                    backBuffer[x + y * 256] = kbdBuffer[x + y * 256];
                }
            }
            break;
        case 1:
            color = RGB565(15, 0, 0);
            for (y = r.top; y < r.bottom; y++) {
                for (x = r.left; x < r.right; x++) {
                    // backBuffer[x+y*256]=AlphaBlendFast(kbdBuffer[x+y*256], color);
                    backBuffer[x + y * 256] = ~kbdBuffer[x + y * 256] | 0x8000;
                }
            }
            break;
    }
#endif /* ifndef USE_CONSOLE */
} /* DispDisk */

void appendIcon(core_crocods_t *core, int x, int y, int timer)
{
    core->iconTimer = timer;
    core->iconToDislay = x * 16 + y;
}

void RunMenu(core_crocods_t *core, int menu)
{
    if (menu != ID_NULL) {
        ExecuteMenu(core, menu, NULL);

        core->wait_key_released = 1;
        core->ipc.keys_pressed = 0;
    }
}

int nds_ReadKey(core_crocods_t *core)
{
    if (AutoType_Active(core)) {
        AutoType_Update(core);
    } else {
        u16 keys_pressed;
        // static u16 oldkey;
//        int n;

        //       scanKeys();
        //     keys_pressed = keysHeld();

        keys_pressed = core->ipc.keys_pressed;

        if (core->ipc.touchDown == 1) {
//            int x, y; // , n;

            ddlog(core, 2, "nds_ReadKey press\n");

//            x = core->ipc.touchXpx;
//            y = core->ipc.touchYpx;

            ExecuteMenu(core, ID_MENU_ENTER, NULL);
            core->ipc.touchDown = 0;

            /* if ((x>0) & (x<32) & (y>=25) & (y<=36)) {
             * ExecuteMenu(core, ID_RESET, NULL);
             * ipc.touchDown=0;
             * }
             */

//            if ((x >= 230) && (x <= 254) && (y >= 1) && (y <= 33)) { // 52
//                core->inMenu = 1;
//                apps_menu_init(core);
//            }

            /*
             * for (n = 0; n < NBCPCKEY; n++) {
             *  if ((x >= keypos[n].left) && (x <= keypos[n].right) && (y >= keypos[n].top) && (y <= keypos[n].bottom)) {
             *      PressKey(core, n);
             *      break;
             *  }
             * } */
        }

        /*
         * keyown[0]=CPC_JOY_UP;
         * keyown[1]=CPC_JOY_DOWN;
         * keyown[2]=CPC_JOY_LEFT;
         * keyown[3]=CPC_JOY_RIGHT;
         * keyown[4]=CPC_RETURN;
         * keyown[5]=CPC_JOY_FIRE1;
         * keyown[6]=CPC_JOY_FIRE2;*/

        if ((keys_pressed & KEY_SELECT) == KEY_SELECT) {
            core->last_keys_pressed = keys_pressed;
            ExecuteMenu(core, ID_MENU_ENTER, NULL);
        }

        if (core->keyEmul == 3) {
            if ((keys_pressed & KEY_UP) == KEY_UP) {
                RunMenu(core, keymenu[0]);
            }

            if ((keys_pressed & KEY_DOWN) == KEY_DOWN) {
                RunMenu(core, keymenu[1]);
            }

            if ((keys_pressed & KEY_LEFT) == KEY_LEFT) {
                RunMenu(core, keymenu[2]);
            }

            if ((keys_pressed & KEY_RIGHT) == KEY_RIGHT) {
                RunMenu(core, keymenu[3]);
            }

            if ((keys_pressed & KEY_START) == KEY_START) {
                RunMenu(core, keymenu[4]);
            }

            if ((keys_pressed & KEY_A) == KEY_A) {
                RunMenu(core, keymenu[5]);
            }

            if ((keys_pressed & KEY_B) == KEY_B) {
                RunMenu(core, keymenu[6]);
            }

            if ((keys_pressed & KEY_X) == KEY_X) {
                RunMenu(core, keymenu[7]);
            }
            if ((keys_pressed & KEY_Y) == KEY_Y) {
                RunMenu(core, keymenu[8]);
            }

            if ((keys_pressed & KEY_L) == KEY_L) {
                RunMenu(core, keymenu[9]);
            }
            if ((keys_pressed & KEY_R) == KEY_R) {
                RunMenu(core, keymenu[10]);
            }

            if ((keys_pressed & KEY_L2) == KEY_L2) {
                RunMenu(core, keymenu[11]);
            }

            if ((keys_pressed & KEY_R2) == KEY_R2) {
                RunMenu(core, keymenu[12]);
            }
        }

        if ((core->keyEmul == 3) && (core->inKeyboard == 0)) {
            if ((keys_pressed & KEY_UP) == KEY_UP) {
                CPC_SetScanCode(core, keyown[0]);
                keyIsSet[0] = 1;
            } else if (keyIsSet[0] != 0) {
                keyIsSet[0] = 0;
                CPC_ClearScanCode(core, keyown[0]);
            }

            if ((keys_pressed & KEY_DOWN) == KEY_DOWN) {
                CPC_SetScanCode(core, keyown[1]);
                keyIsSet[1] = 1;
            } else if (keyIsSet[1] != 0) {
                keyIsSet[1] = 0;
                CPC_ClearScanCode(core, keyown[1]);
            }

            if ((keys_pressed & KEY_LEFT) == KEY_LEFT) {
                CPC_SetScanCode(core, keyown[2]);
                keyIsSet[2] = 1;
            } else if (keyIsSet[2] != 0) {
                keyIsSet[2] = 0;
                CPC_ClearScanCode(core, keyown[2]);
            }

            if ((keys_pressed & KEY_RIGHT) == KEY_RIGHT) {
                CPC_SetScanCode(core, keyown[3]);
                keyIsSet[3] = 1;
            } else if (keyIsSet[3] != 0) {
                keyIsSet[3] = 0;
                CPC_ClearScanCode(core, keyown[3]);
            }

            if ((keys_pressed & KEY_START) == KEY_START) {
                CPC_SetScanCode(core, keyown[4]);
                keyIsSet[4] = 1;
            } else if (keyIsSet[4] != 0) {
                keyIsSet[4] = 0;
                CPC_ClearScanCode(core, keyown[4]);
            }

            if ((keys_pressed & KEY_A) == KEY_A) {
                CPC_SetScanCode(core, keyown[5]);
                keyIsSet[5] = 1;
            } else if (keyIsSet[5] != 0) {
                keyIsSet[5] = 0;
                CPC_ClearScanCode(core, keyown[5]);
            }

            if ((keys_pressed & KEY_B) == KEY_B) {
                CPC_SetScanCode(core, keyown[6]);
                keyIsSet[6] = 1;
            } else if (keyIsSet[6] != 0) {
                keyIsSet[6] = 0;
                CPC_ClearScanCode(core, keyown[6]);
            }

            if ((keys_pressed & KEY_X) == KEY_X) {
                CPC_SetScanCode(core, keyown[7]);
                keyIsSet[7] = 1;
            } else if (keyIsSet[7] != 0) {
                keyIsSet[7] = 0;
                CPC_ClearScanCode(core, keyown[7]);
            }

            if ((keys_pressed & KEY_Y) == KEY_Y) {
                CPC_SetScanCode(core, keyown[8]);
                keyIsSet[8] = 1;
            } else if (keyIsSet[8] != 0) {
                keyIsSet[8] = 0;
                CPC_ClearScanCode(core, keyown[8]);
            }

            if ((keys_pressed & KEY_L) == KEY_L) {
                CPC_SetScanCode(core, keyown[9]);
                keyIsSet[9] = 1;
            } else if (keyIsSet[9] != 0) {
                keyIsSet[9] = 0;
                CPC_ClearScanCode(core, keyown[9]);
            }

            if ((keys_pressed & KEY_R) == KEY_R) {
                CPC_SetScanCode(core, keyown[10]);
                keyIsSet[10] = 1;
            } else if (keyIsSet[10] != 0) {
                keyIsSet[10] = 0;
                CPC_ClearScanCode(core, keyown[10]);
            }

            if ((keys_pressed & KEY_L2) == KEY_L2) {
                CPC_SetScanCode(core, keyown[11]);
                keyIsSet[11] = 1;
            } else if (keyIsSet[11] != 0) {
                keyIsSet[11] = 0;
                CPC_ClearScanCode(core, keyown[11]);
            }

            if ((keys_pressed & KEY_R2) == KEY_R2) {
                CPC_SetScanCode(core, keyown[12]);
                keyIsSet[12] = 1;
            } else if (keyIsSet[12] != 0) {
                keyIsSet[12] = 0;
                CPC_ClearScanCode(core, keyown[12]);
            }
        }

        // oldkey = keys_pressed;
    }

    return 0;
} /* nds_ReadKey */

void videoinit(void)
{
}

void nds_initBorder(core_crocods_t *core, int *_borderX, int *_borderY)
{
    core->borderX = _borderX;
    core->borderY = _borderY;
}

void nds_init(core_crocods_t *core)
{
    core->icons = (u16 *)malloc(448 * 320 * 2);
    ReadBackgroundGif16(core->icons, (unsigned char *)&icons_gif, icons_gif_length);

    core->icons8 = (u16 *)malloc(320 * 8 * 2);
    ReadBackgroundGif16(core->icons8, (unsigned char *)&icons8_gif, icons8_gif_length);

    core->keyboard = (u16 *)malloc(256 * 192 * 2);
    ReadBackgroundGif16(core->keyboard, (unsigned char *)&keyboard_gif, keyboard_gif_length);

    core->tape = (u16 *)malloc(256 * 155 * 2);
    ReadBackgroundGif16(core->tape, (unsigned char *)&tape_gif, tape_gif_length);

    core->select = (u16 *)malloc(256 * 168 * 2);
    ReadBackgroundGif16(core->select, (unsigned char *)&select_gif, select_gif_length);

    core->menu = (u16 *)malloc(256 * 168 * 2);
    ReadBackgroundGif16(core->menu, (unsigned char *)&menu_gif, menu_gif_length);

    core->shortcuts = (u16 *)malloc(256 * 168 * 2);
    ReadBackgroundGif16(core->shortcuts, (unsigned char *)&shortcuts_gif, shortcuts_gif_length);

    core->hack_tabcoul = 0;
    core->UpdateInk = 1;

    core->Regs1 = 0;
    core->Regs2 = 0;
    core->Regs6 = 0;
    core->Regs7 = 0; // Used by the auto-resize

    core->overlayBitmap_height = 0;

    core->soundEnabled = 1;

    AutoType_Init(core);

// Default Config

//    ExecuteMenu(core, ID_USE_CRTC_WINCPC, NULL);  // 9200% on my iMac
//    ExecuteMenu(core, ID_USE_CRTC_ARNOLD, NULL);  // 1800% on my iMac
    ExecuteMenu(core, ID_USE_CRTC_CAP32, NULL);

    ExecuteMenu(core, ID_COLOR_MONITOR, NULL);
    //   ExecuteMenu(ID_SCREEN_AUTO, NULL);

    //   ExecuteMenu(core, ID_SCREEN_320, NULL);
    ExecuteMenu(core, ID_SCREEN_OVERSCAN, NULL);        // ID_SCREEN_AUTO

#ifdef EMSCRIPTEN
    ExecuteMenu(core, ID_KEY_KEYBOARD, NULL);
#else
    ExecuteMenu(core, ID_KEY_JOYSTICK, NULL);
#endif

//    ExecuteMenu(core, ID_KEY_KEYPAD, NULL);

//    ExecuteMenu(core, ID_NODISPFRAMERATE, NULL);
//    ExecuteMenu(core, ID_DISPFRAMERATE, NULL);

    //   ExecuteMenu(core, ID_HACK_TABCOUL, NULL);

    //    ExecuteMenu(core, ID_DEBUG_ENTER, NULL);

    static char *tmp_directory;

    if (core->snap_dir == NULL) {
        core->snap_dir = (char *)calloc(MAX_PATH + 1, 1);

        strcpy(core->snap_dir, core->home_dir);

        apps_disk_path2Abs(core->snap_dir, "snap");
    }

    tmp_directory = malloc(MAX_PATH + 1);

    strcpy(tmp_directory, core->home_dir);
    apps_disk_path2Abs(tmp_directory, "snap");
#ifdef _WIN32
    mkdir(tmp_directory);
#else
    mkdir(tmp_directory, 0777);
#endif

    strcpy(tmp_directory, core->home_dir);
    apps_disk_path2Abs(tmp_directory, "cfg");
#ifdef _WIN32
    mkdir(tmp_directory);
#else
    mkdir(tmp_directory, 0777);
#endif

    loadIni(core, 0);
    strcpy(core->currentfile, "nofile");

    free(tmp_directory);



} /* nds_init */


// int nds_video_unlock(void)
// {
//    return 1; // OK
// }

// int nds_video_lock(void)
// {
//    return 1; // OK
// }

// void nds_video_close(void)
// {
// }

// void myconsoleClear(core_crocods_t *core)
// {
//    memset(core->consolestring, 0, 1024);
//    core->consolepos = 0;
// }

// void myprintf0(core_crocods_t *core, const char *fmt, ...)
// {
//    char tmp[512];
//
//    va_list args;
//
//    va_start(args, fmt);
//    vsprintf(tmp, fmt, args);
//    va_end(args);
//
//    if (tmp[0] == '\n') {
//        core->consolepos++;
//        if (core->consolepos == 8) {
//            memcpy(core->consolestring, core->consolestring + 128, 1024 - 128);
//            core->consolepos = 7;
//        }
//    }
//
//    memcpy(core->consolestring + core->consolepos * 128, tmp, 128);
//    core->consolestring[core->consolepos * 128 - 1] = 0;
// }

void ddlog(core_crocods_t *core, u8 logLevel, const char *fmt, ...)
{
    // ddlog(core, 2, "ddlog(core, 2, "g: %d,%d\n", core->cliVerbose, logLevel);

    if (core->cliVerbose < logLevel) {
        return;
    }

    char tmp[512];

    va_list args;

    va_start(args, fmt);
    vsprintf(tmp, fmt, args);
    va_end(args);

    printf("%s", tmp);
}


// void myprintf(const char *fmt, ...)
// {
//     char tmp[512];
//     int n;

//     va_list args;

//     va_start(args, fmt);
//     vsprintf(tmp, fmt, args);
//     va_end(args);

//     strncpy(msgbuf, tmp, 32);
//     msgbuf[32] = 0;
//     msgframe = frame;
//     for (n = (int)strlen(msgbuf); n < 32; n++) {
//         msgbuf[n] = ' ';
//     }

// #ifdef USE_CONSOLE
//     ddlog(core, 2, "%s\n", tmp);
// #else

//     if (backBuffer == NULL) {
//         return;
//     } else {
//         consolepos++;
//         if (consolepos == 8) {
//             memcpy(consolestring, consolestring + 128, 1024 - 128);
//             consolepos = 7;
//         }
//         // for(n=0;n<20;n++) swiWaitForVBlank();
//     }
// #endif /* ifdef USE_CONSOLE */
// } /* myprintf */

u16 computeColor(int x, int y, int frame)
{
    u8 Sinus[256] = {131, 134, 137, 141, 144, 147, 150, 153, 156, 159, 162, 165, 168, 171, 174, 177, 180, 183, 186, 188, 191, 194, 196, 199, 202, 204, 207, 209, 212, 214, 216, 219, 221, 223, 225, 227, 229, 231, 233, 234, 236, 238, 239, 241, 242, 244, 245, 246, 247, 249, 250, 250, 251, 252, 253, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 254, 254, 253, 252, 251, 250, 250, 249, 247, 246, 245, 244, 242, 241, 239, 238, 236, 234, 233, 231, 229, 227, 225, 223, 221, 219, 216, 214, 212, 209, 207, 204, 202, 199, 196, 194, 191, 188, 186, 183, 180, 177, 174, 171, 168, 165, 162, 159, 156, 153, 150, 147, 144, 141, 137, 134, 131, 128, 125, 122, 119, 115, 112, 109, 106, 103, 100, 97, 94, 91, 88, 85, 82, 79, 76, 73, 70, 68, 65, 62, 60, 57, 54, 52, 49, 47, 44, 42, 40, 37, 35, 33, 31, 29, 27, 25, 23, 22, 20, 18, 17, 15, 14, 12, 11, 10, 9, 7, 6, 6, 5, 4, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 6, 6, 7, 9, 10, 11, 12, 14, 15, 17, 18, 20, 22, 23, 25, 27, 29, 31, 33, 35, 37, 40, 42, 44, 47, 49, 52, 54, 57, 60, 62, 65, 68, 70, 73, 76, 79, 82, 85, 88, 91, 94, 97, 100, 103, 106, 109, 112, 115, 119, 122, 125, 128};

    u8 r, g, b;

    x = 0;
    y = y * 4;

    y = y / 2;
    frame = frame / 2;

    u8 pal = (Sinus[(x + y) % 256] + Sinus[Sinus[(frame + x) % 256]] + Sinus[Sinus[(frame + y) % 256]]) % 256;

    r = Sinus[(pal + 142) % 256];
    g = Sinus[(pal + 112) % 256];
    b = Sinus[(pal + 74) % 256];

    return RGB565(r, g, b);
}

static u8 bFont6x8[768] = {

/*
 * code=32, hex=0x20, ascii=" "
 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=33, hex=0x21, ascii="!"
     */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=34, hex=0x22, ascii="""
     */
    0x6C,  /* 011011 */
    0x6C,  /* 011011 */
    0x48,  /* 010010 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=35, hex=0x23, ascii="#"
     */
    0x00,  /* 000000 */
    0x28,  /* 001010 */
    0x7C,  /* 011111 */
    0x28,  /* 001010 */
    0x28,  /* 001010 */
    0x7C,  /* 011111 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */

    /*
     * code=36, hex=0x24, ascii="$"
     */
    0x20,  /* 001000 */
    0x38,  /* 001110 */
    0x40,  /* 010000 */
    0x30,  /* 001100 */
    0x08,  /* 000010 */
    0x70,  /* 011100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=37, hex=0x25, ascii="%"
     */
    0x64,  /* 011001 */
    0x64,  /* 011001 */
    0x08,  /* 000010 */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x4C,  /* 010011 */
    0x4C,  /* 010011 */
    0x00,  /* 000000 */

    /*
     * code=38, hex=0x26, ascii="&"
     */
    0x20,  /* 001000 */
    0x50,  /* 010100 */
    0x50,  /* 010100 */
    0x20,  /* 001000 */
    0x54,  /* 010101 */
    0x48,  /* 010010 */
    0x34,  /* 001101 */
    0x00,  /* 000000 */

    /*
     * code=39, hex=0x27, ascii="'"
     */
    0x30,  /* 001100 */
    0x30,  /* 001100 */
    0x20,  /* 001000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=40, hex=0x28, ascii="("
     */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=41, hex=0x29, ascii=")"
     */
    0x20,  /* 001000 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x00,  /* 000000 */

    /*
     * code=42, hex=0x2A, ascii="*"
     */
    0x00,  /* 000000 */
    0x28,  /* 001010 */
    0x38,  /* 001110 */
    0x7C,  /* 011111 */
    0x38,  /* 001110 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=43, hex=0x2B, ascii="+"
     */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x7C,  /* 011111 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=44, hex=0x2C, ascii=","
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x30,  /* 001100 */
    0x20,  /* 001000 */

    /*
     * code=45, hex=0x2D, ascii="-"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=46, hex=0x2E, ascii="."
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */

    /*
     * code=47, hex=0x2F, ascii="/"
     */
    0x00,  /* 000000 */
    0x04,  /* 000001 */
    0x08,  /* 000010 */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x40,  /* 010000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=48, hex=0x30, ascii="0"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x4C,  /* 010011 */
    0x54,  /* 010101 */
    0x64,  /* 011001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=49, hex=0x31, ascii="1"
     */
    0x10,  /* 000100 */
    0x30,  /* 001100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=50, hex=0x32, ascii="2"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x04,  /* 000001 */
    0x18,  /* 000110 */
    0x20,  /* 001000 */
    0x40,  /* 010000 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */

    /*
     * code=51, hex=0x33, ascii="3"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x04,  /* 000001 */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=52, hex=0x34, ascii="4"
     */
    0x08,  /* 000010 */
    0x18,  /* 000110 */
    0x28,  /* 001010 */
    0x48,  /* 010010 */
    0x7C,  /* 011111 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x00,  /* 000000 */

    /*
     * code=53, hex=0x35, ascii="5"
     */
    0x7C,  /* 011111 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x04,  /* 000001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=54, hex=0x36, ascii="6"
     */
    0x18,  /* 000110 */
    0x20,  /* 001000 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=55, hex=0x37, ascii="7"
     */
    0x7C,  /* 011111 */
    0x04,  /* 000001 */
    0x08,  /* 000010 */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x00,  /* 000000 */

    /*
     * code=56, hex=0x38, ascii="8"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=57, hex=0x39, ascii="9"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x04,  /* 000001 */
    0x08,  /* 000010 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */

    /*
     * code=58, hex=0x3A, ascii=":"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */

    /*
     * code=59, hex=0x3B, ascii=";"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x30,  /* 001100 */
    0x20,  /* 001000 */

    /*
     * code=60, hex=0x3C, ascii="<"
     */
    0x08,  /* 000010 */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x40,  /* 010000 */
    0x20,  /* 001000 */
    0x10,  /* 000100 */
    0x08,  /* 000010 */
    0x00,  /* 000000 */

    /*
     * code=61, hex=0x3D, ascii="="
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=62, hex=0x3E, ascii=">"
     */
    0x20,  /* 001000 */
    0x10,  /* 000100 */
    0x08,  /* 000010 */
    0x04,  /* 000001 */
    0x08,  /* 000010 */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x00,  /* 000000 */

    /*
     * code=63, hex=0x3F, ascii="?"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x04,  /* 000001 */
    0x18,  /* 000110 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=64, hex=0x40, ascii="@"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x5C,  /* 010111 */
    0x54,  /* 010101 */
    0x5C,  /* 010111 */
    0x40,  /* 010000 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=65, hex=0x41, ascii="A"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x7C,  /* 011111 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */

    /*
     * code=66, hex=0x42, ascii="B"
     */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */

    /*
     * code=67, hex=0x43, ascii="C"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=68, hex=0x44, ascii="D"
     */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */

    /*
     * code=69, hex=0x45, ascii="E"
     */
    0x7C,  /* 011111 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */

    /*
     * code=70, hex=0x46, ascii="F"
     */
    0x7C,  /* 011111 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x00,  /* 000000 */

    /*
     * code=71, hex=0x47, ascii="G"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x40,  /* 010000 */
    0x5C,  /* 010111 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x00,  /* 000000 */

    /*
     * code=72, hex=0x48, ascii="H"
     */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x7C,  /* 011111 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */

    /*
     * code=73, hex=0x49, ascii="I"
     */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=74, hex=0x4A, ascii="J"
     */
    0x04,  /* 000001 */
    0x04,  /* 000001 */
    0x04,  /* 000001 */
    0x04,  /* 000001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=75, hex=0x4B, ascii="K"
     */
    0x44,  /* 010001 */
    0x48,  /* 010010 */
    0x50,  /* 010100 */
    0x60,  /* 011000 */
    0x50,  /* 010100 */
    0x48,  /* 010010 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */

    /*
     * code=76, hex=0x4C, ascii="L"
     */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */

    /*
     * code=77, hex=0x4D, ascii="M"
     */
    0x44,  /* 010001 */
    0x6C,  /* 011011 */
    0x54,  /* 010101 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */

    /*
     * code=78, hex=0x4E, ascii="N"
     */
    0x44,  /* 010001 */
    0x64,  /* 011001 */
    0x54,  /* 010101 */
    0x4C,  /* 010011 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */

    /*
     * code=79, hex=0x4F, ascii="O"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=80, hex=0x50, ascii="P"
     */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x00,  /* 000000 */

    /*
     * code=81, hex=0x51, ascii="Q"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x54,  /* 010101 */
    0x48,  /* 010010 */
    0x34,  /* 001101 */
    0x00,  /* 000000 */

    /*
     * code=82, hex=0x52, ascii="R"
     */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x48,  /* 010010 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */

    /*
     * code=83, hex=0x53, ascii="S"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x40,  /* 010000 */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=84, hex=0x54, ascii="T"
     */
    0x7C,  /* 011111 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=85, hex=0x55, ascii="U"
     */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=86, hex=0x56, ascii="V"
     */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x28,  /* 001010 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=87, hex=0x57, ascii="W"
     */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x54,  /* 010101 */
    0x54,  /* 010101 */
    0x54,  /* 010101 */
    0x54,  /* 010101 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */

    /*
     * code=88, hex=0x58, ascii="X"
     */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x28,  /* 001010 */
    0x10,  /* 000100 */
    0x28,  /* 001010 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */

    /*
     * code=89, hex=0x59, ascii="Y"
     */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x28,  /* 001010 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=90, hex=0x5A, ascii="Z"
     */
    0x78,  /* 011110 */
    0x08,  /* 000010 */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */

    /*
     * code=91, hex=0x5B, ascii="["
     */
    0x38,  /* 001110 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=92, hex=0x5C, ascii="\"
     */
    0x00,  /* 000000 */
    0x40,  /* 010000 */
    0x20,  /* 001000 */
    0x10,  /* 000100 */
    0x08,  /* 000010 */
    0x04,  /* 000001 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=93, hex=0x5D, ascii="]"
     */
    0x38,  /* 001110 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=94, hex=0x5E, ascii="^"
     */
    0x10,  /* 000100 */
    0x28,  /* 001010 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=95, hex=0x5F, ascii="_"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0xFC,  /* 111111 */

    /*
     * code=96, hex=0x60, ascii="`"
     */
    0x30,  /* 001100 */
    0x30,  /* 001100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=97, hex=0x61, ascii="a"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x3C,  /* 001111 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x00,  /* 000000 */

    /*
     * code=98, hex=0x62, ascii="b"
     */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */

    /*
     * code=99, hex=0x63, ascii="c"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x40,  /* 010000 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=100, hex=0x64, ascii="d"
     */
    0x04,  /* 000001 */
    0x04,  /* 000001 */
    0x3C,  /* 001111 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x00,  /* 000000 */

    /*
     * code=101, hex=0x65, ascii="e"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=102, hex=0x66, ascii="f"
     */
    0x18,  /* 000110 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x78,  /* 011110 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x00,  /* 000000 */

    /*
     * code=103, hex=0x67, ascii="g"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x3C,  /* 001111 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x04,  /* 000001 */
    0x38,  /* 001110 */

    /*
     * code=104, hex=0x68, ascii="h"
     */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x70,  /* 011100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x00,  /* 000000 */

    /*
     * code=105, hex=0x69, ascii="i"
     */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x18,  /* 000110 */
    0x00,  /* 000000 */

    /*
     * code=106, hex=0x6A, ascii="j"
     */
    0x08,  /* 000010 */
    0x00,  /* 000000 */
    0x18,  /* 000110 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x48,  /* 010010 */
    0x30,  /* 001100 */

    /*
     * code=107, hex=0x6B, ascii="k"
     */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x48,  /* 010010 */
    0x50,  /* 010100 */
    0x60,  /* 011000 */
    0x50,  /* 010100 */
    0x48,  /* 010010 */
    0x00,  /* 000000 */

    /*
     * code=108, hex=0x6C, ascii="l"
     */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x18,  /* 000110 */
    0x00,  /* 000000 */

    /*
     * code=109, hex=0x6D, ascii="m"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x68,  /* 011010 */
    0x54,  /* 010101 */
    0x54,  /* 010101 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x00,  /* 000000 */

    /*
     * code=110, hex=0x6E, ascii="n"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x70,  /* 011100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x00,  /* 000000 */

    /*
     * code=111, hex=0x6F, ascii="o"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=112, hex=0x70, ascii="p"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */

    /*
     * code=113, hex=0x71, ascii="q"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x3C,  /* 001111 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x04,  /* 000001 */

    /*
     * code=114, hex=0x72, ascii="r"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x58,  /* 010110 */
    0x24,  /* 001001 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x70,  /* 011100 */
    0x00,  /* 000000 */

    /*
     * code=115, hex=0x73, ascii="s"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x40,  /* 010000 */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */

    /*
     * code=116, hex=0x74, ascii="t"
     */
    0x00,  /* 000000 */
    0x20,  /* 001000 */
    0x78,  /* 011110 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x28,  /* 001010 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=117, hex=0x75, ascii="u"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x58,  /* 010110 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */

    /*
     * code=118, hex=0x76, ascii="v"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x28,  /* 001010 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=119, hex=0x77, ascii="w"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x54,  /* 010101 */
    0x7C,  /* 011111 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */

    /*
     * code=120, hex=0x78, ascii="x"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x30,  /* 001100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x00,  /* 000000 */

    /*
     * code=121, hex=0x79, ascii="y"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x60,  /* 011000 */

    /*
     * code=122, hex=0x7A, ascii="z"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x78,  /* 011110 */
    0x08,  /* 000010 */
    0x30,  /* 001100 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x00,  /* 000000 */

    /*
     * code=123, hex=0x7B, ascii="{"
     */
    0x18,  /* 000110 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x60,  /* 011000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x18,  /* 000110 */
    0x00,  /* 000000 */

    /*
     * code=124, hex=0x7C, ascii="|"
     */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */

    /*
     * code=125, hex=0x7D, ascii="}"
     */
    0x30,  /* 001100 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x0C,  /* 000011 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */

    /*
     * code=126, hex=0x7E, ascii="~"
     */
    0x28,  /* 001010 */
    0x50,  /* 010100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */

    /*
     * code=127, hex=0x7F, ascii="^?"
     */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x6C,  /* 011011 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
};


void cpcprint16_6w(core_crocods_t *core, u16 *MemBitmap, u32 MemBitmap_width, int x, int y, char *pchStr, u16 bColor, u16 backgroundColor, int multi, char transparent)
{
    cpcprint16_6w_limit(core, MemBitmap, MemBitmap_width, x, y, pchStr, bColor, backgroundColor, multi, transparent, 0, 8);
}

void cpcprint16_6w_limit(core_crocods_t *core, u16 *MemBitmap, u32 MemBitmap_width, int x, int y, char *pchStr, u16 bColor, u16 backgroundColor, int multi, char transparent, int miny, int maxy)
{
    // static int frame = 0;

    int iLen, iIdx, iRow, iCol;
    u8 bRow;
    u16 *pdwAddr;
    int n;
    int mx, my; // , mz;

    frame++;

    int dbl = 2;

    if (core->screenIsOptimized) {
        dbl = (core->lastMode == 2) ? 2 : 1;
    }
    if (MemBitmap != core->MemBitmap) {
        dbl = 1;
    }

    pdwAddr = (u16 *)MemBitmap + (y * MemBitmap_width * dbl) + x;

    if ((!core->screenIsOptimized) && (MemBitmap == core->MemBitmap)) {
        switch (core->resize) {
            case 2: // 320x200
                pdwAddr += 64;  // 8*8
                break;
            case 1: // Auto
                pdwAddr += core->XStart * 8; // 8*8
                break;
            case 4: // Overscan - do nothing
                break;
        }
    }

    iLen = (int)strlen(pchStr); // number of characters to process
    for (n = 0; n < iLen; n++) {
        u16 *pdwLine;
        iIdx = (int)pchStr[n]; // get the ASCII value
        if ((iIdx < FNT_MIN_CHAR) || (iIdx > FNT_MAX_CHAR)) { // limit it to the range of chars in the font
            iIdx = FNT_BAD_CHAR;
        }
        iIdx -= FNT_MIN_CHAR; // zero base the index
        pdwLine = pdwAddr; // keep a reference to the current screen position

        iIdx = iIdx * 8; // RED

        for (iRow = 0; iRow < FNT_CHAR_HEIGHT; iRow++) { // loop for all rows in the font character
            for (my = 0; my < multi; my++) {
                u16 *pdPixel;
//                char first = 1;

                pdPixel = pdwLine;
                bRow = bFont6x8[iIdx]; // get the bitmap information for one row
                for (iCol = 0; iCol < 8; iCol++) { // loop for all columns in the font character
                    for (mx = 0; mx < multi; mx++) {
                        // bColor = computeColor(iCol + n*8, iRow, frame);
//                        bColor = 255; // computeColor((n) * 8 * multi + iCol * multi + mx, multi + iRow * multi + my, frame);

// transparent=0;      backgroundColor = bColor;

                        if ((iRow >= miny) && (iRow < maxy)) {
                            if (bRow & 0x80) {
                                *pdPixel = bColor;
                            } else if (!transparent) {
                                *pdPixel =  backgroundColor;
                            }
                        }
                        pdPixel++;
                    }

                    bRow <<= 1; // advance to the next bit
                }
                pdwLine += MemBitmap_width * dbl;
            }
            iIdx++;  // = FNT_CHARS; // advance to next row in font data
        }
        pdwAddr += 6 * multi; // set screen address to next character position
    }
} /* cpcprint16 */

void cpcprint16(core_crocods_t *core, u16 *MemBitmap, u32 MemBitmap_width, int x, int y, char *pchStr, u16 bColor, u16 backgroundColor, int multi, char transparent)
{
    // static int frame = 0;

    int iLen, iIdx, iRow, iCol;
    u8 bRow;
    u16 *pdwAddr;
    int n;
    int mx, my; // , mz;

    frame++;

    int dbl = 2;

    if (core->screenIsOptimized) {
        dbl = (core->lastMode == 2) ? 2 : 1;
    }
    if (MemBitmap != core->MemBitmap) {
        dbl = 1;
    }

    pdwAddr = (u16 *)MemBitmap + (y * MemBitmap_width * dbl) + x;

    if (!core->screenIsOptimized) {
        if (MemBitmap == core->MemBitmap) {
            switch (core->resize) {
                case 2: // 320x200
                    pdwAddr += (40 * MemBitmap_width * dbl) + (32 * 2); // 8*8
                    break;
                case 1: // Auto
                    pdwAddr += core->XStart * 8; // 8*8
                    break;
                case 4: // Overscan - do nothing
                    break;
            }
            multi = 2;
        } else {    // Overlay
        }
    }

    iLen = (int)strlen(pchStr); // number of characters to process
    for (n = 0; n < iLen; n++) {
        u16 *pdwLine;
        iIdx = (int)pchStr[n]; // get the ASCII value
        if ((iIdx < FNT_MIN_CHAR) || (iIdx > FNT_MAX_CHAR)) { // limit it to the range of chars in the font
            iIdx = FNT_BAD_CHAR;
        }
        iIdx -= FNT_MIN_CHAR; // zero base the index
        pdwLine = pdwAddr; // keep a reference to the current screen position

        iIdx = iIdx * 8; // RED

        for (iRow = 0; iRow < FNT_CHAR_HEIGHT; iRow++) { // loop for all rows in the font character
            for (my = 0; my < multi; my++) {
                u16 *pdPixel;
//                char first = 1;

                pdPixel = pdwLine;
                bRow = bFont[iIdx]; // get the bitmap information for one row
                for (iCol = 0; iCol < 8; iCol++) { // loop for all columns in the font character
                    for (mx = 0; mx < multi; mx++) {
                        // bColor = computeColor(iCol + n*8, iRow, frame);
//                        bColor = 255; // computeColor((n) * 8 * multi + iCol * multi + mx, multi + iRow * multi + my, frame);

// transparent=0;      backgroundColor = bColor;

                        if (bRow & 0x80) {
                            // if (multi>1) {

                            //     for (mz=1; mz<=2; mz++) {

                            //         if (*(pdPixel-MemBitmap_width*mz)!=bColor) {
                            //             *(pdPixel-MemBitmap_width*mz)=backgroundColor;
                            //         }
                            //         if (*(pdPixel-mz)!=bColor) {
                            //             *(pdPixel-mz)=backgroundColor;
                            //         }

                            //         *(pdPixel+MemBitmap_width*mz)=backgroundColor;
                            //         *(pdPixel+mz)=backgroundColor;
                            //     }
                            // }

                            *pdPixel = bColor;
//                            first = 0;
                        } else if (!transparent) {
                            *pdPixel =  backgroundColor;
                        }

                        pdPixel++;
                    }

                    bRow <<= 1; // advance to the next bit
                }
                pdwLine += MemBitmap_width * dbl;
            }
            iIdx++;  // = FNT_CHARS; // advance to next row in font data
        }
        pdwAddr += FNT_CHAR_WIDTH * multi; // set screen address to next character position
    }
} /* cpcprint16 */

void cpcprint(core_crocods_t *core, int x, int y, char *pchStr, u16 bColor)
{
    if (bColor == 1) {
        bColor = RGB565(0xFF, 0xFF, 0);
    }

    cpcprint16(core, core->MemBitmap, core->MemBitmap_width, x, y, pchStr, bColor, RGB565(0, 0, 0x7F), 1, 0);
}

void cpcprintOnScreen(core_crocods_t *core, char *pchStr)
{
    u16 *MemBitmap = core->MemBitmap;
    u32 MemBitmap_width = core->MemBitmap_width;
    u16 bColor = RGB565(0xFF, 0xFF, 0x00);
    u16 backgroundColor = RGB565(0x00, 0x00, 0xFF);
    int multi = 1;

    int iLen, iIdx, iRow, iCol;
    u8 bRow;
    u16 *pdwAddr;
    int n;
    int mx;

    int dbl = 2;

    if (core->screenIsOptimized) {
        dbl = (core->lastMode == 2) ? 2 : 1;
    }

    pdwAddr = (u16 *)MemBitmap;

    if (!core->screenIsOptimized) {
        int top = 0;
        MemBitmap_width = 384;

        switch (core->resize) {
            case 2:     // 320x200
                top = 40;
                pdwAddr += (40 * MemBitmap_width * dbl) + (32 * 2);
                break;
            case 1:     // Auto
                top = core->y0;
                pdwAddr += (core->y0 * MemBitmap_width * dbl) + (core->x0 * 2);
                break;
            case 4:     // Overscan - do nothing
                top = 0;
                break;
        }
        multi = 2;

        // Background the 8 first line

        u16 *screenAdr = core->MemBitmap + top * MemBitmap_width * dbl;
        int x, y;
        for (y = 0; y < 8; y++) {
            for (x = 0; x < 384 * 2; x++) {
                *screenAdr =  backgroundColor; // AlphaBlendFast((*screenAdr), backgroundColor); backgroundColor;
                screenAdr++;
            }
        }
    }

    iLen = (int)strlen(pchStr);

    for (n = 0; n < iLen; n++) {
        u16 *pdwLine;

        iIdx = (int)pchStr[n];
        if ((iIdx < FNT_MIN_CHAR) || (iIdx > FNT_MAX_CHAR)) {
            iIdx = FNT_BAD_CHAR;
        }

        iIdx -= FNT_MIN_CHAR;
        pdwLine = pdwAddr;

        iIdx = iIdx * 8;

        for (iRow = 0; iRow < FNT_CHAR_HEIGHT; iRow++) { // loop for all rows in the font character
            u16 *pdPixel;

            pdPixel = pdwLine;
            bRow = bFont[iIdx];
            for (iCol = 0; iCol < 8; iCol++) {
                for (mx = 0; mx < multi; mx++) {        // Multi for screen not optimized
                    if (bRow & 0x80) {
                        *pdPixel = bColor;
                    }
                    pdPixel++;
                }

                bRow <<= 1;
            }
            pdwLine += MemBitmap_width * dbl;

            iIdx++;
        }
        pdwAddr += FNT_CHAR_WIDTH * multi; // set screen address to next character position
    }
} /* cpcprintOnScreen */

void dispIcon(core_crocods_t *core, int i, int j, int dispiconX, int dispiconY, char select)
{
    int x, y;

    if ((dispiconX == -1) || (dispiconY == -1)) {
        return;
    }

    //    u16 *pdwAddr = (u16*)core->MemBitmap + ((j*32)*core->MemBitmap_width) + (224+x) - (i*32);

    //    ddlog(core, 2, "%d %d\n", i, j);

    u16 *pdwAddr = (u16 *)core->overlayBitmap + (j * 320) + i;

    for (y = 0; y < 32; y++) {
        u16 *pdPixel;

        pdPixel = pdwAddr;

        for (x = 0; x < 32; x++) {
            u16 car;
            car = core->icons[(x + dispiconX * 32) + (y + dispiconY * 32) * 448];

            if (select != 0) {
                // car = AlphaBlendFast(car, 0);

                int16_t red = ((car & 0xF800) >> 11);
                int16_t green = ((car & 0x07E0) >> 5);
                int16_t blue = (car & 0x001F);
                int16_t grayscale = (0.2126 * red) + (0.7152 * green / 2.0) + (0.0722 * blue);
                //                                                   ^^^^^
                car = (grayscale << 11) + (grayscale << 6) + grayscale;
            }
            *pdPixel = car;

            pdPixel++;
        }

        pdwAddr += 320;
    }
} /* dispIcon */

void dispIcon8(core_crocods_t *core, int i, int j, int icon)
{
    int x, y;

    u16 *pdwAddr = (u16 *)core->overlayBitmap +  (j * 320) + i;

    for (y = 0; y < 8; y++) {
        u16 *pdPixel;

        pdPixel = pdwAddr;

        for (x = 0; x < 8; x++) {
            u16 car;
            car = core->icons8[(x + icon * 8) + y * 320];

            *pdPixel = car;

            pdPixel++;
        }

        pdwAddr += 320;
    }
} /* dispIcon */

void mydebug(core_crocods_t *core, const char *fmt, ...);


// FileSystem

u8 * FS_Readfile(char *filename, u32 *romsize)
{
    u8 *rom = NULL;

    FILE *romfile;

    romfile = os_fopen(filename, "rb");

    if (romfile != NULL) {
        fseek(romfile, 0, SEEK_END);
        *romsize = (u32)ftell(romfile);

        rom = (u8 *)malloc(*romsize);

        fseek(romfile, 0, SEEK_SET);
        fread(rom, 1, *romsize, romfile);

        fclose(romfile);
    }

    if (*romsize == 0) {
        rom = NULL;
    }

    return rom;
} /* FS_Readfile */

// ---

// Create empty ini file
void createDefaultIni(core_crocods_t *core, int local)
{
    FILE *ini;

    char iniFile[MAX_PATH + 1];

    strcpy(iniFile, core->home_dir);

    if (local) {
        char iniFile0[MAX_PATH + 1];
        sprintf(iniFile0, "%s.ini", core->filename);

        apps_disk_path2Abs(iniFile, "cfg");
        apps_disk_path2Abs(iniFile, iniFile0);
    } else {
        apps_disk_path2Abs(iniFile, "crocods.ini");
    }

    ddlog(core, 2, "Create default ini in %s\n", iniFile);

    if ((ini = os_fopen(iniFile, "w")) == NULL) {
        ddlog(core, 2, "iniparser: cannot create example.ini\n");
        return;
    }

    fprintf(ini,
            "#\n"
            "# CrocoDS ini file\n"
            "#\n"
            "\n");
    fclose(ini);
} /* createDefaultIni */

// local: 1 -> load custom ini
void loadIni(core_crocods_t *core, int local)
{
#ifdef HAVE_FS


    if (core->home_dir != NULL) {
        dictionary *ini;

        char iniFile[MAX_PATH + 1];

        strcpy(iniFile, core->home_dir);

        if (core->filename[0] != 0) {
            char iniFile0[MAX_PATH + 1];
            sprintf(iniFile0, "%s.ini", core->filename);

            apps_disk_path2Abs(iniFile, "cfg");
            apps_disk_path2Abs(iniFile, iniFile0);
            FILE *fic = os_fopen(iniFile, "rb");
            if (fic != NULL) {
                fclose(fic);
                local = 1;
            }
        }

        if (local == 0) {
            strcpy(iniFile, core->home_dir);

            apps_disk_path2Abs(iniFile, "crocods.ini");
        }

        if (core->system_dir != NULL) {       // Standalone application
            char systemIniFile[MAX_PATH + 1];

            strcpy(systemIniFile, core->system_dir);
            apps_disk_path2Abs(systemIniFile, "crocods.ini");

            FILE *fic = os_fopen(systemIniFile, "rb");
            if (fic != NULL) {
                strcpy(iniFile, systemIniFile);
                fclose(fic);
            }

//        if (local == 1) && (
        }

        ddlog(core, 1, "Load ini from %s\n", iniFile);

//    mydebug(core, "Load ini: %s\n", iniFile);

        ini = iniparser_load(iniFile);
        if (ini == NULL) {
            if (local == 1) {
                return;
            }
            createDefaultIni(core, local);
            ini = iniparser_load(iniFile);
            if (ini == NULL) {
                return;
            }
        }

        /* Some temporary variables to hold query results */
        int b;
        int i;
//        double d;
        const char *s;

        b = iniparser_getboolean(ini, "display:color", -1);
        if ((b == -1) && (local == 0)) {
            b = 1;
        }
        if (b == 1) {
            ExecuteMenu(core, ID_COLOR_MONITOR, NULL);
        } else if (b == 0) {
            ExecuteMenu(core, ID_GREEN_MONITOR, NULL);
        }

        i = iniparser_getint(ini, "display:resize", -1);
        if ((i == -1)  && (local == 0)) {
#if defined(_WIN32) || defined(TARGET_OS_MAC)
            i = 4; // Overscan by default on computer
#else
            i = 1;
#endif
        }

        if (i == 1) {                            // AUTO
            ExecuteMenu(core, ID_SCREEN_AUTO, NULL);
        } else if (i == 2) {                     // 320
            ExecuteMenu(core, ID_SCREEN_320, NULL);
        } else if (i == 3) {                     // NO-RESIZE - Not used
            ExecuteMenu(core, ID_SCREEN_NORESIZE, NULL);
        } else if (i == 4) {                     // OVERSCAN
            ExecuteMenu(core, ID_SCREEN_OVERSCAN, NULL);
        }

        i = iniparser_getint(ini, "display:scanline", -1);
        if (i == -1) {
            if (local == 0) {
                core->scanline = 0;
            }
        } else {
            core->scanline = i;
        }

        i = iniparser_getint(ini, "display:fullscreen", -1);
        if (i == 1) {
            ExecuteMenu(core, ID_FULLSCREEN_ON, NULL);
        }

        i = iniparser_getint(ini, "key:emulation", -1);
        if ((i == -1)  && (local == 0)) {
#if defined(_WIN32) || defined(TARGET_OS_MAC)
            i = 2;  // True keyboard by default on desktop
#else
            i = 3;
#endif
        }
        if ((i == 2) || (i == 3)) {
            core->keyEmul = i;
        }

        ddlog(core, 2, "key:emulation: %d\n", i);

        /*
         *
         * keyown[0] = CPC_JOY_UP;
         *         keyown[1] = CPC_JOY_DOWN;
         *         keyown[2] = CPC_JOY_LEFT;
         *         keyown[3] = CPC_JOY_RIGHT;
         *         keyown[4] = CPC_RETURN;
         *         keyown[5] = CPC_JOY_FIRE1;
         *         keyown[6] = CPC_JOY_FIRE2;
         *         keyown[7] = CPC_2;
         *         keyown[8] = CPC_3;
         *
         */

        i = iniparser_getint(ini, "joy:up", -1);
        if (i == -1) {
            if (local == 0) {  // global
                keyown[0] = CPC_JOY_UP;
            }
        } else {
            keyown[0] = i;
        }
        i = iniparser_getint(ini, "joy:down", -1);
        if (i == -1) {
            if (local == 0) {    // global
                keyown[1] = CPC_JOY_DOWN;
            }
        } else {
            keyown[1] = i;
        }
        i = iniparser_getint(ini, "joy:left", -1);
        if (i == -1) {
            if (local == 0) {    // global
                keyown[2] = CPC_JOY_LEFT;
            }
        } else {
            keyown[2] = i;
        }
        i = iniparser_getint(ini, "joy:right", -1);
        if (i == -1) {
            if (local == 0) {    // global
                keyown[3] = CPC_JOY_RIGHT;
            }
        } else {
            keyown[3] = i;
        }
        i = iniparser_getint(ini, "joy:start", -1);
        if (i == -1) {
            if (local == 0) {    // global
                keyown[4] = CPC_RETURN;
            }
        } else {
            keyown[4] = i;
        }
        i = iniparser_getint(ini, "joy:a", -1);
        if (i == -1) {
            if (local == 0) {    // global
                keyown[5] = CPC_JOY_FIRE1;
            }
        } else {
            keyown[5] = i;
        }
        i = iniparser_getint(ini, "joy:b", -1);
        if (i == -1) {
            if (local == 0) {    // global
                keyown[6] = CPC_JOY_FIRE2;
            }
        } else {
            keyown[6] = i;
        }
        i = iniparser_getint(ini, "joy:x", -1);
        if (i == -1) {
            if (local == 0) {    // global
                keyown[7] = CPC_2;
            }
        } else {
            keyown[7] = i;
        }
        i = iniparser_getint(ini, "joy:y", -1);
        if (i == -1) {
            if (local == 0) {    // global
                keyown[8] = CPC_3;
            }
        } else {
            keyown[8] = i;
        }
        i = iniparser_getint(ini, "joy:l", -1);
        if (i == -1) {
            if (local == 0) {    // global
                keyown[9] = CPC_4;   // Not used
            }
        } else {
            keyown[9] = i;
        }
        i = iniparser_getint(ini, "joy:r", -1);
        if (i == -1) {
            if (local == 0) {    // global
                keyown[10] = CPC_5;   // Not used
            }
        } else {
            keyown[10] = i;
        }
        i = iniparser_getint(ini, "joy:l2", -1);
        if (i == -1) {
            if (local == 0) {        // global
                keyown[11] = CPC_6;  // Not used
            }
        } else {
            keyown[11] = i;
        }
        i = iniparser_getint(ini, "joy:r2", -1);
        if (i == -1) {
            if (local == 0) {        // global
                keyown[12] = CPC_7;  // Not used
            }
        } else {
            keyown[12] = i;
        }

        // Key Menu

        // apps_menu_IDFromKeyword

        s = iniparser_getstring(ini, "menu:up", NULL);
        if (s == NULL) {
            if (local == 0) {                    // global
                keymenu[0] = ID_NULL;
            }
        } else {
            keymenu[0] = apps_menu_IDFromKeyword(s);
        }
        s = iniparser_getstring(ini, "menu:down", NULL);
        if (s == NULL) {
            if (local == 0) {                      // global
                keymenu[1] = ID_NULL;
            }
        } else {
            keymenu[1] = apps_menu_IDFromKeyword(s);
        }
        s = iniparser_getstring(ini, "menu:left", NULL);
        if (s == NULL) {
            if (local == 0) {                      // global
                keymenu[2] = ID_NULL;
            }
        } else {
            keymenu[2] = apps_menu_IDFromKeyword(s);
        }
        s = iniparser_getstring(ini, "menu:right", NULL);
        if (s == NULL) {
            if (local == 0) {                      // global
                keymenu[3] = ID_NULL;
            }
        } else {
            keymenu[3] = apps_menu_IDFromKeyword(s);
        }
        s = iniparser_getstring(ini, "menu:start", NULL);
        if (s == NULL) {
            if (local == 0) {                      // global
                keymenu[4] = ID_NULL;
            }
        } else {
            keymenu[4] = apps_menu_IDFromKeyword(s);
        }
        s = iniparser_getstring(ini, "menu:a", NULL);
        if (s == NULL) {
            if (local == 0) {                      // global
                keymenu[5] = ID_NULL;
            }
        } else {
            keymenu[5] = apps_menu_IDFromKeyword(s);
        }
        s = iniparser_getstring(ini, "menu:b", NULL);
        if (s == NULL) {
            if (local == 0) {                      // global
                keymenu[6] = ID_NULL;
            }
        } else {
            keymenu[6] = apps_menu_IDFromKeyword(s);
        }
        s = iniparser_getstring(ini, "menu:x", NULL);
        if (s == NULL) {
            if (local == 0) {                      // global
                keymenu[7] = ID_NULL;
            }
        } else {
            keymenu[7] = apps_menu_IDFromKeyword(s);
        }
        s = iniparser_getstring(ini, "menu:y", NULL);
        if (s == NULL) {
            if (local == 0) {                      // global
                keymenu[8] = ID_NULL;
            }
        } else {
            keymenu[8] = apps_menu_IDFromKeyword(s);
        }
        s = iniparser_getstring(ini, "menu:l", NULL);
        if (s == NULL) {
            if (local == 0) {                      // global
                keymenu[9] = ID_SCREEN_NEXT;       // Not used
                keyown[9] = CPC_NIL;
            }
        } else {
            keymenu[9] = apps_menu_IDFromKeyword(s);
        }
        s = iniparser_getstring(ini, "menu:r", NULL);
        if (s == NULL) {
            if (local == 0) {                          // global
                keymenu[10] = ID_SHOW_VIRTUALKEYBOARD; // Not used
                keyown[10] = CPC_NIL;
            }
        } else {
            keymenu[10] = apps_menu_IDFromKeyword(s);
        }
        s = iniparser_getstring(ini, "menu:l2", NULL);
        if (s == NULL) {
            if (local == 0) {                          // global
                keymenu[11] = ID_NULL;                 // Not used
            }
        } else {
            keymenu[11] = apps_menu_IDFromKeyword(s);
        }
        s = iniparser_getstring(ini, "menu:r2", NULL);
        if (s == NULL) {
            if (local == 0) {                          // global
                keymenu[12] = ID_NULL;                 // Not used
            }
        } else {
            keymenu[12] = apps_menu_IDFromKeyword(s);
        }

        // End keymenu

        i = iniparser_getint(ini, "sound:enabled", -1);
        if ((i == -1)  && (local == 0)) {
            i = 1;
        }

        if (i == 1) {
            ExecuteMenu(core, ID_SOUND_ENABLE, NULL);
        } else {
            ExecuteMenu(core, ID_SOUND_DISABLE, NULL);
        }

        s = iniparser_getstring(ini, "path:home", core->home_dir);
        strcpy(core->home_dir, s);
        core->file_dir = (char *)malloc(strlen(s) + 1);
        strcpy(core->file_dir, s);

        s = iniparser_getstring(ini, "standalone:file", NULL);
        if (s != NULL) {
            core->standalone = 1;

            strcpy(core->openFilename, core->system_dir);
            apps_disk_path2Abs(core->openFilename, s);

            ExecuteMenu(core, ID_PAUSE_EXIT, NULL);
            ExecuteMenu(core, ID_AUTORUN, NULL);
        }

        i = iniparser_getint(ini, "guest:width", -1);
        if ((i == -1)  && (local == 0)) {
            core->guest_width = 0;
        } else {
            core->guest_width = i;
        }

        i = iniparser_getint(ini, "guest:height", -1);
        if ((i == -1)  && (local == 0)) {
            core->guest_height = 0;
        } else {
            core->guest_height = i;
        }

        iniparser_freedict(ini);
    } /* loadIni */

#endif /* ifdef HAVE_FS */

}         /* loadIni */

void loadIniGuest(core_crocods_t *core)
{

    int local = 0;

    // Set home_dir if not set before the function nds_init
    if (core->home_dir == NULL) {
        core->home_dir = (char *)calloc(MAX_PATH + 1, 1);

        char *homeDir = getenv("HOME");
        if (homeDir != NULL) {
            strcpy(core->home_dir, homeDir);
        }

#ifdef _WIN32
        if (homeDir == NULL) {
            char homeDir[MAX_PATH + 1];
            sprintf(homeDir, "%s%s", getenv("HOMEDRIVE"), getenv("HOMEPATH"));     // Win32 value - or LOCALAPPDATA ?
            strcpy(core->home_dir, homeDir);
        }
#endif

        apps_disk_path2Abs(core->home_dir, ".crocods");

        ddlog(core, 2, "Homedir final: %s\n", core->home_dir);

#ifdef _WIN32
        mkdir(core->home_dir);
#else
        mkdir(core->home_dir, 0777);
#endif
    }

    if (core->home_dir != NULL) {
        dictionary *ini;

        char iniFile[MAX_PATH + 1];

        strcpy(iniFile, core->home_dir);

        if (core->filename[0] != 0) {
            char iniFile0[MAX_PATH + 1];
            sprintf(iniFile0, "%s.ini", core->filename);

            apps_disk_path2Abs(iniFile, "cfg");
            apps_disk_path2Abs(iniFile, iniFile0);
            FILE *fic = os_fopen(iniFile, "rb");
            if (fic != NULL) {
                fclose(fic);
                local = 1;
            }
        }

        if (local == 0) {
            strcpy(iniFile, core->home_dir);

            apps_disk_path2Abs(iniFile, "crocods.ini");
        }

        if (core->system_dir != NULL) {       // Standalone application
            char systemIniFile[MAX_PATH + 1];

            strcpy(systemIniFile, core->system_dir);
            apps_disk_path2Abs(systemIniFile, "crocods.ini");

            FILE *fic = os_fopen(systemIniFile, "rb");
            if (fic != NULL) {
                strcpy(iniFile, systemIniFile);
                fclose(fic);
            }

//        if (local == 1) && (
        }

        ddlog(core, 1, "Load ini from %s\n", iniFile);

//    mydebug(core, "Load ini: %s\n", iniFile);

        ini = iniparser_load(iniFile);
        if (ini == NULL) {
            if (local == 1) {
                return;
            }
            createDefaultIni(core, local);
            ini = iniparser_load(iniFile);
            if (ini == NULL) {
                return;
            }
        }

        /* Some temporary variables to hold query results */
        int i;

        i = iniparser_getint(ini, "guest:width", -1);
        if ((i == -1)  && (local == 0)) {
            core->guest_width = 0;
        } else {
            core->guest_width = i;
        }

        i = iniparser_getint(ini, "guest:height", -1);
        if ((i == -1)  && (local == 0)) {
            core->guest_height = 0;
        } else {
            core->guest_height = i;
        }

        iniparser_freedict(ini);
    } /* loadIni */


}         /* loadIni */


void saveIni(core_crocods_t *core, int local)
{
    if (core->home_dir != NULL) {
        char s[32];

        dictionary *ini = dictionary_new(0);

        iniparser_set(ini, "display", NULL);

        if (core->lastcolour == 0) {
            iniparser_set(ini, "display:color", "0");
        } else {
            iniparser_set(ini, "display:color", "1");
        }

        if (core->resize == 1) {         // AUTO
            iniparser_set(ini, "display:resize", "1");
        } else if (core->resize == 2) {  // 320
            iniparser_set(ini, "display:resize", "2");
        } else if (core->resize == 3) {  // NO-RESIZE
            iniparser_set(ini, "display:resize", "3");
        } else if (core->resize == 4) {  // OVERSCAN
            iniparser_set(ini, "display:resize", "4");
        }

        sprintf(s, "%d", core->scanline);
        iniparser_set(ini, "display:scanline", s);

        if (guestIsFullscreen(core)) {
            iniparser_set(ini, "display:fullscreen", "1");
        }

        iniparser_set(ini, "sound", NULL);

        if (core->soundEnabled == 1) {
            iniparser_set(ini, "sound:enabled", "1");
        } else {
            iniparser_set(ini, "sound:enabled", "0");
        }

        iniparser_set(ini, "joy", NULL);

        sprintf(s, "%d", keyown[0]);
        iniparser_set(ini, "joy:up", s);
        sprintf(s, "%d", keyown[1]);
        iniparser_set(ini, "joy:down", s);
        sprintf(s, "%d", keyown[2]);
        iniparser_set(ini, "joy:left", s);
        sprintf(s, "%d", keyown[3]);
        iniparser_set(ini, "joy:right", s);
        sprintf(s, "%d", keyown[4]);
        iniparser_set(ini, "joy:start", s);
        sprintf(s, "%d", keyown[5]);
        iniparser_set(ini, "joy:a", s);
        sprintf(s, "%d", keyown[6]);
        iniparser_set(ini, "joy:b", s);
        sprintf(s, "%d", keyown[7]);
        iniparser_set(ini, "joy:x", s);
        sprintf(s, "%d", keyown[8]);
        iniparser_set(ini, "joy:y", s);
        sprintf(s, "%d", keyown[9]);
        iniparser_set(ini, "joy:l", s);
        sprintf(s, "%d", keyown[10]);
        iniparser_set(ini, "joy:r", s);
        sprintf(s, "%d", keyown[11]);
        iniparser_set(ini, "joy:l2", s);
        sprintf(s, "%d", keyown[12]);
        iniparser_set(ini, "joy:r2", s);

        iniparser_set(ini, "menu", NULL);

        sprintf(s, "%s", apps_menu_KeywordFromID(keymenu[0]));
        iniparser_set(ini, "menu:up", s);
        sprintf(s, "%s", apps_menu_KeywordFromID(keymenu[1]));
        iniparser_set(ini, "menu:down", s);
        sprintf(s, "%s", apps_menu_KeywordFromID(keymenu[2]));
        iniparser_set(ini, "menu:left", s);
        sprintf(s, "%s", apps_menu_KeywordFromID(keymenu[3]));
        iniparser_set(ini, "menu:right", s);
        sprintf(s, "%s", apps_menu_KeywordFromID(keymenu[4]));
        iniparser_set(ini, "menu:start", s);
        sprintf(s, "%s", apps_menu_KeywordFromID(keymenu[5]));
        iniparser_set(ini, "menu:a", s);
        sprintf(s, "%s", apps_menu_KeywordFromID(keymenu[6]));
        iniparser_set(ini, "menu:b", s);
        sprintf(s, "%s", apps_menu_KeywordFromID(keymenu[7]));
        iniparser_set(ini, "menu:x", s);
        sprintf(s, "%s", apps_menu_KeywordFromID(keymenu[8]));
        iniparser_set(ini, "menu:y", s);
        sprintf(s, "%s", apps_menu_KeywordFromID(keymenu[9]));
        iniparser_set(ini, "menu:l", s);
        sprintf(s, "%s", apps_menu_KeywordFromID(keymenu[10]));
        iniparser_set(ini, "menu:r", s);
        sprintf(s, "%s", apps_menu_KeywordFromID(keymenu[11]));
        iniparser_set(ini, "menu:l2", s);
        sprintf(s, "%s", apps_menu_KeywordFromID(keymenu[12]));
        iniparser_set(ini, "menu:r2", s);

        iniparser_set(ini, "key", NULL);

        if (core->keyEmul == 2) {
            iniparser_set(ini, "key:emulation", "2");         // Use the true keyboard
        } else if (core->keyEmul == 3) {
            iniparser_set(ini, "key:emulation", "3");         // Use joystick emulation
        }

        iniparser_set(ini, "path", NULL);
        iniparser_set(ini, "path:home", core->home_dir);

        iniparser_set(ini, "guest", NULL);


        sprintf(s, "%d", core->guest_width);
        iniparser_set(ini, "guest:width", s);

        sprintf(s, "%d", core->guest_height);
        iniparser_set(ini, "guest:height", s);

        char iniFile[MAX_PATH + 1];
        FILE *fic;
        // Use global only if local file doesn't exist & local == 0

        if (core->filename[0] != 0) {
            char iniFile0[MAX_PATH + 1];
            sprintf(iniFile0, "%s.ini", core->filename);

            strcpy(iniFile, core->home_dir);
            apps_disk_path2Abs(iniFile, "cfg");
            apps_disk_path2Abs(iniFile, iniFile0);

            fic = os_fopen(iniFile, "rb");
            if (fic != NULL) {
                fclose(fic);
                local = 1;
            }
        }

        if (local == 0) {
            strcpy(iniFile, core->home_dir);
            apps_disk_path2Abs(iniFile, "crocods.ini");
        }

        ddlog(core, 2, "Save ini to %s\n", iniFile);

        fic = os_fopen(iniFile, "wb");
        iniparser_dump_ini(ini, fic);
        fclose(fic);

        iniparser_freedict(ini);
    }
}         /* saveIni */


void saveScreenRaw(core_crocods_t *core)
{
    char snap[MAX_PATH + 1];
    char snapFile[MAX_PATH + 1];

    int i = 0;         // , j;

    do {
        i++;
        strcpy(snap, core->snap_dir);
        if (core->filename[0] != 0) {
            sprintf(snapFile, "%s_%03d.raw", core->filename, i);
        } else {
            sprintf(snapFile, "system_%03d.raw",  i);
        }
        apps_disk_path2Abs(snap, snapFile);

        FILE *fic = os_fopen(snap, "rb");
        if (fic == NULL) {
            break;
        }
        fclose(fic);
    } while (1);

    // mydebug("Save snap: %s\n", snap);
//    SauveSnap(core, snap);

    // Convert to png after
    // magick convert -size 768x288+0  -depth 16 GRAY:brixen.dsk_1.raw  \( -clone 0 -evaluate RightShift 11 -evaluate And 31 -evaluate LeftShift 11 \)  \( -clone 0 -evaluate RightShift 5 -evaluate And 63 -evaluate LeftShift 10 \) \(  -clone 0 -evaluate And 31 -evaluate LeftShift 11 \)  -delete 0   -combine -resize 768x576\!  brixen.dsk_1.jpg


    FILE *fic = os_fopen(snap, "wb");

    fwrite(core->MemBitmap, 1, 384 * 288 * 4, fic);
    fclose(fic);
}         /* saveScreenRaw */


void filenameWithoutExtension(char *out, char *filename)
{
    char *ext;


    if (filename[0] == 0) {
        strcpy(out, "system");
        return;
    }

    strcpy(out, filename);

    ext = strrchr(out, '.');
    if (ext != NULL) {
        *ext = 0;
    }
}

void saveScreen(core_crocods_t *core)
{
    char filename[MAX_PATH + 1];
    char snap[MAX_PATH + 1];
    char snapFile[MAX_PATH + 1];

    int i = 0, j;

    do {
        strcpy(snap, core->snap_dir);

        filenameWithoutExtension(filename, core->filename);

        if (i == 0) {
            sprintf(snapFile, "%s.gif", filename);
        } else {
            sprintf(snapFile, "%s_%03d.gif", filename, i);
        }
        apps_disk_path2Abs(snap, snapFile);

        FILE *fic = os_fopen(snap, "rb");
        if (fic == NULL) {
            break;
        }
        fclose(fic);

        i++;

    } while (1);

    // mydebug("Save snap: %s\n", snap);
//    SauveSnap(core, snap);

    // Convert to png after
    // magick convert -size 768x288+0  -depth 16 GRAY:brixen.dsk_1.raw  \( -clone 0 -evaluate RightShift 11 -evaluate And 31 -evaluate LeftShift 11 \)  \( -clone 0 -evaluate RightShift 5 -evaluate And 63 -evaluate LeftShift 10 \) \(  -clone 0 -evaluate And 31 -evaluate LeftShift 11 \)  -delete 0   -combine -resize 768x576\!  brixen.dsk_1.jpg

    u8 palette[32 * 3];
    u16 palrgb[32];

    for (i = 0; i < 32; i++) {
        int r = (RgbCPCdef[ i ] >> 16) & 0xFF;
        int g = (RgbCPCdef[ i ] >> 8) & 0xFF;
        int b = (RgbCPCdef[ i ] >> 0) & 0xFF;

        palette[i * 3 + 0] = r;
        palette[i * 3 + 1] = g;
        palette[i * 3 + 2] = b;

        palrgb[i] = RGB565(r, g, b);
    }

    ddlog(core, 0, "Save image to %s\n", snap);

    ge_GIF *gif = ge_new_gif(snap, 384 * 2, 272 * 2, palette, 5, -1);

    if (gif == NULL) {
        exit(EXIT_FAILURE);
    }

    ddlog(core, 2, "Gif: %p\n", gif);

    u16 x, y;

    for (y = 0; y < 272; y++) {
        // ddlog(core, 2, "\ny: %d,", y);
        for (x = 0; x < 384 * 2; x++) {
            u16 col = core->MemBitmap[x + y * 384 * 2];

            // ddlog(core, 2, "%d,", x);

            for (j = 0; j < 32; j++) {
                if (palrgb[j] == col) {
                    gif->frame[x + (y * 2) * 384 * 2] = j;
                    gif->frame[x + (y * 2 + 1) * 384 * 2] = j;
                }
            }
        }
    }

    ge_add_frame(gif, 0);
    ge_close_gif(gif);

    #ifdef EMSCRIPTEN
    if (1 == 1) {
        FILE *fic = os_fopen(snap, "rb");
        fseek(fic, 0L, SEEK_END);
        u32 size = ftell(fic);
        fseek(fic, 0L, SEEK_SET);
        char *buffer = (char *)malloc(size);
        fread(buffer, 1, size, fic);
        fclose(fic);

        EM_ASM({
            window.download($0, $1, $2)
        }, snapFile, buffer, size);

        free(buffer);

        return;
    }
    #endif /* ifdef EMSCRIPTEN */

//
//    FILE *fic = os_fopen(snap, "wb");
//    fwrite(core->MemBitmap, 1, 384 * 288 * 4, fic);
//    fclose(fic);
}         /* saveScreen */

void exportDisk(core_crocods_t *core)
{
    char snap[MAX_PATH + 1];
    char snapFile[MAX_PATH + 1];

    int i;

    char filename[MAX_PATH + 1];
    char ext[16];

    strcpy(filename, core->filename);
    if (filename[0] == 0) {
        strcpy(filename, "disk.dsk");
    }

    char *ext0 = strrchr(filename, '.');

    if (ext0 != NULL) {
        strcpy(ext, ext0);
        *ext0 = 0;
    }

    i = 0;

    do {
        strcpy(snap, core->snap_dir);
        if (i == 0) {
            sprintf(snapFile, "%s%s", filename, ext);
        } else {
            sprintf(snapFile, "%s_%03d%s",  filename, i, ext);
        }
        apps_disk_path2Abs(snap, snapFile);

        FILE *fic = os_fopen(snap, "rb");
        if (fic == NULL) {
            break;
        }
        fclose(fic);

        i++;
    } while (1);

    char *resources = NULL;
    int resources_len = 0;

    if (core->resources_len != 0) {
        resources = core->resources;
        resources_len = core->resources_len;
    }
    if (core->dataFile_len != 0) {
        resources = core->dataFile;
        resources_len = core->dataFile_len;
    }

    if (resources != NULL) {
        FILE *fic = os_fopen(snap, "wb");
        fwrite(resources, 1, resources_len, fic);
        fclose(fic);
    }

    #ifdef EMSCRIPTEN
    if (resources != NULL) {
        EM_ASM({
            window.download($0, $1, $2)
        }, snapFile, resources, resources_len);
    }
    #endif /* ifdef EMSCRIPTEN */
}         /* exportDisk */
