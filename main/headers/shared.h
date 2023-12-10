#ifndef SHARED_H
#define SHARED_H

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#ifdef ANDROID
#define SDL2
#ifndef POSIX
#define POSIX
#endif
#endif

#ifdef _WIN32
#define SDL2
#ifndef JOYSTICK
#define JOYSTICK
#endif
#ifndef POSIX
#define POSIX
#endif
#endif

#ifdef TARGET_OS_LINUX
#ifndef JOYSTICK
#define JOYSTICK
#endif
#ifndef POSIX
#define POSIX
#endif
#endif


#ifdef TARGET_OS_MAC
#ifndef JOYSTICK
#define JOYSTICK
#endif
#ifndef POSIX
#define POSIX
#endif
#endif

#ifdef GCW
#ifndef JOYSTICK
#define JOYSTICK
#endif
#ifndef POSIX
#define POSIX
#endif
#endif

#ifdef RPI
#ifndef JOYSTICK
#define JOYSTICK
#endif
#ifndef POSIX
#define POSIX
#endif
#endif


#ifdef BITTBOY
#ifndef POSIX
#define POSIX
#endif
#endif

#ifdef RS97
#ifndef POSIX
#define POSIX
#endif
#endif

#ifdef DINGOO
#ifndef NOWAIT
#define NOWAIT
#endif
#endif

#ifdef DREAMCAST
#ifndef POSIX
#define POSIX
#endif
#ifndef JOYSTICK
#define JOYSTICK
#endif
#endif

#if defined(GCW)
#define PATH_DIRECTORY getenv("HOME")
#define SAVE_DIRECTORY "/.oswan/"
#define EXTENSION      ""
#elif defined(RPI)
#define PATH_DIRECTORY getenv("HOME")
#define SAVE_DIRECTORY "/.oswan/"
#define EXTENSION      ""
#elif defined(DREAMCAST)
#define PATH_DIRECTORY "/ram/"
#define SAVE_DIRECTORY ""
#define EXTENSION      ""
#else
#define PATH_DIRECTORY "./"
#define SAVE_DIRECTORY ""
#define EXTENSION      ""
#endif

#define MAX__PATH      1024
#define FILE_LIST_ROWS 19

#define SYSVID_WIDTH   224
#define SYSVID_HEIGHT  144

#define GF_GAMEINIT    1
#define GF_GAMEQUIT    3
#define GF_GAMERUNNING 4

#define true           1
#define false          0

#define PIX_TO_RGB(fmt, r, g, b) (((r * 8 >> fmt->Rloss) << fmt->Rshift) | ((g * 6 >> fmt->Gloss) << fmt->Gshift) | ((b * 8 >> fmt->Bloss) << fmt->Bshift))

/* CrocoDS dependencies */
// #include "../../crocods-core/plateform.h"
#include "plateform.h"

#define cartridge_IsLoaded()     (strlen(gameName) != 0)

typedef struct {
    uint16_t sndLevel;
    uint16_t m_ScreenRatio;     /* 0 = 1x size, 1 = full screen, 2 = Keep Aspect */
    uint16_t OD_Joy[12];        /* each key mapping	*/
    uint16_t m_DisplayFPS;
    int8_t current_dir_rom[MAX__PATH];
    uint16_t input_layout;
    uint16_t reserved1;
    uint16_t reserved2;
    uint16_t reserved3;
    uint16_t reserved4;
} gamecfg;

extern gamecfg GameConf;
extern uint32_t m_Flag;

extern char gameName[512];


/* menu */
extern void screen_showtopmenu(void);
extern void print_string_video(int16_t x, const int16_t y, const char *s);

extern int16_t button_state[18];
extern uint8_t button_time[18];
extern uint8_t button_virtual[18];

#endif /* ifndef SHARED_H */
