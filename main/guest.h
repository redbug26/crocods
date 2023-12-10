#include "shared.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef enum {
    USBHID_ID_UNKNOWN = 0,

    /**
     *  \name Usage page 0x07
     *
     *  These values are from usage page 0x07 (USB keyboard page).
     */
    /* @{ */

    USBHID_ID_A              = 4,
    USBHID_ID_B              = 5,
    USBHID_ID_C              = 6,
    USBHID_ID_D              = 7,
    USBHID_ID_E              = 8,
    USBHID_ID_F              = 9,
    USBHID_ID_G              = 10,
    USBHID_ID_H              = 11,
    USBHID_ID_I              = 12,
    USBHID_ID_J              = 13,
    USBHID_ID_K              = 14,
    USBHID_ID_L              = 15,
    USBHID_ID_M              = 16,
    USBHID_ID_N              = 17,
    USBHID_ID_O              = 18,
    USBHID_ID_P              = 19,
    USBHID_ID_Q              = 20,
    USBHID_ID_R              = 21,
    USBHID_ID_S              = 22,
    USBHID_ID_T              = 23,
    USBHID_ID_U              = 24,
    USBHID_ID_V              = 25,
    USBHID_ID_W              = 26,
    USBHID_ID_X              = 27,
    USBHID_ID_Y              = 28,
    USBHID_ID_Z              = 29,

    USBHID_ID_1              = 30,
    USBHID_ID_2              = 31,
    USBHID_ID_3              = 32,
    USBHID_ID_4              = 33,
    USBHID_ID_5              = 34,
    USBHID_ID_6              = 35,
    USBHID_ID_7              = 36,
    USBHID_ID_8              = 37,
    USBHID_ID_9              = 38,
    USBHID_ID_0              = 39,

    USBHID_ID_RETURN         = 40,
    USBHID_ID_ESCAPE         = 41,
    USBHID_ID_BACKSPACE      = 42,
    USBHID_ID_TAB            = 43,
    USBHID_ID_SPACE          = 44,

    USBHID_ID_MINUS          = 45,
    USBHID_ID_EQUALS         = 46,
    USBHID_ID_LEFTBRACKET    = 47,
    USBHID_ID_RIGHTBRACKET   = 48,
    USBHID_ID_BACKSLASH      = 49, /**< Located at the lower left of the return
                                    *   key on ISO keyboards and at the right end
                                    *   of the QWERTY row on ANSI keyboards.
                                    *   Produces REVERSE SOLIDUS (backslash) and
                                    *   VERTICAL LINE in a US layout, REVERSE
                                    *   SOLIDUS and VERTICAL LINE in a UK Mac
                                    *   layout, NUMBER SIGN and TILDE in a UK
                                    *   Windows layout, DOLLAR SIGN and POUND SIGN
                                    *   in a Swiss German layout, NUMBER SIGN and
                                    *   APOSTROPHE in a German layout, GRAVE
                                    *   ACCENT and POUND SIGN in a French Mac
                                    *   layout, and ASTERISK and MICRO SIGN in a
                                    *   French Windows layout.
                                    */
    USBHID_ID_NONUSHASH      = 50, /**< ISO USB keyboards actually use this code
                                    *   instead of 49 for the same key, but all
                                    *   OSes I've seen treat the two codes
                                    *   identically. So, as an implementor, unless
                                    *   your keyboard generates both of those
                                    *   codes and your OS treats them differently,
                                    *   you should generate USBHID_ID_BACKSLASH
                                    *   instead of this code. As a user, you
                                    *   should not rely on this code because SDL
                                    *   will never generate it with most (all?)
                                    *   keyboards.
                                    */
    USBHID_ID_SEMICOLON      = 51,
    USBHID_ID_APOSTROPHE     = 52,
    USBHID_ID_GRAVE          = 53, /**< Located in the top left corner (on both ANSI
                                    *   and ISO keyboards). Produces GRAVE ACCENT and
                                    *   TILDE in a US Windows layout and in US and UK
                                    *   Mac layouts on ANSI keyboards, GRAVE ACCENT
                                    *   and NOT SIGN in a UK Windows layout, SECTION
                                    *   SIGN and PLUS-MINUS SIGN in US and UK Mac
                                    *   layouts on ISO keyboards, SECTION SIGN and
                                    *   DEGREE SIGN in a Swiss German layout (Mac:
                                    *   only on ISO keyboards), CIRCUMFLEX ACCENT and
                                    *   DEGREE SIGN in a German layout (Mac: only on
                                    *   ISO keyboards), SUPERSCRIPT TWO and TILDE in a
                                    *   French Windows layout, COMMERCIAL AT and
                                    *   NUMBER SIGN in a French Mac layout on ISO
                                    *   keyboards, and LESS-THAN SIGN and GREATER-THAN
                                    *   SIGN in a Swiss German, German, or French Mac
                                    *   layout on ANSI keyboards.
                                    */
    USBHID_ID_COMMA          = 54,
    USBHID_ID_PERIOD         = 55,
    USBHID_ID_SLASH          = 56,

    USBHID_ID_CAPSLOCK       = 57,

    USBHID_ID_F1             = 58,
    USBHID_ID_F2             = 59,
    USBHID_ID_F3             = 60,
    USBHID_ID_F4             = 61,
    USBHID_ID_F5             = 62,
    USBHID_ID_F6             = 63,
    USBHID_ID_F7             = 64,
    USBHID_ID_F8             = 65,
    USBHID_ID_F9             = 66,
    USBHID_ID_F10            = 67,
    USBHID_ID_F11            = 68,
    USBHID_ID_F12            = 69,

    USBHID_ID_PRINTSCREEN    = 70,
    USBHID_ID_SCROLLLOCK     = 71,
    USBHID_ID_PAUSE          = 72,
    USBHID_ID_INSERT         = 73, /**< insert on PC, help on some Mac keyboards (but
                                    * does send code 73, not 117) */
    USBHID_ID_HOME           = 74,
    USBHID_ID_PAGEUP         = 75,
    USBHID_ID_DELETE         = 76,
    USBHID_ID_END            = 77,
    USBHID_ID_PAGEDOWN       = 78,
    USBHID_ID_RIGHT          = 79,
    USBHID_ID_LEFT           = 80,
    USBHID_ID_DOWN           = 81,
    USBHID_ID_UP             = 82,

    USBHID_ID_NUMLOCKCLEAR   = 83, /**< num lock on PC, clear on Mac keyboards
                                    */
    USBHID_ID_KP_DIVIDE      = 84,
    USBHID_ID_KP_MULTIPLY    = 85,
    USBHID_ID_KP_MINUS       = 86,
    USBHID_ID_KP_PLUS        = 87,
    USBHID_ID_KP_ENTER       = 88,
    USBHID_ID_KP_1           = 89,
    USBHID_ID_KP_2           = 90,
    USBHID_ID_KP_3           = 91,
    USBHID_ID_KP_4           = 92,
    USBHID_ID_KP_5           = 93,
    USBHID_ID_KP_6           = 94,
    USBHID_ID_KP_7           = 95,
    USBHID_ID_KP_8           = 96,
    USBHID_ID_KP_9           = 97,
    USBHID_ID_KP_0           = 98,
    USBHID_ID_KP_PERIOD      = 99,

    USBHID_ID_NONUSBACKSLASH = 100, /**< This is the additional key that ISO
                                     *   keyboards have over ANSI ones,
                                     *   located between left shift and Y.
                                     *   Produces GRAVE ACCENT and TILDE in a
                                     *   US or UK Mac layout, REVERSE SOLIDUS
                                     *   (backslash) and VERTICAL LINE in a
                                     *   US or UK Windows layout, and
                                     *   LESS-THAN SIGN and GREATER-THAN SIGN
                                     *   in a Swiss German, German, or French
                                     *   layout. */
    USBHID_ID_APPLICATION    = 101, /**< windows contextual menu, compose */
    USBHID_ID_POWER          = 102, /**< The USB document says this is a status flag,
                                     *   not a physical key - but some Mac keyboards
                                     *   do have a power key. */
    USBHID_ID_KP_EQUALS      = 103,
    USBHID_ID_F13            = 104,
    USBHID_ID_F14            = 105,
    USBHID_ID_F15            = 106,
    USBHID_ID_F16            = 107,
    USBHID_ID_F17            = 108,
    USBHID_ID_F18            = 109,
    USBHID_ID_F19            = 110,
    USBHID_ID_F20            = 111,
    USBHID_ID_F21            = 112,
    USBHID_ID_F22            = 113,
    USBHID_ID_F23            = 114,
    USBHID_ID_F24            = 115,
    USBHID_ID_EXECUTE        = 116,
    USBHID_ID_HELP           = 117,
    USBHID_ID_MENU           = 118,
    USBHID_ID_SELECT         = 119,
    USBHID_ID_STOP           = 120,
    USBHID_ID_AGAIN          = 121, /**< redo */
    USBHID_ID_UNDO           = 122,
    USBHID_ID_CUT            = 123,
    USBHID_ID_COPY           = 124,
    USBHID_ID_PASTE          = 125,
    USBHID_ID_FIND           = 126,
    USBHID_ID_MUTE           = 127,
    USBHID_ID_VOLUMEUP       = 128,
    USBHID_ID_VOLUMEDOWN     = 129,
