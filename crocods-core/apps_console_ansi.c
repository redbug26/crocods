#include "apps_console_internal.h"

#define MAXARGLEN 128

// https://en.wikipedia.org/wiki/ANSI_escape_code
u16 ansi_palette[16]; // RGB565

int maxHeight;

int maxX;
int maxY;          // size of ansi output window

int maxScroll;

int tabSpaces;

#define DEFAULTANSIBACK  0
#define DEFAULTANSIFRONT 11

void initAnsi(void)
{
    // CW.x = 0;
    // CW.y = 0;

    maxHeight = 0;

    maxX = CW.width;
    maxY = CW.height; // size of ansi output window

    maxScroll = CW.height; // 500;

    tabSpaces = 8;

    ansi_palette[0] = RGB565(0, 0, 0);
    ansi_palette[1] = RGB565(127, 0, 0);
    ansi_palette[2] = RGB565(0, 127, 0);
    ansi_palette[3] = RGB565(127, 127, 0);
    ansi_palette[4] = RGB565(0, 0, 127);
    ansi_palette[5] = RGB565(127, 0, 127);
    ansi_palette[6] = RGB565(0, 127, 127);
    ansi_palette[7] = RGB565(192, 192, 192);
    ansi_palette[8] = RGB565(127, 127, 127);
    ansi_palette[9] = RGB565(255, 0, 0);
    ansi_palette[10] = RGB565(0, 255, 0);
    ansi_palette[11] = RGB565(255, 255, 0);
    ansi_palette[12] = RGB565(0, 0, 255);
    ansi_palette[13] = RGB565(255, 0, 255);
    ansi_palette[14] = RGB565(0, 255, 255);
    ansi_palette[15] = RGB565(255, 255, 255);

} /* initAnsi */

void setColors(char *argbuf, int arglen)
{
    char *p, *pp;
    u16 tmp;

    // char mm[32];
    // memcpy(mm, argbuf, arglen);
    // mm[arglen] = 0;
    // printf("Color:(%s)\n", mm);

    if (argbuf[0] == 0) {
        CW.colorBack = ansi_palette[DEFAULTANSIBACK];
        CW.colorFront = ansi_palette[DEFAULTANSIFRONT];

        // printf("setcolors Back/front %d,%d\n", CW.colorBack, CW.colorFront);

        return;
    }

    if (*argbuf && arglen) {
        pp = argbuf;
        do{
            p = strchr(pp, ';');
            if (p && *p) {
                *p = 0;
                p++;
            }
            // printf("(%d)\n", atoi(pp));
            switch (atoi(pp)) {
                case 0:     // --- all attributes off ---------------------------
                    CW.colorBack = ansi_palette[0]; // background
                    CW.colorFront = ansi_palette[7]; // whiteColor
                    // printf("0 colorFront: %d\n", CW.colorFront);
                    // printf("0 ColorBack: %d\n", CW.colorBack);
                    break;

                case 1:     // --- Bold On --------------------------------------
                    break;

                case 2:     // --- Dim On ---------------------------------------
                    break;

                case 3:     // --- Italic On ------------------------------------
                    break;

                case 4:     // --- Underline On ---------------------------------
                    break;

                case 5:     // --- Blink On -------------------------------------
                    break;

                case 6:     // --- Rapid Blink On -------------------------------
                    break;

                case 7:     // --- Reverse Video On -----------------------------
                    tmp = CW.colorBack;
                    CW.colorBack = CW.colorFront;
                    CW.colorFront = tmp;
                    // printf("7 colorFront: %d\n", CW.colorFront);
                    // printf("7 colorBack: %d\n", CW.colorBack);

                    break;

                case 8:     // --- Invisible On ---------------------------------
                    break;

                case 30:    // --- black fg ------------------------------------
                case 31:    // --- red fg --------------------------------------
                case 32:    // --- green fg ------------------------------------
                case 33:    // --- yellow fg -----------------------------------
                case 34:    // --- blue fg -------------------------------------
                case 35:    // --- magenta fg ----------------------------------
                case 36:    // --- cyan fg -------------------------------------
                case 37:    // --- white fg ------------------------------------
                    CW.colorFront = ansi_palette[atoi(pp) - 30];
                    // printf("colorFront: %d\n", CW.colorFront);
                    break;

                case 38:
                    break;

                case 39: // Default foreground color
                    CW.colorFront = ansi_palette[DEFAULTANSIFRONT];                  // lightGrayColor
                    // printf("colorFront: %d\n", CW.colorFront);
                    break;

                case 40:     // --- black bg ------------------------------------
                case 41:     // --- red bg --------------------------------------
                case 42:     // --- green bg ------------------------------------
                case 43:     // --- yellow bg -----------------------------------
                case 44:     // --- blue bg -------------------------------------
                case 45:     // --- magenta bg ----------------------------------
                case 46:     // --- cyan bg -------------------------------------
                case 47:     // --- white bg ------------------------------------
                    // curAttrBackgroundColor = [self getColorForAnsiColor:[colors[i] integerValue] - 40];
                    CW.colorBack = ansi_palette[atoi(pp) - 40];
                    // printf("ColorBack: %d\n", CW.colorBack);
                    break;

                case 48:
                    break;

                case 49:     // --- Default background color ------------------------------
                    CW.colorBack = ansi_palette[DEFAULTANSIBACK];                     // blackColor
                    // printf("ColorBack: %d\n", CW.colorBack);
                    break;

                case 90:     // --- black bg ------------------------------------
                case 91:     // --- red bg --------------------------------------
                case 92:     // --- green bg ------------------------------------
                case 93:     // --- yellow bg -----------------------------------
                case 94:     // --- blue bg -------------------------------------
                case 95:     // --- magenta bg ----------------------------------
                case 96:     // --- cyan bg -------------------------------------
                case 97:     // --- white bg ------------------------------------
                    CW.colorFront = ansi_palette[atoi(pp) - 90 + 8];
                    // printf("colorFront: %d\n", CW.colorFront);
                    break;

                case 100:    // --- black bg ------------------------------------
                case 101:    // --- red bg --------------------------------------
                case 102:    // --- green bg ------------------------------------
                case 103:    // --- yellow bg -----------------------------------
                case 104:    // --- blue bg -------------------------------------
                case 105:    // --- magenta bg ----------------------------------
                case 106:    // --- cyan bg -------------------------------------
                case 107:    // --- white bg ------------------------------------
                    CW.colorBack = ansi_palette[atoi(pp) - 100 + 8];
                    // printf("colorBack: %d\n", CW.colorBack);
                    break;

                default:     // --- unsupported ---------------------------------
                    break;
            } /* switch */
            pp = p;
        }while (p);
    }
} /* set_colors */


