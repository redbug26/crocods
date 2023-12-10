// #pragma message "apps_console_internal.h"

// #ifndef CLI
// #pragma message "NO CLI"
// #else
// #pragma message "CLI"
// #endif /* ifndef CLI */

#ifndef CLI

#include "plateform.h"
#include "os.h"
#include "z80_cap32.h"

#include <string.h>

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdarg.h>
#include <dirent.h>

#include "miniz.h"
#include "idsk_lite.h"

#include "3rdparty/lua/lua.h"
#include "3rdparty/lua/lauxlib.h"
#include "3rdparty/lua/lualib.h"
#include "3rdparty/lua/ldebug.h"

#include "3rdparty/rasm/rasm.h"

#include "3rdparty/lodepng/lodepng.h"

#include "3rdparty/hxcmod/hxcmod.h"

#include "3rdparty/libstsound/StSoundLibrary.h"

#include "../main/guest.h"

#include "basic.h"

extern t_z80regs z80;

#define KEY_MAX 32 /* Max number of key events we can store at the same time. */

#define SHIFT   0x200
#define OPTION  0x400
#define CTRL    0x800

// https://github.com/Mandragoratools/Bad_duck_esp/tree/master/Arduino_keyboard_LBDL/src

extern unsigned char cpc6128_bin[];



#define CW      (fgb.win[fgb.cur_win])

typedef struct keyState {
    USBHID_id scancode;
    int counter;
} keyState;


typedef struct fark_file_s {
    char fullpath[2048];
    char filename[2048];
    char isDir;
    u32 size;

    char hidden, readonly, archived;
    u8 type;
    u16 exec, load;
    u8 user;

} fark_file_t;


typedef struct fark_openfile_s {
    char *buf;
    long bufsize;
} fark_openfile_t;

typedef struct fark_session_s {
    lua_State *L;
    long long epoch;        // Number of frame since begining of the run
    long long start_ms;
    long long paused_ms;
    long long epoch5s;      // Number of frame since begining of the run
    long long start_ms5s;
    long long paused_ms5s;
} fark_session_t;



typedef struct fark_fs_s {
    char homeDir[2048];
    char currentDir[2048];
    char frkDir[2048];
    fark_file_t **file;
    int fileCount;
    int fileCountMax;

    char allIsLoaded;
    char isAbsolute;
    char archivePath[2048];


    int type; // 0: local, 1: zip, 2: Amstrad DSK

    char *archive_buf;
    int archive_buf_size;
} fark_fs_t;

typedef struct fark_findfile_s {
    fark_file_t *file;

    int c;

    fark_fs_t *fs;
} fark_findfile_t;

typedef struct fark_window_s {

    u8 fromX, fromY;
    u8 height, width;
    u8 x, y;
    u16 colorFront, colorBack;

} fark_window_t;