/* not sure whether there's a reason to enable these */
/*     USBHID_ID_LOCKINGCAPSLOCK = 130,  */
/*     USBHID_ID_LOCKINGNUMLOCK = 131, */
/*     USBHID_ID_LOCKINGSCROLLLOCK = 132, */
    USBHID_ID_KP_COMMA           = 133,
    USBHID_ID_KP_EQUALSAS400     = 134,

    USBHID_ID_INTERNATIONAL1     = 135, /**< used on Asian keyboards, see
                                         *  footnotes in USB doc */
    USBHID_ID_INTERNATIONAL2     = 136,
    USBHID_ID_INTERNATIONAL3     = 137, /**< Yen */
    USBHID_ID_INTERNATIONAL4     = 138,
    USBHID_ID_INTERNATIONAL5     = 139,
    USBHID_ID_INTERNATIONAL6     = 140,
    USBHID_ID_INTERNATIONAL7     = 141,
    USBHID_ID_INTERNATIONAL8     = 142,
    USBHID_ID_INTERNATIONAL9     = 143,
    USBHID_ID_LANG1              = 144, /**< Hangul/English toggle */
    USBHID_ID_LANG2              = 145, /**< Hanja conversion */
    USBHID_ID_LANG3              = 146, /**< Katakana */
    USBHID_ID_LANG4              = 147, /**< Hiragana */
    USBHID_ID_LANG5              = 148, /**< Zenkaku/Hankaku */
    USBHID_ID_LANG6              = 149, /**< reserved */
    USBHID_ID_LANG7              = 150, /**< reserved */
    USBHID_ID_LANG8              = 151, /**< reserved */
    USBHID_ID_LANG9              = 152, /**< reserved */

    USBHID_ID_ALTERASE           = 153, /**< Erase-Eaze */
    USBHID_ID_SYSREQ             = 154,
    USBHID_ID_CANCEL             = 155,
    USBHID_ID_CLEAR              = 156,
    USBHID_ID_PRIOR              = 157,
    USBHID_ID_RETURN2            = 158,
    USBHID_ID_SEPARATOR          = 159,
    USBHID_ID_OUT                = 160,
    USBHID_ID_OPER               = 161,
    USBHID_ID_CLEARAGAIN         = 162,
    USBHID_ID_CRSEL              = 163,
    USBHID_ID_EXSEL              = 164,

    USBHID_ID_KP_00              = 176,
    USBHID_ID_KP_000             = 177,
    USBHID_ID_THOUSANDSSEPARATOR = 178,
    USBHID_ID_DECIMALSEPARATOR   = 179,
    USBHID_ID_CURRENCYUNIT       = 180,
    USBHID_ID_CURRENCYSUBUNIT    = 181,
    USBHID_ID_KP_LEFTPAREN       = 182,
    USBHID_ID_KP_RIGHTPAREN      = 183,
    USBHID_ID_KP_LEFTBRACE       = 184,
    USBHID_ID_KP_RIGHTBRACE      = 185,
    USBHID_ID_KP_TAB             = 186,
    USBHID_ID_KP_BACKSPACE       = 187,
    USBHID_ID_KP_A               = 188,
    USBHID_ID_KP_B               = 189,
    USBHID_ID_KP_C               = 190,
    USBHID_ID_KP_D               = 191,
    USBHID_ID_KP_E               = 192,
    USBHID_ID_KP_F               = 193,
    USBHID_ID_KP_XOR             = 194,
    USBHID_ID_KP_POWER           = 195,
    USBHID_ID_KP_PERCENT         = 196,
    USBHID_ID_KP_LESS            = 197,
    USBHID_ID_KP_GREATER         = 198,
    USBHID_ID_KP_AMPERSAND       = 199,
    USBHID_ID_KP_DBLAMPERSAND    = 200,
    USBHID_ID_KP_VERTICALBAR     = 201,
    USBHID_ID_KP_DBLVERTICALBAR  = 202,
    USBHID_ID_KP_COLON           = 203,
    USBHID_ID_KP_HASH            = 204,
    USBHID_ID_KP_SPACE           = 205,
    USBHID_ID_KP_AT              = 206,
    USBHID_ID_KP_EXCLAM          = 207,
    USBHID_ID_KP_MEMSTORE        = 208,
    USBHID_ID_KP_MEMRECALL       = 209,
    USBHID_ID_KP_MEMCLEAR        = 210,
    USBHID_ID_KP_MEMADD          = 211,
    USBHID_ID_KP_MEMSUBTRACT     = 212,
    USBHID_ID_KP_MEMMULTIPLY     = 213,
    USBHID_ID_KP_MEMDIVIDE       = 214,
    USBHID_ID_KP_PLUSMINUS       = 215,
    USBHID_ID_KP_CLEAR           = 216,
    USBHID_ID_KP_CLEARENTRY      = 217,
    USBHID_ID_KP_BINARY          = 218,
    USBHID_ID_KP_OCTAL           = 219,
    USBHID_ID_KP_DECIMAL         = 220,
    USBHID_ID_KP_HEXADECIMAL     = 221,

    USBHID_ID_LCTRL              = 224,
    USBHID_ID_LSHIFT             = 225,
    USBHID_ID_LALT               = 226, /**< alt, option */
    USBHID_ID_LGUI               = 227, /**< windows, command (apple), meta */
    USBHID_ID_RCTRL              = 228,
    USBHID_ID_RSHIFT             = 229,
    USBHID_ID_RALT               = 230, /**< alt gr, option */
    USBHID_ID_RGUI               = 231, /**< windows, command (apple), meta */

    USBHID_ID_MODE               = 257, /**< I'm not sure if this is really not covered
                                         *   by any of the above, but since there's a
                                         *   special KMOD_MODE for it I'm adding it here
                                         */

    /* @} *//* Usage page 0x07 */

    /**
     *  \name Usage page 0x0C
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     */
    /* @{ */

    USBHID_ID_AUDIONEXT    = 258,
    USBHID_ID_AUDIOPREV    = 259,
    USBHID_ID_AUDIOSTOP    = 260,
    USBHID_ID_AUDIOPLAY    = 261,
    USBHID_ID_AUDIOMUTE    = 262,
    USBHID_ID_MEDIASELECT  = 263,
    USBHID_ID_WWW          = 264,
    USBHID_ID_MAIL         = 265,
    USBHID_ID_CALCULATOR   = 266,
    USBHID_ID_COMPUTER     = 267,
    USBHID_ID_AC_SEARCH    = 268,
    USBHID_ID_AC_HOME      = 269,
    USBHID_ID_AC_BACK      = 270,
    USBHID_ID_AC_FORWARD   = 271,
    USBHID_ID_AC_STOP      = 272,
    USBHID_ID_AC_REFRESH   = 273,
    USBHID_ID_AC_BOOKMARKS = 274,

    /* @} *//* Usage page 0x0C */

    /**
     *  \name Walther keys
     *
     *  These are values that Christian Walther added (for mac keyboard?).
     */
    /* @{ */

    USBHID_ID_BRIGHTNESSDOWN = 275,
    USBHID_ID_BRIGHTNESSUP   = 276,
    USBHID_ID_DISPLAYSWITCH  = 277, /**< display mirroring/dual display
                                     *     switch, video mode switch */
    USBHID_ID_KBDILLUMTOGGLE = 278,
    USBHID_ID_KBDILLUMDOWN   = 279,
    USBHID_ID_KBDILLUMUP     = 280,
    USBHID_ID_EJECT          = 281,
    USBHID_ID_SLEEP          = 282,

    USBHID_ID_APP1           = 283,
    USBHID_ID_APP2           = 284,

    /* @} *//* Walther keys */

    /**
     *  \name Usage page 0x0C (additional media keys)
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     */
    /* @{ */

    USBHID_ID_AUDIOREWIND      = 285,
    USBHID_ID_AUDIOFASTFORWARD = 286,

    /* @} *//* Usage page 0x0C (additional media keys) */

    /* Add any other keys here. */

    USBHID_NUM_ID = 512 /**< not a key, just marks the number of scancodes
                         *       for array bounds */
} USBHID_id;


