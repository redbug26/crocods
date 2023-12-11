#include "apps_console_internal.h"

#include "os.h"

#include <unistd.h> // for ftruncate


// ----- Editor (based on textEd: https://github.com/gauri-singh/textEd)



#define TextEd_VERSION    "0.0.1"
#define TextEd_TAB_STOP   8
#define TextEd_QUIT_TIMES 3

#define CTRL_KEY(k) ((k) & 0x1f)

enum editorKey {
    BACKSPACE  = 127,
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN,
};

enum editorHighlight {
    HL_NORMAL = 0,
    HL_COMMENT,
    HL_MLCOMMENT,
    HL_KEYWORD1,
    HL_KEYWORD2,
    HL_STRING,
    HL_NUMBER,
    HL_MATCH,
};

#define HL_HIGHLIGHT_NUMBERS (1 << 0)
#define HL_HIGHLIGHT_STRINGS (1 << 1)
#define HL_IGNORE_CASE       (1 << 2)

/*** data ***/

struct editorSyntax {
    char *filetype;
    char **filematch;
    char **keywords;
    char *singleline_comment_start;
    char *multiline_comment_start;
    char *multiline_comment_end;
    int flags;
};

typedef struct erow {
    int idx;
    int size;
    int rsize;
    char *chars;
    char *render;
    unsigned char *hl;
    int hl_open_comment;
} erow;

struct editorConfig {
    int cx, cy;
    int rx;
    int rowoff;
    int coloff;
    int screenrows;
    int screencols;
    int numrows;
    erow *row;
    int dirty;      // modified
    char *filename;
    char statusmsg[80];
    time_t statusmsg_time;
    struct editorSyntax *syntax;
    void (*callback)(char *, int, char);
    char promptBuf[128];
    size_t promptBuflen;
    char prompt[128];
    char waitingCtrlK;
    int bx, by, kx, ky;  // Block begin, end
};

struct editorConfig E;

/*** filetypes ***/

char *C_HL_extensions[] = {".c", ".h", ".cpp", NULL};
char *C_HL_keywords[] = {
    "switch", "if", "while", "for", "break", "continue", "return", "else",
    "struct", "union", "typedef", "static", "enum", "class", "case",

    "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
    "void|", NULL
};

char *BAS_HL_extensions[] = {".bas", NULL};
char *BAS_HL_keywords[] = {
    "AFTER", "AUTO", "BORDER", "CALL", "CAT", "CHAIN", "CLEAR", "CLG",
    "CLOSEIN", "CLOSEOUT", "CLS", "CONT", "DATA", "DEF", "DEFINT",
    "DEFREAL", "DEFSTR", "DEG", "DELETE", "DIM", "DRAW", "DRAWR", "EDIT",
    "ELSE", "END", "ENT", "ENV", "ERASE", "ERROR", "EVERY", "FOR",
    "GOSUB", "GOTO", "IF", "INK", "INPUT", "KEY", "LET", "LINE", "LIST",
    "LOAD", "LOCATE", "MEMORY", "MERGE", "MID$", "MODE", "MOVE", "MOVER",
    "NEXT", "NEW", "ON", "ON BREAK", "ON ERROR GOTO", "SQ", "OPENIN",
    "OPENOUT", "ORIGIN", "OUT", "PAPER", "PEN", "PLOT", "PLOTR", "POKE",
    "PRINT", "'", "RAD", "RANDOMIZE", "READ", "RELEASE", "REM", "RENUM",
    "RESTORE", "RESUME", "RETURN", "RUN", "SAVE", "SOUND", "SPEED", "STOP",
    "SYMBOL", "TAG", "TAGOFF", "TROFF", "TRON", "WAIT", "WEND", "WHILE",
    "WIDTH", "WINDOW", "WRITE", "ZONE", "DI", "EI", "FILL", "GRAPHICS",
    "MASK", "FRAME", "CURSOR", "#E2", "ERL", "FN", "SPC", "STEP", "SWAP",
    "#E8", "#E9", "TAB", "THEN", "TO", "USING", ">", "=", ">=", "<", "<>",
    "<=", "+", "-", "*", "/", "^", "\\ ", "AND", "MOD", "OR", "XOR", "NOT",
    "#FF",
    "ABS", "ASC", "ATN", "CHR$", "CINT", "COS", "CREAL", "EXP", "FIX",
    "FRE", "INKEY", "INP", "INT", "JOY", "LEN", "LOG", "LOG10", "LOWER$",
    "PEEK", "REMAIN", "SGN", "SIN", "SPACE$", "SQ", "SQR", "STR$", "TAN",
    "UNT", "UPPER$", "VAL", "EOF", "ERR", "HIMEM", "INKEY$", "PI", "RND",
    "TIME", "XPOS", "YPOS", "DERR", "BIN$", "DEC$", "HEX$",
    "INSTR", "LEFT$", "MAX", "MIN", "POS", "RIGHT$", "ROUND", "STRING$",
    "TEST", "TESTR", "COPYCHR$", "VPOS",
    NULL
};

