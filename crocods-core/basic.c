
// https://www.cpcwiki.eu/index.php/Technical_information_about_Locomotive_BASIC#Floating_Point_data_definition
// https://www.cpcwiki.eu/index.php/Locomotive_BASIC

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// #define STANDALONE

#ifdef STANDALONE
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
#else
#include "crocods.h"
#include "plateform.h"

extern core_crocods_t gb;

#endif

#include "basic.h"


#define CODESIZE 65535
u8 Listing[CODESIZE];

struct keywordLookup {
    char *keyword;
    u8 opcode;
};

static void addError(struct s_basic_info *debug, char *fmt, ...);

static struct keywordLookup opCodes[] = {
    {"ZONE", 0xda},
    {"YPOS", 0x48},         // YPOS
    {"XPOS", 0x47},     // XPOS
    {"XOR", 0xfd},      // XOR
    {"WRITE", 0xd9},    // WRITE
    {"WINDOW", 0xd8},   // WINDOW
    {"WIDTH", 0xd7},    // WIDTH
    {"WHILE", 0xd6},    // WHILE
    {"WEND", 0xd5},     // WEND
    {"WAIT", 0xd4},     // WAIT
    {"VPOS", 0x7f},     // VPOS
    {"VAL", 0x1d},      // VAL
    {"USING", 0xed},    // USING
    {"UPPER$", 0x1c},   // UPPER$
    {"UNT", 0x1b},      // UNT
    {"TRON", 0xd3},     // TRON
    {"TROFF", 0xd2},    // TROFF
    {"TO", 0xec},          // TO
    {"TIME", 0x46},     // TIME
    {"THEN", 0xeb},     // THEN
    {"TESTR", 0x7d},    // TESTR
    {"TEST", 0x7c},     // TEST
    {"TAN", 0x1a},      // TAN
    {"TAGOFF", 0xd1},   // TAFOFF
    {"TAG", 0xd0},      // TAG
    {"TAB", 0xea},      // TAB
    {"SYMBOL", 0xcf},   // SYMBOL
    {"SWAP", 0xe7},     // SWAP
    {"STRING$", 0x7b},  // STRING$
    {"STR$", 0x19},     // STR$
    {"STOP", 0xce},     // STOP
    {"STEP", 0xe6},     // STEP
    {"SQR", 0x18},      // SQR
    {"SQ", 0x17},          // SQ
    {"SPEED", 0xcd},    // SPEED
    {"SPC", 0xe5},      // SPC
    {"SPACE$", 0x16},   // SPACE$
    {"SOUND", 0xcc},    // SOUND
    {"SIN", 0x15},      // SIN
    {"SGN", 0x14},      // SGN
    {"SAVE", 0xcb},     // SAVE
    {"RUN", 0xca},      // RUN
    {"ROUND", 0x7a},    // ROUND
    {"RND", 0x45},      // RND
    {"RIGHT$", 0x79},   // RIGHT$
    {"RETURN", 0xc9},   // RETURN
    {"RESUME", 0xc8},   // RESUME
    {"RESTORE", 0xc7},  // RESTORE
    {"RENUM", 0xc6},    // RENUM
    {"REMAIN", 0x13},   // REMAIN
    {"REM", 0xc5},      // REM
    {"RELEASE", 0xc4},  // RELEASE
    {"READ", 0xc3},     // READ
    {"RANDOMIZE", 0xc2},   // RANDOMIZE
    {"RAD", 0xc1},      // RAD
    {"PRINT", 0xbf},    // PRINT
    {"POS", 0x78},      // POS
    {"POKE", 0xbe},     // POKE
    {"PLOTR", 0xbd},    // PLOTR
    {"PLOT", 0xbc},     // PLOT
    {"PI", 0x44},          // PI
    {"PEN", 0xbb},      // PEN
    {"PEEK", 0x12},     // PEEK
    {"PAPER", 0xba},    // PAPER
    {"OUT", 0xb9},      // OUT
    {"ORIGIN", 0xb8},   // ORIGIN
    {"OR", 0xfc},          // OR
    {"OPENOUT", 0xb7},  // OPENOUT
    {"OPENIN", 0xb6},   // OPENIN
    {"ON SQ", 0xb5},    // ON SQ
    {"ON ERROR GO\x09TO 0", 0xb4},  // ON ERROR GOTO 0, ON ERROR GO TO 0 (but not ON ERROR GOTO/GO TO [n])
    {"ON BREAK", 0xb3}, // ON BREAK
    {"ON", 0xb2},          // ON (and ON ERROR GOTO, ON ERROR GO TO)
    {"NOT", 0xfe},      // NOT
    {"NEW", 0xb1},      // NEW
    {"NEXT", 0xb0},     // NEXT
    {"MOVER", 0xaf},    // MOVER
    {"MOVE", 0xae},     // MOVE
    {"MODE", 0xad},     // MODE
    {"MOD", 0xfb},      // MOD
    {"MIN", 0x77},      // MIN
    {"MID$", 0xac},     // MID$
    {"MERGE", 0xab},    // MERGE
    {"MEMORY", 0xaa},   // MEMORY
    {"MAX", 0x76},      // MAX
    {"MASK", 0xdf},     // MASK
    {"LOWER$", 0x11},   // LOWER$
    {"LOG10", 0x10},    // LOG10
    {"LOG", 0x0f},      // LOG
    {"LOCATE", 0xa9},   // LOCATE
    {"LOAD", 0xa8},     // LOAD
    {"LIST", 0xa7},     // LIST
    {"LINE", 0xa6},     // LINE
    {"LET", 0xa5},      // LET
    {"LEN", 0x0e},      // LEN
    {"LEFT$", 0x75},    // LEFT$
    {"KEY", 0xa4},      // KEY
    {"JOY", 0x0d},      // JOY
    {"INT", 0x0c},      // INT
    {"INSTR", 0x74},    // INSTR
    {"INPUT", 0xa3},    // INPUT
    {"INP", 0x0b},      // INP
    {"INKEY$", 0x43},   // INKEY$
    {"INKEY", 0x0a},    // INKEY
    {"INK", 0xa2},      // INK
    {"IF", 0xa1},          // IF
    {"HIMEM", 0x42},    // HIMEM
    {"HEX$", 0x73},     // HEX$
    {"GRAPHICS", 0xde}, // GRAPHICS
    {"GO\x09TO", 0xa0},  // GO TO, GOTO
    {"GO\x09SUB", 0x9f},  // GO SUB, GOSUB
    {"FRE", 0x09},      // FRE
    {"FRAME", 0xe0},    // FRAME
    {"FOR", 0x9e},      // FOR
    {"FN", 0xe4},          // FN
    {"FIX", 0x08},      // FIX
    {"FILL", 0xdd},     // FILL
    {"EXP", 0x07},      // EXP
    {"EVERY", 0x9d},    // EVERY
    {"ERROR", 0x9c},    // ERROR
    {"ERR", 0x41},      // ERR
    {"ERL", 0xe3},      // ERL
    {"ERASE", 0x9b},    // ERASE
    {"EOF", 0x40},      // EOF
    {"ENV", 0x9a},      // ENV
    {"ENT", 0x99},      // ENT
    {"END", 0x98},      // END
    {"ELSE", 0x97},     // ELSE
    {"EI", 0xdc},          // EI
    {"EDIT", 0x96},     // EDIT
    {"DRAWR", 0x95},    // DRAWR
    {"DRAW", 0x94},     // DRAW
    {"DIM", 0x93},      // DIM
    {"DI", 0xdb},          // DI
    {"DERR", 0x49},     // DERR
    {"DELETE", 0x92},   // DELETE
    {"DEG", 0x91},      // DEG
    {"DEFSTR", 0x90},   // DEFSTR
    {"DEFREAL", 0x8f},  // DEFREAL
    {"DEFINT", 0x8e},   // DEFINT
    {"DEF", 0x8d},      // DEF
    {"DEC$", 0x72},     // DEC$
    {"DATA", 0x8c},     // DATA
    {"CURSOR", 0xe1},   // CURSOR
    {"CREAL", 0x06},    // CREAL
    {"COS", 0x05},      // COS
    {"COPYCHR$", 0x7e}, // COPYCHR$
    {"CONT", 0x8b},     // CONT
    {"CLS", 0x8a},      // CLS
    {"CLOSEOUT", 0x89}, // CLOSEOUT
    {"CLOSEIN", 0x88},  // CLOSEIN
    {"CLG", 0x87},      // CLG
    {"CLEAR", 0x86},    // CLEAR
    {"CINT", 0x04},     // CINT
    {"CHR$", 0x03},     // CHR$
    {"CHAIN", 0x85},    // CHAIN
    {"CAT", 0x84},      // CAT
    {"CALL", 0x83},     // CALL
    {"BORDER", 0x82},   // BORDER
    {"BIN$", 0x71},     // BIN$
    {"AUTO", 0x81},     // AUTO
    {"ATN", 0x02},      // ATN
    {"ASC", 0x01},      // ASC
    {"AND", 0xfa},      // AND
    {"AFTER", 0x80},    // AFTER
    {"ABS", 0x00},      // ABS
    {"^", 0xf8},
    {"\x5c", 0xf9},          // "\"
    {">\x09=", 0xf0},
    {"= >", 0xf0},
    {">", 0xee},
    {"<\x09>", 0xf2},
    {"<\x09=", 0xf3},
    {"= <", 0xf3},
    {"=", 0xef},
    {"<", 0xf1},
    {"/", 0xf7},
    {":", 0x01},
    {"*", 0xf6},
    {"-", 0xf5},
    {"+", 0xf4},
    {"'", 0xc0},
};


