#include "apps_console_internal.h"

#ifdef _WIN32
#define OS_WIN 1
#endif

#ifndef CLI

extern core_crocods_t gb;

u16 _asciimap[256];

const u16 _asciimap_en_us[256] =
{
    0x00,           // NUL
    0x00,           // SOH
    0x00,           // STX
    0x00,           // ETX
    0x00,           // EOT
    0x00,           // ENQ
    0x00,           // ACK
    0x00,           // BEL
    0x2a,    // BS  Backspace
    0x2b,    // TAB  Tab
    0x28,    // LF  Enter
    0x00,           // VT
    0x00,           // FF
    0x00,           // CR
    0x00,           // SO
    0x00,           // SI
    0x00,           // DEL
    0x00,           // DC1
    0x00,           // DC2
    0x00,           // DC3
    0x00,           // DC4
    0x00,           // NAK
    0x00,           // SYN
    0x00,           // ETB
    0x00,           // CAN
    0x00,           // EM
    0x00,           // SUB
    0x00,           // ESC
    0x00,           // FS
    0x00,           // GS
    0x00,           // RS
    0x00,           // US

    0x2c,     //  ' '
    0x1e | SHIFT, // !
    0x34 | SHIFT, // "
    0x20 | SHIFT, // #
    0x21 | SHIFT, // $
    0x22 | SHIFT, // %
    0x24 | SHIFT, // &
    0x34,        // '
    0x26 | SHIFT, // (
    0x27 | SHIFT, // )
    0x25 | SHIFT, // *
    0x2e | SHIFT, // +
    0x36,        // ,
    0x2d,        // -
    0x37,        // .
    0x38,        // /
    0x27,        // 0
    0x1e,        // 1
    0x1f,        // 2
    0x20,        // 3
    0x21,        // 4
    0x22,        // 5
    0x23,        // 6
    0x24,        // 7
    0x25,        // 8
    0x26,        // 9
    0x33 | SHIFT,  // :
    0x33,        // ;
    0x36 | SHIFT,  // <
    0x2e,        // =
    0x37 | SHIFT,  // >
    0x38 | SHIFT,  // ?
    0x1f | SHIFT,  // @
    0x04 | SHIFT,  // A
    0x05 | SHIFT,  // B
    0x06 | SHIFT,  // C
    0x07 | SHIFT,  // D
    0x08 | SHIFT,  // E
    0x09 | SHIFT,  // F
    0x0a | SHIFT,  // G
    0x0b | SHIFT,  // H
    0x0c | SHIFT,  // I
    0x0d | SHIFT,  // J
    0x0e | SHIFT,  // K
    0x0f | SHIFT,  // L
    0x10 | SHIFT,  // M
    0x11 | SHIFT,  // N
    0x12 | SHIFT,  // O
    0x13 | SHIFT,  // P
    0x14 | SHIFT,  // Q
    0x15 | SHIFT,  // R
    0x16 | SHIFT,  // S
    0x17 | SHIFT,  // T
    0x18 | SHIFT,  // U
    0x19 | SHIFT,  // V
    0x1a | SHIFT,  // W
    0x1b | SHIFT,  // X
    0x1c | SHIFT,  // Y
    0x1d | SHIFT,  // Z
    0x2f,  // [
    0x31,  // bslash
    0x30,  // ]
    0x23 | SHIFT, // ^
    0x2d | SHIFT, // _
    0x35,  // `
    0x04,  // a
    0x05,  // b
    0x06,  // c
    0x07,  // d
    0x08,  // e
    0x09,  // f
    0x0a,  // g
    0x0b,  // h
    0x0c,  // i
    0x0d,  // j
    0x0e,  // k
    0x0f,  // l
    0x10,  // m
    0x11,  // n
    0x12,  // o
    0x13,  // p
    0x14,  // q
    0x15,  // r
    0x16,  // s
    0x17,  // t
    0x18,  // u
    0x19,  // v
    0x1a,  // w
    0x1b,  // x
    0x1c,  // y
    0x1d,  // z
    0x2f | SHIFT, // {
    0x31 | SHIFT, // |
    0x30 | SHIFT, // }
    0x35 | SHIFT, // ~
    0      // DEL
};

const u16 _asciimap_fr_fr[256] =
{
    0x00,         // NUL
    0x00,         // SOH
    0x00,         // STX
    0x00,         // ETX
    0x00,         // EOT
    0x00,         // ENQ
    0x00,         // ACK
    0x00,         // BEL
    0x2a,     // BS  Backspace
    0x2b,     // TAB  Tab
    0x28,     // LF  Enter
    0x00,         // VT
    0x00,         // FF
    0x00,         // CR
    0x00,         // SO
    0x00,         // SI
    0x00,         // DEL
    0x00,         // DC1
    0x00,         // DC2
    0x00,         // DC3
    0x00,         // DC4
    0x00,         // NAK
    0x00,         // SYN
    0x00,         // ETB
    0x00,         // CAN
    0x00,         // EM
    0x00,         // SUB
    0x00,         // ESC
    0x00,         // FS
    0x00,         // GS
    0x00,         // RS
    0x00,         // US

    0x2c,      //  ' '
    0x38,         // !
    0x20,         // "
    0x20,         // #
    0x30,         // $
    0x34 | SHIFT, // %
    0x1e,         // &
    0x21,         // '
    0x22,         // (
    0x2d,         // )
    0x31,         // *
    0x2e | SHIFT, // +
    0x10,         // ,
    0x23,         // -
    0x36 | SHIFT, // .
    0x37 | SHIFT, // /
    0x27 | SHIFT, // 0
    0x1e | SHIFT, // 1
    0x1f | SHIFT, // 2
    0x20 | SHIFT, // 3
    0x21 | SHIFT, // 4
    0x22 | SHIFT, // 5
    0x23 | SHIFT, // 6
    0x24 | SHIFT, // 7
    0x25 | SHIFT, // 8
    0x26 | SHIFT, // 9
    0x37,         // :
    0x36,         // ;
    0x64,         // <
    0x2e,         // =
    0x64 | SHIFT,   // >
    0x10 | SHIFT,   // ?
    0x27,         // @
    0x14 | SHIFT,   // A
    0x05 | SHIFT,   // B
    0x06 | SHIFT,   // C
    0x07 | SHIFT,   // D
    0x08 | SHIFT,   // E
    0x09 | SHIFT,   // F
    0x0a | SHIFT,   // G
    0x0b | SHIFT,   // H
    0x0c | SHIFT,   // I
    0x0d | SHIFT,   // J
    0x0e | SHIFT,   // K
    0x0f | SHIFT,   // L
    0x33 | SHIFT,   // M
    0x11 | SHIFT,   // N
    0x12 | SHIFT,   // O
    0x13 | SHIFT,   // P
    0x04 | SHIFT,   // Q
    0x15 | SHIFT,   // R
    0x16 | SHIFT,   // S
    0x17 | SHIFT,   // T
    0x18 | SHIFT,   // U
    0x19 | SHIFT,   // V
    0x1d | SHIFT,   // W
    0x1b | SHIFT,   // X
    0x1c | SHIFT,   // Y
    0x1a | SHIFT,   // Z
    0x22,   // [
    0x25,   // bslash
    0x2d,   // ]
    0x26,   // ^
    0x25,   // _
    0x24,   // `
    0x14,   // a
    0x05,   // b
    0x06,   // c
    0x07,   // d
    0x08,   // e
    0x09,   // f
    0x0a,   // g
    0x0b,   // h
    0x0c,   // i
    0x0d,   // j
    0x0e,   // k
    0x0f,   // l
    0x33,   // m
    0x11,   // n
    0x12,   // o
    0x13,   // p
    0x04,   // q
    0x15,   // r
    0x16,   // s
    0x17,   // t
    0x18,   // u
    0x19,   // v
    0x1d,   // w
    0x1b,   // x
    0x1c,   // y
    0x1a,   // z
    0x21,   // {
    0x23,   // |
    0x2e,   // }
    0x1f,   // ~
    0       // DEL
};


#ifdef OS_WIN
#define DEFSLASH '\\'
#else
#define DEFSLASH '/'
#endif

core_fark_t fgb;

int apps_console_files_flag = 0;
int apps_console_prematuredEnd = 0;   // apps_console_end function already called by the system

void apps_console_end(core_crocods_t *core)
{
    core->runApplication = NULL;

    core->wait_key_released = 1;
}

void apps_console_init(core_crocods_t *core, int flag)
{
    ddlog(core, 2, "apps_console_init: %s\n", core->openFilename);

    core->runApplication = DispConsole;
    apps_console_files_flag = flag;

    apps_console_prematuredEnd = 0;

    core->inConsole = 1;


    core->consoleBitmap = &fgb.MemBitmap[0];
} /* apps_console_init */

// --- test file

struct s_parameter * populateRasmParameters(void)
{
    struct s_parameter *param = (struct s_parameter *)malloc(sizeof(struct s_parameter));

    char tmpDir[MAX_PATH];

    sprintf(tmpDir, "%s%c", fgb.fs->currentDir, DEFSLASH);

    memset(param, 0, sizeof(struct s_parameter));
    param->pathdef = (char **)malloc(sizeof(char *));
    param->pathdef[0] = strdup(tmpDir);
    param->npath = 1;

    return param;
}

void freeRasmParameters(struct s_parameter *param)
{
    free(param->pathdef[0]);
    free(param->pathdef);
    free(param);
}

#define CODESIZE 65535


/// @brief Compile file to check if it could be runned
/// @param filename
/// @param len
/// @param errormsg Allocated string of 256 characters
/// @return 0: no error, 1: invalid extension, 2; error on line
int verifiyFilename(char *filename, int *len, char *errormsg)
{
    char *ext = file_extension(filename);

    if (ext == NULL) {
        return 1;
    }

    if (!strcasecmp(ext, "lua")) {
        return 0;
    }

    if (!strcasecmp(ext, "bas")) {
        // return 0;

        char error = 0;

        struct s_basic_info *debug;
        size_t filelen;
        char ret;
        u8 *basictext;

        u8 basicbin[CODESIZE];

        lodepng_load_file(&basictext, &filelen, filename);

        u16 cleanBasicTextLength;
        u8 *cleanBasicText = clean_basic((u8 *)basictext, filelen, &cleanBasicTextLength);

        free(basictext);

        ddlog(&gb, 2, "Convert (clean)\n%s\n", cleanBasicText);

        u16 length;

        ret = tokenizeBasic(cleanBasicText, basicbin, &length, &debug);

        free(cleanBasicText);

        if (ret != 0) {
            ddlog(&gb, 2, "Couldn't compile\n");
            error = 2;
        } else {
            ddlog(&gb, 2, "Result=%d, Generated code size=%d\n", ret, length);
        }

        if (debug->err != 0) {
            *len = debug->error.line - 1;
            printf("debug line: %d\n", debug->error.line);
            if (debug->error.msg[0] != 0) {
                // printf("debug msg : %s\n", debug->error.msg);
                sprintf(errormsg, "L%d: %s",  debug->error.line, debug->error.msg);
            }
            // printf("debug lenmsg: %d\n", debug->error.lenmsg);
            // printf("debug lenfilename: %d\n", debug->error.lenfilename);
        }

        // printf("debug nberror: %d\n", debug->nberror);
        // printf("debug maxerror: %d\n", debug->maxerror);


        free(debug);

        return error;
    }

    if ((!strcasecmp(ext, "s")) || (!strcasecmp(ext, "asm"))) {
        char error = 0;

        struct s_rasm_info *debug;
        unsigned char *opcode = NULL;
        int opcodelen = 0;
        size_t filelen;
        unsigned char *tmpstr3;
        char ret;

        lodepng_load_file(&tmpstr3, &filelen, filename);

        struct s_parameter *param = populateRasmParameters();
        ret = RasmAssembleInfoParam((const char *)tmpstr3, filelen, &opcode, &opcodelen, &debug, param);
        freeRasmParameters(param);

        if (ret != 0) {
            printf("Couldn't compile\n");
            error = 2;
        } else if (opcodelen == 0) {
            printf("len=0\n");
            error = 2;
        } else {
            printf("Result=%d, Generated code size=%d\n", ret, opcodelen);
        }

        if (debug->error != NULL) {
            *len = debug->error->line - 1;
            printf("debug line: %d\n", debug->error->line);
            if ((debug->error->msg != NULL) && ( debug->error->lenmsg > 0)) {
                printf("debug msg : %s\n", debug->error->msg);
                sprintf(errormsg, "L%d: %s",  debug->error->line, debug->error->msg);
            }
            printf("debug lenmsg: %d\n", debug->error->lenmsg);
            printf("debug lenfilename: %d\n", debug->error->lenfilename);
        }

        printf("debug nberror: %d\n", debug->nberror);
        printf("debug maxerror: %d\n", debug->maxerror);

        printf(" Run: %x\n", debug->run);
        printf(" Start: %x\n", debug->start);

        if (opcode) {
            free(opcode);
        }
        free(debug);
        free(tmpstr3);

        return error;
    }

    return 1;
} /* verifiyFilename */

// --- keyboard

void clearKeyboard(void)
{
    memset(fgb.scancode, 0, sizeof(fgb.scancode));
    memset(fgb.key, 0, sizeof(fgb.key));
}

// --- sound

int audio_align_samples(int given);
uint16_t swapbyte(uint16_t number);

void sendSound(core_crocods_t *core)
{
    // Send sound

    if (fgb.ayIsLoaded) {
        if (!guestCouldSendAudio()) {
            return;
        }

        populateZikinfo(fgb.s->L);

        u16 stream[2048];
        u16 output[2048];
        int len = 4096;
        int n;

        ymMusicCompute(fgb.pMusic,  (msample *)stream, len / 4);

        for (n = 0; n < 4096 / 2; n += 2) {
            switch ( core->audio_format) {
                case CROCODS_AUDIO_S16MSB:
                    output[n] = (s16)swapbyte(stream[n]);
                    output[n + 1] = (s16)swapbyte(stream[n + 1]);
                    break;
                case CROCODS_AUDIO_S16LSB:
                    output[n] = (s16)stream[n];
                    output[n + 1] = (s16)stream[n + 1];
                    break;
                case CROCODS_AUDIO_F32LSB:
                    // todo
                    break;
            }
        }

        guest_queue_audio(output, len);
    } /* sendSound */

    if ((fgb.modIsLoaded) && (fgb.modStatus == 2)) {

        if (!guestCouldSendAudio()) {
            return;
        }

        populateModinfo(fgb.s->L);


        u16 stream[2048];
        u16 output[2048];
        int len = 4096;
        int n;

        if ((fgb.zikFlag & 0x01) != 0) {
            fgb.modTrackbufState.nb_of_state = 0;
            hxcmod_fillbuffer(&fgb.modloaded, (msample *)stream, len / 4, &fgb.modTrackbufState);
            fgb.zikNbChannel = fgb.modTrackbufState.track_state_buf[0].number_of_tracks;
        } else {
            hxcmod_fillbuffer(&fgb.modloaded, (msample *)stream, len / 4, NULL);
        }

        for (n = 0; n < 4096 / 2; n += 2) {
            switch ( core->audio_format) {
                case CROCODS_AUDIO_S16MSB:
                    output[n] = (s16)swapbyte(stream[n]);
                    output[n + 1] = (s16)swapbyte(stream[n + 1]);
                    break;
                case CROCODS_AUDIO_S16LSB:
                    output[n] = (s16)stream[n];
                    output[n + 1] = (s16)stream[n + 1];
                    break;
                case CROCODS_AUDIO_F32LSB:
                    // todo
                    break;
            }
        }

        guest_queue_audio(output, len);
    }

}     /* sendSound */


// --- run

static void init(lua_State *L)  // TODO: check that function exists
{
    lua_getglobal(L, "_init");
    if (!lua_isnil(L, -1)) {
        if (lua_pcall(L, 0, 0, 0)) {
            programError(lua_tostring(L, -1));
        }
    } else {
        lua_pop(L, 1);
    }
}


static void luaapp_exit(lua_State *L) // TODO: check that function exists
{
    lua_getglobal(L, "_exit");
    if (!lua_isnil(L, -1)) {
        if (lua_pcall(L, 0, 0, 0)) {
            programError(lua_tostring(L, -1));
        }
    } else {
        lua_pop(L, 1);
    }
}

int update(lua_State *L) // TODO: check that function exists
{
    lua_getglobal(L, "_update");
    if (!lua_isnil(L, -1)) {
        if (lua_pcall(L, 0, 0, 0)) {
            programError(lua_tostring(L, -1));
            return 1;
        }
    } else {
        lua_pop(L, 1);
        return 1;
    }
    return 0;
}

int draw(lua_State *L) // TODO: check that function exists
{
    lua_getglobal(L, "_draw");
    if (!lua_isnil(L, -1)) {
        if (lua_pcall(L, 0, 0, 0)) {
            programError(lua_tostring(L, -1));
            return 1;
        }
    } else {
        lua_pop(L, 1);
        return 1;
    }
    return 0;
}



// --- fs