char *ASM_HL_extensions[] = {".s", NULL};
char *ASM_HL_keywords[] = {
    "adc", "add", "and", "bit", "call", "ccf", "cpdr", "cpd", "cpir", "cpi", "cpl", "cp", "daa", "dec", "di", "djnz",
    "ei", "exx", "ex", "halt", "im", "inc", "indr", "ind", "inir", "ini", "in", "jp", "jr", "lddr", "ldd", "ldir", "ldi",
    "ld", "neg", "nop", "or", "otdr", "otir", "outd", "outi", "out", "pop", "push", "res", "reti", "retn", "ret", "rla",
    "rlca", "rlc", "rld", "rl", "rra", "rrca", "rrc", "rrd", "rr", "rst", "sbc", "scf", "set", "sla", "sll", "sra", "srl", "sub", "xor",
    "db", "dw",
    NULL
};


struct editorSyntax HLDB[] = {
    {
        "c",
        C_HL_extensions,
        C_HL_keywords,
        "//", "/*", "*/",
        HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
    },
    {
        "bas",
        BAS_HL_extensions,
        BAS_HL_keywords,
        "rem", "", "",
        HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS | HL_IGNORE_CASE
    },
    {
        "s",
        ASM_HL_extensions,
        ASM_HL_keywords,
        ";", "", "",
        HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS | HL_IGNORE_CASE
    }
};

#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))

/*** prototypes ***/

void editorSetStatusMessage(const char *fmt, ...);
void editorRefreshScreen();

void editorPrompt(char *prompt, void (*callback)(char *, int, char));


void editorInsertChar(int c);
void editorInsertNewline(void);
void editorSave(void);

/*** terminal ***/

void editorPaste(char *string)
{
    int n;

    for (n = 0; n < strlen(string); n++) {
        switch (string[n]) {
            case '\n':
            case '\r':
                editorInsertNewline();
                break;

            default:
                editorInsertChar(string[n]);
                break;
        }
    }
}

int editorReadKey()
{
    u16 car;

    if (fgb.editorBufLen == 0) {
        return 0;
    }

    fgb.editorBufLen--;
    car = fgb.editorBuf[fgb.editorBufLen];
    // printf("editorReadKey:car:%d\n", car);

    // printf("(%d)\n", car);

    switch (car) {
        case 10:
            return 13;
        case 256: // USBHID_ID_LEFT
            return ARROW_LEFT;
        case 257: // USBHID_ID_RIGHT
            return ARROW_RIGHT;
        case 258: // USBHID_ID_UP
            return ARROW_UP;
        case 259: // USBHID_ID_DOWN
            return ARROW_DOWN;
        default:
            return car;
    }

    return 0;

    // case '1': return HOME_KEY;
    // case '3': return DEL_KEY;
    // case '4': return END_KEY;
    // case '5': return PAGE_UP;
    // case '6': return PAGE_DOWN;
    // case '7': return HOME_KEY;
    // case '8': return END_KEY;
    // case 'H': return HOME_KEY;
    // case 'F': return END_KEY;
    // case 'H': return HOME_KEY;
    // case 'F': return END_KEY;

} /* editorReadKey */

int getCursorPosition(int *rows, int *cols)
{
    char buf[32];
    unsigned int i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') {
        return -1;
    }
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) {
        return -1;
    }

    return 0;
} /* getCursorPosition */


/*** syntax highlighting ***/

int is_separator(int c)
{
    return (c == 32) || c == '\0' || strchr(",.()+-/*=~%<>[];:", c) != NULL;
}