u8 DproBasic[128] =
{
    0xAB, 0x2C, 0xED, 0xEA, 0x6C, 0x37, 0x3F, 0xEC,
    0x9B, 0xDF, 0x7A, 0x0C, 0x3B, 0xD4, 0x6D, 0xF5,
    0x04, 0x44, 0x03, 0x11, 0xDF, 0x59, 0x8F, 0x21,
    0x73, 0x7A, 0xCC, 0x83, 0xDD, 0x30, 0x6A, 0x30,
    0xD3, 0x8F, 0x02, 0xF0, 0x60, 0x6B, 0x94, 0xE4,
    0xB7, 0xF3, 0x03, 0xA8, 0x60, 0x88, 0xF0, 0x43,
    0xE8, 0x8E, 0x43, 0xA0, 0xCA, 0x84, 0x31, 0x53,
    0xF3, 0x1F, 0xC9, 0xE8, 0xAD, 0xC0, 0xBA, 0x6D,
    0x93, 0x08, 0xD4, 0x6A, 0x2C, 0xB2, 0x07, 0x27,
    0xC0, 0x99, 0xEE, 0x89, 0xAF, 0xC3, 0x53, 0xAB,
    0x2B, 0x34, 0x5C, 0x2F, 0x13, 0xEE, 0xAA, 0x2C,
    0xD9, 0xF4, 0xBC, 0x12, 0xB3, 0xC5, 0x1C, 0x68,
    0x01, 0x20, 0x2C, 0xFA, 0x77, 0xA6, 0xB5, 0xA4,
    0xFC, 0x9B, 0xF1, 0x32, 0x5B, 0xC3, 0x70, 0x77,
    0x85, 0x36, 0xBE, 0x5B, 0x8C, 0xC8, 0xB5, 0xC2,
    0xF0, 0x0B, 0x98, 0x0F, 0x36, 0x9D, 0xD8, 0x96
};