/// @brief Comparaison avec WildCards EX: "fichier.kkr","*.kkr" renvoit 0
/// @param a
/// @param b
/// @return
int kFS_WildCmp(const char *a, const char *b)
{
    int p1, p2;
    int ok, ok2;
    char c1, c2, c3, c4;
    int n;

    int astp1 = -1, astp2 = 0;

    n = 0;

    ok2 = 1;     // --- Pas trouvé -------------------------------------------

    while (b[n] != 0) {
        n += (b[n] == ';');

        ok = 0;

        p1 = 0;
        p2 = n + (b[n] == '-');

        while (1) {
            c1 = a[p1];
            c2 = b[p2];
            c3 = (p2 < strlen(b)) ? b[p2 + 1] : 0;
            c4 = (p2 < strlen(b) - 1) ? b[p2 + 2] : 0;

            if ( (c1 >= 'a') & (c1 <= 'z') ) c1 -= 32;
            if ( (c2 >= 'a') & (c2 <= 'z') ) c2 -= 32;
            if ( (c3 >= 'a') & (c3 <= 'z') ) c3 -= 32;

            if ( ((c1 == 0) & ((c2 == 0) | (c2 == ';')) ) |
                 ((c1 == 0) & (c2 == '*') & ((c3 == 0) | (c3 == ';')) ) |
                 ((c1 == 0) & (c2 == '*') & (c3 == '.') & (c4 == '*')) |
                 ((c1 == 0) & (c2 == '.') & (c3 == '*'))
                 ) break;

            if (c1 == 0) {
                ok = 1;
                break;
            }

            if ( (c1 == c2) | (c2 == '?') ) {
                p1++;
                p2++;
                continue;
            }

            if (c2 == '*') {
                if (c1 == c3) {
                    astp1 = p1;
                    astp2 = p2;
                    p2 += 2;
                }

                p1++;

                continue;
            }

            if (astp1 != -1) {
                p1 = astp1 + 1;
                p2 = astp2;

                astp1 = -1;

                continue;
            }

            ok = 1;
            break;
        }

        if (ok == 0) ok2 = (b[n] == '-');

        while ((b[n] != ';') & (b[n] != 0)) n++;
    }

    return ok2;
}     /* kFS_WildCmp */


void kFindclose(fark_findfile_t *f)
{
    free(f);
}

fark_findfile_t * kFindnext(fark_findfile_t *f)
{
    char ok;

    do {
        ok = 1;

        if (f->c >= f->fs->fileCount) {
            kFindclose(f);
            return NULL;
        }

        f->file = f->fs->file[f->c];
        f->c++;

        if (f->fs->isAbsolute) {
            if (strlen(f->file->fullpath) > strlen(f->fs->currentDir)) {
                if (!strncmp(f->file->fullpath, f->fs->currentDir, strlen(f->fs->currentDir))) {
                    char *file = f->file->fullpath + strlen(f->fs->currentDir) + 1;
                    // printf("%s\n", file);
                    if (strchr(file, '/') != NULL) {     // Not a last folder
                        ok = 0;
                    }
                } else {
                    ok = 0;
                }
            } else {
                ok = 0;
            }
        }
    } while (ok == 0);

    return f;
}     /* kFindnext */

fark_findfile_t * kFindfirst(fark_fs_t *fs, char *wildcard)
{
    fark_findfile_t *findfile = (fark_findfile_t *)malloc(sizeof(fark_findfile_t));

    findfile->c = 0;
    findfile->fs = fs;

    return kFindnext(findfile);
}

// --- Binding

int switchmainfsBinding(lua_State *L)
{
    // printf("Switch to mainFS: %s\n", fgb.main_fs->currentDir);
    fgb.fs = fgb.main_fs;
    return 0;
}

int switchapplifsBinding(lua_State *L)
{
    // printf("Switch to Appli: %s\n", fgb.appli_fs->currentDir);
    fgb.fs = fgb.appli_fs;
    return 0;
}

int btnpBinding(lua_State *L)   // TODO: handle repeat
{
    int x;

    x = lua_tonumber(L, -1);

    if (x == 0) { // LEFT
        return ((fgb.btnMask & 0x0001) != 0) | ((fgb.btnMask & 0x0100) != 0);
    } else if (x == 1) {   // RIGHT
        return ((fgb.btnMask & 0x0002) != 0) | ((fgb.btnMask & 0x0200) != 0);
    } else if (x == 2) {  // UP
        return ((fgb.btnMask & 0x0004) != 0) | ((fgb.btnMask & 0x0400) != 0);
    } else if (x == 3) {  // DOWN
        return ((fgb.btnMask & 0x0008) != 0) | ((fgb.btnMask & 0x0800) != 0);
    } else if (x == 4) { // O
        return ((fgb.btnMask & 0x0010) != 0) | ((fgb.btnMask & 0x1000) != 0);
    } else if (x == 5) { // X
        return ((fgb.btnMask & 0x0020) != 0) | ((fgb.btnMask & 0x2000) != 0);
    } else if (x == 6) { // Pause
        return ((fgb.btnMask & 0x0040) != 0) | ((fgb.btnMask & 0x4000) != 0);
    } else if (x == 7) { // h/a
        return ((fgb.btnMask & 0x0080) != 0) | ((fgb.btnMask & 0x8000) != 0);
    }

    return 0;
} /* btnpBinding */

int btnBinding(lua_State *L)
{
    int x;

    x = lua_tonumber(L, -1);

    if (x == 0) { // LEFT
        return ((fgb.btnMask & 0x0001) != 0) | ((fgb.btnMask & 0x0100) != 0);
    } else if (x == 1) {   // RIGHT
        return ((fgb.btnMask & 0x0002) != 0) | ((fgb.btnMask & 0x0200) != 0);
    } else if (x == 2) {  // UP
        return ((fgb.btnMask & 0x0004) != 0) | ((fgb.btnMask & 0x0400) != 0);
    } else if (x == 3) {  // DOWN
        return ((fgb.btnMask & 0x0008) != 0) | ((fgb.btnMask & 0x0800) != 0);
    } else if (x == 4) { // O
        return ((fgb.btnMask & 0x0010) != 0) | ((fgb.btnMask & 0x1000) != 0);
    } else if (x == 5) { // X
        return ((fgb.btnMask & 0x0020) != 0) | ((fgb.btnMask & 0x2000) != 0);
    } else if (x == 6) { // Pause
        return ((fgb.btnMask & 0x0040) != 0) | ((fgb.btnMask & 0x4000) != 0);
    } else if (x == 7) { // h/a
        return ((fgb.btnMask & 0x0080) != 0) | ((fgb.btnMask & 0x8000) != 0);
    }

    return 0;
} /* btnBinding */

// --- Binding Module

void populateZikinfo(lua_State *L)
{
    char value[32];
    int j;




    sprintf(value, "%06.2f", ymMusicGetPos(fgb.pMusic) / 10000.0);
    setTableFieldString(L, "module", "pattern", value);
    setTableFieldNumber(L, "module", "pos", 0);
    setTableFieldNumber(L, "module", "bpm", 0);
    setTableFieldNumber(L, "module", "speed", 0);

    setTableFieldNumber(L, "module", "tracks", 3);

    u8 channels[3];

    ymMusicGetChannels(fgb.pMusic, (u8 *)channels);

    for (j = 0; j < 3; j++) {
        char fieldname[32];
        sprintf(fieldname, "vol_%d", j + 1);
        setTableFieldNumber(L, "module", fieldname, 0);

        sprintf(fieldname, "per_%d", j + 1);
        setTableFieldNumber(L, "module", fieldname, channels[j]);
    }

    // pInfo->pSongAuthor = pSongAuthor;
    // pInfo->pSongComment = pSongComment;
    // pInfo->pSongType = pSongType;
    // pInfo->pSongPlayer = pSongPlayer;

    // pInfo->musicTimeInMs = getMusicTime();
    // pInfo->musicTimeInSec = pInfo->music



} /* populateZikinfo */

void populateModinfo(lua_State *L)
{
    char value[32];

    tracker_buffer_state *tb = &fgb.modTrackbufState;

    int i, j;



    i = tb->cur_rd_index;

    if (tb->nb_of_state > i) {
        setTableFieldNumber(L, "module", "tracks", tb->track_state_buf[i].number_of_tracks);

        sprintf(value, "%.3d:%.2d", tb->track_state_buf[i].cur_pattern, tb->track_state_buf[i].cur_pattern_pos);
        setTableFieldString(L, "module", "pattern", value);
        setTableFieldNumber(L, "module", "pos", tb->track_state_buf[i].cur_pattern_table_pos);
        setTableFieldNumber(L, "module", "bpm", tb->track_state_buf[i].bpm);
        setTableFieldNumber(L, "module", "speed", tb->track_state_buf[i].speed);

        for (j = 0; j < tb->track_state_buf[i].number_of_tracks; j++) {
            char fieldname[32];
            sprintf(fieldname, "vol_%d", j + 1);
            setTableFieldNumber(L, "module", fieldname, tb->track_state_buf[i].tracks[j].cur_volume);

            sprintf(fieldname, "per_%d", j + 1);

            u16 period;
            if (tb->track_state_buf[i].tracks[j].cur_period < 1200) {
                period = (u16)((tb->track_state_buf[i].tracks[j].cur_period / (float)1200) * 255.0);
            } else {
                period = 255.0;
            }
            setTableFieldNumber(L, "module", fieldname, period);
        }
    }

} /* populateModinfo */

int zikLoadBinding(lua_State *L)
{
    const char *s;
    size_t len;
    char filename[PATH_MAX];

    if (lua_gettop(L) == 1) {
        fgb.zikFlag = 0;

        s = lua_tolstring(L, -1, &len);
        if (!s) return 0;
    } else if (lua_gettop(L) == 2) {
        s = lua_tolstring(L, -2, &len);
        if (!s) return 0;

        fgb.zikFlag = (u16)lua_tonumber(L, -1);
    } else {
        luaG_runerror(L, "Invalid argument");
        return 1;
    }

    strcpy(filename, fgb.fs->currentDir);
    path2Abs(filename, s);

    char *ext = file_extension(filename);

    if (ext == NULL) {
        luaG_runerror(L, "Extension not found");
        return 1;
    }

    zikStopBinding(NULL);

    lua_pushnil(L);
    setTableField(L, "module", "name");
    lua_pushnil(L);
    setTableField(L, "module", "tracks");

    setTableFieldNumber(L, "module", "description_count", 0);

    if (!strcasecmp(ext, "ym")) {

        fgb.pMusic = ymMusicCreate();

        printf("Load %s\n", filename);

        ymMusicLoad(fgb.pMusic, filename);
        ymMusicSetLoopMode(fgb.pMusic, 1);

        fgb.ayIsLoaded = 1;

        ymMusicInfo_t Info;

        ymMusicGetInfo(fgb.pMusic, &Info);

        setTableFieldString(L, "module", "name", Info.pSongName);

        u8 count = 0;

        if ( Info.pSongName[0] != 0) {
            char field[32];
            char value[256];
            sprintf(field, "description_%d", count);
            sprintf(value, "Name.....: %s\n", Info.pSongName);
            setTableFieldString(L, "module", field, value);
            count++;
        }
        if ( Info.pSongAuthor[0] != 0) {
            char field[32];
            char value[256];
            sprintf(field, "description_%d", count);
            sprintf(value, "Author...: %s\n", Info.pSongAuthor);
            setTableFieldString(L, "module", field, value);
            count++;
        }
        if ( Info.pSongComment[0] != 0) {
            char field[32];
            char value[256];
            sprintf(field, "description_%d", count);
            sprintf(value, "Comment..: %s\n", Info.pSongComment);
            setTableFieldString(L, "module", field, value);
            count++;
        }
        if ( 1 == 1) {
            char field[32];
            char value[256];
            sprintf(field, "description_%d", count);
            sprintf(value, "Duration.: %d:%02d\n", (int)(Info.musicTimeInSec / 60), (int)(Info.musicTimeInSec % 60));
            setTableFieldString(L, "module", field, value);
            count++;
        }
        if ( Info.pSongPlayer[0] != 0) {
            char field[32];
            char value[256];
            sprintf(field, "description_%d", count);
            sprintf(value, "Driver...: %s\n", Info.pSongPlayer);
            setTableFieldString(L, "module", field, value);
            count++;
        }
        setTableFieldNumber(L, "module", "description_count", count);

    } else if (!strcasecmp(ext, "mod")) {
        fark_openfile_t *file = fk_readFile(fgb.fs, s, "mod");

        if (file == NULL) {
            console_print("%s: ", s);
            luaG_runerror(L, "File not found!");
            return 0;
        }

        char *modbuf;

        fgb.modIsLoaded = 0;
        hxcmod_unload(&fgb.modloaded);

        if (fgb.currentmod == 0) {
            fgb.currentmod = 1;
            fgb.modbuf1 = malloc(file->bufsize);
            modbuf = fgb.modbuf1;
        } else if (fgb.currentmod == 1) {
            fgb.currentmod = 2;
            if (fgb.modbuf2 != NULL) {
                free(fgb.modbuf2);
            }
            fgb.modbuf2 = malloc(file->bufsize);
            modbuf = fgb.modbuf2;
        } else {
            fgb.currentmod = 1;
            if (fgb.modbuf1 != NULL) {
                free(fgb.modbuf1);
            }
            fgb.modbuf1 = malloc(file->bufsize);
            modbuf = fgb.modbuf1;
        }

        memcpy(modbuf, file->buf, file->bufsize);

        hxcmod_load(&fgb.modloaded, (void *)modbuf, (int)file->bufsize);

        u8 count = 0;
        int i;
        for (i = 0; i < 31; i++) {

            if (fgb.modloaded.song.samples[i].name[0] != 0) {
                char fieldname[32];
                sprintf(fieldname, "description_%d", count);
                setTableFieldString(L, "module", fieldname, (char *)fgb.modloaded.song.samples[i].name);

                printf("%s\n", fgb.modloaded.song.samples[i].name);
                count++;
            }
        }
        setTableFieldNumber(L, "module", "description_count", count);

        setTableFieldString(L, "module", "name", (char *)fgb.modloaded.song.title);

        kFS_closeFile(file);

        fgb.modIsLoaded = 1;
        fgb.modStatus = 1;
    } else {
        luaG_runerror(L, "Invalid extension: %s", ext);
        return 1;
    }

    return 1;
}         /* modulePlayBinding */

int zikPlayBinding(lua_State *L)
{
    fgb.modStatus = 2;

    return 1;
}         /* modulePlayBinding */

int zikStopBinding(lua_State *L)
{
    if (fgb.modIsLoaded == 1) {
        hxcmod_unload(&fgb.modloaded);
        fgb.modIsLoaded = 0;
    }
    if (fgb.ayIsLoaded == 1) {
        printf("Stop mod\n");
        ymMusicDestroy(fgb.pMusic);
        fgb.ayIsLoaded = 0;
    }

    return 1;
}

static const luaL_Reg ziklib[] = {
    {"load", zikLoadBinding},
    {"play", zikPlayBinding},
    {"stop", zikStopBinding},

    {NULL,   NULL}
};

LUAMOD_API int luaopen_zik(lua_State *L)
{
    luaL_newlib(L, ziklib);
    return 1;
}

void install_lib_zik(lua_State *program)
{
    luaL_requiref(program, "zik", luaopen_zik, 1);
    lua_pop(program, 1);         /* remove lib */
}

// --- Binding FS

int fs_download(lua_State *L)
{
    const char *s;
    size_t len;
    char filename[PATH_MAX];

    if (lua_gettop(L) != 1) {
        return 0;
    }

    s = lua_tolstring(L, -1, &len);

    strcpy(filename, fgb.fs->currentDir);
    path2Abs(filename, "..");
    path2Abs(filename, s);

    fark_openfile_t *file = fk_readFile(fgb.fs, s, "");

    if (file == NULL) {
        console_print("%s: ", s);
        luaG_runerror(L, "File not found!");
        return 0;
    }

    FILE *fic = os_fopen(filename, "wb");

    if (fic != NULL) {
        fwrite(file->buf, file->bufsize, 1, fic);
        fclose(fic);

        console_print("File saved to %s\n", filename);
    } else {
        luaG_runerror(L, "Could not save file to %s\n", filename);
    }

    kFS_closeFile(file);


    return 0;
} /* fs_download */


int currentDirBinding(lua_State *L)
{
    lua_pushstring(L, fgb.fs->currentDir);

    console_print("Current dir: %s\n", fgb.fs->currentDir);

    return 1;
}

int resolveBinding(lua_State *L)
{
    int n;

    size_t len;
    char filename[PATH_MAX];

    strcpy(filename, fgb.fs->currentDir);

    for (n = 0; n < lua_gettop(L); n++) {
        const char *s;

        s = lua_tolstring(L, n - lua_gettop(L), &len);
        path2Abs(filename, s);

    }

    lua_pushstring(L, filename);
    return 1;
} /* resolveBinding */


static const luaL_Reg fslib[] = {
    {"cd",       cdBinding},
    {"download", fs_download},
    {"pwd", currentDirBinding},
    {"resolve", resolveBinding},

    {NULL,       NULL}
};

LUAMOD_API int luaopen_fs(lua_State *L)
{
    luaL_newlib(L, fslib);
    return 1;
}

void install_lib_fs(lua_State *program)
{
    luaL_requiref(program, "fs", luaopen_fs, 1);
    lua_pop(program, 1); /* remove lib */
}

// --- Binding sprite

typedef struct {
    int numSprite;
    u16 *sprite;
    int w, h;
    u8 map[256];
} SpriteTable;

// LUA Binding fonction

#define RGB565(R, G, B) ((((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))

void guest_PutSp(u16 *sprite, u16 x, u16 y, u16 w, u16 h)
{
//    printf("sprite %d,%d\n",x,y);

    u16 x0, y0;

    for (y0 = 0; y0 < h; y0++) {
        for (x0 = 0; x0 < w; x0++) {
            fgb.MemBitmap[ (y + y0) * CW.width * 8 + (x + x0)] = sprite[x0 + y0 * w];
        }
    }
} /* guest_putcpc */