void editorUpdateSyntax(erow *row)
{
    row->hl = realloc(row->hl, row->rsize);
    memset(row->hl, HL_NORMAL, row->rsize);

    if (E.syntax == NULL) return;

    char **keywords = E.syntax->keywords;

    char *scs = E.syntax->singleline_comment_start;
    char *mcs = E.syntax->multiline_comment_start;
    char *mce = E.syntax->multiline_comment_end;

    int scs_len = scs ? strlen(scs) : 0;
    int mcs_len = mcs ? strlen(mcs) : 0;
    int mce_len = mce ? strlen(mce) : 0;

    int prev_sep = 1;
    int in_string = 0;
    int in_comment = (row->idx > 0 && E.row[row->idx - 1].hl_open_comment);

    int i = 0;

    while (i < row->rsize) {
        char c = row->render[i];
        unsigned char prev_hl = (i > 0) ? row->hl[i - 1] : HL_NORMAL;

        if (scs_len && !in_string && !in_comment) {
            if (!strncmp(&row->render[i], scs, scs_len)) {
                memset(&row->hl[i], HL_COMMENT, row->rsize - i);
                break;
            }
        }

        if (mcs_len && mce_len && !in_string) {
            if (in_comment) {
                row->hl[i] = HL_MLCOMMENT;
                if (!strncmp(&row->render[i], mce, mce_len)) {
                    memset(&row->hl[i], HL_MLCOMMENT, mce_len);
                    i += mce_len;
                    in_comment = 0;
                    prev_sep = 1;
                    continue;
                } else {
                    i++;
                    continue;
                }
            } else if (!strncmp(&row->render[i], mcs, mcs_len)) {
                memset(&row->hl[i], HL_MLCOMMENT, mcs_len);
                i += mcs_len;
                in_comment = 1;
                continue;
            }
        }

        if (E.syntax->flags & HL_HIGHLIGHT_STRINGS) {
            if (in_string) {
                row->hl[i] = HL_STRING;
                if (c == '\\' && i + 1 < row->rsize) {
                    row->hl[i + 1] = HL_STRING;
                    i += 2;
                    continue;
                }
                if (c == in_string) in_string = 0;
                i++;
                prev_sep = 1;
                continue;
            } else {
                if (c == '"' || c == '\'') {
                    in_string = c;
                    row->hl[i] = HL_STRING;
                    i++;
                    continue;
                }
            }
        }

        if (E.syntax->flags & HL_HIGHLIGHT_NUMBERS) {
            if (((c >= '0') && (c <= '9') && (prev_sep || prev_hl == HL_NUMBER)) ||
                (c == '.' && prev_hl == HL_NUMBER)) {
                row->hl[i] = HL_NUMBER;
                i++;
                prev_sep = 0;
                continue;
            }
        }

        if (prev_sep) {
            int j;
            for (j = 0; keywords[j]; j++) {
                int klen = strlen(keywords[j]);
                int kw2 = keywords[j][klen - 1] == '|';
                if (kw2) klen--;

                if (!strncmp(&row->render[i], keywords[j], klen) &&
                    is_separator(row->render[i + klen])) {
                    memset(&row->hl[i], kw2 ? HL_KEYWORD2 : HL_KEYWORD1, klen);
                    i += klen;
                    break;
                }
                if ( (E.syntax->flags & HL_IGNORE_CASE) &&
                     (!strncasecmp(&row->render[i], keywords[j], klen)) &&
                     (is_separator(row->render[i + klen])) ) {
                    memset(&row->hl[i], kw2 ? HL_KEYWORD2 : HL_KEYWORD1, klen);
                    i += klen;
                    break;
                }
            }
            if (keywords[j] != NULL) {
                prev_sep = 0;
                continue;
            }
        }

        prev_sep = is_separator(c);
        i++;
    }

    int changed = (row->hl_open_comment != in_comment);

    row->hl_open_comment = in_comment;
    if (changed && row->idx + 1 < E.numrows) {
        editorUpdateSyntax(&E.row[row->idx + 1]);
    }
} /* editorUpdateSyntax */

// https://en.wikipedia.org/wiki/ANSI_escape_code

int editorSyntaxToColor(int hl)
{
    switch (hl) {
        case HL_COMMENT:
        case HL_MLCOMMENT: return 36;
        case HL_KEYWORD1: return 92;
        case HL_KEYWORD2: return 32;
        case HL_STRING: return 37;
        case HL_NUMBER: return 91;
        case HL_MATCH: return 36;
        default: return 93;
    }
}

void editorSelectSyntaxHighlight()
{
    E.syntax = NULL;
    if (E.filename == NULL) return;

    char *ext = strrchr(E.filename, '.');

    for (unsigned int j = 0; j < HLDB_ENTRIES; j++) {
        struct editorSyntax *s = &HLDB[j];
        unsigned int i = 0;
        while (s->filematch[i]) {
            int is_ext = (s->filematch[i][0] == '.');
            if ((is_ext && ext && !strcmp(ext, s->filematch[i])) ||
                (!is_ext && strstr(E.filename, s->filematch[i]))) {
                E.syntax = s;

                int filerow;
                for (filerow = 0; filerow < E.numrows; filerow++) {
                    editorUpdateSyntax(&E.row[filerow]);
                }

                return;
            }
            i++;
        }
    }
} /* editorSelectSyntaxHighlight */

/*** row operations ***/

int editorRowCxToRx(erow *row, int cx)
{
    int rx = 0;
    int j;

    for (j = 0; j < cx; j++) {
        if (row->chars[j] == '\t')
            rx += (TextEd_TAB_STOP - 1) - (rx % TextEd_TAB_STOP);
        rx++;
    }
    return rx;
}