static const char *numbers[11] =
{
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"
};
static const char *keywords[0x80] =
{
    "AFTER", "AUTO", "BORDER", "CALL", "CAT", "CHAIN", "CLEAR", "CLG", "CLOSEIN", "CLOSEOUT",
    "CLS", "CONT", "DATA", "DEF", "DEFINT", "DEFREAL", "DEFSTR", "DEG", "DELETE", "DIM",
    "DRAW", "DRAWR", "EDIT",  "ELSE", "END", "ENT", "ENV", "ERASE", "ERROR", "EVERY",
    "FOR", "GOSUB", "GOTO", "IF", "INK", "INPUT", "KEY", "LET", "LINE", "LIST",
    "LOAD", "LOCATE", "MEMORY", "MERGE", "MID$", "MODE", "MOVE", "MOVER", "NEXT", "NEW",
    "ON", "ON BREAK", "ON ERROR GOTO", "SQ", "OPENIN", "OPENOUT", "ORIGIN", "OUT", "PAPER", "PEN",
    "PLOT", "PLOTR", "POKE", "PRINT", "'", "RAD", "RANDOMIZE", "READ", "RELEASE", "REM",

    "RENUM",
    "RESTORE", "RESUME", "RETURN", "RUN", "SAVE", "SOUND", "SPEED", "STOP",
    "SYMBOL", "TAG", "TAGOFF", "TROFF", "TRON", "WAIT", "WEND", "WHILE",
    "WIDTH", "WINDOW", "WRITE", "ZONE", "DI", "EI", "FILL", "GRAPHICS",
    "MASK", "FRAME", "CURSOR", "#E2", "ERL", "FN", "SPC", "STEP", "SWAP",
    "#E8", "#E9", "TAB", "THEN", "TO", "USING", ">", "=", ">=", "<", "<>",
    "<=", "+", "-", "*", "/", "^", "\\ ", "AND", "MOD", "OR", "XOR", "NOT",
    "#FF"
};

static const char *functions[0x80] =
{
    "ABS", "ASC", "ATN", "CHR$", "CINT", "COS", "CREAL", "EXP", "FIX",
    "FRE", "INKEY", "INP", "INT", "JOY", "LEN", "LOG", "LOG10", "LOWER$",
    "PEEK", "REMAIN", "SGN", "SIN", "SPACE$", "SQ", "SQR", "STR$", "TAN",
    "UNT", "UPPER$", "VAL", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "EOF", "ERR", "HIMEM", "INKEY$", "PI", "RND",
    "TIME", "XPOS", "YPOS", "DERR", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "BIN$", "DEC$", "HEX$",
    "INSTR", "LEFT$", "MAX", "MIN", "POS", "RIGHT$", "ROUND", "STRING$",
    "TEST", "TESTR", "COPYCHR$", "VPOS"
};

void putByte(u8 *BufFile, int Pos, u8 b)
{
    // printf("putByte %04x:%d\n", Pos + 0x170, b);
    BufFile[Pos] = b;
}

void putWord(u8 *BufFile, int Pos, u16 w)
{
    putByte(BufFile, Pos + 1, w >> 8);
    putByte(BufFile, Pos, w & 255);
}

u8 GetByte(u8 *BufFile, int Pos)
{
    return (u8)(BufFile[Pos]);
}

int GetWord(u8 *BufFile, int Pos)
{
    return (GetByte(BufFile, Pos + 1) << 8) | GetByte(BufFile, Pos);
}

int AddWord(u8 *BufFile, int Pos, char *Listing)
{
    int LenVar = 0, l = strlen(Listing);
    u8 b;

    do{
        b = GetByte(BufFile, Pos++);
        Listing[l++] = (char)(b & 0x7F);
    }while (!(b & 0x80) && LenVar++ < 0xFF);
    Listing[l] = 0;
    return(Pos);
}