int pngloadBinding(lua_State *L)
{
    const char *s;
    size_t len;

//    char filename[PATH_MAX];

    s = lua_tolstring(L, -1, &len);
    if (!s) return 0;

    fark_openfile_t *file = fk_readFile(fgb.fs, s, "png");

    if (file == NULL) {
        console_print("%s", s);
        luaG_runerror(L, "File not found!");
        return 0;
    }

//    unsigned lodepng_decode_memory(unsigned char** out, unsigned* w, unsigned* h,
//                                   const unsigned char* in, size_t insize,
//                                   LodePNGColorType colortype, unsigned bitdepth);

    unsigned char *decoded = 0;
    unsigned w, h;



    lodepng_decode_memory(&decoded, &w, &h, (unsigned char *)file->buf, file->bufsize, LCT_RGB, 8);

    // guest_PutSp(decoded, 0, 0, w, h);

    u16 x, y;

    u16 *out = (u16 *)fgb.MemBitmap;
    u8 *in = decoded;

    for (y = 0; y < h; y++) {
        out = (u16 *)fgb.MemBitmap + (y * CW.width * 8);
        for (x = 0; x < w; x++) {
            *out = RGB565(*(in + 0), *(in + 1), *(in + 2));
            in += 3;
            out++;
        }
    }

    free(decoded);

    kFS_closeFile(file);

    return 0;
} /* pngloadBinding */


int rectBinding(lua_State *L)
{
    int x, y, x0, y0, width, height, color;

    x0 = lua_tonumber(L, -5);
    y0 = lua_tonumber(L, -4);
    width = lua_tonumber(L, -3);
    height = lua_tonumber(L, -2);
    color = lua_tonumber(L, -1);

    int x1, y1, x2, y2;

    x1 = (x0 < 0) ? 0 : x0;
    y1 = (y0 < 0) ? 0 : y0;
    x2 = (x0 + width) > (CW.width * 8) ? (CW.width * 8) : (x0 + width);
    y2 = (y0 + height) > (CW.height * 8) ? (CW.height * 8) : (y0 + height);

    // printf("Rect %d,%d %dx%d %d\n", x1, y1, width, height, fgb.palette[color]);

    color = fgb.palette[color];

    for (y = y1; y < y2; y++) {
        u16 *dest = fgb.MemBitmap + y * (CW.width * 8) + x1;
        for (x = x1; x < x2; x++) {
            *dest = color;
            dest++;
        }
    }

    return 0;
} /* rectBinding */


SpriteTable *spriteTable[32];
int spriteTableCount = 0;

int spriteloadBinding(lua_State *L)
{
    int sprite_width, sprite_height;
    const char *s;
    size_t len;
    char filename[PATH_MAX];

    s = lua_tolstring(L, -3, &len);
    sprite_width = lua_tonumber(L, -2);
    sprite_height = lua_tonumber(L, -1);

    strcpy(filename, fgb.fs->currentDir);
    path2Abs(filename, s);

    fark_openfile_t *file = fk_readFile(fgb.fs, s, "png");

    if (file == NULL) {
        console_print("%s: ", s);
        luaG_runerror(L, "File not found!");
        return 0;
    }

    unsigned char *decoded = 0;
    unsigned w, h;

    lodepng_decode_memory(&decoded, &w, &h, (unsigned char *)file->buf, file->bufsize, LCT_RGB, 8);

//    console_print("ok (%dx%d)\n", w,h);

//    lua_newtable(L);
//    l_pushtablestring(L, "fname", "john");
//    l_pushtablestring(L, "lname", "stewart");

    lua_pushnumber(L, spriteTableCount);

    // a=spriteload() print(a)
    // for i,v in ipairs(a) do print(i,v) end

    spriteTable[spriteTableCount] = (SpriteTable *)malloc(sizeof(SpriteTable));
    spriteTable[spriteTableCount]->numSprite = (h / sprite_height) * (w / sprite_width);
    spriteTable[spriteTableCount]->sprite = (u16 *)malloc(w * h * 2);
    spriteTable[spriteTableCount]->w = sprite_width;
    spriteTable[spriteTableCount]->h = sprite_height;

    u16 *out = (u16 *)spriteTable[spriteTableCount]->sprite;
    u8 *in;

    int x, y, n;

    int x0 = 0, y0 = 0;

    for (n = 0; n < spriteTable[spriteTableCount]->numSprite; n++) {
        for (y = 0; y < sprite_height; y++) {
            in = decoded + (x0 * 3) + (y0 + y) * w * 3;

            for (x = 0; x < sprite_width; x++) {
                *out = RGB565(*(in + 0), *(in + 1), *(in + 2));
                in += 3;
                out++;
            }
        }

        x0 += sprite_width;
        if (x0 >= w) {
            x0 = 0;
            y0 += sprite_height;
        }
    }

    spriteTableCount++;

// TEST: a=spriteload("sprite_allbrick_60x24.png",60,24)
// spritedisp(0,0,0,0)

    kFS_closeFile(file);

//    bfWriteString(l81.fb,x,y,s,len,l81.r,l81.g,l81.b,l81.alpha);
    return 1;   // Number of result
} /* spriteloadBinding */

int spritedispBinding(lua_State *L)
{
    int x, y, idx;
    int sprite;

    sprite = lua_tonumber(L, -4);
    x = lua_tonumber(L, -3);
    y = lua_tonumber(L, -2);
    idx = lua_tonumber(L, -1);

    SpriteTable *sp = spriteTable[sprite];

    guest_PutSp(sp->sprite + (sp->w * sp->h) * idx, x, y, sp->w, sp->h);

    return 0;
}

int spritemapBinding(lua_State *L)
{
    int sprite;
    const u8 *s;
    size_t len;

    sprite = lua_tonumber(L, -2);
    s = (u8 *)lua_tolstring(L, -1, &len);
    if (!s) return 0;

    SpriteTable *sp = spriteTable[sprite];

    memset(sp->map, 0, 256);

    int n = 0;

    while (s[n] != 0) {
        sp->map[s[n]] = n;
        n++;
    }

    return 0;
} /* spritemapBinding */

int spritetextBinding(lua_State *L)
{
    int sprite;
    int x, y;
    const char *s;
    size_t len;

    sprite = lua_tonumber(L, -4);
    x = lua_tonumber(L, -3);
    y = lua_tonumber(L, -2);
    s = lua_tolstring(L, -1, &len);
    if (!s) return 0;

    SpriteTable *sp = spriteTable[sprite];

    if (sp == NULL) {
        luaG_runerror(L, "Sprite not available!");
        return 0;
    }

    u8 *car = (u8 *)s;

    while (*car != 0) {
        guest_PutSp(sp->sprite + (sp->w * sp->h) * sp->map[*car], x, y, sp->w, sp->h);

        car++;
        x += sp->w;
    }

    return 0;
} /* spritetextBinding */

static const luaL_Reg spritelib[] = {
    {"text", spritetextBinding},
    {"map",  spritemapBinding},
    {"disp", spritedispBinding},
    {"load", spriteloadBinding},

    {NULL,   NULL}
};

LUAMOD_API int luaopen_sprite(lua_State *L)
{
    luaL_newlib(L, spritelib);
    return 1;
}

void install_lib_sprite(lua_State *program)
{
    luaL_requiref(program, "sprite", luaopen_sprite, 1);
    lua_pop(program, 1); /* remove lib */
}

// --- Binding CPC

extern t_z80regs z80;

int cpcPeekBinding(lua_State *L)
{

    if (lua_gettop(L) == 2) {
        int adr, y;

        adr = lua_tonumber(L, -2);
        y = lua_tonumber(L, -1);

        lua_pushnumber(L, fgb.core->ROMEXT[y & 255][adr & 0x3FFF]);

        return 1;
    } else if (lua_gettop(L) == 1) {
        int adr;

        adr = lua_tonumber(L, -1);

        lua_pushnumber(L, fgb.core->ROMINF[adr & 0x3FFF]);

        return 1;
    }

    return 0;
}         /* cpcPeekBinding */

int cpcCallBinding(lua_State *L)
{
    if (lua_gettop(L) == 1) {
        int adr;

        adr = lua_tonumber(L, -1);

        z80.PC.w.l = (adr & 65535);

        return 0;
    }

    return 0;
}         /* cpcPokeBinding */

int cpcPokeBinding(lua_State *L)
{


    if (lua_gettop(L) == 3) {
        int adr, y;
        char val;

        adr = lua_tonumber(L, -3);
        y = lua_tonumber(L, -2);
        val = (char)lua_tonumber(L, -1);

        fgb.core->ROMEXT[y & 255][adr & 0x3FFF] = val;

        return 0;
    } else if (lua_gettop(L) == 2) {
        int adr;
        char val;

        adr = lua_tonumber(L, -2);
        val = (char)lua_tonumber(L, -1);

        fgb.core->ROMINF[adr & 0x3FFF] = val;

        return 0;
    }

    return 0;
}         /* cpcPokeBinding */

int cpcCaptureBinding(lua_State *L)
{


    if (lua_gettop(L) == 4) {
        int x0, y0, w, h;

        x0 = lua_tonumber(L, -4);
        y0 = lua_tonumber(L, -3);
        w = lua_tonumber(L, -2);
        h = lua_tonumber(L, -1);

        u32 incX[768], incY[360];

        u32 x, y;

        int w2, h2;

        w2 = w;
        if (w2 > 640 - x0) {
            w2 = 640 - x0;
        }
        h2 = h;
        if (h2 > 360 - y0) {
            h2 = 360 - y0;
        }

        for (x = 0; x < w2; x++) {
            incX[x] = ((x * fgb.core->screenBufferWidth * 2) / w);
        }
        for (y = 0; y < h2; y++) {
            incY[y] = (((y * fgb.core->screenBufferHeight) / (h))) * 768;
        }

        u16 *buffer_scr = fgb.MemBitmap + x0 + y0 * 640;

        for (y = 0; y < h2; y++) {
            for (x = 0; x < w2; x++) {
                int pos = incX[x] + incY[y];

                u16 car = fgb.core->MemBitmap[pos];

                *buffer_scr = car;
                buffer_scr++;
            }
            buffer_scr += (640 - w2);
        }

        return 0;
    }

    return 0;
}         /* cpcCaptureBinding */

int cpcSetRegBinding(lua_State *L)
{

    if (lua_gettop(L) == 2) {
        const char *s;
        size_t len;
        int val;

        s = lua_tolstring(L, -2, &len);
        if (!s) return 0;
        val = lua_tonumber(L, -1);

// #define _A        z80.AF.b.h
        if (!strcasecmp(s, "A")) z80.AF.b.h = (val & 255);
// #define _F        z80.AF.b.l
        if (!strcasecmp(s, "F")) z80.AF.b.l = (val & 255);
// #define _AF       z80.AF.w.l
        if (!strcasecmp(s, "AF")) z80.AF.w.l = (val & 65535);
// #define _B        z80.BC.b.h
        if (!strcasecmp(s, "B")) z80.BC.b.h = (val & 255);
// #define _C        z80.BC.b.l
        if (!strcasecmp(s, "C")) z80.BC.b.l = (val & 255);
// #define _BC       z80.BC.w.l
        if (!strcasecmp(s, "BC")) z80.BC.w.l = (val & 65535);
// #define _D        z80.DE.b.h
        if (!strcasecmp(s, "D")) z80.DE.b.h = (val & 255);
// #define _E        z80.DE.b.l
        if (!strcasecmp(s, "E")) z80.DE.b.l = (val & 255);
// #define _DE       z80.DE.w.l
        if (!strcasecmp(s, "DE")) z80.DE.w.l = (val & 65535);
// #define _H        z80.HL.b.h
        if (!strcasecmp(s, "H")) z80.HL.b.h = (val & 255);
// #define _L        z80.HL.b.l
        if (!strcasecmp(s, "L")) z80.HL.b.l = (val & 255);
// #define _HL       z80.HL.w.l
        if (!strcasecmp(s, "HL")) z80.HL.w.l = (val & 65535);
// #define _PC       z80.PC.w.l
        if (!strcasecmp(s, "PC")) z80.PC.w.l = (val & 65535);
// #define _SP       z80.SP.w.l
        if (!strcasecmp(s, "SP")) z80.SP.w.l = (val & 65535);
// #define _IXh      z80.IX.b.h
        if (!strcasecmp(s, "IXH")) z80.IX.b.h = (val & 255);
// #define _IXl      z80.IX.b.l
        if (!strcasecmp(s, "IXL")) z80.IX.b.l = (val & 255);
// #define _IX       z80.IX.w.l
        if (!strcasecmp(s, "IX")) z80.IX.w.l = (val & 65535);
// #define _IYh      z80.IY.b.h
        if (!strcasecmp(s, "IYH")) z80.IY.b.h = (val & 255);
// #define _IYl      z80.IY.b.l
        if (!strcasecmp(s, "IYL")) z80.IY.b.l = (val & 255);
// #define _IY       z80.IY.w.l
        if (!strcasecmp(s, "IY")) z80.IY.w.l = (val & 65535);
// #define _I        z80.I
        if (!strcasecmp(s, "I")) z80.I = (val & 255);
// #define _R        z80.R
        if (!strcasecmp(s, "R")) z80.R = (val & 255);
// #define _Rb7      z80.Rb7
        if (!strcasecmp(s, "RB7")) z80.Rb7 = (val & 255);
// #define _IFF1     z80.IFF1
        if (!strcasecmp(s, "IFF1")) z80.IFF1 = (val & 255);
// #define _IFF2     z80.IFF2
        if (!strcasecmp(s, "IFF2")) z80.IFF2 = (val & 255);
// #define _IM       z80.IM
        if (!strcasecmp(s, "IM")) z80.IM = (val & 255);
// #define _HALT     z80.HALT
        if (!strcasecmp(s, "HALT")) z80.HALT = (val & 255);

        return 0;
    }

    return 0;
}         /* cpcSetRegBinding */

int cpcGetRegBinding(lua_State *L)
{

    if (lua_gettop(L) == 1) {
        const char *s;
        size_t len;
        int val;

        s = lua_tolstring(L, -1, &len);
        if (!s) return 0;

// #define _A        z80.AF.b.h
        if (!strcasecmp(s, "A")) val = z80.AF.b.h; else
// #define _F        z80.AF.b.l
        if (!strcasecmp(s, "F")) val = z80.AF.b.l; else
// #define _AF       z80.AF.w.l
        if (!strcasecmp(s, "AF")) val = z80.AF.w.l; else
// #define _B        z80.BC.b.h
        if (!strcasecmp(s, "B")) val = z80.BC.b.h; else
// #define _C        z80.BC.b.l
        if (!strcasecmp(s, "C")) val = z80.BC.b.l; else
// #define _BC       z80.BC.w.l
        if (!strcasecmp(s, "BC")) val = z80.BC.w.l; else
// #define _D        z80.DE.b.h
        if (!strcasecmp(s, "D")) val = z80.DE.b.h; else
// #define _E        z80.DE.b.l
        if (!strcasecmp(s, "E")) val = z80.DE.b.l; else
// #define _DE       z80.DE.w.l
        if (!strcasecmp(s, "DE")) val = z80.DE.w.l; else
// #define _H        z80.HL.b.h
        if (!strcasecmp(s, "H")) val = z80.HL.b.h; else
// #define _L        z80.HL.b.l
        if (!strcasecmp(s, "L")) val = z80.HL.b.l; else
// #define _HL       z80.HL.w.l
        if (!strcasecmp(s, "HL")) val = z80.HL.w.l; else
// #define _PC       z80.PC.w.l
        if (!strcasecmp(s, "PC")) val = z80.PC.w.l; else
// #define _SP       z80.SP.w.l
        if (!strcasecmp(s, "SP")) val = z80.SP.w.l; else
// #define _IXh      z80.IX.b.h
        if (!strcasecmp(s, "IXH")) val = z80.IX.b.h; else
// #define _IXl      z80.IX.b.l
        if (!strcasecmp(s, "IXL")) val = z80.IX.b.l; else
// #define _IX       z80.IX.w.l
        if (!strcasecmp(s, "IX")) val = z80.IX.w.l; else
// #define _IYh      z80.IY.b.h
        if (!strcasecmp(s, "IYH")) val = z80.IY.b.h; else
// #define _IYl      z80.IY.b.l
        if (!strcasecmp(s, "IYL")) val = z80.IY.b.l; else
// #define _IY       z80.IY.w.l
        if (!strcasecmp(s, "IY")) val = z80.IY.w.l; else
// #define _I        z80.I
        if (!strcasecmp(s, "I")) val = z80.I; else
// #define _R        z80.R
        if (!strcasecmp(s, "R")) val = z80.R; else
// #define _Rb7      z80.Rb7
        if (!strcasecmp(s, "RB7")) val = z80.Rb7; else
// #define _IFF1     z80.IFF1
        if (!strcasecmp(s, "IFF1")) val = z80.IFF1; else
// #define _IFF2     z80.IFF2
        if (!strcasecmp(s, "IFF2")) val = z80.IFF2; else
// #define _IM       z80.IM
        if (!strcasecmp(s, "IM")) val = z80.IM; else
// #define _HALT     z80.HALT
        if (!strcasecmp(s, "HALT")) val = z80.HALT; else {
            return 0;
        }

        lua_pushinteger(L,  val);
        return 1;
    }

    return 0;
}         /* cpcGetRegBinding */