int editorRowRxToCx(erow *row, int rx)
{
    int cur_rx = 0;
    int cx;

    for (cx = 0; cx < row->size; cx++) {
        if (row->chars[cx] == '\t')
            cur_rx += (TextEd_TAB_STOP - 1) - (cur_rx % TextEd_TAB_STOP);
        cur_rx++;

        if (cur_rx > rx) return cx;
    }
    return cx;
}

void editorUpdateRow(erow *row)
{
    int tabs = 0;
    int j;

    for (j = 0; j < row->size; j++) {
        if (row->chars[j] == '\t') tabs++;
    }

    free(row->render);
    row->render = malloc(row->size + tabs * (TextEd_TAB_STOP - 1) + 1);

    int idx = 0;

    for (j = 0; j < row->size; j++) {
        if (row->chars[j] == '\t') {
            row->render[idx++] = ' ';
            while (idx % TextEd_TAB_STOP != 0) row->render[idx++] = ' ';
        } else {
            row->render[idx++] = row->chars[j];
        }
    }
    row->render[idx] = '\0';
    row->rsize = idx;

    editorUpdateSyntax(row);
} /* editorUpdateRow */

void editorInsertRow(int at, char *s, size_t len)
{
    if (at < 0 || at > E.numrows) return;
    E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
    memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.numrows - at));
    for (int j = at + 1; j <= E.numrows; j++) {
        E.row[j].idx++;
    }

    E.row[at].idx = at;

    E.row[at].size = len;
    E.row[at].chars = malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';

    E.row[at].rsize = 0;
    E.row[at].render = NULL;
    E.row[at].hl = NULL;
    E.row[at].hl_open_comment = 0;
    editorUpdateRow(&E.row[at]);

    E.numrows++;
    E.dirty++;
} /* editorInsertRow */

void editorFreeRow(erow *row)
{
    free(row->render);
    free(row->chars);
    free(row->hl);
}

void editorDelRow(int at)
{
    if (at < 0 || at >= E.numrows) return;
    editorFreeRow(&E.row[at]);
    memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.numrows - at - 1));
    for (int j = at; j < E.numrows - 1; j++) {
        E.row[j].idx--;
    }
    E.numrows--;
    E.dirty++;
}

void editorRowInsertChar(erow *row, int at, int c)
{
    if (at < 0 || at > row->size) at = row->size;
    row->chars = realloc(row->chars, row->size + 2);
    memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
    row->size++;
    row->chars[at] = c;
    editorUpdateRow(row);
    E.dirty++;
}

void editorRowAppendString(erow *row, char *s, size_t len)
{
    row->chars = realloc(row->chars, row->size + len + 1);
    memcpy(&row->chars[row->size], s, len);
    row->size += len;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);
    E.dirty++;
}

void editorRowDelChar(erow *row, int at)
{
    if (at < 0 || at >= row->size) return;
    memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
    row->size--;
    editorUpdateRow(row);
    E.dirty++;
}

/*** editor operations ***/

void editorInsertChar(int c)
{
    if (E.cy == E.numrows) {
        editorInsertRow(E.numrows, "", 0);
    }
    editorRowInsertChar(&E.row[E.cy], E.cx, c);
    E.cx++;
}

void editorInsertNewline()
{
    if (E.cx == 0) {
        editorInsertRow(E.cy, "", 0);
    } else {
        erow *row = &E.row[E.cy];
        editorInsertRow(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
        row = &E.row[E.cy];
        row->size = E.cx;
        row->chars[row->size] = '\0';
        editorUpdateRow(row);
    }
    E.cy++;
    E.cx = 0;
}

void editorDelChar()
{
    if (E.cy == E.numrows) return;
    if (E.cx == 0 && E.cy == 0) return;

    erow *row = &E.row[E.cy];

    if (E.cx > 0) {
        editorRowDelChar(row, E.cx - 1);
        E.cx--;
    } else {
        E.cx = E.row[E.cy - 1].size;
        editorRowAppendString(&E.row[E.cy - 1], row->chars, row->size);
        editorDelRow(E.cy);
        E.cy--;
    }
}

/*** file i/o ***/

char * editorRowsToString(int *buflen)
{
    int totlen = 0;
    int j;

    for (j = 0; j < E.numrows; j++) {
        totlen += E.row[j].size + 1;
    }
    *buflen = totlen;
    char *buf = malloc(totlen);
    char *p = buf;

    for (j = 0; j < E.numrows; j++) {
        memcpy(p, E.row[j].chars, E.row[j].size);
        p += E.row[j].size;
        *p = '\n';
        p++;
    }
    return buf;
}

void editorOpen(char *filename)
{
    free(E.filename);
    E.filename = strdup(filename);

    editorSelectSyntaxHighlight();

    FILE *fp = os_fopen(filename, "r");

    if (!fp) {
        // die("fopen");
        return; // TODO: error message
    }

    char line[1024];
    // size_t linecap = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        size_t linelen = strlen(line);

        while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
            linelen--;

        editorInsertRow(E.numrows, line, linelen);
    }

    fclose(fp);
    E.dirty = 0;
} /* editorOpen */