void untokenizeBasic(u8 *input, char *output)
{
    static char Tmp[32];
    int Pos = 0, Token = 0;
    char *p;
    double f;
    int exp;


    *output = 0;
    Token = GetByte(input, 0);
    for (;;) {
        int lg = GetWord(input, Pos);
        Pos += 2;
        if (!lg)
            break;

        int NumLigne = GetWord(input, Pos);
        Pos += 2;
        sprintf(Tmp, "%d ", NumLigne);
        strcat(output, Tmp);


        int DansChaine = 0;
        do{
            Token = GetByte(input, Pos++);

            if (DansChaine) {
                Tmp[0] = (char)Token;
                Tmp[1] = 0;
                strcat(output, Tmp);
                if (Token == '"') {
                    DansChaine ^= 1;
                }
            } else if (Token > 0x7F && Token < 0xFF) {
                if ( (output[strlen(output) - 1] == ':') && (Token == 0x97)) {     // Else
                    output[strlen(output) - 1] = 0;
                }
                if ( (output[strlen(output) - 1] == ':') && (Token == 0xC0)) {     // '
                    output[strlen(output) - 1] = 0;
                }

                strcat(output, keywords[Token & 0x7F]);
            } else if ((Token >= 0x0E) && (Token <= 0x18)) {
                strcat(output, numbers[Token - 0x0E]);
            } else if ((Token >= 0x20) && (Token < 0x7C)) {
                Tmp[0] = (char)Token;
                Tmp[1] = 0;
                strcat(output, Tmp);
                if (Token == '"')
                    DansChaine ^= 1;
            } else {
                // cout << "Token:" << Token <<endl;
                switch (Token) {
                    case 0x00:     // Used for what ?
                        break;

                    case 0x01:
                        Tmp[0] = ':';
                        Tmp[1] = 0;
                        strcat(output, Tmp);
                        break;

                    case 0x02:     // Variable entière (type %)
                        Pos = AddWord(input, 2 + Pos, output);
                        strcat(output, "%");
                        break;

                    case 0x03:     // Variable chaine (type $)
                        Pos = AddWord(input, 2 + Pos, output);
                        strcat(output, "$");
                        break;

                    case 0x04:     // Variable float (type !)
                        Pos = AddWord(input, 2 + Pos, output);
                        strcat(output, "!");
                        break;

                    case 0x0B:
                    case 0x0C:
                    case 0x0D:     // Variable "standard"
                        Pos = AddWord(input, 2 + Pos, output);
                        break;

                    case 0x19:     // Constante entière 8 bits
                        sprintf(output + strlen(output), "%d", (u8)GetByte(input, Pos));
                        Pos++;
                        break;

                    case 0x1A:
                    case 0x1E:     // Constante entière 16 bits
                        sprintf(output + strlen(output), "%d", GetWord(input, Pos));
                        Pos += 2;
                        break;

                    case 0x1B:
                        sprintf(Tmp, "&X%X", GetWord(input, Pos));
                        strcat(output, Tmp);
                        Pos += 2;
                        break;

                    case 0x1C:
                        sprintf(Tmp, "&%X", GetWord(input, Pos));
                        strcat(output, Tmp);
                        Pos += 2;
                        break;

                    case 0x1D: // 16-bit BASIC program line memory address pointer (see notes)
                    {
                        u16 adr = GetWord(input, Pos);

                        printf("%04x\n", Pos);
                        printf("%04x\n", adr);

                        sprintf(Tmp, "%d", GetWord(input, adr - 0x170 + 1 + 2));
                        strcat(output, Tmp);

                        printf("%s\n", Tmp);


                        Pos += 2;
                    }
                    break;

                    case 0x1F:     // Constante flottante
                        f = (GetByte(input, Pos + 2) << 16)
                            + (GetByte(input, Pos + 1) << 8)
                            + GetByte(input, Pos)
                            + ((GetByte(input, Pos + 3) & 0x7F) << 24);
                        f = 1 + (f / 0x80000000);

                        if (GetByte(input, Pos + 3) & 0x80)
                            f = -f;

                        exp = GetByte(input, Pos + 4) - 129;
                        Pos += 5;
                        // sprintf(Tmp, "%f", f * pow((double)2, exp));
                        sprintf(Tmp, "%f", f * (float)(1 << exp));
                        // Suppression des '0' inutiles
                        p = &Tmp[strlen(Tmp) - 1];
                        while (*p == '0')
                            *p-- = 0;

                        if (*p == '.')
                            *p = 0;

                        strcat(output, Tmp);
                        break;

                    case 0x7C:
                        strcat(output, "|");
                        Pos = AddWord(input, 1 + Pos, output);
                        break;

                    case 0xFF:
                        if (GetByte(input, Pos) < 0x80)
                            strcat(output, functions[GetByte(input, Pos++)]);
                        else {
                            Tmp[1] = 0;
                            Tmp[0] = (char)(GetByte(input, Pos++) & 0x7F);
                            strcat(output, Tmp);
                        }
                        break;

                    default:
                        ddlog(&gb, 2, "Unknown token: %02x\n", Token);
                        // Token = Token;
                }     /* switch */
            }
        }while (Token);

        strcat(output, "\r\n");
    }

}     /* Basic */

/// @brief
/// @param test
/// @param k
/// @return 1: found, 0: not found
u8 isKeyword(u8 *text, u8 k, u8 *len)
{
    char *opcode = opCodes[k].keyword;

    u8 posText = 0, posOpcode = 0;

    while (opcode[posOpcode] != 0) {
        if (toupper(opcode[posOpcode]) == toupper(text[posText])) {
            posText++;
            posOpcode++;
        } else if (opcode[posOpcode] == 0x09) {  //     GO\x09SUB
            if (text[posText] == 0x20) {
                posText++;
            } else if (toupper(opcode[posOpcode + 1]) == toupper(text[posText])) {
                posText++;
                posOpcode += 2;
            } else {
                return 0;
            }
        } else {
            return 0;
        }     // TOOD: add test with 32

    }


    *len = posText;

    return 1;
} /* isKeyword */