int cpcTextBinding(lua_State *L)
{
    if (lua_gettop(L) == 1) {
        const char *autoString;
        size_t len;

        autoString = lua_tolstring(L, -1, &len);
        if (!autoString) return 0;

        AutoType_SetString(fgb.core, autoString, 0);         // Rest & Run

        return 0;
    }

    return 0;
}

int cpcKeyBinding(lua_State *L)
{
    if (lua_gettop(L) == 1) {
        const char *autoString;
        size_t len;

        autoString = lua_tolstring(L, -1, &len);
        if (!autoString) return 0;

        // AutoType_SetString(fgb.core, autoString, 0);         // Rest & Run

        return 0;
    }

    return 0;
}

int cpcRunFrameBinding(lua_State *L)
{
    if (lua_gettop(L) == 1) {
        int frame;

        frame = lua_tonumber(L, -1);

        apps_console_end(fgb.core);

        apps_console_prematuredEnd = 1;
        fgb.core->inConsole = 0;
        fgb.core->frameBeforeConsole = frame;

        return 0;
    }

    return 0;
}

int cpcSetScanCodeBinding(lua_State *L)
{
    if (lua_gettop(L) == 1) {
        int code;

        code = lua_tonumber(L, -1);

        CPC_SetScanCode(fgb.core, code);

        return 0;
    }

    return 0;
}

int cpcClearScanCodeBinding(lua_State *L)
{
    if (lua_gettop(L) == 1) {
        int code;

        code = lua_tonumber(L, -1);

        CPC_ClearScanCode(fgb.core, code);

        return 0;
    }

    return 0;
}

int cpcInsertBinding(lua_State *L)
{
    if (lua_gettop(L) == 1) {
        const char *filename;
        size_t len;

        filename = lua_tolstring(L, -1, &len);
        if (!filename) return 0;

        fark_openfile_t *file = fk_readFile(fgb.fs, filename, "dsk");
        if (file == NULL) {
            console_print("%s: ", filename);
            luaG_runerror(L, "File not found!");
            return 0;
        }

        LireDiskMem(fgb.core, (u8 *)file->buf, (u32)file->bufsize);

        kFS_closeFile(file);

        return 0;
    }

    return 0;
}         /* cpcInsertBinding */


/// @brief
/// @param filename
void cpcRun(char *filename, u8 exec)
{
    char *ext = file_extension(filename);

    printf("extension: %s\n", ext);

    if (ext == NULL) {
        return;
    }

    if (!strcasecmp(ext, "lua")) {
        // fgb.inEditor = 0;

        fk_run(filename);
        return;
    }


    if (!strcasecmp(ext, "bas")) {

        u32 length;
        u16 bascicLen;

        u8 basicbin[CODESIZE];
        u8 basictext[CODESIZE];

        FILE *f = os_fopen(filename, "rb");
        if (!f) {
            printf("file not found\n");
            return;
        }
        printf("file found\n");

        // get file size
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);

        if (length > CODESIZE) {
            printf("file too big\n");
            return;
        }

        printf("length: %d\n", length);


        length = fread((char *)basictext, 1, CODESIZE, f);
        fclose(f);
        basictext[length] = 0;


        printf("clean basic\n");

        // ddlog(&gb, 2, "Convert\n%s\n", basictext);

        u16 cleanBasicTextLength;
        u8 *cleanBasicText = clean_basic((u8 *)basictext, length, &cleanBasicTextLength);

        // ddlog(&gb, 2, "Convert (clean)\n%s\n", cleanBasicText);

        printf("tokenizeBasic\n");

        tokenizeBasic(cleanBasicText, basicbin, &bascicLen, NULL);

        free(cleanBasicText);

        printf("basicLen: %d\n", bascicLen);


        if (bascicLen > 0) {
            int n;
            memcpy(fgb.core->MemCPC + 0x170, basicbin, bascicLen);

            u16 endadr;  // Adresse de fin

            switch (fgb.core->hw) {
                case CPC_HW_CPC:
                case CPC_HW_CPC464:
                case CPC_HW_CPC464_COST_DOWN:
                    endadr = 0xAE83;
                    break;
                case CPC_HW_CPCPLUS:
                case CPC_HW_CPC664:
                case CPC_HW_CPC6128:
                case CPC_HW_CPC6128_COST_DOWN:
                case CPC_HW_KCCOMPACT:
                default:
                    endadr = 0xAE66;
                    break;
            }

            printf("End of basic: %d\n", bascicLen);

            u16 adr = 0x170 + bascicLen;
            for (n = 0; n < 4; n++) {
                fgb.core->MemCPC[endadr + n * 2] = adr & 0xFF; // LO HI
                fgb.core->MemCPC[endadr + n * 2 + 1] = adr >> 8;
            }
        }

        if (exec) {
            AutoType_SetString(fgb.core, "run\r", 0);             // Rest & Run
        }

        apps_console_end(fgb.core);
        apps_console_prematuredEnd = 1;
        fgb.core->inConsole = 0;
        // fgb.inEditor = 0;
        return;
    }

    if (!strcasecmp(ext, "s")) {

        struct s_rasm_info *debug;
        unsigned char *opcode = NULL;
        int opcodelen = 0;


        size_t filelen;
        unsigned char *tmpstr3;

        lodepng_load_file(&tmpstr3, &filelen, filename);

        struct s_parameter *param = populateRasmParameters();
        int ret = RasmAssembleInfoParam((const char *)tmpstr3, filelen, &opcode, &opcodelen, &debug, param);
        freeRasmParameters(param);

        if (ret != 0) {
            printf("Couldn't compile\n");
        } else if (opcodelen == 0) {
            printf("len=0\n");
        } else {
            int ramblock = debug->start / 16384;
            memcpy(fgb.core->ROMEXT[ramblock] + debug->start - (ramblock * 16384), opcode, opcodelen);
            memcpy(fgb.core->MemCPC + debug->start, opcode, opcodelen);

            printf("start: %04x\n", debug->start);

            // z80.PC.w.l = debug->start;

            char autoString[32];

            if (exec) {
                sprintf(autoString, "CALL &%04X\r", debug->start);
            } else {
                sprintf(autoString, "REM Start at &%04X\r", debug->start);
            }
            if (opcode)
                free(opcode);

            if (debug)
                free(debug);


            AutoType_SetString(fgb.core, autoString, 0);     // Rest & Run

            apps_console_end(fgb.core);
            apps_console_prematuredEnd = 1;
            fgb.core->inConsole = 0;
            // fgb.inEditor = 0;
            return;
        }
    }


    ddlog(&gb, 2, "Run %s\n", filename);

    strcpy(fgb.core->openFilename, filename);

    apps_console_end(fgb.core);

    apps_console_prematuredEnd = 1;

    fgb.core->inConsole = 0;

    fgb.inEditor = 0;

    ExecuteMenu(fgb.core, ID_RESET, NULL);
} /* cpcRun */

int cpcRunBinding(lua_State *L)
{
    const char *s;
    size_t len;
    char filename[PATH_MAX];

    if (lua_gettop(L) != 1) {
        return 0;
    }

    s = lua_tolstring(L, -1, &len);

    strcpy(filename, fgb.fs->currentDir);
    path2Abs(filename, s);

    cpcRun(filename, 1);

    return 0;
} /* cpcRunBinding */

int cpcLoadBinding(lua_State *L)
{
    const char *s;
    size_t len;
    char filename[PATH_MAX];

    if (lua_gettop(L) != 1) {
        return 0;
    }

    s = lua_tolstring(L, -1, &len);

    strcpy(filename, fgb.fs->currentDir);
    path2Abs(filename, s);

    cpcRun(filename, 0);

    return 0;
} /* cpcRunBinding */


int cpcInfoBinding(lua_State *L)
{
    int xPitch;
    u16 maxColor;

    u16 oldBack = CW.colorBack;
    u16 oldFront = CW.colorFront;

    console_print("Mode %d", fgb.core->lastMode);
    switch (fgb.core->lastMode) {
        case 0:
            xPitch = 1;
            maxColor = 16;
            break;
        case 3:
            xPitch = 1;
            maxColor = 4;
            break;
        case 1:
            xPitch = 2;
            maxColor = 4;
            break;
        case 2:
        default:
            xPitch = 4;
            maxColor = 2;
            break;
    }

//    core->XStart = max( ( 50 - core->RegsCRTC[2] ) << 1, 0);
//    core->XEnd = min(core->XStart + ( core->RegsCRTC[1] << 1 ), 96);

    u16 x1, x2, y1, y2;

    x1 = max( (50 - fgb.core->RegsCRTC[2]) << 2, 0);
    x2 = min(x1 + (fgb.core->RegsCRTC[1] << 2), 192);

    y1 = max( (35 - fgb.core->RegsCRTC[7]) << 3, 0);
    y2 = min(y1 + (fgb.core->RegsCRTC[6] << 3), 272);

    console_print(" (%dx%d)", (x2 - x1) * xPitch, (y2 - y1));

    u16 scrAdr = (((fgb.core->RegsCRTC[12] >> 4) & 3) * 0x4000) | ((fgb.core->RegsCRTC[12] & 3) << 8) | fgb.core->RegsCRTC[13];

    console_print("at adr #%04x", scrAdr);
    console_print("\n");

    int x;


    console_print("Palette (HW): ");
    for (x = 0; x <= maxColor; x++) {
        if (x == maxColor) {
            CW.colorBack = oldBack;
            CW.colorFront =  oldFront;
            console_print(" ");

            CW.colorBack = fgb.core->BG_PALETTE[fgb.core->TabCoul[16]];            // Border color
            CW.colorFront = (CW.colorBack < 32768) ? 65535 : 0;
            console_print(" %02d ", fgb.core->TabCoul[16]);
        } else {
            CW.colorBack = fgb.core->BG_PALETTE[fgb.core->TabCoul[x]];
            CW.colorFront = (CW.colorBack < 32768) ? 65535 : 0;
            console_print(" %02d ", fgb.core->TabCoul[x]);
        }
    }
    CW.colorBack = oldBack;
    CW.colorFront =  oldFront;

    console_print("\n");

    console_print("6845 ");
    for (x = 0; x < 18; x++) {
        if (x & 1) {
            CW.colorBack = fgb.palette[4];
            CW.colorFront = fgb.palette[10];
        } else {
            CW.colorBack = fgb.palette[21];
            CW.colorFront = fgb.palette[10];
        }
        console_print(" R%-2d: %02X ", x, fgb.core->RegsCRTC[x]);
        if (x == 8) {
            CW.colorBack = oldBack;
            CW.colorFront =  oldFront;
            console_print("\nRegs ");
        }
    }

    CW.colorBack = oldBack;
    CW.colorFront =  oldFront;
    console_print("\n");


    // CW.colorBack = oldBack;
    // CW.colorFront =  oldFront;
    // console_print("\n");



//     if (y != 0) {
//         console_print("+%02X  ", y * 16);
//     } else {
//         console_print("     ");
//     }
//     for (x = 0; x  < 16; x++) {
//         console_print("%02x ", fgb.core->MemCPC[adr + x + y * 16]);
//     }

//     console_print(" |");

//     for (x = 0; x  < 16; x++) {
//         u8 c = fgb.core->MemCPC[adr + x + y * 16];
//         console_print("%c", ((c >= 32) && (c < 128)) ? c : '.');
//     }

//     console_print("|\n");
// }
    return 0;
} /* cpcInfoBinding */

int cpcDumpBinding(lua_State *L)
{
    if (lua_gettop(L) != 1) {
        return 0;
    }

    u16 adr = (u16)lua_tointeger(L, -1);

    int x, y;

    for (y = 0; y  < 10; y++) {
        console_print("%04X  ", adr + y * 16);

        if (y != 0) {
            console_print("+%02X  ", y * 16);
        } else {
            console_print("     ");
        }
        for (x = 0; x  < 16; x++) {
            console_print("%02x ", fgb.core->MemCPC[adr + x + y * 16]);
        }

        console_print(" |");

        for (x = 0; x  < 16; x++) {
            u8 c = fgb.core->MemCPC[adr + x + y * 16];
            console_print("%c", ((c >= 32) && (c < 128)) ? c : '.');
        }

        console_print("|\n");
    }
    return 0;
} /* cpcDumpBinding */

int cpcPrnbufBinding(lua_State *L)
{
    char *str = (char *)malloc(fgb.core->printerLength + 1);

    memcpy(str, fgb.core->printerBuffer, fgb.core->printerLength);
    str[fgb.core->printerLength] = 0;

    lua_pushstring(L, str);
    free(str);
    return 1;
}

int cpcPrnwriteBinding(lua_State *L)
{
    char snap[MAX_PATH + 1];
    char snapFile[MAX_PATH + 1];

    int i = 0;

    do {
        strcpy(snap, fgb.core->home_dir);

        if (i == 0) {
            sprintf(snapFile, "printer.raw");
        } else {
            sprintf(snapFile, "printer_%03d.raw", i);
        }
        apps_disk_path2Abs(snap, snapFile);

        FILE *fic = os_fopen(snap, "rb");
        if (fic == NULL) {
            break;
        }
        fclose(fic);

        i++;

    } while (1);

    printf("Save to %s\n", snap);

    FILE *fic = os_fopen(snap, "wb");

    if (fic != NULL) {
        fwrite(fgb.core->printerBuffer, fgb.core->printerLength, 1, fic);
        fclose(fic);

        fgb.core->printerLength = 0;
    }

    return 1;
} /* cpcPrnwriteBinding */

int cpcBasicTextBinding(lua_State *L)
{
    u8 bin[65535];
    char text[65535];

    memcpy(bin, fgb.core->MemCPC + 0x170, 65535 - 0x170);
    untokenizeBasic(bin, text);

    lua_pushstring(L, text);
    return 1;
}

int cpcMemoryBinding(lua_State *L)
{
    lua_pushlstring(L, (char *)fgb.core->MemCPC, 65536);
    return 1;
}

int cpcMemfindBinding(lua_State *L)
{
    u8 *searchString;
    size_t searchLength;
    u16 fromAddr = 0;

    if (lua_gettop(L) == 2) {
        searchString = (u8 *)lua_tolstring(L, -2, &searchLength);
        fromAddr = lua_tonumber(L, -1);
    } else if (lua_gettop(L) == 1) {
        searchString = (u8 *)lua_tolstring(L, -1, &searchLength);
    } else {
        return 0;
    }

    int i, j;

    for (i = fromAddr; i < 65535 - searchLength; i++) {
        char ok = 1;

        for (j = 0; j < searchLength; j++) {
            if (searchString[j] !=  fgb.core->MemCPC[i + j]) {
                ok = 0;
                break;
            }
        }

        if (ok) {
            lua_pushinteger(L,  i);
            return 1;
        }
    }

    lua_pushinteger(L,  -1);
    return 1;
}     /* cpcMemfindBinding */


int cpcMemmarkBinding(lua_State *L)
{
    u8 *name;
    u16 fromAddr = 0;

    if (lua_gettop(L) == 2) {
        name = (u8 *)lua_tostring(L, -2);
        fromAddr = lua_tonumber(L, -1);

        console_print("Set %s=%d\n", name, fromAddr);

        // cpc.memmark("score",0x200)
    }

    return 0;
}



static const luaL_Reg cpclib[] = {
    {"peek",    cpcPeekBinding},
    {"poke",    cpcPokeBinding},
    {"setreg",  cpcSetRegBinding},
    {"getreg",  cpcGetRegBinding},
    {"text",    cpcTextBinding},
    {"info",    cpcInfoBinding},
    {"insert",  cpcInsertBinding},
    {"capture", cpcCaptureBinding},
    {"call",    cpcCallBinding},
    {"load",     cpcLoadBinding},
    {"run",     cpcRunBinding},
    {"dump",    cpcDumpBinding},
    {"prnbuf",  cpcPrnbufBinding},
    {"prnwrite",  cpcPrnwriteBinding},
    {"basictext", cpcBasicTextBinding},
    {"memfind",  cpcMemfindBinding},
    {"memmark",  cpcMemmarkBinding},
    {"memory",  cpcMemoryBinding},
    {"key",  cpcKeyBinding},
    {"runframe",  cpcRunFrameBinding},
    {"setscancode",  cpcSetScanCodeBinding},
    {"clearscancode",  cpcClearScanCodeBinding},


    {NULL,      NULL}
};

LUAMOD_API int luaopen_cpc(lua_State *L)
{
    luaL_newlib(L, cpclib);
    return 1;
}

/// @brief ceci est un test
/// @param program
void install_lib_cpc(lua_State *program)
{
    luaL_requiref(program, "cpc", luaopen_cpc, 1);
    lua_pop(program, 1);         /* remove lib */
}


// fark lib

int farkEventQuitBinding(lua_State *L)
{
    fgb.luaerr = 1;

    return 0;
}



static const luaL_Reg farklib[] = {
    {"exit",    cpcPeekBinding},


    {NULL,      NULL}
};

LUAMOD_API int luaopen_fark(lua_State *L)
{
    luaL_newlib(L, farklib);
    return 1;
}

int fark_fs_preload(lua_State *L)
{
    static luaL_Reg fs_funcs[] =  {
        {"pwd", currentDirBinding},
        {NULL, NULL}
    };

    lua_getglobal(L, "fark");
    luaL_newlib(L, fs_funcs);
    lua_setfield(L, -2, "fs");

    return 1;
}