void editorSaveCallback(char *query, int key, char end)
{
    if (end == 1) {
        if (query[0] != 0) {
            free(E.filename);
            E.filename = strdup(query); // TODO: relative to current path !

            editorSelectSyntaxHighlight();
            editorSave();
        } else {
            editorSetStatusMessage("Save aborted");
        }
    }
}

void editorSave(void)
{
    if (E.filename == NULL) {
        editorPrompt("Save as: %s\x8f (ESC to cancel)", editorSaveCallback);
        return;
    }

    int len;
    char *buf = editorRowsToString(&len);
    int fd = open(E.filename, O_RDWR | O_CREAT, 0644);

    if (fd != -1) {
        if (ftruncate(fd, len) != -1) {
            if (write(fd, buf, len) == len) {
                close(fd);
                free(buf);
                E.dirty = 0;
                editorSetStatusMessage("%d bytes written to disk", len);
                return;
            }
        }
        close(fd);
    }
    free(buf);
    editorSetStatusMessage("Can't save! I/O error");
} /* editorSave */

/*** find ***/

void editorFindCallback(char *query, int key, char end)
{
    static int last_match = -1;
    static int direction = 1;

    static int saved_hl_line;
    static char *saved_hl = NULL;

    if (saved_hl) {
        memcpy(E.row[saved_hl_line].hl, saved_hl, E.row[saved_hl_line].rsize);
        free(saved_hl);
        saved_hl = NULL;
    }

    if (key == '\r' || key == '\x1b') {
        last_match = -1;
        direction = 1;
        return;
    } else if (key == ARROW_RIGHT || key == ARROW_DOWN) {
        direction = 1;
    } else if (key == ARROW_LEFT || key == ARROW_UP) {
        direction = -1;
    } else {
        last_match = -1;
        direction = 1;
    }

    if (last_match == -1) direction = 1;
    int current = last_match;
    int i;

    for (i = 0; i < E.numrows; i++) {
        current += direction;
        if (current == -1) current = E.numrows - 1;
        else if (current == E.numrows) current = 0;

        erow *row = &E.row[current];
        char *match = strstr(row->render, query);
        if (match) {
            last_match = current;
            E.cy = current;
            E.cx = editorRowRxToCx(row, match - row->render);
            E.rowoff = E.numrows;

            saved_hl_line = current;
            saved_hl = malloc(row->rsize);
            memcpy(saved_hl, row->hl, row->rsize);
            memset(&row->hl[match - row->render], HL_MATCH, strlen(query));
            break;
        }
    }
} /* editorFindCallback */

void editorBlockPaste(void)
{
    int totlen = 0;
    int j;

    for (j = E.by; j <= E.ky; j++) {
        totlen += E.row[j].size + 1;
    }
    totlen++; // last zero

    char *buf = malloc(totlen); // Alloc to much as we don't care of bx,kx
    char *p = buf;

    for (j = E.by; j <= E.ky; j++) {
        int from, len;
        int bx, kx;

        bx = (E.bx > E.row[j].size) ? E.row[j].size : E.bx;
        kx = (E.kx > E.row[j].size) ? E.row[j].size : E.kx;

        from = 0;
        len = E.row[j].size;

        if ((j == E.by) && (j == E.ky)) {
            from = bx;
            len = (kx - bx);
        } else if (j == E.by) {
            from = bx;
            len = (E.row[j].size - bx);
        } else if (j == E.ky) {
            from = 0;
            len = kx;
        }

        memcpy(p, E.row[j].chars + from, len);
        p += len;

        if (j != E.ky) {
            *p = '\n';
            p++;
        }
    }

    *p = 0;

    editorPaste(buf);
    free(buf);


    return;
} /* editorBlockPaste */

void editorBlockMove(void)
{

}

void waitingCtrlKCallback(char *query, int key, char end)
{
    if (end == 1) {
        return;
    }

    switch (key) {
        case 'b':
        case CTRL_KEY('b'):
            E.bx = E.cx;
            E.by = E.cy;
            editorSetStatusMessage("");
            E.callback = NULL;
            break;
        case 'k':
        case CTRL_KEY('k'):
            E.kx = E.cx;
            E.ky = E.cy;
            editorSetStatusMessage("");
            E.callback = NULL;
            break;
        case 'c':
        case CTRL_KEY('c'):
            editorBlockPaste();
            editorSetStatusMessage("");
            E.callback = NULL;
            break;
        case 'v':
        case CTRL_KEY('v'):
            editorBlockMove();
            editorSetStatusMessage("");
            E.callback = NULL;
            break;
    } /* switch */

} /* waitingCtrlKCallback */