/// @brief Object representing fark
typedef struct core_fark_s {
    char a;
    u16 MemBitmap[384 * 2 * 288];
    u32 buttons;

    // u8 amstrad;  // 0: off, 1: running, 2: pause
    // u8 amstradZoom;  // 0: off, 1: full, 2: 320
    // u8 amstradHWZoom;  // 0: off, 1: on
    u8 frameChanged;
    u8 f1_pressed;

    u8 mx0, my0, mx1, my1;

    u8 winHeight, winWidth;
    u8 curX, curY;
    u8 shiftX, shiftY, shiftActivated;

#define MAXLINE 1000

    u8 text[96 * MAXLINE];
    u16 textColorFront[96 * MAXLINE];
    u16 textColorBack[96 * MAXLINE];

    fark_window_t win[8];
    u8 cur_win;

    u8 inEditor;
    u16 editorBuf[256];
    u16 editorBufLen;

    u16 palette[32];

    keyState key[KEY_MAX];   /* Remember if a key is pressed / repeated. */
    time_t lastevent;   /* Last event time, so we can go standby */
    char scancode[USBHID_NUM_ID];

    int frameCursor;

    fark_session_t *s;

    u8 cmdline[256 * 32];
    u16 cmdlinePos;
    u16 cmdLineAutoIndentPos; // 8193: not used
    s16 cmdLineAutoIndentIdx;  // Index in iteration table for auto indent
    u8 cmdLineAutoIndentBefore[256 * 32];

    char *runCmd;
    char *filenameRunCmd;
    char runningApplication;

    int luaerr; /* True if there was an error in the latest iteration. */
    long long start_ms;

    long lastTicks;
    int fps;
    char showfps;

    int beep;

    /// @brief
    u16 btnMask;

    // mod

    modcontext modloaded;
    u8 modStatus;  // 1: pause, 2: playing
    tracker_buffer_state modTrackbufState;

    char *modbuf1, *modbuf2;
    int currentmod;
    char modIsLoaded;

    // ay

    char ayIsLoaded;
    YMMUSIC *pMusic;

    // all song

    u8 zikFlag;  // &0x01: display overlay
    u16 zikLastHeight[16];
    u16 zikLastVol[16];
    u8 zikNbChannel;

    // ---

    fark_fs_t *appli_fs;     // Filesystem of the application
    fark_fs_t *main_fs;      // Main filesystem

    fark_fs_t *fs;           // Current filesystem

    core_crocods_t *core;
} core_fark_t;

enum {
    core_kbd_pressed = 1 << 0,
    core_kbd_repeat  = 1 << 1
};


#define RGB565(R, G, B) ((((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))

void DispConsole(core_crocods_t *core, u16 keys_pressed0);

void cmdLine_insert(char car);
void console_print(const char *fmt, ...);
void programError(const char *e);
void path2Abs(char *p, const char *relatif);

void l_pushtableint(lua_State *L, int key, char *value);

void displayBeauty(void);

fark_openfile_t * fk_readFile(fark_fs_t *fs, const char *filename0, const char *extension);
void kFS_closeFile(fark_openfile_t *file);
void fk_changeDir(fark_fs_t *fs, const char *dir);
void fk_readfolder(fark_fs_t *fs);

void kReadfolder_dsk(fark_fs_t *fs, const char *mem, int memSize);
void kReadfolder_zip(fark_fs_t *fs, const char *mem, int memSize);
void kReadfolder_local(fark_fs_t *fs);

void setTableField(lua_State *L, char *name, char *field);
void setTableFieldString(lua_State *L, char *name, char *field, char *s);
void setTableFieldNumber(lua_State *L, char *name, char *field, lua_Number n);

void newLine(void);

int cdBinding(lua_State *L);
void shift_disable(void);

int zikStopBinding(lua_State *L);

char pressed_or_repeated(core_fark_t *core, int counter);

u32 fk_sdl_pressed(core_fark_t *core, USBHID_id usbhid_code, char flag, void *object);

char * file_extension(const char *path);

void populateModinfo(lua_State *L);
void populateZikinfo(lua_State *L);

void putOnText(u16 xp, u16 yp, u8 c, u16 back, u16 front);
void fk_displaychar(u16 xp, u16 yp, u8 c, u16 back, u16 front);
void fk_displaycursor(u16 xp, u16 yp, u8 display);

void displayHeader(char *str);
void displayFooter(char *str);
void displayBeauty(void);

void console_loop(void);

void cls(u16 col);
int verifiyFilename(char *filename, int *len, char *errormsg);
void cpcRun(char *filename, u8 exec);

int fk_run(const char *s);

// in editor
int edBinding(lua_State *L);
void editorPaste(char *string);
void console_editor_loop(void);
void editor_gotoBasicLine(u16 line);

// in ansi
void ansiToScreen(char *b, int len);
void initAnsi(void);
// void ansiSave(void);
extern u16 ansi_palette[16]; // RGB565


extern core_fark_t fgb;

#endif /* ifndef CLI */