int fark_event_preload(lua_State *L)
{
    static luaL_Reg event_funcs[] =  {
        {"quit", farkEventQuitBinding},
        {NULL, NULL}
    };

    lua_getglobal(L, "fark");
    luaL_newlib(L, event_funcs);
    lua_setfield(L, -2, "event");

    return 1;
}

void install_lib_fark(lua_State *program)
{
    luaL_requiref(program, "fark", luaopen_fark, 1);
    lua_pop(program, 1);         /* remove lib */

    fark_fs_preload(program);
    fark_event_preload(program);
}


// --- Other binding

int diffBinding(lua_State *L)
{
    if (lua_gettop(L) != 2) {
        luaG_runerror(L, "Strings missing!");

        return 0;
    }

    size_t binLength1;
    u8 *binString1 = (u8 *)lua_tolstring(L, -1, &binLength1);

    size_t binLength2;
    u8 *binString2 = (u8 *)lua_tolstring(L, -2, &binLength2);

    if (binLength1 != binLength2) {
        // console_print("%02x ", binString[adr + x + y * 16]);
    }

    size_t binLength = min(binLength1, binLength2);

    int n;

    int first = 0;
    int same = (binString1[0] == binString2[0]);

    for (n = 1; n < binLength; n++) {
        if ((binString1[n] != binString2[n]) && (same)) {
            console_print("Was same from %02x to %02x\n", first, n - 1);
            first = n;
            same = 0;
        } else if ((binString1[n] == binString2[n]) && (!same)) {
            console_print("Was different from %02x to %02x\n", first, n - 1);
            first = n;
            same = 1;
        }
    }


    return 0;
}         /* catBinding */

int hexBinding(lua_State *L)
{
    if (lua_gettop(L) != 1) {
        luaG_runerror(L, "String missing!");

        return 0;
    }

    size_t binLength;
    u8 *binString = (u8 *)lua_tolstring(L, -1, &binLength);

    printf("String is %zu bytes length\n", binLength);

    int x, y, adr;

    adr = 0;

    for (y = 0; y  < 10; y++) {
        u8 stop = 0;

        console_print("%04X  ", adr + y * 16);

        if (y != 0) {
            console_print("+%02X  ", y * 16);
        } else {
            console_print("     ");
        }
        for (x = 0; x  < 16; x++) {
            if (adr + x + y * 16 < binLength) {
                console_print("%02x ", binString[adr + x + y * 16]);
            } else {
                stop = 1;
                console_print("   ");
            }
        }

        console_print(" |");

        for (x = 0; x  < 16; x++) {
            if (adr + x + y * 16 < binLength) {

                u8 c = binString[adr + x + y * 16];
                console_print("%c", ((c >= 32) && (c < 128)) ? c : '.');
            } else {
                console_print(" ");
            }
        }

        console_print("|\n");
        if (stop) {
            break;
        }
    }

    return 0;
}         /* catBinding */

int catBinding(lua_State *L)
{
    const char *s;
    size_t len;
    char filename[PATH_MAX];

    if (lua_gettop(L) != 1) {
        luaG_runerror(L, "File missing!");

        return 0;
    }

    s = lua_tolstring(L, -1, &len);

    strcpy(filename, fgb.fs->currentDir);
    path2Abs(filename, s);

    fark_openfile_t *file = fk_readFile(fgb.fs, s, "");

    if (file == NULL) {
        console_print("%s: ", s);
        luaG_runerror(L, "File not found!");
        return 0;
    }

    char *ext = file_extension(s);

// TODO: detect binary


// http://www.roysac.com/roy_earlyansishow.html#2

    if (!strcasecmp(ext, "ans")) {

        initAnsi();
        ansiToScreen(file->buf, file->bufsize);
        kFS_closeFile(file);

        return 0;
    }


    if ((file->bufsize == 16384) && ((!strcasecmp(ext, "bin")) || (!strcasecmp(ext, "scr")))) {     // We assume that we have an image

    }

    int beg = 0, current = 0;
    char *body = file->buf;

    while (current < file->bufsize) {
        if ((body[current] == 10) || (body[current] == 13)) {
            body[current] = 0;
            console_print("%s\n", body + beg);
            // printf("%s\n", body + beg);
            beg = current + 1;
        }

        current++;
    }

    if (beg != current) {
        char *str = (char *)malloc(current - beg + 1);
        memcpy(str, body + beg, current - beg);
        str[current - beg] = 0;

        console_print("%s\n", str);
        // printf("%s\n", str);

        free(str);
    }

    kFS_closeFile(file);

    return 0;
}         /* catBinding */

void cls(u16 col)
{
    u16 x, y;

    for (y = CW.fromY; y < CW.fromY + CW.height; y++) {
        for (x = CW.fromX; x < CW.fromX + CW.width; x++) {
            putOnText(x, y, 32, col,  CW.colorFront);
        }
    }

    CW.x = 0;
    CW.y = 0;

    CW.colorBack = col;
}     /* cls */

int clsBinding(lua_State *L)
{
    int col;

    if (lua_gettop(L) == 1) {
        col = (u16)lua_tonumber(L, -1);
    } else {
        col = CW.colorBack;
    }

    cls(col);



    return 0;
}         /* clsBinding */


int locateBinding(lua_State *L)
{
    int x, y;

    if (lua_gettop(L) != 2) {
        return 0;
    }

    x = (u16)lua_tonumber(L, -2);
    y = (u16)lua_tonumber(L, -1);

    CW.x = x;
    CW.y = y;

    return 0;
}

int windowBinding(lua_State *L)
{
    int x, y, w, h;

    if (lua_gettop(L) != 4) {
        return 0;
    }

    x = (u16)lua_tonumber(L, -4);
    y = (u16)lua_tonumber(L, -3);
    w = (u16)lua_tonumber(L, -2);
    h = (u16)lua_tonumber(L, -1);

    CW.fromX = x;
    CW.fromY = y;
    CW.width = w;
    CW.height = h;

    CW.x = 0;
    CW.y = 0;

    return 0;
} /* windowBinding */


int cdBinding(lua_State *L)
{
    const char *s;
    size_t len;

    s = lua_tolstring(L, -1, &len);
    if (!s) {
        fk_changeDir(fgb.fs, fgb.core->file_dir);
        console_print("Change dir to %s\n", fgb.fs->currentDir);

        fk_readfolder(fgb.fs);
        return 0;
    }

    fk_changeDir(fgb.fs, s);
    console_print("%d files in %s\n", fgb.fs->fileCount, fgb.fs->currentDir);

    return 0;
}         /* cdBinding */

int exitBinding(lua_State *L)
{
    ExecuteMenu(fgb.core, ID_EXIT, NULL);
    luaG_runerror(L, "Ok");
    return 0;
}

int dirBinding(lua_State *L)
{
    char filename[512];
    int i = 1;
    u16 oldColorBack = CW.colorBack;
    u16 oldColorFront = CW.colorFront;

    const char *filter;
    size_t len;

    filter = lua_tolstring(L, -1, &len);         // Filter (facultative)

    lua_newtable(L);

    fark_findfile_t *findfile = kFindfirst(fgb.fs, NULL);

    if (fgb.fs->type != 2) {     // TODO: when display the long view ?
        if (findfile != NULL) {
            do {
                if ((filter == NULL) || (filter[0] == 0) || (!kFS_WildCmp(findfile->file->filename, filter))) {
                    int c = 0, n;

//
                    lua_pushnumber(L, i);

                    lua_newtable(L);
                    lua_pushstring(L, findfile->file->filename);
                    lua_setfield(L, -2, "filename");

                    if (findfile->file->isDir) {
                        lua_pushstring(L, "folder");
                    } else {
                        lua_pushstring(L, "file");
                    }
                    lua_setfield(L, -2, "type");

                    lua_settable(L, -3);
//

                    i++;

                    if (!fgb.runningApplication) {
                        if (findfile->file->isDir) {
                            sprintf(filename, "[%s]", findfile->file->filename);
                        } else {
                            sprintf(filename, "%s", findfile->file->filename);
                        }

                        ddlog(&gb, 2, "Dir: %s %s\n", findfile->file->filename, findfile->file->fullpath);


                        c = 24 - ((int)strlen(filename) % 24);
                        if (CW.x + strlen(filename) + c >= CW.width) {
                            console_print("\n");
                        }

                        CW.colorFront = oldColorFront;

                        char *ext = strrchr(findfile->file->filename, '.');
                        if (ext != NULL) {
                            if (!strcasecmp(ext, ".mod")) {
                                CW.colorFront = fgb.palette[12];
                            }
                            if (!strcasecmp(ext, ".lua")) {
                                CW.colorFront = fgb.palette[2];
                            }
                            if (!strcasecmp(ext, ".bas")) {
                                CW.colorFront = fgb.palette[2];
                            }
                            if (!strcasecmp(ext, ".bin")) {
                                CW.colorFront = fgb.palette[2];
                            }
                        }

                        console_print("%s", filename);
                        for (n = 0; n < c; n++) {
                            console_print(" ");
                        }
                    }
                    // printf("%s\n", filename);
                }
                findfile = kFindnext(findfile);
            } while (findfile != NULL);
        }
    }


    if (fgb.fs->type == 2) {
        if (findfile != NULL) {
            do {
                if ((filter == NULL) || (filter[0] == 0) || (!kFS_WildCmp(findfile->file->filename, filter))) {

                    l_pushtableint(L, i, findfile->file->filename);
                    i++;

                    char add[256];

                    sprintf(add, "%c%c%c %02x",
                            (findfile->file->readonly) ? 'R' : '-',
                            (findfile->file->hidden) ? 'H' : '-',
                            (findfile->file->archived) ? 'A' : '-',
                            findfile->file->user
                            );

                    if ((findfile->file->load != 0) || (findfile->file->exec != 0)) {
                        char tmp[32];
                        sprintf(tmp, " L=%04X E=%04X",
                                findfile->file->load,
                                findfile->file->exec
                                );
                        strcat(add, tmp);
                    }

                    char type = '?';
                    switch (findfile->file->type) {
                        case 0:
                            type = 'A';
                            break;
                        case 1:
                            type = 'P';
                            break;
                        case 2:
                            type = 'B';
                            break;
                    }

                    console_print("%-12s   %7d %c %s\n", findfile->file->filename, findfile->file->size, type, add);

                }
                findfile = kFindnext(findfile);
            } while (findfile != NULL);
        }
    }


//    3 -> 5 // 3
//    7 -> 1 // 7
//    8 -> 8 // 0
//    9 -> 7 // 2
//   13 -> 3 // 5

    console_print("\n");

    CW.colorBack = oldColorBack;
    CW.colorFront = oldColorFront;

    return 1;
}         /* dirBinding */

/// @brief
/// @param s
/// @return 1: file not found
int fk_run(const char *s)
{
    char filename[PATH_MAX];

    fark_openfile_t *file = fk_readFile(fgb.fs, s, "lua");

    if (file == NULL) {
        printf("File not found: %s\n", s);
        console_print("%s: ", s);
        return 1;
    }

    strcpy(filename, fgb.fs->currentDir);
    path2Abs(filename, s);

    fgb.runCmd = malloc(file->bufsize + 1);
    memcpy(fgb.runCmd, file->buf, file->bufsize);
    fgb.runCmd[file->bufsize] = 0;

    kFS_closeFile(file);

    if (fgb.filenameRunCmd != NULL) {
        free(fgb.filenameRunCmd);
        fgb.filenameRunCmd = NULL;
    }
    fgb.filenameRunCmd = (char *)malloc(strlen(filename) + 1);
    strcpy(fgb.filenameRunCmd, filename);

    // K64 ?

    return 0;
}     /* fk_run */

int fk_runBinding(lua_State *L)
{
    const char *s;
    int retour;
    size_t len;

    s = lua_tolstring(L, -1, &len);
    if (!s) return 0;

    // printf("Open file: %s\n", s);

    // if (fgb.fs->type == 2) {
    //     exec_amstrad(fgb.fs->archive_buf, fgb.fs->archive_buf_size, s);
    //     return 0;
    // }

    retour = fk_run(s);

    if (retour == 1) {
        luaG_runerror(L, "File not found!");
        return 1;
    }
    return 0;

}         /* runBinding */

// ------

void kReadfolder_init(fark_fs_t *fs)
{
    int i;

    for (i = 0; i < fs->fileCount; i++) {
        free(fs->file[i]);
        fs->file[i] = NULL;
    }

    fs->fileCount = 0;

    // printf("filecount 0\n");
}

/// @brief Only used by readFolder functions to add file to fs
void kFS_addtoFolder(fark_fs_t *fs, fark_file_t *file)
{
    if ((file->filename[0] == 0) & (file->fullpath[0] == 0)) {
        return;
    }

    // Extract filename from fullpath
    if (file->filename[0] == 0) {
        if (file->fullpath[strlen(file->fullpath) - 1] == DEFSLASH) {
            file->fullpath[strlen(file->fullpath) - 1] = 0;
        }

        char *lastslash = strrchr(file->fullpath, DEFSLASH);
        if (lastslash != NULL) {
            strcpy(file->filename, lastslash + 1);
        } else {
            strcpy(file->filename, file->fullpath);
        }
    }

    // printf("[%s]\n", file->filename);

    if (fs->fileCount >= fs->fileCountMax) {
        fs->fileCountMax += 1000;
        fs->file = (fark_file_t **)realloc(fs->file, fs->fileCountMax * sizeof(fark_file_t *));
    }

    fs->file[fs->fileCount] = file;
    fs->fileCount++;

}         /* kFS_addtoFolder */

/// @brief Return file extension of filename
/// @param path Filename
/// @return file extension of filename or null if not found
char * file_extension(const char *path)
{
    char *ext = strrchr(path, '.');

    return (ext != NULL) ? (ext + 1) : NULL;
}

// TODO: check if the destination dir exists
void fk_changeDir(fark_fs_t *fs, const char *dir)
{
    char old[PATH_MAX];

    strcpy(old, fs->currentDir);

    path2Abs(fs->currentDir, dir);

    if (fs->archivePath[0] != 0) {
        if (!strncmp(fs->currentDir, fs->archivePath, strlen(fs->archivePath))) {
            // ok
        } else {
            fs->allIsLoaded = 0;
        }
    }

    if (!fs->allIsLoaded) {
        fk_readfolder(fs);
    }

    fark_findfile_t *findfile = kFindfirst(fs, NULL);

    if (findfile != NULL) {
        kFindclose(findfile);
    } else {
        strcpy(fs->currentDir, old);

        if (!fs->allIsLoaded) {
            fk_readfolder(fs);
        }
    }
}         /* kFS_changeDir */

fark_openfile_t * fk_readFile(fark_fs_t *fs, const char *filename0, const char *extension)
{
    if (fs->type == 0) {
        char filename[PATH_MAX];

        strcpy(filename, fs->currentDir);
        path2Abs(filename, filename0);

        // printf("Try to open %s\n", filename);

        FILE *f = os_fopen(filename, "rb");
        if (f == NULL) {
            strcat(filename, ".");
            strcat(filename, extension);

            f = os_fopen(filename, "rb");
            if (f == NULL) {
                return NULL;
            }
        }

        fark_openfile_t *file = (fark_openfile_t *)malloc(sizeof(fark_openfile_t));

        fseek(f, 0, SEEK_END);
        file->bufsize = ftell(f);
        fseek(f, 0, SEEK_SET);         /* same as rewind(f); */

        file->buf = malloc(file->bufsize);
        fread(file->buf, 1, file->bufsize, f);
        fclose(f);

        return file;
    }

    if (fs->type == 1) {
        char filename[PATH_MAX];

        mz_zip_archive *pZip;

        pZip = malloc(sizeof(mz_zip_archive));
        memset(pZip, 0, sizeof(mz_zip_archive));

        mz_bool status = mz_zip_reader_init_mem(pZip, fs->archive_buf, fs->archive_buf_size, 0);
        if (status == 0) {
            return NULL;
        }
        strcpy(filename, fs->currentDir);
        path2Abs(filename, filename0);

        mz_uint flags = 0;
        mz_uint32 file_index;
        if (!mz_zip_reader_locate_file_v2(pZip, filename + 1, NULL, flags, &file_index)) {
            strcat(filename, ".");
            strcat(filename, extension);
            if (!mz_zip_reader_locate_file_v2(pZip, filename + 1, NULL, flags, &file_index)) {
                return NULL;
            }
        }
        mz_zip_archive_file_stat stat;

        if (!mz_zip_reader_file_stat(pZip, file_index, &stat)) {
            return NULL;
        }

        fark_openfile_t *file = (fark_openfile_t *)malloc(sizeof(fark_openfile_t));
        file->bufsize = stat.m_uncomp_size;
        file->buf = malloc(file->bufsize);

        mz_zip_reader_extract_to_mem_no_alloc(pZip, file_index, file->buf, file->bufsize, flags, NULL, 0);

        return file;
    }

    if (fs->type == 2) {
        int j;
        for (  j = 0; j < 64; j++ ) {
            idsk_StDirEntry Dir;
            memcpy(&Dir, idsk_getInfoDirEntry((u8 *)fs->archive_buf, j), sizeof( idsk_StDirEntry ));

            if ((Dir.User != USER_DELETED) & (Dir.NumPage == 0)) {
                int k;
                char filename[PATH_MAX];

                char nom[8 + 1 + 5];
                char ext[3 + 1];
                char *space;

                memcpy(nom, Dir.Nom, 8);
                nom[8] = 0;
                space = strchr(nom, ' ');
                if (space != NULL) {
                    *space = 0;
                }
                memcpy(ext, Dir.Ext, 3);
                for (k = 0; k < 3; k++) {
                    ext[k] = ext[k] & 0x7F;
                }
                ext[3] = 0;
                space = strchr(ext, ' ');
                if (space != NULL) {
                    *space = 0;
                }

                strcpy(filename, nom);

                if (ext[0] != 0) {
                    strcat(filename, ".");
                    strcat(filename, ext);
                }

                if (Dir.User != 0) {
                    sprintf(nom, ":%d", Dir.User);
                    strcat(filename, nom);
                }

                int n = 0;
                while (filename[n] != 0) {
                    if ((filename[n] >= 'A') & (filename[n] <= 'Z')) {
                        filename[n] = filename[n] + 'a' - 'A';
                    }
                    n++;
                }

                // printf("(%s)(%s)\n", filename, Dir.Nom);

                if (!strcmp(filename, filename0)) {
                    fark_openfile_t *file = (fark_openfile_t *)malloc(sizeof(fark_openfile_t));

                    file->bufsize = Dir.NbPages * 128;
                    file->buf = (char *)malloc(file->bufsize);

                    idsk_onViewFic((u8 *)fs->archive_buf, j,  (u8 *)file->buf,  file->bufsize);

                    // printf("Size: %ld\n", file->bufsize);


                    return file;
                }

            }

        }

        return NULL;
    }

    return NULL;
}     /* fk_readFile */