void ansiOut(u8 b)
{

    static int arglen = 0, ansistate = 0, x;
    static char argbuf[MAXARGLEN] = "";

    switch (ansistate) {
        case 0:
            switch (b) {
                case 27:
                    ansistate = 1;
                    break;
                /*			  case '\r':
                 * CW.x = 0;
                 * break;
                 *
                 * case '\n':
                 * CW.y++;
                 * break;
                 */

                case '\n':
                    CW.x = 0;
                    newLine();

                    break;

                case '\r':
                    CW.x = 0;
                    break;

                case '\t': {
                    int toDo;         //  = maxX;

                    toDo = tabSpaces - (CW.x % tabSpaces);

                    for (x = 0; x < toDo; x++) {
                        putOnText((CW.x + CW.fromX), (CW.y + CW.fromY), ' ', CW.colorBack, CW.colorFront);
                        CW.x++;
                        if (CW.x > maxX - 1) {
                            CW.x = 0;
                            newLine();
                        }
                    }
                    break;
                }
                case '\b':
                    if (CW.x)
                        CW.x--;
                    break;

                case '\07':                  // The beep -------------------
                    // AudioServicesPlaySystemSound(1053);

                    break;

                default:
                    if (CW.x > maxX - 1) {
                        CW.x = 0;
                        newLine();
                    }

                    putOnText( (CW.x + CW.fromX), (CW.y + CW.fromY), b, CW.colorBack, CW.colorFront);
                    CW.x++;

                    break;
            }         /* switch */
            break;

        case 1:
            if (b == '[') {
                ansistate = 2;
                arglen = 0;
                *argbuf = 0;
                break;
            }
            ansistate = 0;
            break;

        case 2:
            if (strchr("HFABCDnsuJKLmpPM@hl", (int)b)) { // ansiTerminators

                switch ((int)b) {
                    case 'H':        // --- set cursor position - cursorhome
                    case 'F':        // Moves cursor to beginning of the line n
                    {
                        int y, x;
                        char *p;

                        if (!*argbuf || !arglen) {
                            CW.x = 0;
                            CW.y = 0;
                        }

                        y = atoi(argbuf) - 1;
                        p = strchr(argbuf, ';');

                        if ((y >= 0) & (p != NULL)) {
                            x = atoi(p + 1) - 1;
                            if (x >= 0) {
                                CW.x = x;
                                CW.y = y;
                            }
                        }

                        // printf("goto:%d,%d\n", CW.x, CW.y);
                    }
                    break;

                    case 'A':                            // --- up -------------------------------------
                    {
                        int cnt;

                        if (arglen == 0) {
                            cnt = 1;
                        } else {
                            cnt = atoi(argbuf);
                        }
                        CW.y -= cnt;
                    }
                        // printf("goUp\n");
                        // [self goUp:argbuf];
                        break;

                    case 'B':                            // --- down -----------------------------------
                    {
                        int cnt;

                        if (arglen == 0) {
                            cnt = 1;
                        } else {
                            cnt = atoi(argbuf);
                        }
                        CW.y += cnt;
                    }   // printf("goDown\n");
// [self goDown:argbuf];
break;

                    case 'C':                            // --- right ----------------------------------
                    {
                        int cnt;

                        if (arglen == 0) {
                            cnt = 1;
                        } else {
                            cnt = atoi(argbuf);
                        }
                        CW.x += cnt;
                    } // printf("goRight\n");
// [self goRight:argbuf];
break;

                    case 'D':                            // --- left -----------------------------------
                    {
                        int cnt;

                        if (arglen == 0) {
                            cnt = 1;
                        } else {
                            cnt = atoi(argbuf);
                        }
                        CW.x -= cnt;
                    }
                        // printf("goLeft\n");
                        // [self goLeft:argbuf];
                        break;

                    case 's':                            // --- save pos -------------------------------
                        // [self savePos];
                        break;

                    case 'u':                            // --- restore pos ----------------------------
                        // [self restorePos];
                        break;

                    case 'J':                            // --- clear screen ---------------------------
                        CW.x = CW.y = 0;

                        CW.colorBack = ansi_palette[0]; // background
                        CW.colorFront = ansi_palette[7]; // whiteColor

                        cls(CW.colorBack);

                        // printf("cls\n");

                        // [self clearScreen];
                        break;

                    case 'K':         // --- delete to eol --------------------------
                        // [self clearToEnd:argbuf withLen: arglen];
                    {

                        int type;

                        if (arglen == 0) {
                            type = 0;
                        } else {
                            type = atoi(argbuf);
                        }

                        if (type == 0) {
                            for (int x = CW.x; x < maxX; x++) {
                                putOnText( (x + CW.fromX), (CW.y + CW.fromY), ' ', CW.colorBack, CW.colorFront);
                            }
                        }
                    }
                    break;

                    case 'M':         // --- set video attribs ----------------------
                        // [self deleteLines:argbuf withLen: arglen];
                        break;

                    case 'h':
                        fk_displaycursor( (CW.x + CW.fromX) * 8, (CW.y + CW.fromY) * 8, 1);
                        // printf("h\n");  // CSI ? 25 h - Shows the cursor
                        break;

                    case 'l':
                        // printf("l\n");  // CSI ? 25 l - Hides the cursor
                        break;

                    case 'L': // --- set video attribs ----------------------
                        // [self insertLines:argbuf withLen: arglen];
                        break;

                    case 'm': // --- set video attribs ----------------------
                        setColors(argbuf, arglen);
                        // [self setColors:argbuf withLen: arglen];
                        break;

                    case 'P': // --- backspace ----------------------
                        // [self backspace:argbuf];
                        break;

                    case '@': // --- backspace ----------------------
                        // [self insertChar:argbuf];
                        break;

                    //                    case 'p':         //--- keyboard redef -------------------------
                    //                    case 'n':         //--- device statusreport pos - https://ispltd.org/mini_howto:ansi_terminal_codes

                    default:  // --- unsupported ----------------------------
                        // printf("ERROR ! (e[%s%c)\n", argbuf, b);
                        break;
                }             /* switch */
                ansistate = 0;
                arglen = 0;
                *argbuf = 0;
            } else {
                if (arglen < MAXARGLEN) {
                    argbuf[arglen] = b;
                    argbuf[arglen + 1] = 0;
                    arglen++;
                }
            }
            break;

        default:
            // printf("Error of ANSI code\n");
            break;
    }     /* switch */
}         /* ansiOut */

static int first = 1;

// void ansiSave(void)
// {
//     first = 1;
// }

void ansiToScreen(char *b, int len)
{

    CW.colorBack = ansi_palette[DEFAULTANSIBACK];
    CW.colorFront = ansi_palette[DEFAULTANSIFRONT];

    if (first) {
        FILE *fic = fopen("ansiout.txt", "wb");
        fwrite(b, len, 1, fic);
        fclose(fic);
        first = 0;
    }

    int n;

    for (n = 0; n < len; n++) {
        ansiOut((u8)b[n]);
    }

    // exit(1);
} /* ansiToScreen */