/// @brief Search the next keyword in line
/// @param bin
/// @param keyword Keyword ready (from the input text)
/// @param keywordLen Keyword length (from the input text)
/// @return id in opcodes list
u8 getNextKeyword(u8 *bin, u8 *keyword, u8 *keywordLen)
{
    u8 k;
    u8 found;
    u8 len;

    // printf("Search: %ld\n", sizeof(opCodes));

    len = 0xff;
    found = 0xff;

    for (k = 0; k < sizeof(opCodes) / sizeof(struct keywordLookup); k++) {
        u8 l;
        if (isKeyword(bin, k, &l)) {
            if ((found == 0xff) || (strlen(opCodes[k].keyword) > found)) {
                found = k;
                len = l;
            }
        }
    }

    if (found != 0xff) {
        // printf("return %d %d\n", found, len);

        if (opCodes[found].opcode != 0xe4) { // FN allow to have alphanumeric after
            u8 beg = bin[0];
            u8 car = bin[len];
            if ( (((beg >= 'a') & (beg <= 'z')) || ((beg >= 'A') & (beg <= 'Z'))) &&
                 (((car >= 'a') & (car <= 'z')) || ((car >= 'A') & (car <= 'Z')) || ((car >= '0') & (car <= '9'))) ) {
                return 0xff; // variable
            }
            if ( (beg == '&') &&
                 (((car >= 'a') & (car <= 'z')) || ((car >= 'A') & (car <= 'Z')) || ((car >= '0') & (car <= '9'))) ) {
                return 0xff; // Binary or hexa
            }
        }

        *keywordLen = len;
        memcpy(keyword, bin, len);
        keyword[len] = 0;

        return found;
    }

    return 0xff;

}     /* getNextKeyword */

char isVariable(char *keyword)
{
    int n = 0;

    ddlog(&gb, 2, "isVariable: %s\n", keyword);

    while (keyword[n] != 0) {
        u8 car = keyword[n];
        if (!(((car >= 'a') & (car <= 'z')) || ((car >= 'A') & (car <= 'Z'))) ) {
            return 0;
        }
        n++;
    }
    return 1;
}

void getLineNumber(u8 *bin, u8 *keyword, u8 *keywordLen)
{
    u8 n = 1;

    if ((bin[0] == 13) ||  (bin[0] == 10)) {
        *keywordLen = 0;
        keyword[0] = 0;
        return;
    }

    if (strchr(" :()=;%!$,*/<>+", bin[0])) {
        *keywordLen = 1;
        keyword[0] =  bin[0];
        keyword[1] =  0;
        return;
    }

    keyword[0] = bin[0];

    while ((!strchr(" :()=;%!$,*/<>+-", bin[n])) & (bin[n] != 13) & (bin[ n] != 10) & (n != 255)) {
        keyword[n] = bin[n];
        n++;
    }
    *keywordLen = n;
    keyword[n] = 0;

    // printf("Line: (%s,%d)\n", keyword, *keywordLen);

} /* getLineNumber */

int isInteger(u8 *str)
{
    int n;

    for (n = 0; n < strlen((char *)str); n++) {
        if ((str[n] < '0') || (str[n] > '9')) {
            return 0;
        }
    }
    return 1;
}

int isNumber010(u8 *str, u8 *b)
{
    if (isInteger(str)) {
        int i = atoi((char *)str);
        if ((i >= 0) && (i < 10)) {
            *b = (u8)i;
            return 1;
        }
    }
    return 0;
}

int isByte(u8 *str, u8 *b)
{
    if (isInteger(str)) {
        int i = atoi((char *)str);
        if ((i >= 0) && (i < 256)) {
            *b = (u8)i;
            return 1;
        }
    }
    return 0;
}

int isHexa(u8 *str, u16 *w)
{
    int pos = 0;
    int n;

    if (str[pos] != '&') {
        return 0;
    }

    pos++;
    if ( (str[pos] == 'H') || (str[pos] == 'h') ) {
        pos++;
    }

    u16 val = 0;

    for (n = 0; n < 4; n++) {
        char byte = str[pos + n];

        if (byte >= '0' && byte <= '9') {
            byte = byte - '0';
        } else if (byte >= 'a' && byte <= 'f') {
            byte = byte - 'a' + 10;
        } else if (byte >= 'A' && byte <= 'F') {
            byte = byte - 'A' + 10;
        } else {
            break;
        }

        val = (val << 4) | (byte & 0xF);
    }

    *w = val;

    return 1;
} /* isHexa */

int isWord(u8 *str, u16 *w)
{
    if (isInteger(str)) {
        int i = atoi((char *)str);
        if ((i >= 0) && (i < 65536)) {
            *w = (u16)i;
            return 1;
        }
    }
    return 0;
}