void kFS_closeFile(fark_openfile_t *file)
{
    free(file->buf);
    free(file);
}


void kReadfolder_local(fark_fs_t *fs)
{
    DIR *d;
    char directory[PATH_MAX];

    kReadfolder_init(fs);

    strcpy(directory, fs->currentDir);

    d = opendir(directory);
    if (d == NULL) {
        return;
    }

    do {
        char filename[PATH_MAX];
        struct dirent *dir;
        struct stat s;

        dir = readdir(d);
        if (dir == NULL) {
            break;
        }

        strcpy(filename, directory);
        path2Abs(filename, dir->d_name);
        stat(filename, &s);

        fark_file_t *file = (fark_file_t *)malloc(sizeof(fark_file_t));
        file->isDir = (((s.st_mode) & S_IFMT) == S_IFDIR);
        strcpy(file->filename, dir->d_name);

        kFS_addtoFolder(fs, file);
    } while (1);
}         /* kReadfolder_local */

void kFS_openDskMemory(fark_fs_t *fs, const char *mem, int memSize)
{
    fs->type = 2;
    fs->allIsLoaded = 1;
    fs->isAbsolute = 1;

    if (fs->archive_buf != NULL) {
        free(fs->archive_buf);
    }
    fs->archive_buf_size = memSize;
    fs->archive_buf = (char *)malloc(memSize);
    memcpy(fs->archive_buf, mem, memSize);

//    path2abs(fs->currentDir, "/");

    kReadfolder_dsk(fs, fs->archive_buf, fs->archive_buf_size);
}

void kFS_openZipMemory(fark_fs_t *fs, const char *mem, int memSize)
{
    fs->type = 1;
    fs->allIsLoaded = 1;
    fs->isAbsolute = 1;

    if (fs->archive_buf != NULL) {
        free(fs->archive_buf);
    }
    fs->archive_buf_size = memSize;
    fs->archive_buf = (char *)malloc(memSize);
    memcpy(fs->archive_buf, mem, memSize);

//    strcpy(fs->currentDir, "/");

    kReadfolder_zip(fs, fs->archive_buf, fs->archive_buf_size);
}

void fk_readfolder(fark_fs_t *fs)
{
    fs->type = 0;
    fs->allIsLoaded = 0;
    fs->isAbsolute = 0;

    char *ext = file_extension(fs->currentDir);

    if ((ext != NULL) && (!strcasecmp(ext, "zip"))) {
        fark_openfile_t *file = fk_readFile(fs, "", "");
        if (file != NULL) {
            kFS_openZipMemory(fs, file->buf, (int)file->bufsize);
        }
        return;
    }

    if ((ext != NULL) && (!strcasecmp(file_extension(fs->currentDir), "dsk"))) {
        fark_openfile_t *file = fk_readFile(fs, "", "");
        if (file != NULL) {
            kFS_openDskMemory(fs, file->buf, (int)file->bufsize);
        }
        return;
    }

    kReadfolder_local(fs);
}              /* kReadfolder */


void kReadfolder_zip(fark_fs_t *fs, const char *mem, int memSize)
{
    int i;
    mz_zip_archive *pZip;

    strcpy(fs->archivePath, fs->currentDir);

    kReadfolder_init(fs);

    pZip = malloc(sizeof(mz_zip_archive));
    memset(pZip, 0, sizeof(mz_zip_archive));

    mz_bool status = mz_zip_reader_init_mem(pZip, mem, memSize, 0);

    if (!status) {
        fprintf(stderr, "mz_zip_reader_init_mem() failed.\n");
        mz_zip_reader_end(pZip);
        free(pZip);

        return;
    }

    for (i = 0; i < (int)mz_zip_reader_get_num_files(pZip); i++) {
        char filename[PATH_MAX];

        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(pZip, i, &file_stat)) {
            mz_zip_reader_end(pZip);
            free(pZip);

            return;
        }

        fark_file_t *file = (fark_file_t *)malloc(sizeof(fark_file_t));
        memset(file, 0, sizeof(fark_file_t));

        file->isDir = mz_zip_reader_is_file_a_directory(pZip, i);

        if (file_stat.m_filename[0] != '/') {
            strcpy(filename, fs->currentDir);
            path2Abs(filename, file_stat.m_filename);
        }

        strcpy(file->fullpath, filename);

//        printf("Filename: \"%s\", Comment: \"%s\", Uncompressed size: %u, Compressed size: %u, Is Dir: %u\n", file_stat.m_filename, file_stat.m_comment, (uint)file_stat.m_uncomp_size, (uint)file_stat.m_comp_size, mz_zip_reader_is_file_a_directory(pZip, i));

        kFS_addtoFolder(fs, file);
    }

    mz_zip_reader_end(pZip);
    free(pZip);
}     /* kReadfolder_zip */

void kReadfolder_dsk(fark_fs_t *fs, const char *mem, int memSize)
{
    int i;

    strcpy(fs->archivePath, fs->currentDir);

    kReadfolder_init(fs);

    // WHY ?

    // fark_file_t *file = (fark_file_t *)malloc(sizeof(fark_file_t));

    // memset(file, 0, sizeof(fark_file_t));

    // file->isDir = 1;
    // strcpy(file->fullpath, fs->currentDir);

    // kFS_addtoFolder(fs, file);

    for (i = 0; i < 64; i++) {
        idsk_StDirEntry *Dir = idsk_getInfoDirEntry((unsigned char *)mem, i);
        if ((Dir->User != USER_DELETED) & (Dir->NumPage == 0)) {
            int j;
            char filename[PATH_MAX];

            char nom[8 + 1];
            char ext[3 + 1];
            char *space;

            memcpy(nom, Dir->Nom, 8);
            nom[8] = 0;
            space = strchr(nom, ' ');
            if (space != NULL) {
                *space = 0;
            }

            memcpy(ext, Dir->Ext, 3);
            for (j = 0; j < 3; j++) {
                ext[j] = ext[j] & 0x7F;
            }
            ext[3] = 0;
            space = strchr(ext, ' ');
            if (space != NULL) {
                *space = 0;
            }

            strcpy(filename, fs->currentDir);
            path2Abs(filename, nom);

            if (ext[0] != 0) {
                strcat(filename, ".");
                strcat(filename, ext);
            }

            if (Dir->User != 0) {
                sprintf(nom, ":%d", Dir->User);
                strcat(filename, nom);
            }

            int n = (int)strlen(fs->currentDir);
            while (filename[n] != 0) {
                if ((filename[n] >= 'A') & (filename[n] <= 'Z')) {
                    filename[n] = filename[n] + 'a' - 'A';
                }
                n++;
            }

            fark_file_t *file = (fark_file_t *)malloc(sizeof(fark_file_t));
            memset(file, 0, sizeof(fark_file_t));

            file->readonly = ((Dir->Ext[0] & 0x80) == 0x80);
            file->hidden = ((Dir->Ext[1] & 0x80) == 0x80);
            file->archived = ((Dir->Ext[2] & 0x80) == 0x80);
            file->user = Dir->User;

            {
                u8 FirstBlock = 1;

                file->size = 0;
                file->load = 0;
                file->exec = 0;

                // u8 haveAmsdos = 0;
                file->type = 0;

                int l = ( Dir->NbPages + 7 ) >> 3;
                for ( int j = 0; j < l; j++ ) {
                    // int TailleBloc = 1024;
                    unsigned char *p = idsk_readBloc((unsigned char *)mem, Dir->Blocks[j]);
                    if ( FirstBlock ) {
                        if ( idsk_checkAmsdos(p) ) {
                            file->type = p[ 0x12 ];

                            file->size = p[ 0x18 + 1 ] * 256 + p[ 0x18 ];
                            file->load = p[ 0x15 + 1 ] * 256 + p[ 0x15 ];
                            file->exec = p[ 0x1a + 1 ] * 256 + p[ 0x1a ];

                            // haveAmsdos = 1;

                            // TailleBloc -= sizeof( idsk_StAmsdos );
                            // memcpy(p, &p[ 0x80 ], TailleBloc);
                        } else {
                            file->size = Dir->NbPages * 128;
                        }
                        FirstBlock = 0;
                    }
                }

            }

            file->isDir = 0;
            strcpy(file->fullpath, filename);

            kFS_addtoFolder(fs, file);
        }
    }
}     /* kReadfolder_dsk */




int fk_collapse_dotdot(char *const path)
{
    char *p;     /* string copy input */
    char *out;     /* string copy output */
    unsigned int i = 0;

    /* Fail if not passed an absolute path */
    if (*path != '/') return -1;

    p = path; out = path;

    while (*p != '\0') {
        /* Abort if we're too close to the end of the buffer */
        if (i >= (PATH_MAX - 3)) return -2;

        /* Skip repeated slashes */
        while (*p == '/' && *(p + 1) == '/') {
            p++; i++;
        }

        /* Scan for '/./', '/..', '/.\0' combinations */
        if (*p == '/' && *(p + 1) == '.'
            && (*(p + 2) == '.' || *(p + 2) == '/' || *(p + 2) == '\0')) {
            /* Check for '../' or terminal '..' */
            if (*(p + 2) == '.' && (*(p + 3) == '/' || *(p + 3) == '\0')) {
                /* Found a dot-dot; pull everything back to the previous directory */
                p += 3; i += 3;
                /* If already at root, skip over the dot-dot */
                if (i == 0) continue;
                /* Don't seek back past the first character */
                if ((unsigned long)out == (unsigned long)path) continue;
                out--;
                while (*out != '/') out--;
                if (*p == '\0') break;
                continue;
            } else if (*(p + 2) == '/' || *(p + 2) == '\0') {
                /* Found a single dot; seek input ptr past it */
                p += 2; i += 2;
                if (*p == '\0') break;
                continue;
            }
            /* Fall through: not a dot or dot-dot, just a slash */
        }

        /* Copy all remaining text */
        *out = *p;
        p++; out++; i++;
    }

    /* If only a root slash remains, be sure to keep it */
    if ((unsigned long)out == (unsigned long)path) {
        *out = '/';
        out++;
    }

    /* Output must always be terminated properly */
    *out = '\0';

    return 0;
}         /* collapse_dotdot */

void path2Abs(char *p, const char *relatif)
{
    u16 drive = (p[1] == ':') ? 2 : 0;

    if (p[drive] != DEFSLASH) {
        printf("Error !!!!");
    }

    if (relatif[0] == DEFSLASH) {
        strcpy(p + drive, relatif);
    } else {
        if (p[strlen(p) - 1] != DEFSLASH) {
            int pos = (int)strlen(p);
            p[pos] = DEFSLASH;
            p[pos + 1] = 0;
        }

        strcat(p + drive, relatif);
    }

    fk_collapse_dotdot(p + drive);

    if (strlen(p) > 1) {     // Remove trailing slash (if not the first)
        if (p[strlen(p) - 1] == DEFSLASH) {
            p[strlen(p) - 1] = 0;
        }
    }

}                        /* apps_disk_tpath2Abs */

/// @brief
/// @param
/// @return
fark_fs_t * fk_createFSSession(void)
{
    fark_fs_t *s = (fark_fs_t *)malloc(sizeof(fark_fs_t));

    memset(s, 0, sizeof(fark_fs_t));

    strcpy(s->homeDir, fgb.core->file_dir);
    strcpy(s->currentDir, s->homeDir);

    s->fileCountMax = 5;     // Will be allocted by first read
    s->file = (fark_file_t **)malloc(s->fileCountMax * sizeof(fark_file_t *));

    fk_readfolder(s);

    return s;
}                        /* createFSSession */

// ---- core

/// @brief
/// @return Return the UNIX time in milliseconds
static long long mstime(void)
{
    return guestGetMilliSeconds() / 1000;
}

void fps_reset(fark_session_t *s)
{
    s->epoch = 0;
    s->start_ms = mstime();
    s->paused_ms = 0;

    s->epoch5s = 0;
    s->start_ms5s = mstime();
    s->paused_ms5s = 0;
}

void fps_increase(fark_session_t *s)
{
    s->epoch++;
    s->epoch5s++;

    if (s->epoch5s == 100) {
        s->epoch = s->epoch5s;
        s->start_ms = s->start_ms5s;
        s->paused_ms = s->paused_ms5s;

        s->epoch5s = 0;
        s->start_ms5s = mstime();
        s->paused_ms5s = 0;
    }
}

void showFPS(fark_session_t *s)
{
    int elapsed_ms = (int)(mstime() - s->start_ms);

    if (!elapsed_ms) return;

    if ((float)(s->epoch * 1000) / elapsed_ms <= 999) {
        printf("FPS:%3d AVG:%3d%%\n",
               (int)((s->epoch * 1000) / elapsed_ms),
               100 - (u32)((s->paused_ms * 100) / elapsed_ms)
               );

    } else {
        printf("error");
    }
}

// -------

void l_pushtableint(lua_State *L, int key, char *value)
{
    lua_pushnumber(L, key);
    lua_pushstring(L, value);
    lua_settable(L, -3);
}

void setNumber(lua_State *L, char *name, lua_Number n)
{
    lua_pushnumber(L, n);
    lua_setglobal(L, name);
}

void setTableField(lua_State *L, char *name, char *field)
{
    lua_getglobal(L, name);         /* Stack: val table */
    /* Create the table if needed */
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);              /* Stack: val */
        lua_newtable(L);            /* Stack: val table */
        lua_setglobal(L, name);     /* Stack: val */
        lua_getglobal(L, name);     /* Stack: val table */
    }
    /* Set the field */
    if (lua_istable(L, -1)) {
        lua_pushstring(L, field);     /* Stack: val table field */
        lua_pushvalue(L, -3);       /* Stack: val table field val */
        lua_settable(L, -3);        /* Stack: val table */
    }
    lua_pop(L, 2);                  /* Stack: (empty) */
}

void setTableFieldNumber(lua_State *L, char *name, char *field, lua_Number n)
{
    lua_pushnumber(L, n);
    setTableField(L, name, field);
}

void setTableFieldString(lua_State *L, char *name, char *field, char *s)
{
//    printf("setTableFieldString %s['%s'] = '%s'\n", name, field,s);

    lua_pushstring(L, s);
    setTableField(L, name, field);
}

void lua_deleteVariable(lua_State *L, char *field)
{
    lua_pushnil(L);
    lua_setglobal(L, field);
}