void waitingCtrlK(void)
{
    editorPrompt("Press B to begin block, K to end block, C to copy, V to move", waitingCtrlKCallback);
}

void editor_gotoBasicLine(u16 line)
{
    int j;
    int line0;

    for (j = 0; j < E.numrows; j++) {
        sscanf(E.row[j].chars, "%d", &line0);
        printf("%d\n", line0);
        if (line0 == line) {
            printf("Found %d\n", j);
            E.cy = j;
            break;
        }
    }
}


/*** append buffer ***/

struct abuf {
    char *b;
    int len;
};

#define ABUF_INIT {NULL, 0 \
}

void abAppend(struct abuf *ab, const char *s, int len)
{
    char *new = realloc(ab->b, ab->len + len);

    if (new == NULL) return;
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

void abFree(struct abuf *ab)
{
    free(ab->b);
}

/*** output ***/

void editorScroll()
{
    E.rx = 0;
    if (E.cy < E.numrows) {
        E.rx = editorRowCxToRx(&E.row[E.cy], E.cx);
    }

    if (E.cy < E.rowoff) {
        E.rowoff = E.cy;
    }
    if (E.cy >= E.rowoff + E.screenrows) {
        E.rowoff = E.cy - E.screenrows + 1;
    }
    if (E.rx < E.coloff) {
        E.coloff = E.rx;
    }
    if (E.rx >= E.coloff + E.screencols) {
        E.coloff = E.rx - E.screencols + 1;
    }
}

void sendColor(struct abuf *ab, int color)
{
    char buf[16];
    int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", color);

    abAppend(ab, buf, clen);
}

void editorDrawRows(struct abuf *ab)
{
    int y;

    for (y = 0; y < E.screenrows; y++) {
        int filerow = y + E.rowoff;
        if (filerow >= E.numrows) {
            if (E.numrows == 0 && y == E.screenrows / 3) {
                char welcome[80];
                int welcomelen = snprintf(welcome, sizeof(welcome), "TextEd editor -- version %s", TextEd_VERSION);
                if (welcomelen > E.screencols) welcomelen = E.screencols;
                int padding = (E.screencols - welcomelen) / 2;
                if (padding) {
                    abAppend(ab, "~", 1);
                    padding--;
                }
                while (padding--) abAppend(ab, " ", 1);
                abAppend(ab, welcome, welcomelen);
            } else {
                abAppend(ab, "~", 1);
            }
        } else {
            int len = E.row[filerow].rsize - E.coloff;
            if (len < 0) {
                len = 0;
            }
            if (len > E.screencols) {
                len = E.screencols;
            }
            char *c = &E.row[filerow].render[E.coloff];
            unsigned char *hl = &E.row[filerow].hl[E.coloff];
            int current_color = -1;
            int j;
            for (j = 0; j < len; j++) {
                u8 reverse = 0;


                if ( (filerow >= E.by) && (filerow <= E.ky)) {
                    reverse = 1;
                    if ((filerow == E.by) && (j + E.coloff < E.bx)) {
                        reverse = 0;
                    }
                    if ((filerow == E.ky) && (j + E.coloff >= E.kx)) {
                        reverse = 0;
                    }

                    if ((j + E.coloff == E.bx) && (filerow == E.by)) {
                        sendColor(ab, 44);     // Blue background color
                    }
                    if ((j + E.coloff == E.kx) && (filerow == E.ky)) {
                        sendColor(ab, 49);     // Default background color
                    }

                }

                if (((c[j] >= 0) && (c[j] < 32)) || (c[j] == 127)) {
                    char sym = (c[j] <= 26) ? '@' + c[j] : '?';
                    abAppend(ab, "\x1b[7m", 4);
                    abAppend(ab, &sym, 1);
                    abAppend(ab, "\x1b[m", 3);
                    if (current_color != -1) {
                        if (reverse == 1) {
                            sendColor(ab, 44);
                        } else {
                            sendColor(ab, 49);                  // Default background color
                        }
                        sendColor(ab, current_color);
                    }
                } else if (hl[j] == HL_NORMAL) {
                    if (current_color != -1) {
                        if (reverse == 1) {
                            sendColor(ab, 44);
                        } else {
                            sendColor(ab, 49);                  // Default background color
                        }
                        sendColor(ab, 39);
                        current_color = -1;
                    }
                    abAppend(ab, &c[j], 1);
                } else {
                    int color = editorSyntaxToColor(hl[j]);
                    if (color != current_color) {
                        current_color = color;
                        if (reverse == 1) {
                            sendColor(ab, 44);
                        } else {
                            sendColor(ab, 49);                  // Default background color
                        }
                        sendColor(ab, current_color);
                    }
                    abAppend(ab, &c[j], 1);
                }
            }
            sendColor(ab, 39);                                  // Default foreground color
        }

        abAppend(ab, "\x1b[K", 3);
        if (y != E.screenrows - 1) {
            abAppend(ab, "\r\n", 2);
        }
    }
} /* editorDrawRows */

void editorDrawStatusBar(struct abuf *ab)
{
    char buf[94 + 1];


    char status[80], rstatus[80];
    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
                       E.filename ? E.filename : "[No Name]", E.numrows,
                       E.dirty ? "(modified)" : "");
    int rlen = snprintf(rstatus, sizeof(rstatus), "%s | %d/%d",
                        E.syntax ? E.syntax->filetype : "no ft", E.cy + 1, E.numrows);

    if (len > 94) len = 94;

    strcpy(buf, status);

    while (len < 94) {
        if (94 - len == rlen) {
            strcpy(buf + len, rstatus);
            break;
        } else {
            buf[len] = 32;
            len++;
        }
    }
    displayHeader(buf);
} /* editorDrawStatusBar */