void makeAmsdosREAL(double v, char *dest, struct s_basic_info *debug)
{
    static unsigned char rc[5];
    int mesbits[32] = {0};   // must be reseted!
    int j, ib, ibb;
    int ibit = 0, exp = 0;
    // v2
    unsigned long mantissa;
    unsigned long deci;
    unsigned long mask;
    int isneg;

    memset(rc, 0, sizeof(rc));

    if (v < 0.0) {
        isneg = 1;
        v = -v;
    } else isneg = 0;

    // decimal hack
    deci = v;

    ddlog(&gb, 2, "Deci: %lu\n", deci);

    /*******************************************************************
    *                    values >= 1.0
    *******************************************************************/
    if (deci) {
        mask = 0x80000000;
        // find first significant bit of decimal part in order to get exponent value
        while (!(deci & mask)) mask = mask / 2;
        while (mask) {
            exp++;
            mask = mask / 2;
        }
        mantissa = v * (double)((long)1 << (32 - exp)) + 0.5; // 32 bits unsigned is the maximum value allowed  = pow(2.0, 32 - exp)


        ddlog(&gb, 2, "Double mantissa G is %g\n", v * (double)(1 << (32 - exp)) + 0.5);
        ddlog(&gb, 2, "Mantissa is %lu\n", mantissa);
        ddlog(&gb, 2, "v is %g\n", v);
        ddlog(&gb, 2, "32-exp is %g\n", (double)(1 << (32 - exp)));


        if (mantissa & 0xFF00000000L) mantissa = 0xFFFFFFFF;

        ddlog(&gb, 2, "decimal part has %d bits\n", exp);
        ddlog(&gb, 2, "32 bits mantissa is %lu\n", mantissa);

        mask = 0x80000000;
        while (mask) {
            mesbits[ibit] = !!(mantissa & mask);
            ibit++;
            mask = mask / 2;
        }
    } else {
        /*******************************************************************
        *                    negative exponent or zero
        *******************************************************************/
        /* handling zero special case */
        if (v == 0.0) {
            exp = -128;
        } else {
            mantissa = (v * 4294967296.0 + 0.5);       // as v is ALWAYS <1.0 we never reach the 32 bits maximum
            if (mantissa & 0xFF00000000L) mantissa = 0xFFFFFFFF;
            mask = 0x80000000;

            ddlog(&gb, 2, "32 bits mantissa for fraction is %lu\n", mantissa);

            // find first significant bit of fraction part
            while (!(mantissa & mask)) {
                mask = mask / 2;
                exp--;
            }

            mantissa = (v * (float)((long)1 << (32 - exp)) + 0.5);    // as v is ALWAYS <1.0 we never reach the 32 bits maximum
            if (mantissa & 0xFF00000000L) mantissa = 0xFFFFFFFF;
            mask = 0x80000000;

            while (mask) {
                mesbits[ibit] = !!(mantissa & mask);
                ibit++;
                mask = mask / 2;
            }
        }

        ddlog(&gb, 2, "\n%d bits used for mantissa\n", ibit);

    }

    /* pack bits */
    ib = 3; ibb = 0x80;
    for (j = 0; j < ibit; j++) {
        if (mesbits[j]) rc[ib] |= ibb;
        ibb /= 2;
        if (ibb == 0) {
            ibb = 0x80;
            ib--;
        }
    }
    /* exponent */
    exp += 128;
    if (exp < 0 || exp > 255) {
        addError(debug, "Exponent overflow");
        exp = 128;
    }
    rc[4] = exp;

    /* REAL sign replace the most significant implied bit */
    if (!isneg) {
        rc[3] &= 0x7F;
    } else {
        rc[3] |= 0x80;
    }

    for (j = 0; j < 5; j++) {
        ddlog(&gb, 2, "%02X ", rc[j]);
    }
    ddlog(&gb, 2, "\n------------------\n");

    memcpy(dest, rc, 5);
} /* makeAmsdosREAL */


int isDouble(u8 *str, double *f)
{
    int n;

    for (n = 0; n < strlen((char *)str); n++) {
        if ( ((str[n] < '0') || (str[n] > '9')) && (str[n] != '.') ) {
            return 0;
        }
    }

    *f = atof((char *)str);

    return 1;
}

// https://www.cpcwiki.eu/index.php/Technical_information_about_Locomotive_BASIC

// struct k_lineidx {
//     u16 line;
//     u16 adr;
// };

static void addError(struct s_basic_info *debug, char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(debug->error.msg, 256, fmt, ap);
    va_end(ap);

    ddlog(&gb, 2, "AddError %s\n", debug->error.msg);

    debug->err = 1;
}