lua_State * resetProgram(lua_State *previousState)
{
    lua_State *program;

    char *initscript =
        "keyboard={}; keyboard['pressed']={}; keyboard['pressedcode']={};"
        "mouse={}; mouse['pressed']={};"
        "module={};";

    fgb.s->epoch = 0;
    if (previousState) {
        lua_close(previousState);
    }

    program = luaL_newstate();
    luaL_openlibs(program);
//    luaopen_base(program);
//    luaopen_table(program);
//    luaopen_string(program);
//    luaopen_math(program);
//    luaopen_debug(program);
//    luaopen_os(program);
//    luaopen_io(program);

    install_lib_sprite(program);
    install_lib_fs(program);
    install_lib_zik(program);
    install_lib_cpc(program);
    install_lib_fark(program);

    setNumber(program, "WIDTH", 640);
    setNumber(program, "HEIGHT", 360);
    luaL_loadbuffer(program, initscript, strlen(initscript), "initscript");
    lua_pcall(program, 0, 0, 0);

    /* Make sure that mouse parameters make sense even before the first
     * mouse event captured by SDL */
    setTableFieldNumber(program, "mouse", "x", 0);
    setTableFieldNumber(program, "mouse", "y", 0);
    setTableFieldNumber(program, "mouse", "xrel", 0);
    setTableFieldNumber(program, "mouse", "yrel", 0);

    /* Register API */
//    lua_pushcfunction(fgb.s->L,fillBinding);
//    lua_setglobal(fgb.s->L,"fill");
//    lua_pushcfunction(fgb.s->L,rectBinding);
//    lua_setglobal(fgb.s->L,"rect");
//    lua_pushcfunction(fgb.s->L,ellipseBinding);
//    lua_setglobal(fgb.s->L,"ellipse");
//    lua_pushcfunction(fgb.s->L,backgroundBinding);
//    lua_setglobal(fgb.s->L,"background");
//    lua_pushcfunction(fgb.s->L,triangleBinding);
//    lua_setglobal(fgb.s->L,"triangle");
//    lua_pushcfunction(fgb.s->L,lineBinding);
//    lua_setglobal(fgb.s->L,"line");
    lua_pushcfunction(program, exitBinding);
    lua_setglobal(program, "exit");
    // lua_pushcfunction(program, menuBinding);
    // lua_setglobal(program, "menu");
    // lua_pushcfunction(program, textBinding);
    // lua_setglobal(program, "text");
    lua_pushcfunction(program, rectBinding);
    lua_setglobal(program, "rect");
    // lua_pushcfunction(program, psetBinding);
    // lua_setglobal(program, "pset");
    lua_pushcfunction(program, fk_runBinding);
    lua_setglobal(program, "run");

    lua_pushcfunction(program, dirBinding);
    lua_setglobal(program, "dir");
    lua_pushcfunction(program, cdBinding);
    lua_setglobal(program, "cd");


    lua_pushcfunction(program, edBinding);
    lua_setglobal(program, "ed");

    lua_pushcfunction(program, pngloadBinding);
    lua_setglobal(program, "pngload");
    lua_pushcfunction(program, btnBinding);
    lua_setglobal(program, "btn");
    lua_pushcfunction(program, btnpBinding);
    lua_setglobal(program, "btnp");
    lua_pushcfunction(program, clsBinding);
    lua_setglobal(program, "cls");
    lua_pushcfunction(program, windowBinding);
    lua_setglobal(program, "window");
    lua_pushcfunction(program, locateBinding);
    lua_setglobal(program, "locate");
    lua_pushcfunction(program, catBinding);
    lua_setglobal(program, "cat");
    lua_pushcfunction(program, hexBinding);
    lua_setglobal(program, "hex");
    lua_pushcfunction(program, diffBinding);
    lua_setglobal(program, "diff");

    lua_pushcfunction(program, switchmainfsBinding);
    lua_setglobal(program, "switchmainfs");
    lua_pushcfunction(program, switchapplifsBinding);
    lua_setglobal(program, "switchapplifs");

    //    lua_pushcfunction(fgb.s->L,setFPSBinding);
//    lua_setglobal(fgb.s->L,"setFPS");
//    lua_pushcfunction(fgb.s->L,getpixelBinding);
//    lua_setglobal(fgb.s->L,"getpixel");
//    lua_pushcfunction(fgb.s->L,spriteBinding);
//    lua_setglobal(fgb.s->L,"sprite");

//    initSpriteEngine(fgb.s->L);

    /* Start with a black screen */
//    fillBackground(fgb.fb,0,0,0);

    return program;
}         /* resetProgram */

static void pastefromguest(char *string);
static int console_keypressed(void *object, USBHID_id usbhid_code, char pressed);

void console_loop(void)
{

// Wait things

    fps_increase(fgb.s);

    // showFPS(fgb.s);

    long ticks = guestGetMilliSeconds();
    long nextTicks = fgb.lastTicks + (1000000 / fgb.fps);

    if (ticks < nextTicks) {
        fgb.s->paused_ms += (u32)((nextTicks - ticks) / 1000);
        fgb.s->paused_ms5s += (u32)((nextTicks - ticks) / 1000);

        guestSleep((u32)((nextTicks - ticks) / 1000));
    }

    fgb.lastTicks = guestGetMilliSeconds();

    sendSound(fgb.core);

    guestButtons(fgb.core, console_keypressed, NULL, pastefromguest);
    guestScreenDraw(fgb.core);

// key repeat

    fgb.btnMask = 0;

    int j;

    for (j = 0; j < KEY_MAX; j++) {
        u8 flag = 0;

        if (fgb.key[j].counter == 1) {
            flag = core_kbd_pressed;
        } else if (pressed_or_repeated(&fgb, fgb.key[j].counter)) {
            flag = core_kbd_pressed | core_kbd_repeat;
        }

        if (flag != 0) {
            // printf("keypress: %d %d %d\n", j, fgb.key[j].scancode, fgb.key[j].counter);
            fgb.lastevent = (time_t)time(NULL);
            fgb.key[j].counter++;     // auto repeat counter

            fk_sdl_pressed(&fgb, fgb.key[j].scancode, core_kbd_pressed, NULL);

            // fct(fgb.core, fgb.key[j].scancode, flag, object);
        } else {
            if (fgb.key[j].counter != 0) {
                fgb.key[j].counter++;
            }
        }
    }
} /* console_loop */

static int processSdlEvents(fark_session_t *s)
{
    char err = 0;
    lua_State *L = s->L;

    // guest_InputGetState(&fgb, run_keypressed, NULL);

    err = draw(L);

    if (!err) {
        err = update(L);
    }


    return err || fgb.luaerr;
}


void console_lua_exit(void)
{
    clearKeyboard();

    if (fgb.cmdline[0] != 0) {
        console_print("Ready\n");
    }

    fgb.cmdline[0] = 0;
    fgb.cmdlinePos = 0;
    fgb.cmdLineAutoIndentPos = 8193;

    displayBeauty();
}


/// @brief Execute .lua application
/// @param fgb link to fark
/// @param buf
/// @param filename
void fk_exec_program(char *buf, char *filename)   // lua_init
{
    // printf("fk_exec_program\n");

    lua_State *l = fgb.s->L;                // Keep the current state

    fgb.appli_fs = fk_createFSSession();
    fgb.fs = fgb.appli_fs;

    if (fgb.main_fs->type == 0) {           // Local
        char folder[PATH_MAX];
        strcpy(folder, fgb.main_fs->currentDir);
        path2Abs(folder, filename);
        path2Abs(folder, "..");
        strcpy(fgb.fs->currentDir, folder);     // ../Dropbox/Sources/sdl2/lua/root/modplayer/modplayer.lua

        fk_readfolder(fgb.fs);
    }

    int buflen = (int)strlen(buf);

    if (luaL_loadbuffer(l, buf, buflen, filename)) {
        programError(lua_tostring(l, -1));
        return;
    }

    if (lua_pcall(l, 0, 0, 0)) {
        programError(lua_tostring(l, -1));
        return;
    }

    if (fgb.luaerr) {
        console_lua_exit();
        return;
    }

    init(l);

    fgb.runningApplication = 1;

    fps_reset(fgb.s);

    // printf("begin loop\n");

} /* fk_exec_program */


void console_lua_loop(void)
{
    // console_loop();

    if (processSdlEvents(fgb.s)) {
        luaapp_exit(fgb.s->L);

        // Delete _exit, _init, _update & _draw function

        lua_deleteVariable(fgb.s->L, "_exit");
        lua_deleteVariable(fgb.s->L, "_init");
        lua_deleteVariable(fgb.s->L, "_update");
        lua_deleteVariable(fgb.s->L, "_draw");

        fgb.runningApplication = 0;

        fgb.fs = fgb.main_fs;

        // printf("end loop\n");

        console_lua_exit();
    }

} /* console_lua_loop */

void programError(const char *e)
{
    // printf("programError\n");

    console_print( (char *)e);
    console_print("\n");

    fgb.luaerr = 1;
}

/* Load the editor program into Lua. Returns 0 on success, 1 on error. */
int loadProgram(char *buf)
{
    int buflen = (int)strlen(buf);

    if (luaL_loadbuffer(fgb.s->L, buf, buflen, "")) {

        // Command not found: try command without the parentheses

        lua_getglobal(fgb.s->L, buf);
        if (lua_pcall(fgb.s->L, 0, 0, 0) == 0) {
            return 0;
        }

        programError(lua_tostring(fgb.s->L, -1));
        return 1;
    }

    if (lua_pcall(fgb.s->L, 0, 0, 0)) {
        programError(lua_tostring(fgb.s->L, -1));
        return 1;
    }


//    fgb.luaerr = 0;
//    editorClearError();
    return 0;
}         /* loadProgram */

void exec_lua(u8 *str)
{
    // printf("exec_lua\n");

    fgb.luaerr = 0;

    loadProgram( (char *)str);

    if (fgb.runCmd != NULL) {      // Fill by the run cmd lua
        printf("Runcmd: %s\n", (fgb.runCmd == NULL) ? "NULL" : fgb.runCmd);
        fk_exec_program( (char *)fgb.runCmd, (fgb.filenameRunCmd == NULL) ? "program.lua" : fgb.filenameRunCmd);

        free(fgb.runCmd);
        fgb.runCmd = NULL;

        return;
    }

    console_lua_exit();
}

void fk_displaycursor(u16 xp, u16 yp, u8 display)
{
    u16 back, front;

    if (display) {
        back = fgb.textColorFront[xp / 8 + (yp / 8) * CW.width];
        front = fgb.textColorBack[xp / 8 + (yp / 8) * CW.width];
    } else {
        back = fgb.textColorBack[xp / 8 + (yp / 8) * CW.width];
        front = fgb.textColorFront[xp / 8 + (yp / 8) * CW.width];
    }

    u16 c = fgb.text[xp / 8 + (yp / 8) * CW.width];

    if (c == 0) {
        c = 32;
    }

    u16 x, y;
    const u8 *bitmap = cpc6128_bin + 0x3800 + c * 8;
    u16 *dest =  fgb.MemBitmap;

    for (y = 0; y < 8; y++) {
        u8 byte = *bitmap;

        for (x = 0; x < 8; x++) {
            *(dest + (yp + y) * 384 * 2 + (xp + x) ) = ((byte & 128) == 128) ? front : back;
            byte = (byte << 1);
        }
        bitmap++;
    }
}     /* fk_displaycursor */

void putOnText(u16 xp, u16 yp, u8 c, u16 back, u16 front)
{
    fgb.text[xp + yp * CW.width] = c;
    fgb.textColorBack[xp + yp * CW.width] = back;
    fgb.textColorFront[xp + yp * CW.width] = front;

    fk_displaychar(xp * 8, yp * 8, c, back, front);

}

void fk_displaychar(u16 xp, u16 yp, u8 c, u16 back, u16 front)
{
    if (c == 0) {
        fk_displaycursor(xp, yp, 0);     // Erase
        return;
    }


//    if ((c < 32) || (c > 127)) {
//        guest_displaycursor(xp, yp, CW.colorBack);
//        return;
//    }


    u16 x, y;
    const u8 *bitmap = cpc6128_bin + 0x3800 + c * 8;
    u16 *dest =  fgb.MemBitmap;

    for (y = 0; y < 8; y++) {
        u8 byte = *bitmap;

        for (x = 0; x < 8; x++) {
            *(dest + (yp + y) * 384 * 2 + (xp + x) ) = ((byte & 128) == 128) ? front : back;
            byte = (byte << 1);
        }
        bitmap++;
    }
}         /* guest_displaychar */

void newLine(void)
{
    CW.y++;
    CW.x = 0;
    if (CW.y == CW.height) {
        CW.y = CW.height - 1;

        u16 x, y;
        for (y = CW.fromY + 1; y <  CW.fromY + CW.height; y++) {
            for (x = CW.fromX; x < CW.fromX + CW.width; x++) {
                fgb.text[x + (y - 1) * CW.width] = fgb.text[x + y * CW.width];
                fgb.textColorBack[x + (y - 1) * CW.width] = fgb.textColorBack[x + y * CW.width];
                fgb.textColorFront[x + (y - 1) * CW.width] = fgb.textColorFront[x + y * CW.width];
                fk_displaychar(x * 8, (y - 1) * 8, fgb.text[x + (y - 1) * CW.width], fgb.textColorBack[x + (y - 1) * CW.width], fgb.textColorFront[x + (y - 1) * CW.width]);
            }
        }
        for (x = CW.fromX; x < CW.fromX + CW.width; x++) {
            fgb.text[x + (CW.fromY + CW.y)  * CW.width] = 0;
            fgb.textColorBack[x +  (CW.fromY + CW.y)  * CW.width] = CW.colorBack;
            fgb.textColorFront[x +  (CW.fromY + CW.y)  * CW.width] = CW.colorFront;
            fk_displaychar(x * 8,  (CW.fromY + CW.y)  * 8, 0, CW.colorBack, CW.colorFront);
        }
    }
}         /* newLine */

void console_print(const char *fmt, ...)
{
    u8 str[4096];

    if (fmt == NULL) {
        printf("Invalid argument\n");
        return;
    }

    va_list args;

    va_start(args, fmt);
    vsprintf((char *)str, fmt, args);
    va_end(args);



    fk_displaycursor( (CW.fromX + CW.x) * 8, (CW.fromY + CW.y) * 8, 0);

    int n = 0;

    while (str[n] != 0) {
        if (str[n] == '\n') {
            CW.x = 0;
            newLine();
        } else {
            putOnText( (CW.fromX + CW.x),  (CW.fromY + CW.y), str[n], CW.colorBack, CW.colorFront);
            CW.x++;
            if (CW.x ==  CW.width) {
                newLine();
            }
        }
        n++;
    }

    fgb.frameCursor = 0;
}         /* console_print */

void console_print_default(int len, const char *string)
{
    ddlog(&gb, 2, "console_print_default: %d\n", len);
    if ((len == 0) || (len > 4095)) {
        return;
    }

    console_print("%s", string);
}


void console_init(core_crocods_t *core)
{
    u16 i;

    // printf("fgb:%p -init\n", &fgb);

    fgb.core = core;

    // printf("Console Init %x\n", &fgb);

    fgb.s = (fark_session_t *)malloc(sizeof(fark_session_t));
    memset(fgb.s, 0, sizeof(fark_session_t));

    fgb.fps = 50;

    fgb.s->L = resetProgram(NULL);

    fps_reset(fgb.s);

    // fgb.firstLine = 0;

    fgb.winWidth = (384 * 2) / 8;
    fgb.winHeight = (288 - 16) / 8;         // 34

    CW.x = 0;
    CW.y = 1;


    for (i = 0; i < 32; i++) {
        int r = (RgbCPCdef[ i ] >> 16) & 0xFF;
        int g = (RgbCPCdef[ i ] >> 8) & 0xFF;
        int b = (RgbCPCdef[ i ] >> 0) & 0xFF;

        fgb.palette[i] = RGB565(r, g, b);
    }

    CW.colorBack = fgb.palette[0];
    CW.colorFront = fgb.palette[10];

    // for (i = 0; i < 80 * 45; i++) {
    //     fgb.text[i] = 0;
    //     fgb.textColorBack[i] = CW.colorBack;
    //     fgb.textColorFront[i] = CW.colorFront;
    // }

    fgb.cur_win = 0;


    CW.fromX = 0;
    CW.fromY = 1;
    CW.width = fgb.winWidth;
    CW.height = fgb.winHeight - 2;
    CW.x = 0;
    CW.y = 0;

    // CW.fromX = 5;
    // CW.fromY = 5;
    // CW.width = fgb.winWidth - 10;
    // CW.height = fgb.winHeight - 12;

    fgb.inEditor = 0;

    fgb.cmdline[0] = 0;
    fgb.cmdlinePos = 0;
    fgb.cmdLineAutoIndentPos = 8193;
    // frame32 = 0;
    // posBackground = 0;

    // menu_part = MENU_RUNNING;

    fgb.main_fs = fk_createFSSession();
    fgb.fs = fgb.main_fs;
    ddlog(&gb, 2, "mainFS: %s\n", fgb.main_fs->currentDir);

    clsBinding(fgb.s->L);

    // console_print(" FaRK ONE 512M Microcomputer (%s)\n\n", CROCOVERSION);
    // console_print(" %c2020 Crazy Piri\n", 0xA4);
    // console_print("         and Lua.org, PUC-Rio\n");
    // console_print("\n");
    // console_print(" %s\n\n", LUA_RELEASE);
    // console_print("Ready\n");

    console_print(" CrocoDS console (%s)\n\n", CROCOVERSION);
    console_print(" %c2022 Crazy Piri\n", 0xA4);
    console_print("\n");
    console_print(" %s\n\n", LUA_RELEASE);
    console_print("Ready\n");

    fk_displaycursor( (CW.fromX + CW.x) * 8,  (CW.fromY + CW.y) * 8, 1);

    displayBeauty();

    if (fk_run("autoexec.lua") == 0) {
        // printf("autoexec: %s\n", (fgb.runCmd == NULL) ? "NULL" : fgb.runCmd);

        fk_exec_program((char *)fgb.runCmd, (fgb.filenameRunCmd == NULL) ? "program.lua" : fgb.filenameRunCmd);

        free(fgb.runCmd);
        fgb.runCmd = NULL;
    }

    // Init Modplayer

    hxcmod_init(&fgb.modloaded);
    hxcmod_setcfg(&fgb.modloaded, 44100, 1, 0);

    fgb.modTrackbufState.nb_max_of_state = 100;
    fgb.modTrackbufState.track_state_buf = malloc(sizeof(tracker_state) * fgb.modTrackbufState.nb_max_of_state);
    memset(fgb.modTrackbufState.track_state_buf, 0, sizeof(tracker_state) * fgb.modTrackbufState.nb_max_of_state);
    fgb.modTrackbufState.sample_step = (audio_align_samples(44100 * 20 / 1000) * 4) / fgb.modTrackbufState.nb_max_of_state;

// keyboard

    if (core->keyboardLayout == 1) {
        memcpy((void *)_asciimap, (void *)_asciimap_fr_fr, sizeof(_asciimap));
    } else {
        memcpy((void *)_asciimap, (void *)_asciimap_en_us, sizeof(_asciimap));
    }

}         /* console_int */