void editorDrawMessageBar(struct abuf *ab)
{
    int msglen = strlen(E.statusmsg);

    if (msglen && time(NULL) - E.statusmsg_time < 5) {
        displayFooter(E.statusmsg);
    } else {
        displayFooter("");
    }

    // abAppend(ab, "\x1b[K", 3);

    // if (msglen > E.screencols) msglen = E.screencols;
    // if (msglen && time(NULL) - E.statusmsg_time < 5) {
    //     abAppend(ab, E.statusmsg, msglen);
    // }

}

void editorRefreshScreen(void)
{
    editorScroll();

    struct abuf ab = ABUF_INIT;

    abAppend(&ab, "\x1b[?25l", 6); // Hide cursor
    abAppend(&ab, "\x1b[H", 3); // Clear screen

    editorDrawRows(&ab);
    editorDrawStatusBar(&ab);
    editorDrawMessageBar(&ab);


// Set cursor to position

    char buf[32];

    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1, (E.rx - E.coloff) + 1);
    abAppend(&ab, buf, strlen(buf));

    abAppend(&ab, "\x1b[?25h", 6);

    ansiToScreen(ab.b, ab.len);
    abFree(&ab);
} /* editorRefreshScreen */

void editorSetStatusMessage(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
    va_end(ap);
    E.statusmsg_time = time(NULL);
}

/*** input ***/



void editorHandlePrompt(void)
{
    editorSetStatusMessage(E.prompt, E.promptBuf);
    editorRefreshScreen();

    int c = editorReadKey();
    if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE) {
        if (E.promptBuflen != 0) E.promptBuf[--E.promptBuflen] = '\0';
    } else if (c == '\x1b') {
        editorSetStatusMessage("");
        E.callback(E.promptBuf, c, 1);
        E.callback = NULL;
        return;
    } else if (c == '\r') {
        if (E.promptBuflen != 0) {
            editorSetStatusMessage("");
            E.callback(E.promptBuf, c, 1);
            E.callback = NULL;
            return;
        }
    } else if ((c > 31) && (c < 127)) {
        if (E.promptBuflen < 128 - 1) {
            E.promptBuf[E.promptBuflen++] = c;
            E.promptBuf[E.promptBuflen] = '\0';
        }
    }

    E.callback(E.promptBuf, c, 0);
} /* editorHandlePrompt */

void editorPrompt(char *prompt0, void (*callback)(char *, int, char))
{
    E.callback = callback;
    strcpy(E.prompt, prompt0);

    return;
}


void editorMoveCursor(int key)
{
    erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];

    switch (key) {
        case ARROW_LEFT:
            if (E.cx != 0) {
                E.cx--;
            } else if (E.cy > 0) {
                E.cy--;
                E.cx = E.row[E.cy].size;
            }
            break;
        case ARROW_RIGHT:
            if (row && E.cx < row->size) {
                E.cx++;
            } else if (row && E.cx == row->size) {
                E.cy++;
                E.cx = 0;
            }
            break;
        case ARROW_UP:
            if (E.cy != 0) {
                E.cy--;
            }
            break;
        case ARROW_DOWN:
            if (E.cy < E.numrows) {
                E.cy++;
            }
            break;
    } /* switch */

    row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
    int rowlen = row ? row->size : 0;

    if (E.cx > rowlen) {
        E.cx = rowlen;
    }
} /* editorMoveCursor */