int tokenizeBasic(u8 *text, u8 *bin, u16 *finallength, struct s_basic_info **userDebug)
{
    struct s_basic_info *debug, localDebug;

    int begpos, pos = 0;
    int len = 0;
    int lineAddr;
    u8 n;

    u8 ignoreComment = 1;

    if (userDebug != NULL) {
        *userDebug = malloc(sizeof(struct s_basic_info));
        debug = *userDebug;
    } else {
        debug = &localDebug;
    }
    debug->error.line = 1;
    debug->err = 0;

    do {
        u8 continueWithComment = 0;
        u8 continueWithString = 0;
        u8 nextIsLineNumber = 0;
        u8 continueWithData = 0;
        u8 lastVariable = 0;

        lineAddr = len;
        len += 2;

        begpos = pos;

        u8 keywordLen;
        u8 keyword[256];

        getLineNumber(text + pos, keyword, &keywordLen);

        pos += keywordLen + 1;
        if (keywordLen == 0) {

            len -= 2;
            ddlog(&gb, 2, "Empty line %04x length: %04x-%04x=%04x\n", len + 0x170, (u16)len + 0x170, lineAddr + 0x170, (u16)len - lineAddr);

            break;
        }

        int currentLine = atoi((char *)keyword);

        if ((currentLine == 0) || (currentLine > 65535)) {
            addError(debug, "Incorrect line number: %s", keyword);
            break;
        }

        putWord(bin, len, (u16)currentLine);
        len += 2;

        ddlog(&gb, 2, "** Line: %d (at 0x%04x)\n", currentLine, len + 0x170 - 4);

        do {

            if ( (!continueWithComment) && (!continueWithString)  && (!continueWithData) ) {
                u8 token, tokenLine;

                keyword[0] = 0;

                tokenLine = getNextKeyword(text + pos, keyword, &keywordLen);
                token = (tokenLine == 0xff) ? 0xff : opCodes[tokenLine].opcode;

                ddlog(&gb, 2, "Keyword: (%s), 0x%02x at 0x%04x\n", keyword, token, len + 0x170);

                if (token != 0xff) {
                    if (keywordLen == 0) {
                        exit(1);
                    }

                    if ((token == 0xc0) || (token == 0x97)) {          // Add : before ' and else if not already set
                        if (bin[len - 1] != 0x01) {
                            putByte(bin, len, 0x01);
                            len++;
                        }
                    }

                    if (opCodes[tokenLine].keyword[0] == ':') {
                        putByte(bin, len, 0x01);
                        len++;

                        nextIsLineNumber = 0;  // Reset line number

                    } else if (token < 0x80) {
                        putByte(bin, len, 0xff);
                        len++;
                        putByte(bin, len, token);
                        len++;
                    } else {
                        putByte(bin, len, token);
                        len++;
                    }

                    if ((token == 0x9f) || (token == 0xa0) || (token == 0xc7)) {  // gosub, goto
                        nextIsLineNumber = 1;
                    }

                    if ((token == 0x8c) || (token == 0x8e) || (token == 0x90) || (token == 0x8f)) {  // data, defint, defstr, defreal
                        continueWithData = 1;
                    }

                    if ((token == 0xC0) || (token == 0xC5)) {     // We have a comment. Don't need to handle the rest until CR
                        continueWithComment = 1;
                    }

                    pos += keywordLen;

                    lastVariable = token;
                } else {
                    u16 w;
                    u8 b;
                    double f;
                    u8 found = 0;

                    getLineNumber(text + pos, keyword, &keywordLen);
                    ddlog(&gb, 2, "Variable ? (%s), %d, at 0x%04x\n", keyword, keywordLen, len + 0x170);

                    if (keywordLen != 0) {

                        if ((nextIsLineNumber) || (lastVariable == 0x97) || (lastVariable == 0xeb)) {  // Or number after ELSE or THEN
                            u16 w;

                            if (isWord(keyword, &w)) {

                                putByte(bin, len, 0x1E);
                                len++;

                                putWord(bin, len, w);
                                len += 2;

                                pos += keywordLen;
                                found = 1;
                            }

                        } else {

                            // if ((!found) && (isNumber010(keyword, &b))) {

                            //     printf("Found 0-10\n");

                            //     putByte(bin, len, b + 0x0e);
                            //     len++;

                            //     pos += keywordLen;
                            //     found = 1;
                            // }


                            if ((!found) && (isHexa(keyword, &w))) {
                                putByte(bin, len, 0x1c);
                                len++;
                                putWord(bin, len, w);
                                len += 2;

                                pos += keywordLen;
                                found = 1;
                            }

                            if ((!found) && (isByte(keyword, &b))) {
                                putByte(bin, len, 0x19);
                                len++;
                                putByte(bin, len, b);
                                len++;

                                pos += keywordLen;
                                found = 1;
                            }

                            if ((!found) && (isWord(keyword, &w))) {
                                ddlog(&gb, 2, "Print word %d at %04x\n", w, len + 0x170);
                                putByte(bin, len, 0x1A);
                                len++;
                                putWord(bin, len, w);
                                len += 2;

                                pos += keywordLen;
                                found = 1;
                            }

                            if ((!found) && (isDouble(keyword, &f))) {
                                ddlog(&gb, 2, "Print double %f at %04x\n", f, len + 0x170);

                                putByte(bin, len, 0x1F);
                                len++;

                                makeAmsdosREAL(f, (char *)bin + len, debug);
                                len += 5;

                                pos += keywordLen;
                                found = 1;
                            }

                        }

                        if (!found) {
                            if ( (isVariable((char *)keyword)) || (strchr("%!$",  text[pos + keywordLen])) ) {
                                switch (text[pos + keywordLen]) {
                                    case '%':
                                        putByte(bin, len, 0x02);
                                        len++;
                                        pos++;
                                        break;
                                    case '$':
                                        putByte(bin, len, 0x03);
                                        len++;
                                        pos++;
                                        break;
                                    case '!':
                                        putByte(bin, len, 0x04);
                                        len++;
                                        pos++;
                                        break;
                                    default:
                                        putByte(bin, len, 0x0D);
                                        len++;
                                        break;
                                }  /* switch */

                                putByte(bin, len, 0);
                                len++;
                                putByte(bin, len, 0);
                                len++;

                                for (n = 0; n < strlen((char *)keyword); n++) {
                                    u8 car = keyword[n];
                                    if (n == strlen((char *)keyword) - 1) {
                                        car += 0x80;
                                    }
                                    putByte(bin, len, car);
                                    len++;
                                }

                                pos += keywordLen;
                                found = 1;
                            }

                        }

                    }

                    if (!found) {
                        if (text[pos] == 0x22) {
                            continueWithString = 1;
                        }
                        putByte(bin, len, text[pos]);
                        len++;
                        pos++;
                    }
                }
            } else {
                u8 car = text[pos];

                if ((!continueWithString) && (continueWithData) && (text[pos] == ':')) {
                    continueWithData = 0;
                    car = 0x01;
                } else if ( (text[pos] == 0x22) && (!continueWithComment)) {
                    continueWithString = !continueWithString;
                }

                if (!((continueWithComment) && (ignoreComment))) {
                    putByte(bin, len, car);
                    len++;
                }
                pos++;
            }

        }while ((text[pos] != 13) && (text[pos] != 10));

        if (pos - begpos >= 256) {
            addError(debug, "Line too long");
            break;
        }

        if (text[pos] == 13) {
            pos++;
        }
        if (text[pos] == 10) {
            pos++;
        }

        putByte(bin, len, 0);
        len++;

        ddlog(&gb, 2, "** End of line (beg:%04x end:%04x len:%04x)\n\n", lineAddr + 0x170, (u16)len + 0x170, (u16)len - lineAddr);

        if (debug->err != 0) {
            ddlog(&gb, 2, "Err: %d\n", debug->err);
            break;
        }

        debug->error.line++;

        putWord(bin, lineAddr, (u16)len - lineAddr);

    }     /* tokenizeBasic */
    while (text[pos] != 0);

    ddlog(&gb, 2, "End: %04x\n", len + 0x170);

    putByte(bin, len, 0);
    len++;
    putByte(bin, len, 0);
    len++;


    *finallength = len;

    return debug->err;
}     /* tokenizeBasic */