typedef int (*frk_keyPress) (void *object, USBHID_id usbhid_code, char pressed);
typedef u8 (*frk_keyWasHandled) (void *object, USBHID_id usbhid_code);
typedef void (*frk_handlePaste) (char *string);


void guestInit(void);
void guestStartAudio(void);

// Populate button_state array
void guestButtons(core_crocods_t *core,  frk_keyPress fct, frk_keyWasHandled handledfct, frk_handlePaste handlePaste);

void guestExit(void);

/// @brief
/// @return Return the UNIX time in microseconds
uint32_t guestGetMilliSeconds(void);

void guestSleep(u32 milisec);

void guestScreenDraw(core_crocods_t *core);

void guestGetAllKeyPressed(core_crocods_t *core, char *string);
void guestGetJoystick(core_crocods_t *core, char *string);

void guest_queue_audio(void *data, int len);
char guestCouldSendAudio(void);

/** @brief Switch the screen to fullscreen (on/off)
 *
 *  @param core CrocoDS core.
 *  @param on true or false
 */
void guestFullscreen(core_crocods_t *core, char on);

/** @brief Get the fullscreen status
 *
 *  @param core CrocoDS core
 *  @return 1 (true), 0 (false), 99 (not available).
 */
char guestIsFullscreen(core_crocods_t *core);

#ifdef __cplusplus
}
#endif