void editorProcessKeypress()
{
    static int quit_times = TextEd_QUIT_TIMES;

    int c = editorReadKey();

    switch (c) {
        case 0:
            return;

        case '\r':
            editorInsertNewline();
            break;

        case CTRL_KEY('q'):

            strcpy(fgb.core->messageToDislay, "[CTRL][Q]");
            fgb.core->messageTimer = 50;

            if (E.dirty && quit_times > 0) {
                editorSetStatusMessage("WARNING!!! File has unsaved changes. "
                                       "Press Ctrl-Q %d more times to quit.", quit_times);
                quit_times--;
                return;
            }
            fgb.inEditor = 0;
            // write(STDOUT_FILENO, "\x1b[2J", 4);
            // write(STDOUT_FILENO, "\x1b[H", 3);
            // exit(0);

            CW.colorBack = fgb.palette[0];
            CW.colorFront = fgb.palette[10];

            cls(CW.colorBack);
            break;

        // case CTRL_KEY('p'):
        //     ansiSave();
        //     break;

        case CTRL_KEY('r'):
        {

            strcpy(fgb.core->messageToDislay, "[CTRL][R]");
            fgb.core->messageTimer = 50;

            int errorLine;
            char errorMsg[256];
            editorSave();

            int error = verifiyFilename(E.filename, &errorLine, errorMsg);
            switch (error) {
                case 1:
                    editorSetStatusMessage("ERROR: Invalid extension");
                    break;
                case 2:
                    editorSetStatusMessage(errorMsg);
                    E.cx = 0;
                    E.cy = errorLine;
                    break;
                default:
                    cpcRun(E.filename, 0);

                    CW.colorBack = fgb.palette[0];
                    CW.colorFront = fgb.palette[10];

                    cls(CW.colorBack);
            }
        }
        break;

        case CTRL_KEY('s'):

            strcpy(fgb.core->messageToDislay, "[CTRL][S]");
            fgb.core->messageTimer = 50;

            editorSave();
            break;

        case HOME_KEY:
            E.cx = 0;
            break;

        case END_KEY:
            if (E.cy < E.numrows)
                E.cx = E.row[E.cy].size;
            break;

        case CTRL_KEY('f'):

            strcpy(fgb.core->messageToDislay, "[CTRL][F]");
            fgb.core->messageTimer = 50;

            editorPrompt("Search: %s\x8f (Use ESC/Arrows/Enter)", editorFindCallback);
            break;

        case CTRL_KEY('k'):

            strcpy(fgb.core->messageToDislay, "[CTRL][K]");
            fgb.core->messageTimer = 50;

            waitingCtrlK();
            break;

        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY:
            if (c == DEL_KEY) editorMoveCursor(ARROW_RIGHT);
            editorDelChar();
            break;

        case PAGE_UP:
        case PAGE_DOWN:
        {
            if (c == PAGE_UP) {
                E.cy = E.rowoff;
            } else if (c == PAGE_DOWN) {
                E.cy = E.rowoff + E.screenrows - 1;
                if (E.cy > E.numrows) E.cy = E.numrows;
            }

            int times = E.screenrows;
            while (times--)
                editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
        }
        break;

        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editorMoveCursor(c);
            break;

        case CTRL_KEY('l'):
        case '\x1b':
            break;

        default:
            editorInsertChar(c);
            break;
    } /* switch */

    quit_times = TextEd_QUIT_TIMES;
} /* editorProcessKeypress */

/*** init ***/

void initEditor()
{
// Init ansi

    initAnsi();

    cls(ansi_palette[7]);

// Init editor
    E.cx = 0;
    E.cy = 0;
    E.rx = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.numrows = 0;
    E.row = NULL;
    E.dirty = 0;
    E.filename = NULL;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;
    E.syntax = NULL;

    E.screenrows = CW.height;
    E.screencols = CW.width;

    E.waitingCtrlK = 0;

    E.callback = NULL;

    E.promptBuflen = 0;

} /* initEditor */

void console_editor_loop(void)
{
    editorRefreshScreen();

    if (E.callback != NULL) {
        editorHandlePrompt();
    } else {
        editorProcessKeypress();
    }
}

int edBinding(lua_State *L)
{
    fgb.inEditor = 1;
    fgb.editorBufLen = 0;

    initEditor();

    if (lua_gettop(L) == 1) {
        const char *s;
        size_t len;
        char filename[PATH_MAX];

        s = lua_tolstring(L, -1, &len);

        strcpy(filename, fgb.fs->currentDir);
        path2Abs(filename, s);

        editorOpen(filename);
    }

    editorSetStatusMessage("HELP: Ctrl-S = save | Ctrl-R = run | Ctrl-Q = quit | Ctrl-F = find");

    return 0;
} /* edBinding */