/// @brief
/// @param dsk
/// @param dsk_size
/// @param length
/// @return buffer with clean basic to clean
u8 * clean_basic(u8 *dsk, long dsk_size, u16 *length)
{
    u8 *bas = (u8 *)malloc(128000);
    u16 n, m = 0;

    for (n = 0; n < dsk_size; n++) {
        if (dsk[n] == 0x0a) {
            bas[m] = 0x0d;
            m++;
            bas[m] = 0x0a;
            m++;
        } else if (dsk[n] == 0xc2) {
            n++;
            if (dsk[n] == 0xb0) {         // °
                bas[m] = 0x82;
                m++;
            }
        } else if (dsk[n] == 0xc3) {
            n++;
            if (dsk[n] == 0xa0) {         // à
                bas[m] = 0x40;
                m++;
            } else if (dsk[n] == 0xa7) {  // ç
                bas[m] = 0x5c;
                m++;
            } else if (dsk[n] == 0xa9) {  // é
                bas[m] = 0x7B;
                m++;
            } else if (dsk[n] == 0xb9) {  // ù
                bas[m] = 0x7c;
                m++;
            } else if (dsk[n] == 0xa8) {  // è
                bas[m] = 0x7d;
                m++;
            }
        } else if (dsk[n] == 0x0d) {
        } else {
            bas[m] = dsk[n];
            m++;
        }
    }
    bas[m] = 0x0d;
    m++;
    bas[m] = 0x0a;
    m++;

    *length = m;

    bas[m] = 0;
    m++;


    return bas;
}  /* clean_basic */

#ifdef STANDALONE

void untokenize(void)
{
    u16 i;
    FILE *f;
    u16 adr = 0;
    char s[80];
    int length;

    u8 basicbin[CODESIZE];

    f = fopen("BEWITCHB.BAS", "rb");
    if (!f) return;
    length = fread((char *)basicbin, 1, CODESIZE, f);
    fclose(f);

    if (basicbin[0x12] == 1) {     // Unprotect the code
        for (i = 0x80; i < length; i++) {
            basicbin[i] = basicbin[i] ^ DproBasic[i & 0x7f];
        }
    }

    untokenizeBasic((u8 *)basicbin + 128, (char *)Listing);

    printf("%s\n", Listing);
}     /* untokenize */

void tokenize(void)
{
    u16 i;
    FILE *f;
    u16 adr = 0;
    char s[80];
    int length;

    u8 basictext[CODESIZE];
    u8 basicbin[CODESIZE];

    f = fopen("/Users/miguelvanhove/Downloads/crocods/miguel.bas", "rb");


    // f = fopen("BEWITCH.BAS", "rb");
    if (!f) return;
    length = fread((char *)basictext, 1, CODESIZE, f);
    fclose(f);

    int len = tokenizeBasic((u8 *)basictext, (u8 *)basicbin);


    int x, y;
    u8 stop = 0;

    y = 0;
    do {
        printf("%08x  ", 0x80 + y * 16);     // 00000080

        for (x = 0; x  < 8; x++) {
            if (x + y * 16 >= len) {
                break;
            }
            printf("%02x ", basicbin[x + y * 16]);
        }
        printf(" ");

        for (x = 8; x  < 16; x++) {
            if (x + y * 16 >= len) {
                break;
            }
            printf("%02x ", basicbin[x + y * 16]);
        }

        printf(" |");

        for (x = 0; x  < 16; x++) {
            if (x + y * 16 >= len) {
                stop = 1;
                break;
            }
            u8 c = basicbin[ x + y * 16];
            printf("%c", ((c >= 32) && (c < 128)) ? c : '.');
        }

        printf("|\n");
        y++;

    }while (!stop);

    printf("Len: %d\n", len);
}     /* tokenize */

int main(void)
{
    // untokenize();

    tokenize();

}     /* main */

#endif /* ifdef STANDALONE */