void consoleEscape(void)
{
    console_print("*Break*\n");

    fgb.cmdline[0] = 0;
    fgb.cmdlinePos = 0;
    fgb.cmdLineAutoIndentPos = 8193;

    shift_disable();
}

void console_delchar(void)
{
    if (CW.x > 0) {
        fk_displaycursor( (CW.fromX + CW.x) * 8,  (CW.fromY + CW.y) * 8, 0);

        CW.x--;

        putOnText( (CW.fromX + CW.x),  (CW.fromY + CW.y), 32, CW.colorBack, CW.colorFront);
    } else {
        // console_beep(fgb);
    }
}


void cmdLine_shift(char dir)
{
    if (fgb.shiftActivated) {
        fk_displaycursor( (CW.fromX + fgb.shiftX) * 8,  (CW.fromY + fgb.shiftY) * 8, 0);
    } else {
        fgb.shiftX = CW.x;
        fgb.shiftY = CW.y;
        fgb.shiftActivated = 1;
    }

    if (dir == 0) {     // LEFT
        if (fgb.shiftX != 0) {
            fgb.shiftX--;
        } else if (fgb.shiftY != 0) {
            fgb.shiftY--;
            fgb.shiftX = CW.width - 1;
        }
    } else if (dir == 1) {     // RIGHT
        if (fgb.shiftX != CW.width - 1) {
            fgb.shiftX++;
        } else if (fgb.shiftY != CW.height - 1) {
            fgb.shiftY++;
            fgb.shiftX = 0;
        }
    } else if (dir == 2) {     // UP
        if (fgb.shiftY != 0) {
            fgb.shiftY--;
        }
    } else if (dir == 3) {     // DOWN
        if (fgb.shiftY != CW.height - 1) {
            fgb.shiftY++;
        }
    } else if (dir == 4) {     // COPY
        cmdLine_insert(fgb.text[ (CW.fromX + fgb.shiftX) +  (CW.fromY + fgb.shiftY) * CW.width]);
        if (fgb.shiftX < CW.width) {
            fgb.shiftX++;
        } else if (fgb.shiftY < CW.height) {
            fgb.shiftY++;
            fgb.shiftX = 0;
        }
    }

    // Invert

    fk_displaycursor( (CW.fromX + fgb.shiftX) * 8,  (CW.fromY + fgb.shiftY) * 8, 1);
}     /* cmdLine_shift */

void shift_disable(void)
{
    if (fgb.shiftActivated) {
        fk_displaycursor( (CW.fromX + fgb.shiftX) * 8,  (CW.fromY + fgb.shiftY) * 8, 0);
        fgb.shiftActivated = 0;
    }
}

void pastefromguest(char *string)
{

// TODO: paste

    if (fgb.inEditor == 1) {
        editorPaste(string);
    } else {
        int n;

        for (n = 0; n < strlen(string); n++) {
            cmdLine_insert(string[n]);
        }
    }
} /* pastefromguest */


void displayHeader(char *str)
{
    int n;
    char buffer[96 + 1];

    if (strlen(str) > 94) {
        str[94] = 0;
    }

    sprintf(buffer, " %-94s ", str);

    u16 colorBack = RGB565(255, 32, 32);
    u16 colorFront = RGB565(255, 255, 255);

    for (n = 0; n < 96; n++) {
        putOnText(n, 0, buffer[n], colorBack, colorFront);
    }
}

void displayFooter(char *str)
{
    int n;
    char buffer[96 + 1];

    if (strlen(str) > 94) {
        str[94] = 0;
    }

    sprintf(buffer, " %-94s ", str);

    u16 colorBack = RGB565(255, 32, 32);
    u16 colorFront = RGB565(255, 255, 255);

    for (n = 0; n < 96; n++) {
        putOnText(n, fgb.winHeight - 1, buffer[n], colorBack, colorFront);
    }
}

/// @brief Insert header and footer
/// @param
void displayBeauty(void)
{
    char buffer[96 + 1];

    sprintf(buffer, "CrocoDS");

    displayFooter(buffer);
    displayHeader(buffer);

}     /* displayFooter */

void cmdLine_insert_completion_fs(char *filter, u16 from)
{
    // printf("Filter: [%s,%d]\n", filter, from);

    u16 idx = 0;
    fark_findfile_t *findfile = kFindfirst(fgb.fs, NULL);

    if (findfile != NULL) {
        do {
            if ((filter[0] == 0) || (!kFS_WildCmp(findfile->file->filename, filter))) {
                // printf("(%s)", findfile->file->filename);

                if (fgb.cmdLineAutoIndentIdx == idx) {
                    char *str = findfile->file->filename;

                    u16 n;

                    for (n = 0; n < fgb.cmdlinePos - from; n++) {
                        console_delchar();
                    }

                    for (n = 0; n < strlen(str); n++) {
                        fgb.cmdline[from + n] = str[n];
                    }

                    fgb.cmdlinePos = from + strlen(str);
                    fgb.cmdline[fgb.cmdlinePos] = 0;

                    console_print(str);
                }
                idx++;
            }

            findfile = kFindnext(findfile);

        } while (findfile != NULL);
    }

    if (fgb.cmdLineAutoIndentIdx + 1 == idx) {
        fgb.cmdLineAutoIndentIdx = -1;
    }

    // printf("\n");
} /* cmdLine_insert_completion_fs */

/// @brief Insert a character in the command line
void cmdLine_insert(char car)
{
    if (car == 0) {
        car = 32;
    }

    if (car == 9) {
        if (fgb.cmdLineAutoIndentPos == 8193) {
            fgb.cmdLineAutoIndentPos = fgb.cmdlinePos;
            fgb.cmdLineAutoIndentIdx = 0;
            strcpy((char *)fgb.cmdLineAutoIndentBefore, (char *)fgb.cmdline);
        } else {
            fgb.cmdLineAutoIndentIdx++;
            strcpy((char *)fgb.cmdline, (char *)fgb.cmdLineAutoIndentBefore);
        }

        char filter[1024];

        filter[0] = 0;
        if (fgb.cmdLineAutoIndentPos > 0) {
            u16 n = fgb.cmdLineAutoIndentPos - 1;

            fgb.cmdline[fgb.cmdLineAutoIndentPos] = 0;

            while (n > 0) {
                if (fgb.cmdline[n] == '"') {
                    strcpy(filter, (char *)(fgb.cmdline + n + 1));
                    strcat(filter, "*");

                    cmdLine_insert_completion_fs(filter, n + 1);
                    break;
                }
                n--;
            }
        }



        return;
    }

    if (car == '\n') {
        shift_disable();

        console_print("\n");

        exec_lua(fgb.cmdline);

        return;
    }

    if (car == 8) {
        fgb.cmdLineAutoIndentPos = 8193;

        console_delchar();

        if (fgb.cmdlinePos > 0) {
            fgb.cmdlinePos--;
            fgb.cmdline[fgb.cmdlinePos] = 0;
        }
        return;
    }

    fgb.cmdLineAutoIndentPos = 8193;

    fgb.cmdline[fgb.cmdlinePos] = car;
    fgb.cmdline[fgb.cmdlinePos + 1] = 0;
    fgb.cmdlinePos++;

    // printf("fgb:%p - cmdline:%s (%d)\n", fgb, fgb.cmdline, CW.x);


    char str[2];

    str[0] = car;
    str[1] = 0;
    console_print(str);
}     /* cmdLine_insert */

// Debug functions

void traceBasicLine(u16 line)
{
    if (line == 330) {
        if (fgb.inEditor) {
            apps_console_init(fgb.core, 1);
            editor_gotoBasicLine(line);
        }
    }
    printf("Goto %d\n", line);

}

// keyboaard functions

#define KEY_REPEAT_PERIOD_FAST 3
#define KEY_REPEAT_PERIOD      6
#define KEY_REPEAT_DELAY       20

char pressed_or_repeated(core_fark_t *core, int counter)
{
    int period;

//    if (counter != 0) printf("Counter: %d - ", counter);


    int delay = core->fps / 3;
    int period0 = core->fps / 10;
    int period_fast = core->fps / 20;

    if (counter > delay + (period0 * 3)) {
        period = period_fast;
    } else {
        period = period0;
    }

    // printf("pressed_or_repeated: %d %d %d\n", counter, delay, period);

    if (counter > 1 && counter < delay) {
//        if (counter != 0) printf("KEY_REPEAT_DELAY (0)\n");
        return 0;
    }
//    if (counter != 0) printf("RETURN %d\n", ((counter + period - 1) % period) == 0);
    return (((counter + period - 1) % period) == 0);
}         /* pressed_or_repeated */


static keyState * fk_editorGetKeyState(core_fark_t *core, int scancode)
{
    int free = -1;
    int j;

    for (j = 0; j < KEY_MAX; j++) {
        if (core->key[j].scancode == scancode) return &core->key[j];
        if (core->key[j].scancode == 0) free = j;
    }
    if (free == -1) return NULL;

    // printf("keypress (free): %d %d %d\n", free, fgb.key[free].scancode, fgb.key[free].counter);

    core->key[free].scancode = scancode;
    core->key[free].counter = 0;
    return &core->key[free];
}



u32 fk_sdl_pressed(core_fark_t *core, USBHID_id usbhid_code, char flag, void *object)
{
    if ((flag & core_kbd_pressed) == core_kbd_pressed) {

        u16 ctrl = (core->scancode[224] | core->scancode[228]) ? CTRL : 0;
        u16 shift = (core->scancode[225] | core->scancode[229]) ? SHIFT : 0;
        u16 option = (core->scancode[226] | core->scancode[230]) ? OPTION : 0;
        u16 ralt = core->scancode[USBHID_ID_RALT];         // SDL_SCANCODE_RALT

        u8 down = 1;

        if (fgb.runningApplication) {
            if (usbhid_code == USBHID_ID_ESCAPE) {
                console_print("Break in Poll\n");
                fgb.luaerr = 1;
            } else if (usbhid_code == USBHID_ID_LEFT) {
                fgb.btnMask = (down) ? (fgb.btnMask | 0x0001) : (fgb.btnMask & ~0x0001);
            } else if (usbhid_code == USBHID_ID_RIGHT) {
                fgb.btnMask = (down) ? (fgb.btnMask | 0x0002) : (fgb.btnMask & ~0x0002);
            } else if (usbhid_code == USBHID_ID_UP) {
                fgb.btnMask = (down) ? (fgb.btnMask | 0x0004) : (fgb.btnMask & ~0x0004);
            } else if (usbhid_code == USBHID_ID_DOWN) {
                fgb.btnMask = (down) ? (fgb.btnMask | 0x0008) : (fgb.btnMask & ~0x0008);
            } else if ((usbhid_code == USBHID_ID_Z) | (usbhid_code == USBHID_ID_C) | (usbhid_code == USBHID_ID_SPACE)) {
                fgb.btnMask = (down) ? (fgb.btnMask | 0x0010) : (fgb.btnMask & ~0x0010);
            } else if ((usbhid_code == USBHID_ID_X) | (usbhid_code == USBHID_ID_V) | (usbhid_code == USBHID_ID_M) | (usbhid_code == USBHID_ID_8)) {
                fgb.btnMask = (down) ? (fgb.btnMask | 0x0020) : (fgb.btnMask & ~0x0020);
            } else if (usbhid_code == USBHID_ID_RETURN) {
                fgb.btnMask = (down) ? (fgb.btnMask | 0x0040) : (fgb.btnMask & ~0x0040);
            } else if ((usbhid_code == USBHID_ID_Q) ||  (usbhid_code == USBHID_ID_A)) {
                fgb.btnMask = (down) ? (fgb.btnMask | 0x0080) : (fgb.btnMask & ~0x0080);
            } else if (usbhid_code == USBHID_ID_S) {
                fgb.btnMask = (down) ? (fgb.btnMask | 0x0100) : (fgb.btnMask & ~0x0100);
            } else if (usbhid_code == USBHID_ID_F) {
                fgb.btnMask = (down) ? (fgb.btnMask | 0x0200) : (fgb.btnMask & ~0x0200);
            } else if (usbhid_code == USBHID_ID_E) {
                fgb.btnMask = (down) ? (fgb.btnMask | 0x0400) : (fgb.btnMask & ~0x0400);
            } else if (usbhid_code == USBHID_ID_D) {
                fgb.btnMask = (down) ? (fgb.btnMask | 0x0800) : (fgb.btnMask & ~0x0800);
            } else if ((usbhid_code == USBHID_ID_LSHIFT) | (usbhid_code == USBHID_ID_TAB)) {
                fgb.btnMask = (down) ? (fgb.btnMask | 0x1000) : (fgb.btnMask & ~0x1000);
            } else if ((usbhid_code == USBHID_ID_A) | (usbhid_code == USBHID_ID_Q)) {
                fgb.btnMask = (down) ? (fgb.btnMask | 0x2000) : (fgb.btnMask & ~0x2000);
            }



        } else if (fgb.inEditor == 1) {
            int i;

            usbhid_code = usbhid_code | shift | option;

            if (ctrl) {
                for (i = 0; i < 256; i++) {
                    if (_asciimap[i] == usbhid_code) {
                        // printf("Pressed: %d,ctrl %c\n", i, i);
                        fgb.editorBuf[fgb.editorBufLen] = ((i) & 0x1f);
                        fgb.editorBufLen++;
                        break;
                    }
                }
            } else
            if ((usbhid_code == USBHID_ID_LEFT) && (shift == 0)) {
                fgb.editorBuf[fgb.editorBufLen] = 256;
                fgb.editorBufLen++;
            } else
            if ((usbhid_code == USBHID_ID_RIGHT) && (shift == 0)) {
                fgb.editorBuf[fgb.editorBufLen] = 257;
                fgb.editorBufLen++;
            } else
            if ((usbhid_code == USBHID_ID_UP) && (shift == 0)) {
                fgb.editorBuf[fgb.editorBufLen] = 258;
                fgb.editorBufLen++;
            } else
            if ((usbhid_code == USBHID_ID_DOWN) && (shift == 0)) {
                fgb.editorBuf[fgb.editorBufLen] = 259;
                fgb.editorBufLen++;
            } else if (usbhid_code == USBHID_ID_ESCAPE) {
                fgb.editorBuf[fgb.editorBufLen] = 27;
                fgb.editorBufLen++;
            } else {
                for (i = 0; i < 256; i++) {
                    if (_asciimap[i] == usbhid_code) {
                        // printf("Pressed: %d,%c\n", i, i);
                        fgb.editorBuf[fgb.editorBufLen] = i;
                        fgb.editorBufLen++;
                        break;
                    }
                }
            }

        } else {
            if (usbhid_code == USBHID_ID_F1) {

            } else if (ralt != 0) {
                cmdLine_shift(4);              // COPY
            } else
            if ((usbhid_code == USBHID_ID_LEFT) && (shift != 0)) {
                cmdLine_shift(0);
            } else
            if ((usbhid_code == USBHID_ID_RIGHT) && (shift != 0)) {
                cmdLine_shift(1);
            } else
            if ((usbhid_code == USBHID_ID_UP) && (shift != 0)) {
                cmdLine_shift(2);
            } else
            if ((usbhid_code == USBHID_ID_DOWN) && (shift != 0)) {
                cmdLine_shift(3);
            } else if ((usbhid_code == USBHID_ID_ESCAPE) && (shift != 0)) {
                // menuF12();
            } else if (usbhid_code == USBHID_ID_ESCAPE) {
                consoleEscape();
            } else {
                int i;

                usbhid_code = usbhid_code | shift | option;

                for (i = 0; i < 256; i++) {
                    if (_asciimap[i] == usbhid_code) {
                        cmdLine_insert(i);
                        break;
                    }
                }
            }

        }
    }
    return 0;
}                             /* console_sdl_pressed */



static int console_keypressed(void *object, USBHID_id usbhid_code, char pressed)
{
    core_fark_t *core = &fgb;     // (core_fark_t *)object;

    core->scancode[usbhid_code] = pressed;

    if (pressed) {
        keyState *ks = fk_editorGetKeyState(core, usbhid_code);
        if (ks) {
            ks->counter = 1;
        }
    } else {

        keyState *ks = fk_editorGetKeyState(core, usbhid_code);
        if (ks) {
//            printf("keypress: ? %d %d\n", ks->scancode, ks->counter);

            ks->counter = 0;
            ks->scancode = USBHID_ID_UNKNOWN;
        }
    }

    return 0;
}         /* console_keypressed */

// ------

void DispConsole(core_crocods_t *core, u16 keys_pressed0)
{
    appli_begin(core, keys_pressed0);

    core->overlayBitmap_width = 256;
    core->overlayBitmap_height = 0;
    core->overlayBitmap_posx = (320 - 256) / 2;
    core->overlayBitmap_posy = (240 - 168) / 2;
    core->overlayBitmap_center = 1;

    clearKeyboard();

    do {
        if (fgb.runningApplication == 1) {
            console_lua_loop();
        } else if (fgb.inEditor == 1) {
            console_editor_loop();
        } else if (fgb.runningApplication == 0) {
            fk_displaycursor( (CW.fromX + CW.x) * 8,  (CW.fromY + CW.y) * 8, 1);
        }

        console_loop();

    } while (core->inConsole);

    if (apps_console_prematuredEnd == 0) {
        core->inKeyboard = 0;
        apps_console_end(core);
    }

}         /* DispKeyboard */

#endif /* ifndef CLI */