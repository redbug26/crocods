#include "guest.h"

#ifndef _WIN32
#include <sys/time.h>
#endif

#include "SDL.h"
#include "cpcfont.h"

void refreshJoystickConfig(void);

SDL_Window *sdlWindow;

SDL_Texture *sdlTexture;
SDL_Renderer *sdlRenderer;
uint16_t textureBytes[384 * 2 * 288];
u32 bufferWidth, bufferHeight;

#define PAD_UP     SDL_SCANCODE_UP
#define PAD_LEFT   SDL_SCANCODE_LEFT;
#define PAD_RIGHT  SDL_SCANCODE_RIGHT
#define PAD_DOWN   SDL_SCANCODE_DOWN


// #define PAD_A      SDL_SCANCODE_LCTRL
// #define PAD_B      SDL_SCANCODE_LALT

#define PAD_A      SDL_SCANCODE_SPACE
#define PAD_B      SDL_SCANCODE_ESCAPE

#define PAD_X      SDL_SCANCODE_LSHIFT
#define PAD_Y      SDL_SCANCODE_X

#define PAD_L      SDL_SCANCODE_TAB
#define PAD_R      SDL_SCANCODE_BACKSPACE

#define PAD_START  SDL_SCANCODE_RETURN

// #define PAD_SELECT SDL_SCANCODE_F12
#define PAD_SELECT SDL_SCANCODE_F12

#define PAD_QUIT   SDL_SCANCODE_C

#define PAD_L2     SDL_SCANCODE_PAGEUP
#define PAD_R2     SDL_SCANCODE_PAGEDOWN

int32_t mySDLflags;
u32 *incX, *incY;

void mixaudioCallback(void *userdata, uint8_t *stream, int32_t len);
int audio_align_samples(int given);

extern core_crocods_t gb;

static int sdlframe = 0;

SDL_AudioDeviceID sdl_dev;


u16 scanlineMask[] = {0b1110111101011101,
                      0b1110011100011100,
                      0b1100011000011000,
                      0b1000010000010000};

#define RGB565(R, G, B) ((((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))


void guestInit(void)
{
// Set Video

    ddlog(&gb, 2, "Guest Init\n");

    mySDLflags = SDL_SWSURFACE;

    uint16_t w = 320, h = 240;

    ddlog(&gb, 2, "SDL_Init\n");

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        ddlog(&gb, 0, "Erreur lors de l'initialisation de la SDL : %s\n", SDL_GetError());
        ExecuteMenu(&gb, ID_EXIT, NULL);

        // SDL_Quit();
        // exit;
    }

#ifdef RPI

    w = 384;
    h = 272;

    ddlog(&gb, 2, "Init SDL\n");

    mySDLflags = SDL_WINDOW_FULLSCREEN;

    int ratio = 2;

    int display_count;
    if ((display_count = SDL_GetNumVideoDisplays()) < 1) {
        ddlog(&gb, 2, "SDL_GetNumVideoDisplays returned: %i\n", display_count);
        // exit;
    }

    SDL_DisplayMode mode;
    int display_index = 0, mode_index = 0;
    if (SDL_GetDisplayMode(display_index, mode_index, &mode) == 0) {
        ddlog(&gb, 2, "%i x %i\n", mode.w, mode.h); // Dimensions
        if (mode.h >= 768) {
            ratio = 3;
        }
    } else {
        printf("SDL_GetDisplayMode failed: %s\n", SDL_GetError());
        // exit;
    }

    SDL_Window *sdlWindow = SDL_CreateWindow("CrocoDS",
                                             SDL_WINDOWPOS_CENTERED,
                                             SDL_WINDOWPOS_CENTERED,
                                             w * ratio, h * ratio,
                                             mySDLflags);

    printf("Window at %p\n", sdlWindow);

    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (sdlRenderer == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateRenderer() failed: %s\n", SDL_GetError());
    }

    printf("Renderer at %p\n", sdlRenderer);

    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 230, 255);

    bufferWidth = w;
    bufferHeight = h;

    sdlTexture = SDL_CreateTexture(sdlRenderer,
                                   SDL_PIXELFORMAT_RGB565,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   bufferWidth, bufferHeight);

    printf("Texture at %p\n", sdlTexture);

#else  /* ifdef RPI */

    w = 384;
    h = 272;

    // printf("Init SDL\n");

    // mySDLflags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    mySDLflags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE; // | SDL_WINDOW_FULLSCREEN_DESKTOP

#ifdef EMSCRIPTEN
    ddlog(&gb, 2, "Using emscripten\n");

    mySDLflags = 0;
#endif

    int ratio = 1; // 2

    SDL_DisplayMode mode;
    if (SDL_GetDisplayMode(0, 0, &mode) == 0) {
        ddlog(&gb, 2, "%i x %i\n", mode.w, mode.h);  // Dimensions
        if (mode.h >= 768) {
            ratio = 1; //  3;
        }
    }

    if ((gb.guest_width != 0) && (gb.guest_height != 0)) {

        printf("open sdl2 %dx%d\n", gb.guest_width, gb.guest_height);

        sdlWindow = SDL_CreateWindow("CrocoDS",
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     gb.guest_width, gb.guest_height,
                                     mySDLflags);

    } else {
        printf("open sdl2 with ratio %d\n", ratio);

        sdlWindow = SDL_CreateWindow("CrocoDS",
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     w * ratio, h * ratio,
                                     mySDLflags);

    }


    ddlog(&gb, 2, "Window at %p\n", sdlWindow);

    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (sdlRenderer == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateRenderer() failed: %s\n", SDL_GetError());
    }

    ddlog(&gb, 2, "Renderer at %p\n", sdlRenderer);

    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 230, 255);

    bufferWidth = w;
    bufferHeight = h;

    sdlTexture = SDL_CreateTexture(sdlRenderer,
                                   SDL_PIXELFORMAT_RGB565,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   bufferWidth, bufferHeight);

    ddlog(&gb, 2, "Texture at %p\n", sdlTexture);

#endif /* ifdef SDL2 */

#ifdef GCW
    SDL_ShowCursor(SDL_DISABLE);
    actualScreen = SDL_SetVideoMode(w, h, 16, mySDLflags);
#endif

    // SDL_FillRect(actualScreen, NULL, 0);

    incX = (u32 *)malloc(384 * 2 * sizeof(u32));     // malloc the max width
    incY = (u32 *)malloc(272 * sizeof(u32));           // malloc the max height

// Set Audio

#ifndef NOSOUND

    ddlog(&gb, 2, "Init sound\n");

#ifdef _WIN32
    putenv("SDL_AUDIODRIVER=DirectSound");
#endif

    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec fmt, retFmt;

    /*	Set up SDL sound */
    SDL_memset(&fmt, 0, sizeof(fmt)); /* or SDL_zero(want) */

    fmt.freq = 44100;
    fmt.samples = audio_align_samples(fmt.freq * 20 / 1000); // FRAME_PERIOD_MS = 20
    fmt.format = AUDIO_S16LSB;      // little-endian
    fmt.channels = 2;
    // fmt.callback = mixaudioCallback;
    fmt.userdata = NULL;

#ifdef TARGET_OS_MAC
    fmt.format = AUDIO_S16MSB;      // why in big-endian ? MACOS is little endian !
#endif

#ifdef _WIN32
    fmt.format = AUDIO_S16LSB;
#endif

#ifdef EMSCRIPTEN
    fmt.format = AUDIO_F32LSB;
#endif


// #define  AUDIO_U16LSB   0x0010
// #define  AUDIO_S16LSB   0x8010
// #define  AUDIO_U16MSB   0x1010
// #define  AUDIO_S16MSB   0x9010

    int i, count = SDL_GetNumAudioDevices(0);

    for (i = 0; i < count; ++i) {
        ddlog(&gb, 2, "Audio device %d: %s\n", i, SDL_GetAudioDeviceName(i, 0));
    }

    /* Open the audio device and start playing sound! */
    sdl_dev = SDL_OpenAudioDevice(NULL, 0, &fmt, &retFmt, SDL_AUDIO_ALLOW_FORMAT_CHANGE);

    if (sdl_dev == 0) {
        ddlog(&gb, 0, "Failed to open audio: %s\n", SDL_GetError());
        ExecuteMenu(&gb, ID_EXIT, NULL);
    }

    if (retFmt.format != fmt.format) {
        ddlog(&gb, 2, "We didn't get the audio format (%04X vs %04X)\n", retFmt.format, fmt.format);
    }

    switch (retFmt.format) {
        case AUDIO_S16LSB:
            gb.audio_format = CROCODS_AUDIO_S16LSB;
            break;
        case AUDIO_S16MSB:
            gb.audio_format = CROCODS_AUDIO_S16MSB;
            break;
        case AUDIO_F32LSB:
            gb.audio_format = CROCODS_AUDIO_F32LSB;
            break;
        default:
            ddlog(&gb, 0, "Coudln't use audio format %d\n", retFmt.format);
            ExecuteMenu(&gb, ID_EXIT, NULL);
            break;
    }

    ddlog(&gb, 2, "Audio buffer size: %d\n", retFmt.size);

    SDL_PauseAudioDevice(sdl_dev, 0);

    // SDL_PauseAudio(1); // audio callback is stopped when this returns.

#else  /* ifndef NOSOUND */
    ddlog(&gb, 2, "Compiled without sound\n");

#endif /* ifndef NOSOUND */

    // SDL_Delay(1000); // audio device plays silence for 5 seconds
    // SDL_PauseAudio(0);

#ifdef JOYSTICK
    SDL_Init(SDL_INIT_JOYSTICK);
//    SDL_JoystickEventState(SDL_IGNORE);

    refreshJoystickConfig();
#endif

    const int CodeToTest = SDL_GetKeyFromScancode(SDL_SCANCODE_Q) +
        SDL_GetKeyFromScancode(SDL_SCANCODE_W) +
        SDL_GetKeyFromScancode(SDL_SCANCODE_Y);
    switch (CodeToTest) {   // Magic numbers got from testing
        case 353: // Qwerty
            gb.keyboardLayout = 0;
            break;
        case 354:  // Qwertz
            gb.keyboardLayout = 0;
            break;
        case 340: // Azerty
            gb.keyboardLayout = 1;
            break;
        case 185: // Dvorak
            gb.keyboardLayout = 0;
            break;
        case 338: // Colemak
            gb.keyboardLayout = 0;
            break;
        case 1073742155: // Bepo
            gb.keyboardLayout = 0;
            break;
        case 1986: // Greek
            gb.keyboardLayout = 0;
            break;
        default:
            gb.keyboardLayout = 0;
            break;
    } /* switch */


} /* guestInit */

// len in bytes
void guest_queue_audio(void *data, int len)
{
    // ddlog(&gb, 2, "Received %d bytes (Queue is %d len)\n", len, SDL_GetQueuedAudioSize(sdl_dev));
    int ret = SDL_QueueAudio(sdl_dev, data, len);

    if (ret != 0) {
        ddlog(&gb, 2, "Failed to queue audio: %s\n", SDL_GetError());
    }
}

int audio_align_samples(int given)
{
    int actual = 1;

    while (actual < given)
        actual <<= 1;
    return actual; // return the closest match as 2^n
}

// len in bytes

void mixaudioCallback(void *userdata, uint8_t *stream, int32_t len) // len is length buffer in byte
{
    // TODO - crocods_copy_sound_buffer;

    if ((gb.isPaused) || (gb.turbo)) {
        memset(stream, 0, len);
        return;
    }

    #ifdef EMSCRIPTEN

#define DIVBY32767 3.05185094759972e-05f

    // len is nbr sample * 8

    int i;
    Uint16 *src = (Uint16 *)malloc(len / 2);

    crocods_copy_sound_buffer(&gb, (GB_sample_t *)src, len / 8);
    float *dst = (float *)stream; // Sizeof(float) = 4;

    for (i = 0; i < len / 4; i++) {   // nbr of sample * 2
        // MSB
        const float val = (((float)((Sint16)(*src))) * DIVBY32767);
        *dst = SDL_SwapFloatLE(val);

        // LSB
        // const float val = (((float)((Sint16) * src)) * DIVBY32767);
        // *dst = SDL_SwapFloatLE(val);

        src++;
        dst++;
    }

    free(src);
    #else  /* ifdef EMSCRIPTEN */

    crocods_copy_sound_buffer(&gb, (GB_sample_t *)stream, len / 4);

    #endif /* ifdef EMSCRIPTEN */


} /* mixaudioCallback */

void screen_draw_resize(int w, int h);

/*
 * 0: PAD_LEFT;
 * 1: PAD_RIGHT;
 * 2: PAD_UP;
 * 3: PAD_DOWN;
 * 4: PAD_A;
 * 5: PAD_B;
 * 6: PAD_X;
 * 7: PAD_Y;
 * 8: PAD_L;
 * 9: PAD_R;
 * 10: PAD_START;
 * 11: PAD_SELECT;
 * 12: PAD_QUIT;
 * 13: PAD_L2;
 * 14: PAD_R2;
 */

int16_t button_state[18] =  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t button_time[18] =  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t button_virtual[18] =  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#ifdef JOYSTICK

SDL_Joystick *joystick_sdl[2];
int16_t joystick_axies[4] = {0, 0, 0, 0};

int joystick_num_axes, joystick_num_buttons;

        #define JOYSTICK_UP          (joystick_axies[1] < -2048 ? 1 : 0)
        #define JOYSTICK_RIGHT       (joystick_axies[0] > 2048 ? 1 : 0)
        #define JOYSTICK_LEFT        (joystick_axies[0] < -2048 ? 1 : 0)
        #define JOYSTICK_DOWN        (joystick_axies[1] > 2048 ? 1 : 0)

        #define JOYSTICK_AXIS2_UP    (joystick_axies[3] < -2048 ? 1 : 0)
        #define JOYSTICK_AXIS2_RIGHT (joystick_axies[2] > 2048 ? 1 : 0)
        #define JOYSTICK_AXIS2_LEFT  (joystick_axies[2] < -2048 ? 1 : 0)
        #define JOYSTICK_AXIS2_DOWN  (joystick_axies[3] > 2048 ? 1 : 0)
#else
        #define JOYSTICK_UP          (0)
        #define JOYSTICK_RIGHT       (0)
        #define JOYSTICK_LEFT        (0)
        #define JOYSTICK_DOWN        (0)
        #define JOYSTICK_AXIS2_UP    (0)
        #define JOYSTICK_AXIS2_RIGHT (0)
        #define JOYSTICK_AXIS2_LEFT  (0)
        #define JOYSTICK_AXIS2_DOWN  (0)
#endif /* ifdef JOYSTICK */

void guestStartAudio(void)
{
#ifndef NOSOUND
    // SDL_PauseAudio(0);
#endif
}

// Keyboard things

u8 scancodes[SDL_NUM_SCANCODES];

typedef struct {
    CPC_SCANCODE cpcScanCode;
    SDL_Scancode sdlScanCode;
} SDL_keyMapping;

SDL_keyMapping SDLkeysymFromCPCkeys_us[] = {
    {CPC_ZERO,                 SDL_SCANCODE_0},
    {CPC_1,                    SDL_SCANCODE_1},
    {CPC_2,                    SDL_SCANCODE_2},
    {CPC_3,                    SDL_SCANCODE_3},
    {CPC_4,                    SDL_SCANCODE_4},
    {CPC_5,                    SDL_SCANCODE_5},
    {CPC_6,                    SDL_SCANCODE_6},
    {CPC_7,                    SDL_SCANCODE_7},
    {CPC_8,                    SDL_SCANCODE_8},
    {CPC_9,                    SDL_SCANCODE_9},
    {CPC_MINUS,                SDL_SCANCODE_MINUS},
    {CPC_HAT,                  SDL_SCANCODE_EQUALS},

    {CPC_AT,                   SDL_SCANCODE_LEFTBRACKET},
    {CPC_OPEN_SQUARE_BRACKET,  SDL_SCANCODE_RIGHTBRACKET},

    {CPC_COLON,                SDL_SCANCODE_SEMICOLON},
    {CPC_SEMICOLON,            SDL_SCANCODE_APOSTROPHE},
    {CPC_CLOSE_SQUARE_BRACKET, SDL_SCANCODE_BACKSLASH},

    {CPC_COMMA,                SDL_SCANCODE_COMMA},
    {CPC_DOT,                  SDL_SCANCODE_PERIOD},
    {CPC_BACKSLASH,            SDL_SCANCODE_SLASH},
    {CPC_FORWARD_SLASH,        SDL_SCANCODE_NONUSBACKSLASH},

    {CPC_A,                    SDL_SCANCODE_A},
    {CPC_B,                    SDL_SCANCODE_B},
    {CPC_C,                    SDL_SCANCODE_C},
    {CPC_D,                    SDL_SCANCODE_D},
    {CPC_E,                    SDL_SCANCODE_E},
    {CPC_F,                    SDL_SCANCODE_F},
    {CPC_G,                    SDL_SCANCODE_G},
    {CPC_H,                    SDL_SCANCODE_H},
    {CPC_I,                    SDL_SCANCODE_I},
    {CPC_J,                    SDL_SCANCODE_J},
    {CPC_K,                    SDL_SCANCODE_K},
    {CPC_L,                    SDL_SCANCODE_L},
    {CPC_M,                    SDL_SCANCODE_M},
    {CPC_N,                    SDL_SCANCODE_N},
    {CPC_O,                    SDL_SCANCODE_O},
    {CPC_P,                    SDL_SCANCODE_P},
    {CPC_Q,                    SDL_SCANCODE_Q},
    {CPC_R,                    SDL_SCANCODE_R},
    {CPC_S,                    SDL_SCANCODE_S},
    {CPC_T,                    SDL_SCANCODE_T},
    {CPC_U,                    SDL_SCANCODE_U},
    {CPC_V,                    SDL_SCANCODE_V},
    {CPC_W,                    SDL_SCANCODE_W},
    {CPC_X,                    SDL_SCANCODE_X},
    {CPC_Y,                    SDL_SCANCODE_Y},
    {CPC_Z,                    SDL_SCANCODE_Z},
    {CPC_SHIFT,                SDL_SCANCODE_RSHIFT},
    {CPC_SHIFT,                SDL_SCANCODE_LSHIFT},
    {CPC_TAB,                  SDL_SCANCODE_TAB},
    {CPC_RETURN,               SDL_SCANCODE_RETURN},

    {CPC_DEL,                  SDL_SCANCODE_BACKSPACE},
    {CPC_CLR,                  SDL_SCANCODE_DELETE},
    {CPC_SPACE,                SDL_SCANCODE_SPACE},

    {CPC_F1,                   SDL_SCANCODE_F1},
    {CPC_F2,                   SDL_SCANCODE_F2},
    {CPC_F3,                   SDL_SCANCODE_F3},
    {CPC_F4,                   SDL_SCANCODE_F4},
    {CPC_F5,                   SDL_SCANCODE_F5},
    {CPC_F6,                   SDL_SCANCODE_F6},
    {CPC_F7,                   SDL_SCANCODE_F7},
    {CPC_F8,                   SDL_SCANCODE_F8},
    {CPC_F9,                   SDL_SCANCODE_F9},
    {CPC_F0,                   SDL_SCANCODE_F10},
    {CPC_NIL,                  SDL_SCANCODE_F11},
    {CPC_NIL,                  SDL_SCANCODE_F12},

    {CPC_ESC,                  SDL_SCANCODE_ESCAPE},

    {CPC_F1,                   SDL_SCANCODE_KP_1},
    {CPC_F2,                   SDL_SCANCODE_KP_2},
    {CPC_F3,                   SDL_SCANCODE_KP_3},
    {CPC_F4,                   SDL_SCANCODE_KP_4},
    {CPC_F5,                   SDL_SCANCODE_KP_5},
    {CPC_F6,                   SDL_SCANCODE_KP_6},
    {CPC_F7,                   SDL_SCANCODE_KP_7},
    {CPC_F8,                   SDL_SCANCODE_KP_8},
    {CPC_F9,                   SDL_SCANCODE_KP_9},
    {CPC_F0,                   SDL_SCANCODE_KP_0},
    {CPC_SMALL_ENTER,          SDL_SCANCODE_KP_ENTER},
    {CPC_FDOT,                 SDL_SCANCODE_KP_PERIOD},

    {CPC_CURSOR_UP,            SDL_SCANCODE_UP},
    {CPC_CURSOR_DOWN,          SDL_SCANCODE_DOWN},
    {CPC_CURSOR_RIGHT,         SDL_SCANCODE_RIGHT},
    {CPC_CURSOR_LEFT,          SDL_SCANCODE_LEFT},
    {CPC_COPY,                 SDL_SCANCODE_RALT},                                                                                                                                                                                                                                           // Right OPTION on macOS
    {CPC_COPY,                 SDL_SCANCODE_LALT},                                                                                                                                                                                                                                           // Left OPTION on macOS
    {CPC_NIL,                  SDL_SCANCODE_RGUI},                                                                                                                                                                                                                                           // Right COMMAND on macOS
    {CPC_NIL,                  SDL_SCANCODE_LGUI},                                                                                                                                                                                                                                           // Left COMMAND on macOS
    {CPC_CONTROL,              SDL_SCANCODE_LCTRL},                                                                                                                                                                                                                                          // Left Control on macOS
    {CPC_CONTROL,              SDL_SCANCODE_RCTRL}                                                                                                                                                                                                                                           // Right Control on macOS
};

char guestIsFullscreen(core_crocods_t *core)
{
    if (SDL_GetWindowFlags(sdlWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP) {
        return 1;
    } else {
        return 0;
    }
}

void guestFullscreen(core_crocods_t *core, char on)
{
    if (on) {
        SDL_SetWindowFullscreen(sdlWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
        SDL_SetWindowFullscreen(sdlWindow, 0);
    }
}



/// @brief
/// @param core
/// @param keySym
/// @param pressed
/// @return 1: ignore result, 0: use result
int old_sdl_keypressed(core_crocods_t *core, SDL_Keysym keySym, u8 pressed)
{
    u8 gui_pressed = (((keySym.mod & KMOD_LGUI) == KMOD_LGUI) ||
                      ((keySym.mod & KMOD_RGUI) == KMOD_RGUI) ||
                      (((keySym.mod & KMOD_LCTRL) == KMOD_LCTRL) && ((keySym.mod & KMOD_LSHIFT) == KMOD_LSHIFT)));

    if ((pressed) && (gui_pressed)) {

        if (((keySym.mod & KMOD_LCTRL) == KMOD_LCTRL) && ((keySym.mod & KMOD_LSHIFT) == KMOD_LSHIFT)) {
            strcpy(gb.messageToDislay, "[CTRL][SHIFT]");
        } else {
            strcpy(gb.messageToDislay, "[CMD]");
        }

        if (keySym.sym == 'r') {
            gb.messageTimer = 50;
            strcat(gb.messageToDislay, "[R]");

            ExecuteMenu(core, ID_RESET, NULL);
            return 1;
        }

        if (keySym.sym == 'a') {
            gb.messageTimer = 50;
            strcat(gb.messageToDislay, "[A]");

            ExecuteMenu(core, ID_AUTORUN, NULL);
            return 1;
        }

        if (keySym.sym == 'q') {
            gb.messageTimer = 50;
            strcat(gb.messageToDislay, "[Q]");

            ExecuteMenu(core, ID_EXIT, NULL);
            return 1;
        }

        if (keySym.sym == 'f') {
            gb.messageTimer = 50;
            strcat(gb.messageToDislay, "[F]");

            ExecuteMenu(core, ID_FULLSCREEN_SWITCH, NULL);
            return 1;
        }

        if (keySym.sym == 'e') {
            gb.messageTimer = 50;
            strcat(gb.messageToDislay, "[E]");

            ExecuteMenu(core, ID_SWITCH_TURBO, NULL);
            return 1;
        }

        if (keySym.sym == 'c') {
            gb.messageTimer = 50;
            strcat(gb.messageToDislay, "[C]");

            ExecuteMenu(core, ID_CONSOLE, NULL);
            return 1;
        }

        gb.messageToDislay[0] = 0;
    }

    return 0;
} /* old_sdl_keypressed */

int sdl_keypressed(void *object, USBHID_id sdl_key, char pressed)
{
    core_crocods_t *core = (core_crocods_t *)object;

    scancodes[sdl_key] = pressed;


// Called from plateform.h !!
    // if ((pressed) && ((keySym.scancode == 227) || (keySym.scancode == 231))) {
    //     ExecuteMenu(core, ID_MENU_ENTER, NULL);
    //     return;
    // }

    if (core->keyEmul != 2) {
        return 0;
    }

//    if (pressed) {
//        printf("sdl_keypressed (scan:%d, pressed: %d, sym:%d, mod:%d)\n", keySym.scancode, pressed, keySym.sym, keySym.mod);
//    }

    int n;
    CPC_SCANCODE cpcScanCode = CPC_NIL;

    for (n = 0; n < sizeof(SDLkeysymFromCPCkeys_us) / sizeof(SDL_keyMapping); n++) {
        if (sdl_key == (USBHID_id)SDLkeysymFromCPCkeys_us[n].sdlScanCode) {
            cpcScanCode = SDLkeysymFromCPCkeys_us[n].cpcScanCode;
            break;
        }
    }

    if (cpcScanCode != CPC_NIL) {
        if (pressed) {
            CPC_SetScanCode(core, cpcScanCode);
        } else {
            CPC_ClearScanCode(core, cpcScanCode);
        }
    }

    return 0;
} /* keypressed */

void guestGetJoystick(core_crocods_t *core, char *string)
{
    string[0] = 0;

#ifdef JOYSTICK
    int i; // , n;
    SDL_Joystick *joy = joystick_sdl[0];

    if (joy == NULL) {
        return;
    }

    SDL_JoystickUpdate();

    for (i = 0; i < joystick_num_buttons; i++) {
        char buf[32];
        sprintf(buf, "%d", SDL_JoystickGetButton(joy, i));
        strcat(string, buf);
    }

    for (i = 0; i < joystick_num_axes; i++) {
        char buf[32];
        sprintf(buf, " %c%c", SDL_JoystickGetAxis(joy, i) > 2048 ? '-' : ' ', SDL_JoystickGetAxis(joy, i) < -2048 ? '+' : ' ');
        strcat(string, buf);
    }

#endif /* ifdef JOYSTICK */
} /* guestGetJoystick */

void guestGetAllKeyPressed(core_crocods_t *core, char *string)
{
    int n;

    string[0] = 0;

    for (n = 0; n < SDL_NUM_SCANCODES; n++) {
        char *keyName = NULL;
        if (scancodes[n] != 0) {
            switch (n) {
                case SDL_SCANCODE_UNKNOWN: keyName = "UNKNOWN"; break;
                case SDL_SCANCODE_A: keyName = "A"; break;
                case SDL_SCANCODE_B: keyName = "B"; break;
                case SDL_SCANCODE_C: keyName = "C"; break;
                case SDL_SCANCODE_D: keyName = "D"; break;
                case SDL_SCANCODE_E: keyName = "E"; break;
                case SDL_SCANCODE_F: keyName = "F"; break;
                case SDL_SCANCODE_G: keyName = "G"; break;
                case SDL_SCANCODE_H: keyName = "H"; break;
                case SDL_SCANCODE_I: keyName = "I"; break;
                case SDL_SCANCODE_J: keyName = "J"; break;
                case SDL_SCANCODE_K: keyName = "K"; break;
                case SDL_SCANCODE_L: keyName = "L"; break;
                case SDL_SCANCODE_M: keyName = "M"; break;
                case SDL_SCANCODE_N: keyName = "N"; break;
                case SDL_SCANCODE_O: keyName = "O"; break;
                case SDL_SCANCODE_P: keyName = "P"; break;
                case SDL_SCANCODE_Q: keyName = "Q"; break;
                case SDL_SCANCODE_R: keyName = "R"; break;
                case SDL_SCANCODE_S: keyName = "S"; break;
                case SDL_SCANCODE_T: keyName = "T"; break;
                case SDL_SCANCODE_U: keyName = "U"; break;
                case SDL_SCANCODE_V: keyName = "V"; break;
                case SDL_SCANCODE_W: keyName = "W"; break;
                case SDL_SCANCODE_X: keyName = "X"; break;
                case SDL_SCANCODE_Y: keyName = "Y"; break;
                case SDL_SCANCODE_Z: keyName = "Z"; break;
                case SDL_SCANCODE_1: keyName = "1"; break;
                case SDL_SCANCODE_2: keyName = "2"; break;
                case SDL_SCANCODE_3: keyName = "3"; break;
                case SDL_SCANCODE_4: keyName = "4"; break;
                case SDL_SCANCODE_5: keyName = "5"; break;
                case SDL_SCANCODE_6: keyName = "6"; break;
                case SDL_SCANCODE_7: keyName = "7"; break;
                case SDL_SCANCODE_8: keyName = "8"; break;
                case SDL_SCANCODE_9: keyName = "9"; break;
                case SDL_SCANCODE_0: keyName = "0"; break;
                case SDL_SCANCODE_RETURN: keyName = "RETURN"; break;
                case SDL_SCANCODE_ESCAPE: keyName = "ESCAPE"; break;
                case SDL_SCANCODE_BACKSPACE: keyName = "BACKSPACE"; break;
                case SDL_SCANCODE_TAB: keyName = "TAB"; break;
                case SDL_SCANCODE_SPACE: keyName = "SPACE"; break;
                case SDL_SCANCODE_MINUS: keyName = "MINUS"; break;
                case SDL_SCANCODE_EQUALS: keyName = "EQUALS"; break;
                case SDL_SCANCODE_LEFTBRACKET: keyName = "LEFTBRACKET"; break;
                case SDL_SCANCODE_RIGHTBRACKET: keyName = "RIGHTBRACKET"; break;
                case SDL_SCANCODE_BACKSLASH: keyName = "BACKSLASH"; break;
                case SDL_SCANCODE_NONUSHASH: keyName = "NONUSHASH"; break;
                case SDL_SCANCODE_SEMICOLON: keyName = "SEMICOLON"; break;
                case SDL_SCANCODE_APOSTROPHE: keyName = "APOSTROPHE"; break;
                case SDL_SCANCODE_GRAVE: keyName = "GRAVE"; break;
                case SDL_SCANCODE_COMMA: keyName = "COMMA"; break;
                case SDL_SCANCODE_PERIOD: keyName = "PERIOD"; break;
                case SDL_SCANCODE_SLASH: keyName = "SLASH"; break;
                case SDL_SCANCODE_CAPSLOCK: keyName = "CAPSLOCK"; break;
                case SDL_SCANCODE_F1: keyName = "F1"; break;
                case SDL_SCANCODE_F2: keyName = "F2"; break;
                case SDL_SCANCODE_F3: keyName = "F3"; break;
                case SDL_SCANCODE_F4: keyName = "F4"; break;
                case SDL_SCANCODE_F5: keyName = "F5"; break;
                case SDL_SCANCODE_F6: keyName = "F6"; break;
                case SDL_SCANCODE_F7: keyName = "F7"; break;
                case SDL_SCANCODE_F8: keyName = "F8"; break;
                case SDL_SCANCODE_F9: keyName = "F9"; break;
                case SDL_SCANCODE_F10: keyName = "F10"; break;
                case SDL_SCANCODE_F11: keyName = "F11"; break;
                case SDL_SCANCODE_F12: keyName = "F12"; break;
                case SDL_SCANCODE_PRINTSCREEN: keyName = "PRINTSCREEN"; break;
                case SDL_SCANCODE_SCROLLLOCK: keyName = "SCROLLLOCK"; break;
                case SDL_SCANCODE_PAUSE: keyName = "PAUSE"; break;
                case SDL_SCANCODE_INSERT: keyName = "INSERT"; break;
                case SDL_SCANCODE_HOME: keyName = "HOME"; break;
                case SDL_SCANCODE_PAGEUP: keyName = "PAGEUP"; break;
                case SDL_SCANCODE_DELETE: keyName = "DELETE"; break;
                case SDL_SCANCODE_END: keyName = "END"; break;
                case SDL_SCANCODE_PAGEDOWN: keyName = "PAGEDOWN"; break;
                case SDL_SCANCODE_RIGHT: keyName = "RIGHT"; break;
                case SDL_SCANCODE_LEFT: keyName = "LEFT"; break;
                case SDL_SCANCODE_DOWN: keyName = "DOWN"; break;
                case SDL_SCANCODE_UP: keyName = "UP"; break;
                case SDL_SCANCODE_NUMLOCKCLEAR: keyName = "NUMLOCKCLEAR"; break;
                case SDL_SCANCODE_KP_DIVIDE: keyName = "KP_DIVIDE"; break;
                case SDL_SCANCODE_KP_MULTIPLY: keyName = "KP_MULTIPLY"; break;
                case SDL_SCANCODE_KP_MINUS: keyName = "KP_MINUS"; break;
                case SDL_SCANCODE_KP_PLUS: keyName = "KP_PLUS"; break;
                case SDL_SCANCODE_KP_ENTER: keyName = "KP_ENTER"; break;
                case SDL_SCANCODE_KP_1: keyName = "KP_1"; break;
                case SDL_SCANCODE_KP_2: keyName = "KP_2"; break;
                case SDL_SCANCODE_KP_3: keyName = "KP_3"; break;
                case SDL_SCANCODE_KP_4: keyName = "KP_4"; break;
                case SDL_SCANCODE_KP_5: keyName = "KP_5"; break;
                case SDL_SCANCODE_KP_6: keyName = "KP_6"; break;
                case SDL_SCANCODE_KP_7: keyName = "KP_7"; break;
                case SDL_SCANCODE_KP_8: keyName = "KP_8"; break;
                case SDL_SCANCODE_KP_9: keyName = "KP_9"; break;
                case SDL_SCANCODE_KP_0: keyName = "KP_0"; break;
                case SDL_SCANCODE_KP_PERIOD: keyName = "KP_PERIOD"; break;
                case SDL_SCANCODE_NONUSBACKSLASH: keyName = "NONUSBACKSLASH"; break;
                case SDL_SCANCODE_APPLICATION: keyName = "APPLICATION"; break;
                case SDL_SCANCODE_POWER: keyName = "POWER"; break;
                case SDL_SCANCODE_KP_EQUALS: keyName = "KP_EQUALS"; break;
                case SDL_SCANCODE_F13: keyName = "F13"; break;
                case SDL_SCANCODE_F14: keyName = "F14"; break;
                case SDL_SCANCODE_F15: keyName = "F15"; break;
                case SDL_SCANCODE_F16: keyName = "F16"; break;
                case SDL_SCANCODE_F17: keyName = "F17"; break;
                case SDL_SCANCODE_F18: keyName = "F18"; break;
                case SDL_SCANCODE_F19: keyName = "F19"; break;
                case SDL_SCANCODE_F20: keyName = "F20"; break;
                case SDL_SCANCODE_F21: keyName = "F21"; break;
                case SDL_SCANCODE_F22: keyName = "F22"; break;
                case SDL_SCANCODE_F23: keyName = "F23"; break;
                case SDL_SCANCODE_F24: keyName = "F24"; break;
                case SDL_SCANCODE_EXECUTE: keyName = "EXECUTE"; break;
                case SDL_SCANCODE_HELP: keyName = "HELP"; break;
                case SDL_SCANCODE_MENU: keyName = "MENU"; break;
                case SDL_SCANCODE_SELECT: keyName = "SELECT"; break;
                case SDL_SCANCODE_STOP: keyName = "STOP"; break;
                case SDL_SCANCODE_AGAIN: keyName = "AGAIN"; break;
                case SDL_SCANCODE_UNDO: keyName = "UNDO"; break;
                case SDL_SCANCODE_CUT: keyName = "CUT"; break;
                case SDL_SCANCODE_COPY: keyName = "COPY"; break;
                case SDL_SCANCODE_PASTE: keyName = "PASTE"; break;
                case SDL_SCANCODE_FIND: keyName = "FIND"; break;
                case SDL_SCANCODE_MUTE: keyName = "MUTE"; break;
                case SDL_SCANCODE_VOLUMEUP: keyName = "VOLUMEUP"; break;
                case SDL_SCANCODE_VOLUMEDOWN: keyName = "VOLUMEDOWN"; break;
                case SDL_SCANCODE_KP_COMMA: keyName = "KP_COMMA"; break;
                case SDL_SCANCODE_KP_EQUALSAS400: keyName = "KP_EQUALSAS400"; break;
                case SDL_SCANCODE_INTERNATIONAL1: keyName = "INTERNATIONAL1"; break;
                case SDL_SCANCODE_INTERNATIONAL2: keyName = "INTERNATIONAL2"; break;
                case SDL_SCANCODE_INTERNATIONAL3: keyName = "INTERNATIONAL3"; break;
                case SDL_SCANCODE_INTERNATIONAL4: keyName = "INTERNATIONAL4"; break;
                case SDL_SCANCODE_INTERNATIONAL5: keyName = "INTERNATIONAL5"; break;
                case SDL_SCANCODE_INTERNATIONAL6: keyName = "INTERNATIONAL6"; break;
                case SDL_SCANCODE_INTERNATIONAL7: keyName = "INTERNATIONAL7"; break;
                case SDL_SCANCODE_INTERNATIONAL8: keyName = "INTERNATIONAL8"; break;
                case SDL_SCANCODE_INTERNATIONAL9: keyName = "INTERNATIONAL9"; break;
                case SDL_SCANCODE_LANG1: keyName = "LANG1"; break;
                case SDL_SCANCODE_LANG2: keyName = "LANG2"; break;
                case SDL_SCANCODE_LANG3: keyName = "LANG3"; break;
                case SDL_SCANCODE_LANG4: keyName = "LANG4"; break;
                case SDL_SCANCODE_LANG5: keyName = "LANG5"; break;
                case SDL_SCANCODE_LANG6: keyName = "LANG6"; break;
                case SDL_SCANCODE_LANG7: keyName = "LANG7"; break;
                case SDL_SCANCODE_LANG8: keyName = "LANG8"; break;
                case SDL_SCANCODE_LANG9: keyName = "LANG9"; break;
                case SDL_SCANCODE_ALTERASE: keyName = "ALTERASE"; break;
                case SDL_SCANCODE_SYSREQ: keyName = "SYSREQ"; break;
                case SDL_SCANCODE_CANCEL: keyName = "CANCEL"; break;
                case SDL_SCANCODE_CLEAR: keyName = "CLEAR"; break;
                case SDL_SCANCODE_PRIOR: keyName = "PRIOR"; break;
                case SDL_SCANCODE_RETURN2: keyName = "RETURN2"; break;
                case SDL_SCANCODE_SEPARATOR: keyName = "SEPARATOR"; break;
                case SDL_SCANCODE_OUT: keyName = "OUT"; break;
                case SDL_SCANCODE_OPER: keyName = "OPER"; break;
                case SDL_SCANCODE_CLEARAGAIN: keyName = "CLEARAGAIN"; break;
                case SDL_SCANCODE_CRSEL: keyName = "CRSEL"; break;
                case SDL_SCANCODE_EXSEL: keyName = "EXSEL"; break;
                case SDL_SCANCODE_KP_00: keyName = "KP_00"; break;
                case SDL_SCANCODE_KP_000: keyName = "KP_000"; break;
                case SDL_SCANCODE_THOUSANDSSEPARATOR: keyName = "THOUSANDSSEPARATOR"; break;
                case SDL_SCANCODE_DECIMALSEPARATOR: keyName = "DECIMALSEPARATOR"; break;
                case SDL_SCANCODE_CURRENCYUNIT: keyName = "CURRENCYUNIT"; break;
                case SDL_SCANCODE_CURRENCYSUBUNIT: keyName = "CURRENCYSUBUNIT"; break;
                case SDL_SCANCODE_KP_LEFTPAREN: keyName = "KP_LEFTPAREN"; break;
                case SDL_SCANCODE_KP_RIGHTPAREN: keyName = "KP_RIGHTPAREN"; break;
                case SDL_SCANCODE_KP_LEFTBRACE: keyName = "KP_LEFTBRACE"; break;
                case SDL_SCANCODE_KP_RIGHTBRACE: keyName = "KP_RIGHTBRACE"; break;
                case SDL_SCANCODE_KP_TAB: keyName = "KP_TAB"; break;
                case SDL_SCANCODE_KP_BACKSPACE: keyName = "KP_BACKSPACE"; break;
                case SDL_SCANCODE_KP_A: keyName = "KP_A"; break;
                case SDL_SCANCODE_KP_B: keyName = "KP_B"; break;
                case SDL_SCANCODE_KP_C: keyName = "KP_C"; break;
                case SDL_SCANCODE_KP_D: keyName = "KP_D"; break;
                case SDL_SCANCODE_KP_E: keyName = "KP_E"; break;
                case SDL_SCANCODE_KP_F: keyName = "KP_F"; break;
                case SDL_SCANCODE_KP_XOR: keyName = "KP_XOR"; break;
                case SDL_SCANCODE_KP_POWER: keyName = "KP_POWER"; break;
                case SDL_SCANCODE_KP_PERCENT: keyName = "KP_PERCENT"; break;
                case SDL_SCANCODE_KP_LESS: keyName = "KP_LESS"; break;
                case SDL_SCANCODE_KP_GREATER: keyName = "KP_GREATER"; break;
                case SDL_SCANCODE_KP_AMPERSAND: keyName = "KP_AMPERSAND"; break;
                case SDL_SCANCODE_KP_DBLAMPERSAND: keyName = "KP_DBLAMPERSAND"; break;
                case SDL_SCANCODE_KP_VERTICALBAR: keyName = "KP_VERTICALBAR"; break;
                case SDL_SCANCODE_KP_DBLVERTICALBAR: keyName = "KP_DBLVERTICALBAR"; break;
                case SDL_SCANCODE_KP_COLON: keyName = "KP_COLON"; break;
                case SDL_SCANCODE_KP_HASH: keyName = "KP_HASH"; break;
                case SDL_SCANCODE_KP_SPACE: keyName = "KP_SPACE"; break;
                case SDL_SCANCODE_KP_AT: keyName = "KP_AT"; break;
                case SDL_SCANCODE_KP_EXCLAM: keyName = "KP_EXCLAM"; break;
                case SDL_SCANCODE_KP_MEMSTORE: keyName = "KP_MEMSTORE"; break;
                case SDL_SCANCODE_KP_MEMRECALL: keyName = "KP_MEMRECALL"; break;
                case SDL_SCANCODE_KP_MEMCLEAR: keyName = "KP_MEMCLEAR"; break;
                case SDL_SCANCODE_KP_MEMADD: keyName = "KP_MEMADD"; break;
                case SDL_SCANCODE_KP_MEMSUBTRACT: keyName = "KP_MEMSUBTRACT"; break;
                case SDL_SCANCODE_KP_MEMMULTIPLY: keyName = "KP_MEMMULTIPLY"; break;
                case SDL_SCANCODE_KP_MEMDIVIDE: keyName = "KP_MEMDIVIDE"; break;
                case SDL_SCANCODE_KP_PLUSMINUS: keyName = "KP_PLUSMINUS"; break;
                case SDL_SCANCODE_KP_CLEAR: keyName = "KP_CLEAR"; break;
                case SDL_SCANCODE_KP_CLEARENTRY: keyName = "KP_CLEARENTRY"; break;
                case SDL_SCANCODE_KP_BINARY: keyName = "KP_BINARY"; break;
                case SDL_SCANCODE_KP_OCTAL: keyName = "KP_OCTAL"; break;
                case SDL_SCANCODE_KP_DECIMAL: keyName = "KP_DECIMAL"; break;
                case SDL_SCANCODE_KP_HEXADECIMAL: keyName = "KP_HEXADECIMAL"; break;
                case SDL_SCANCODE_LCTRL: keyName = "LCTRL"; break;
                case SDL_SCANCODE_LSHIFT: keyName = "LSHIFT"; break;
                case SDL_SCANCODE_LALT: keyName = "LALT"; break;
                case SDL_SCANCODE_LGUI: keyName = "LGUI"; break;
                case SDL_SCANCODE_RCTRL: keyName = "RCTRL"; break;
                case SDL_SCANCODE_RSHIFT: keyName = "RSHIFT"; break;
                case SDL_SCANCODE_RALT: keyName = "RALT"; break;
                case SDL_SCANCODE_RGUI: keyName = "RGUI"; break;
                case SDL_SCANCODE_MODE: keyName = "MODE"; break;
                case SDL_SCANCODE_AUDIONEXT: keyName = "AUDIONEXT"; break;
                case SDL_SCANCODE_AUDIOPREV: keyName = "AUDIOPREV"; break;
                case SDL_SCANCODE_AUDIOSTOP: keyName = "AUDIOSTOP"; break;
                case SDL_SCANCODE_AUDIOPLAY: keyName = "AUDIOPLAY"; break;
                case SDL_SCANCODE_AUDIOMUTE: keyName = "AUDIOMUTE"; break;
                case SDL_SCANCODE_MEDIASELECT: keyName = "MEDIASELECT"; break;
                case SDL_SCANCODE_WWW: keyName = "WWW"; break;
                case SDL_SCANCODE_MAIL: keyName = "MAIL"; break;
                case SDL_SCANCODE_CALCULATOR: keyName = "CALCULATOR"; break;
                case SDL_SCANCODE_COMPUTER: keyName = "COMPUTER"; break;
                case SDL_SCANCODE_AC_SEARCH: keyName = "AC_SEARCH"; break;
                case SDL_SCANCODE_AC_HOME: keyName = "AC_HOME"; break;
                case SDL_SCANCODE_AC_BACK: keyName = "AC_BACK"; break;
                case SDL_SCANCODE_AC_FORWARD: keyName = "AC_FORWARD"; break;
                case SDL_SCANCODE_AC_STOP: keyName = "AC_STOP"; break;
                case SDL_SCANCODE_AC_REFRESH: keyName = "AC_REFRESH"; break;
                case SDL_SCANCODE_AC_BOOKMARKS: keyName = "AC_BOOKMARKS"; break;
                case SDL_SCANCODE_BRIGHTNESSDOWN: keyName = "BRIGHTNESSDOWN"; break;
                case SDL_SCANCODE_BRIGHTNESSUP: keyName = "BRIGHTNESSUP"; break;
                case SDL_SCANCODE_DISPLAYSWITCH: keyName = "DISPLAYSWITCH"; break;
                case SDL_SCANCODE_KBDILLUMTOGGLE: keyName = "KBDILLUMTOGGLE"; break;
                case SDL_SCANCODE_KBDILLUMDOWN: keyName = "KBDILLUMDOWN"; break;
                case SDL_SCANCODE_KBDILLUMUP: keyName = "KBDILLUMUP"; break;
                case SDL_SCANCODE_EJECT: keyName = "EJECT"; break;
                case SDL_SCANCODE_SLEEP: keyName = "SLEEP"; break;
                case SDL_SCANCODE_APP1: keyName = "APP1"; break;
                case SDL_SCANCODE_APP2: keyName = "APP2"; break;
                case SDL_SCANCODE_AUDIOREWIND: keyName = "AUDIOREWIND"; break;
                case SDL_SCANCODE_AUDIOFASTFORWARD: keyName = "AUDIOFASTFORWARD"; break;
            } /* switch */
        }
        if (keyName != NULL) {
            if (string[0] == 0) {
                strcpy(string, keyName);
            } else {
                strcat(string, ", ");
                strcat(string, keyName);
            }
        }
    }
} /* guestGetAllKeyPressed */

#ifdef EMSCRIPTEN

void
Emscripten_ResumeAudio(void)
{
    /* this is a workaround for chrome disabling audio unless it was caused by user interaction*/
    EM_ASM({
        var SDL2 = Module['SDL2'];
        if (!SDL2 || !SDL2.audioContext) {
            console.log("audiocontext not found");
            return;
        }

        if (SDL2.audioContext.state == 'suspended') {
            console.log("resume");
            SDL2.audioContext.resume();
        } else {
            console.log("not suspended");
        }

        if (SDL2.audioContext && SDL2.audioContext.currentTime == 0) {
            console.log("attempting to unlock");
            var buffer = SDL2.audioContext.createBuffer(1, 1, 22050);
            var source = SDL2.audioContext.createBufferSource();
            source.buffer = buffer;
            source.connect(SDL2.audioContext.destination);
            source.start();
            // source.noteOn(0);
        }
    });
} /* Emscripten_ResumeAudio */

#endif /* ifdef EMSCRIPTEN */

void refreshJoystickConfig(void)
{
#ifdef JOYSTICK
    int numJoysticks = SDL_NumJoysticks();
    if (numJoysticks > 0) {
        joystick_sdl[0] = SDL_JoystickOpen(0);

        printf("Name: %s\n", SDL_JoystickNameForIndex(0));

        if (joystick_sdl[0]) {
            printf("Opened Joystick 0\n");

            joystick_num_axes = SDL_JoystickNumAxes(joystick_sdl[0]);
            joystick_num_buttons = SDL_JoystickNumButtons(joystick_sdl[0]);

            printf("Number of Axes: %d\n", joystick_num_axes);
            printf("Number of Buttons: %d\n", joystick_num_buttons);
            printf("Number of Balls: %d\n", SDL_JoystickNumBalls(joystick_sdl[0]));

            int n;
            for (n = 0; n < SDL_JoystickNumAxes(joystick_sdl[0]); n++) {
                printf("Axis %d: %s\n", n, SDL_GameControllerGetStringForAxis(n));
            }
        } else {
            printf("Couldn't open Joystick 0\n");
            printf("SDL_JoystickOpen failed: %s\n", SDL_GetError());
        }

        printf("End of joystick configuration\n");
    }
#endif /* ifdef JOYSTICK */
} /* refreshJoystickConfig */

/* Uses button_state global */
// Populate button_state array
void guestButtons(core_crocods_t *core,  frk_keyPress fct, frk_keyWasHandled handledfct, frk_handlePaste handlePaste)
{
    uint8_t i = 0;

    int32_t pad, pad2, pad3;

    sdlframe++;

    if (fct == NULL) {
        fct = sdl_keypressed;
    }

#ifdef JOYSTICK

    SDL_JoystickUpdate();

    for (i = 0; i < 4; i++) {
        joystick_axies[i] = SDL_JoystickGetAxis(joystick_sdl[0], i);
    }

//    for (i = 0; i < SDL_JoystickNumAxes(joystick_sdl[0]); i++) {
//        printf("%d\n", SDL_JoystickGetAxis(joystick_sdl[0], i));
//
//               }
//    printf("-\n", SDL_JoystickGetAxis(joystick_sdl[0], i));

        #define CHECK_PAD (keys[pad] || SDL_JoystickGetButton(joystick_sdl[0], pad2) || (pad3) || (button_virtual[i]))
#else
        #define CHECK_PAD (keys[pad] || (button_virtual[i]))
#endif

    BOOL stopPollEvent = false;
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_MOUSEMOTION:
            case SDL_WINDOWEVENT:

                switch (e.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:              // better than RESIZED, more general
                    {
                        if (SDL_GetWindowFlags(sdlWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP) {
                            // printf("fullscreen. not saving dimension\n");
                        } else {
                            core->guest_width = e.window.data1;
                            core->guest_height = e.window.data2;
                        }
                        break;
                    }
                }
                break;

            case SDL_QUIT:
                printf("SDL_PollEvent: SQL_Quit\n");
                ExecuteMenu(core, ID_EXIT, NULL);
                break;

            case SDL_JOYAXISMOTION:
                // printf("SDL_PollEvent: SDL_JOYAXISMOTION\n");
                // printf("axis %d state %d\n", e.jaxis.axis, e.jaxis.value);
                break;

            case SDL_KEYDOWN:
                // printf("SDL_PollEvent: SDL_KEYDOWN %d %d (rep: %d)\n", e.key.keysym.mod, e.key.keysym.sym, e.key.repeat);
                if (e.key.repeat == 0) {
                    if (old_sdl_keypressed(core, e.key.keysym, 1) == 0) {
                        fct(core, (USBHID_id)e.key.keysym.scancode, 1);
                    }
                    stopPollEvent = true;             // To avoid KEYUP in the current look
                }
                break;

            case SDL_KEYUP:
                // printf("SDL_PollEvent: SDL_KEYUP %d %d (rep: %d)\n", e.key.keysym.mod, e.key.keysym.sym, e.key.repeat);
                if (e.key.repeat == 0) {
                    if (old_sdl_keypressed(core, e.key.keysym, 0) == 0) {
                        fct(core, (USBHID_id)e.key.keysym.scancode, 0);
                    }
                }
                break;

            case SDL_DROPFILE:
                printf("Drop file: %s\n", e.drop.file);

                if ((!core->isPaused) && (!core->inMenu)) {
                    strcpy(core->openFilename, e.drop.file);

                    ExecuteMenu(core, ID_INSERTDISK, NULL);             // Press cmd+a to load autorun
                } else {
                    printf("Paused (%d) or inMenu (%d)... we skip\n", core->isPaused, core->inMenu);
                }
                break;

            case SDL_MOUSEBUTTONDOWN: {
                int w, h;

#ifdef EMSCRIPTEN
                // Emscripten_ResumeAudio();
#endif

                SDL_GetRendererOutputSize(sdlRenderer, &w, &h);
                // printf("size: %d,%d\n", w, h);

                core->ipc.touchDown = 1;
                core->ipc.touchXpx = (int)((float)(e.button.x * core->screenBufferWidth) / w);
                core->ipc.touchYpx = (int)((float)(e.button.y * core->screenBufferHeight) / h);

                if (handlePaste != NULL) {
                    if (e.button.button == SDL_BUTTON_RIGHT) { // paste
                        char *buffer = SDL_GetClipboardText();
                        if (buffer != NULL) {
                            handlePaste(buffer);
                            SDL_free(buffer);
                        }

                        core->ipc.touchDown = 0;
                    }
                }

                printf("SDL.event.button.x: %d, SDL.event.button.y: %d\n", e.button.x, e.button.y);
                printf("x: %d, y: %d\n", core->ipc.touchXpx, core->ipc.touchYpx);
            }
            break;

            case SDL_MOUSEWHEEL: {

                // printf("wheel: %d,%d\n", (int)e.wheel.x, (int)e.wheel.y);

                core->ipc.wheelY = (int)e.wheel.y;
            }
            break;

            case SDL_MOUSEBUTTONUP:

#ifdef EMSCRIPTEN
                // Emscripten_ResumeAudio();
#endif

                break;

            default:
//                printf("Event: %d\n", e.type);
                break;
        }         /* switch */

        if (stopPollEvent) {
            break;
        }
    }             /* switch */

    const uint8_t *keys = SDL_GetKeyboardState(NULL);
    // printf("keys: %p\n", keys);
    // return;

    pad = pad2 = pad3 = 0;

    for (i = 0; i < 18; i++) {
        switch (i) {
            case 0:
                pad = PAD_LEFT;
                pad2 = 16;
                pad3 = JOYSTICK_LEFT;
                break;

            case 1:
                pad = PAD_RIGHT;
                pad2 = 16;
                pad3 = JOYSTICK_RIGHT;
                break;

            case 2:
                pad = PAD_UP;
                pad2 = 16;
                pad3 = JOYSTICK_UP;
                break;

            case 3:
                pad = PAD_DOWN;
                pad2 = 16;
                pad3 = JOYSTICK_DOWN;
                break;

            case 4:
                pad = PAD_A;
                pad2 = 2;
                pad3 = 0;
                break;

            case 5:
                pad = PAD_B;
                pad2 = 1;
                pad3 = 0;
                break;

            case 6:
                pad = PAD_X;
                pad2 = 0;
                pad3 = 0;
                break;

            case 7:
                pad = PAD_Y;
                pad2 = 3;
                pad3 = 0;
                break;

            case 8:
                pad = PAD_L;
                pad2 = 4;
                pad3 = 0;
                break;

            case 9:
                pad = PAD_R;
                pad2 = 5;
                pad3 = 0;
                break;

            case 10:
                pad = PAD_START;
                pad2 = 9;
                pad3 = 0;
                break;

            case 11:
                pad = PAD_SELECT;
                pad2 = 8;
                pad3 = 0;
                break;

            case 12:
                pad = PAD_QUIT;
                pad2 = 16;
                pad3 = 0;
                break;

            case 13:
                pad = PAD_L2;
                pad2 = 0;
                pad3 = 0;
                break;

            case 14:
                pad = PAD_R2;
                pad2 = 0;
                pad3 = 0;
                break;
        }             /* switch */

        // printf("Test pad: %d\n", pad);

        /*  if (CHECK_PAD) {
         *    printf("%02d ",i);
         * } else {
         *    printf("-- ");
         * }
         */

        if ((i == 11) && (CHECK_PAD)) {
            printf("guestButtons: select pressed %d\n", sdlframe);
        }

        switch (button_state[i]) {
            /* To avoid for the button for being pressed again */
            case -1:
                if (!(CHECK_PAD)) {
                    button_time[i]++;
                }

                if (button_time[i] > 1) {
                    button_state[i] = 0;
                    button_time[i] = 0;
                }
                break;

            case 0:
                if (CHECK_PAD) {
                    button_state[i] = 1;
                    button_time[i] = 0;
                }
                break;

            case 1:
                button_time[i]++;

                if (button_time[i] > 0) {
                    button_state[i] = 2;
                    button_time[i] = 0;
                }
                break;

            case 2:
                if (!(CHECK_PAD)) {
                    button_state[i] = 0;
                    button_time[i] = 0;
                }
                break;
        } /* switch */
    }
//    printf("\n");

    // printf("end: guestButtons\n");
}         /* Buttons */

void guestExit(void)
{
    ddlog(&gb, 2, "Guest exit\n");

    // SDL_PauseAudio(1);

    /* Free memory	*/

#ifdef SDL1
    if (actualScreen != NULL) {
        SDL_FreeSurface(actualScreen);
    }
#endif
// TODO: Free actualscreen on sdl2 ?

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    SDL_Quit();
}

#ifdef _WIN32

// int strcasecmp(char *a, char *b)
// {
// return strcmp(a, b);
// }

#endif

#ifndef _WIN32


uint32_t guestGetMilliSeconds(void)
{
    struct timeval tval;         /* timing	*/

    gettimeofday(&tval, 0);
    return (uint32_t)(((tval.tv_sec * 1000000) + (tval.tv_usec)));
}

#else
uint32_t guestGetMilliSeconds(void)
{
    // return GetTickCount();
    return SDL_GetTicks();

}

#endif

char guestCouldSendAudio(void)
{
    return (SDL_GetQueuedAudioSize(sdl_dev) < 4000);
}

void guestSleep(u32 milisec)
{
// /*
//  * nanosleep is better in every way.
//  * Only use SDL_Delay as a last resort.
//  */
// #if defined(POSIX) && !defined(_WIN32)

//     struct timespec req = {0};
//     time_t sec = (uint16_t)(milisec / 1000);

//     milisec = milisec - (sec * 1000);
//     req.tv_sec = sec;
//     req.tv_nsec = milisec * 1000000L;

//     while (nanosleep(&req, &req) == -1)
//         continue;
// #else

    if (milisec == 0) {
        while (SDL_GetQueuedAudioSize(sdl_dev) > 4000)
            SDL_Delay(2);
    }

    SDL_Delay(milisec);
// #endif
}         /* guestSleep */

#ifdef SDL1

void screen_draw_resize(int w, int h)
{
    if ((w < 320) || (h < 200)) {
        return;
    }

    w = w & 0xFFFE;         // Force odd
    h = h & 0xFFFE;

    actualScreen = SDL_SetVideoMode(w, h, 16, mySDLflags);

    if (incX != NULL) {
        free(incX);
    }
    if (incY != NULL) {
        free(incY);
    }
    incX = (u32 *)malloc(384 * 2 * sizeof(u32)); // malloc the max width
    incY = (u32 *)malloc(272 * sizeof(u32));     // malloc the max height

    printf("Resize to %dx%d\n", actualScreen->w, actualScreen->h);
}                                                /* screen_draw_resize */

#endif /* ifdef SDL1 */

u32 old_width1 = 0, old_height1 = 0, old_left1 = 0, old_top1 = 0, old_bpl1 = 0;
u16 old_width2 = 0, old_height2 = 0;

void guestBlit(core_crocods_t *core, u16 *memBitmap, u32 width1, u32 height1, u32 left1, u32 top1, u32 bpl1, u16 *buffer_scr, u16 width2, u16 height2)
{
    u16 myScanlineMask = 0;

    if ((core->scanline > 0) && (core->scanline < 5)) {
        myScanlineMask = scanlineMask[core->scanline - 1];
    }

    u32 x, y;

    if ((old_width1 != width1) || (old_height1 != height1) || (old_left1 != left1) || (old_top1 != top1) || (old_bpl1 != bpl1) || (old_width2 != width2) || (old_height2 != height2)) {
        if ((width2 > 384 * 2) || (height2 > 272)) {         // Games/demos that use splits or others video hack
            // ExecuteMenu(core, ID_SCREEN_OVERSCAN, NULL);
            // return;
        }

        for (x = 0; x < width2; x++) {
            incX[x] = ((x * width1) / width2) + left1;
        }
        for (y = 0; y < height2; y++) {
            incY[y] = (((y * height1) / (height2)) + top1) *  bpl1;
        }
        old_width1 = width1;
        old_height1 = height1;
        old_left1 = left1;
        old_top1 = top1;
        old_bpl1 = bpl1;
        old_width2 = width2;
        old_height2 = height2;
    }

    if (myScanlineMask != 0) {
        for (y = 0; y < height2; y++) {
            if (((y % 2) == 1)  & (myScanlineMask != 0)) {
                for (x = 0; x < width2; x++) {
                    int pos = incX[x] + incY[y];

                    u16 color = memBitmap[pos] & myScanlineMask;

                    *buffer_scr = color;         /// AlphaBlendFast(memBitmap[pos],AlphaBlendFast(memBitmap[pos],AlphaBlendFast(memBitmap[pos], memBitmap[pos-bpl1])));
                    buffer_scr++;
                }
            } else {
                for (x = 0; x < width2; x++) {
                    int pos = incX[x] + incY[y];

                    *buffer_scr = memBitmap[pos];
                    buffer_scr++;
                }
            }
        }
    } else {
        for (y = 0; y < height2; y++) {
            for (x = 0; x < width2; x++) {
                int pos = incX[x] + incY[y];

                *buffer_scr = memBitmap[pos];
                buffer_scr++;
            }
        }
    }
}         /* guestBlit */

void dispIcon8_over(core_crocods_t *core, u16 *bitmap, int width, int dbl_x, int dbl_y, int icon)
{
    int x, y;
    int x0;

    u16 *pdwAddr = bitmap;

    for (y = 0; y < 8; y++) {
        u16 *pdPixel;

        pdPixel = pdwAddr;

        for (x = 0; x < 8; x++) {
            u16 car;
            car = core->icons8[(x + icon * 8) + y * 320];

            for (x0 = 0; x0 < dbl_x; x0++) {
                if (car != RGB565(255, 0, 255)) {
                    *pdPixel = car;
                }
                pdPixel++;
            }
        }

        pdwAddr += width;
    }
} /* dispIcon */

void guestScreenDraw(core_crocods_t *core)
{
    if ((gb.turbo) && (core->runApplication == NULL)) {
        static int iframe = 0;

        iframe++;
        if (iframe >= 50) {
            iframe = 0;
        }

        if (iframe != 0) {
            return;
        }
    }

    uint16_t *buffer_scr = textureBytes;

    uint16_t *memBitmap;

    if (core->screenIsOptimized) {
        memBitmap = core->MemBitmap;
    } else {
        int ignoreRow = 0;         // 3; // only in caprice32 actually
        memBitmap = core->MemBitmap + 384 * 2 * ignoreRow;
    }

// printf("resize: %d\n", core->resize);

// printf("%x\n", buffer_scr);

#ifdef RPISDL1
    width2 = 384;
    height2 = 272;
#endif

    u32 width2 = 0, height2 = 0;

    if (core->resize == 2) {                     // 320x200
        width2 = 320;
        height2 = 200;
    }

    if (core->resize == 1) {                     // AUTO
        width2 = core->screenBufferWidth;
        height2 = core->screenBufferHeight;

        if ((width2 < 256) || (height2 < 128)) { // Games/demos that use splits or others video hack
            ExecuteMenu(core, ID_SCREEN_OVERSCAN, NULL);
        }
    }

    if (core->resize == 4) {                     // Overscan
        width2 = 384;
        height2 = 272;
    }

//    if ((!core->screenIsOptimized) || (core->lastMode == 2)) {
//      width2 = width2 * 2;
//    }

    if (core->lastMode == 2) {
        width2 = width2 * 2;
    }


    if (core->inConsole) {
        width2 = 384 * 2;
    }

    if (bufferWidth != width2) {
        bufferWidth = width2;
        bufferHeight = height2;

        printf("changeSize (%d) %dx%d\n", core->resize, width2, height2);

// free texture ?
        sdlTexture = SDL_CreateTexture(sdlRenderer,
                                       SDL_PIXELFORMAT_RGB565,
                                       SDL_TEXTUREACCESS_STREAMING,
                                       bufferWidth, bufferHeight);
    }

    int x, y;

    if (core->resize == 2) {
        // ID_SCREEN_320 x 200 - keep ratio

        if (core->screenIsOptimized) {
            int dbl;

            if (core->lastMode == 2) {
                dbl = 2;
            } else {
                dbl = 1;
            }

            // buffer_scr += 20 * actualScreen->w;      // TODO: add

            for (y = 0; y < height2; y++) {
                for (x = 0; x < width2; x++) {
                    int pos = (x * core->screenBufferWidth * dbl) / (width2) + ((y * core->screenBufferHeight) / (height2)) * core->screenBufferWidth * dbl;

                    *buffer_scr = memBitmap[pos];
                    buffer_scr++;
                }
            }

            // for (y = 0; y < 200; y++) {
            //     memcpy(buffer_scr + 320 * 20 + y * 320, core->MemBitmap + y * core->MemBitmap_width, 320 * 2);
            // }

            // for (y = 0; y < 240; y++) {
            //     for (x = 0; x < 320; x++) {
            //         int pos = (x * 320 * dbl) / 320 + ((y * 200) / 240) * core->screenBufferWidth;

            //         *buffer_scr = core->MemBitmap[pos];
            //         buffer_scr++;
            //     }
            // }

            #define PG_LBMASK565 0xF7DE

            #define AlphaBlendFast(pixel, backpixel) (((((pixel)&PG_LBMASK565) >> 1) | (((backpixel)&PG_LBMASK565) >> 1)))

            uint16_t *buffer_scr = textureBytes;
            // char pos[]={10,8,6,4,3,3,2,2,1,1};
            char pos[] = {10, 7, 5, 4, 3, 2, 2, 1, 1, 1};

            // uint16_t col = core->MemBitmap[0];

            uint16_t col = core->BG_PALETTE[core->TabCoul[ 16 ]];         // Border color

            for (y = 0; y < 20; y++) {
                for (x = 0; x < 320; x++) {
                    buffer_scr[x + y * 320] = col;
                    buffer_scr[x + (y + 220) * 320] = col;
                }
            }

            for (y = 0; y < 10; y++) {
                for (x = 0; x < pos[y]; x++) {
                    buffer_scr[x + y * 320] = 0;
                    buffer_scr[(319 - x) + y * 320] = 0;
                    buffer_scr[x + (239 - y) * 320] = 0;
                    buffer_scr[(319 - x) + (239 - y) * 320] = 0;
                }
            }
        } else {
            u32 left1 = 32 * 2;
            u32 top1 = 40;
            u32 width1 = 320 * 2;
            u32 height1 = 200;
            u32 bpl1 = 768;         // byte per line

            // TODO: fix
            if (height2 == 240) {   // Round border on GCW
                u32 x, y;

                char pos[] = {10, 7, 5, 4, 3, 2, 2, 1, 1, 1};

                u16 myScanlineMask = 0;

                if ((core->scanline > 0) && (core->scanline < 5)) {
                    myScanlineMask = scanlineMask[core->scanline - 1];
                }

                uint16_t col = core->BG_PALETTE[core->TabCoul[ 16 ]];         // Border color

                for (y = 0; y < 20; y++) {
                    for (x = 0; x < 320; x++) {
                        if (((y % 2) == 1)  & (myScanlineMask != 0)) {
                            u16 color = col & myScanlineMask;

                            buffer_scr[x + y * 320] = color;
                            buffer_scr[x + (y + 220) * 320] = color;
                        } else {
                            buffer_scr[x + y * 320] = col;
                            buffer_scr[x + (y + 220) * 320] = col;
                        }
                    }
                }

                for (y = 0; y < 10; y++) {
                    for (x = 0; x < pos[y]; x++) {
                        buffer_scr[x + y * 320] = 0;
                        buffer_scr[(319 - x) + y * 320] = 0;
                        buffer_scr[x + (239 - y) * 320] = 0;
                        buffer_scr[(319 - x) + (239 - y) * 320] = 0;
                    }
                }

                height2 = 200;
                buffer_scr += 20 * width2;
            }

            guestBlit(core, memBitmap, width1, height1, left1, top1, bpl1,
                      buffer_scr, width2, height2);
        }
    }

// printf("bef resize 1\n");

    if (core->resize == 1) {         // TODO: improve resize
        // ID_SCREEN_AUTO

        // int width = core->screenBufferWidth;
        // if (width > 320)
        //  width = 320;

        // for (y = 0; y < 200; y++)
        // {
        //  memcpy(buffer_scr + 320 * 20 + y * 320, core->MemBitmap + y * core->MemBitmap_width, width * 2);
        // }

        // printf("resize 1\n");

        // printf("opt: %d\n", core->screenIsOptimized);

        if (core->screenIsOptimized) {
            int dbl;

            if (core->lastMode == 2) {
                dbl = 2;
            } else {
                dbl = 1;
            }

            for (y = 0; y < height2; y++) {
                for (x = 0; x < width2; x++) {
                    int pos = (x * core->screenBufferWidth * dbl) / (width2) + ((y * core->screenBufferHeight) / (height2)) * core->screenBufferWidth * dbl;

                    *buffer_scr = memBitmap[pos];
                    buffer_scr++;
                }
            }
        } else {
            // Copy 1 to 2

            // printf("draw %p %p %dx%d\n", incX, incY, width2, height2);

            u32 left1 = core->x0 * 2;
            u32 top1 = core->y0;
            u32 width1 = core->screenBufferWidth * 2;
            u32 height1 = core->screenBufferHeight;
            u32 bpl1 = 768;         // byte per line

            guestBlit(core, memBitmap, width1, height1, left1, top1, bpl1,
                      buffer_scr, width2, height2);

            /*
             * // printf(" incx\n");
             *
             * for (x = 0; x < width2; x++) {
             *  incX[x] = ((x * width1) / width2) + left1;
             * }
             *
             * // printf("end incx\n");
             *
             * for (y = 0; y < height2; y++) {
             *  incY[y] = (((y * height1) / (height2)) + top1) *  bpl1;
             * }
             *
             * // printf("end incy\n");
             * // printf("print to %p from %p\n", buffer_scr, memBitmap);
             *
             * for (y = 0; y < height2; y++) {
             *  if (((y % 2) == 1)  & (myScanlineMask != 0)) {
             *      for (x = 0; x < width2; x++) {
             *          int pos = incX[x] + incY[y];
             *
             * buffer_scr = memBitmap[pos] & myScanlineMask;
             *          buffer_scr++;
             *      }
             *  } else {
             *      for (x = 0; x < width2; x++) {
             *          int pos = incX[x] + incY[y];
             *
             * buffer_scr = memBitmap[pos];
             *          buffer_scr++;
             *      }
             *  }
             * }
             */

            // printf("end of draw\n");
        }
    }

    if (core->resize == 4) {         // TODO: improve resize
        // ID_SCREEN_OVERSCAN

        if (core->screenIsOptimized) {
            int dbl;

            if (core->lastMode == 2) {
                dbl = 2;
            } else {
                dbl = 1;
            }

            for (y = 0; y < height2; y++) {
                for (x = 0; x < width2; x++) {
                    int pos = (x * 384 * dbl) / (width2) + ((y * 272) / (height2)) * 384 * dbl;

                    *buffer_scr = memBitmap[pos];
                    buffer_scr++;
                }
            }
        } else {
            u32 left1 = 0;
            u32 top1 = 0;
            u32 width1 = 768;
            u32 height1 = 272;
            u32 bpl1 = 768;         // byte per line

            // u32 left2 = 0;
            // u32 top2 = 0;

            guestBlit(core, memBitmap, width1, height1, left1, top1, bpl1,
                      buffer_scr, width2, height2);

            /*
             *
             * for (x = 0; x < width2; x++) {
             *  incX[x] = ((x * width1) / width2) + left1;
             * }
             * for (y = 0; y < height2; y++) {
             *  incY[y] = (((y * height1) / (height2)) + top1) *  bpl1;
             * }
             *
             * for (y = 0; y < height2; y++) {
             *  if (((y % 2) == 1)  & (myScanlineMask != 0)) {
             *      for (x = 0; x < width2; x++) {
             *          int pos = incX[x] + incY[y];
             *
             * buffer_scr = memBitmap[pos] & myScanlineMask;
             *          buffer_scr++;
             *      }
             *  } else {
             *      for (x = 0; x < width2; x++) {
             *          int pos = incX[x] + incY[y];
             *
             * buffer_scr = memBitmap[pos];
             *          buffer_scr++;
             *      }
             *  }
             * }
             */
        }
    }

    // printf("%d\n", core->overlayBitmap_width);

    if ((core->messageTimer > 0) || (core->iconTimer > 0) || (core->overlayBitmap_width != 0)) {
        int dbl_x = (core->lastMode == 2) ? 2 : 1;
        int dbl_y = 1;

        if (core->inConsole) {
            memcpy(textureBytes, core->consoleBitmap,  384 * 2 * 288 * sizeof(uint16_t));
        }


        dbl_x = (width2 / 320);
        dbl_y = (height2 / 200);

        if (dbl_x < 1) {
            dbl_x = 1;
        }
        if (dbl_y < 1) {
            dbl_y = 1;
        }

        // Draw message



        if (core->messageTimer > 0) {
            int n;

            core->messageTimer--;

            u16 *buffer_scr = textureBytes;

            // buffer_scr += (width2 - (strlen(core->messageToDislay) * 8 * dbl_x)) / 2;
            buffer_scr += (width2 - ((strlen(core->messageToDislay) + 2) * 8 * dbl_x));

            buffer_scr += width2 * (height2 - 30);

            u16 bColor = RGB565(0, 0, 0);
            // u16 bColor = RGB565(0xFF, 0xFF, 0xFF);
            u16 backgroundColor = RGB565(132, 155, 174);


            // u16 *pdPixel = buffer_scr;

            for (n = 0; n < strlen(core->messageToDislay); n++) {
                if (core->messageToDislay[n] == '[') {
                    dispIcon8_over(core, buffer_scr + 8 * n * dbl_x, width2, dbl_x, dbl_y, 11);
                    dispIcon8_over(core, buffer_scr + 8 * n * dbl_x + width2 * dbl_y * 8, width2, dbl_x, dbl_y, 14);
                } else if (core->messageToDislay[n] == ']') {
                    dispIcon8_over(core, buffer_scr + 8 * n * dbl_x, width2, dbl_x, dbl_y, 13);
                    dispIcon8_over(core, buffer_scr + 8 * n * dbl_x + width2 * dbl_y * 8, width2, dbl_x, dbl_y, 16);
                } else {
                    dispIcon8_over(core, buffer_scr + 8 * n * dbl_x, width2, dbl_x, dbl_y, 12);
                    dispIcon8_over(core, buffer_scr + 8 * n * dbl_x + width2 * dbl_y * 8, width2, dbl_x, dbl_y, 15);
                }
            }

            buffer_scr += width2 * 4 * dbl_y;

            if (1 == 1) {
                buffer_scr -= width2;

                for (n = 0; n < strlen(core->messageToDislay); n++) {
                    int iRow, iCol;
                    int mx, my;
                    u16 *pdwLine;


                    if (core->messageToDislay[n] == '[') {
                        // pdPixel += (8 * dbl_x);
                    } else if (core->messageToDislay[n] == ']') {
                        // pdPixel += (8 * dbl_x);
                    } else {

                        u16 iIdx = (int)core->messageToDislay[n];
                        iIdx -= FNT_MIN_CHAR;
                        iIdx = iIdx * 8;

                        pdwLine = buffer_scr + n * 8 * dbl_x;

                        for (iRow = 0; iRow < FNT_CHAR_HEIGHT; iRow++) { // loop for all rows in the font character
                            for (my = 0; my < dbl_y; my++) {
                                u16 *pdPixel;

                                pdPixel = pdwLine;
                                u8 bRow = bFont[iIdx];


                                for (iCol = 0; iCol < 8; iCol++) { // loop for all columns in the font character
                                    for (mx = 0; mx < dbl_x; mx++) {

                                        if (bRow & 0x80) {
                                            *pdPixel = bColor;
                                        } else {
                                            *pdPixel =  backgroundColor;
                                        }

                                        pdPixel++;
                                    }

                                    bRow <<= 1; // advance to the next bit
                                }
                                pdwLine += width2;
                            }
                            iIdx++;
                        }
                    }
                }
            }

            if (1 == 1) {
                buffer_scr += width2 - 1;

                u16 bColor = RGB565(0xFF, 0xFF, 0xFF);

                for (n = 0; n < strlen(core->messageToDislay); n++) {
                    int iRow, iCol;
                    int mx, my;
                    u16 *pdwLine;


                    if (core->messageToDislay[n] == '[') {
                        // pdPixel += (8 * dbl_x);
                    } else if (core->messageToDislay[n] == ']') {
                        // pdPixel += (8 * dbl_x);
                    } else {

                        u16 iIdx = (int)core->messageToDislay[n];
                        iIdx -= FNT_MIN_CHAR;
                        iIdx = iIdx * 8;

                        pdwLine = buffer_scr + n * 8 * dbl_x;

                        for (iRow = 0; iRow < FNT_CHAR_HEIGHT; iRow++) { // loop for all rows in the font character
                            for (my = 0; my < dbl_y; my++) {
                                u16 *pdPixel;

                                pdPixel = pdwLine;
                                u8 bRow = bFont[iIdx];


                                for (iCol = 0; iCol < 8; iCol++) { // loop for all columns in the font character
                                    for (mx = 0; mx < dbl_x; mx++) {

                                        if (bRow & 0x80) {
                                            *pdPixel = bColor;
                                        }

                                        pdPixel++;
                                    }

                                    bRow <<= 1; // advance to the next bit
                                }
                                pdwLine += width2;
                            }
                            iIdx++;
                        }
                    }
                }
            }
        }


        // Draw icon

        if (core->iconTimer > 0) {
            int x, y;
            int y0;

            int dispiconX = core->iconToDislay / 16;
            int dispiconY = core->iconToDislay % 16;

            uint16_t *buffer_scr = textureBytes;

            buffer_scr += +(8 * width2) + 8;

            for (y = 0; y < 32; y++) {
                for (y0 = 0; y0 < dbl_y; y0++) {
                    int step_x = 0;
                    u16 *src = core->icons + (dispiconX * 32) + (y + dispiconY * 32) * 448;

                    for (x = 0; x < 32 * dbl_x; x++) {
                        u16 car;
                        car = *src;
                        if (car != 33840) {
                            *buffer_scr = car;
                        }
                        buffer_scr++;
                        step_x++;
                        if (step_x == dbl_x) {
                            step_x = 0;
                        }
                        if (step_x == 0) {
                            src++;
                        }
                    }

                    buffer_scr += (width2 - 32 * dbl_x);
                }
            }

            // dispIcon(core, 0, 0, core->iconToDislay / 16, core->iconToDislay % 16, 0);
            core->iconTimer--;
        }         // End of icon



        if (core->overlayBitmap_width != 0) {
            int y0;

            uint16_t *buffer_scr = textureBytes;

            for (y = 0; y < core->overlayBitmap_height; y++) {
                for (y0 = 0; y0 < dbl_y; y0++) {
                    u16 *dest = buffer_scr;
                    if (core->overlayBitmap_center == 1) {
                        core->overlayBitmap_posx = ((width2 -  core->overlayBitmap_width * dbl_x) / 2);
                        core->overlayBitmap_posy = ((height2 -  core->overlayBitmap_height * dbl_y) / 2);
                    }
                    dest += width2 * core->overlayBitmap_posy + core->overlayBitmap_posx + (y * dbl_y + y0) * width2;

                    // +actualScreen->w * core->overlayBitmap_posy + core->overlayBitmap_posx + y * actualScreen->w;
                    u16 *src = core->overlayBitmap + y * 320;
                    int step_x = 0;
                    for (x = 0; x < core->overlayBitmap_width * dbl_x; x++) {
                        u16 car = *src;
                        if (car != 63519) {         // RGB565(255,0,255)
                            *dest = car;
                        }
                        dest++;
                        if (step_x == 0) {
                            src++;
                        }
                        step_x++;
                        if (step_x == dbl_x) {
                            step_x = 0;
                        }
                    }
                }
            }
        }         // End of overlay

    }

// printf("SDL_RenderClear\n");
    SDL_RenderClear(sdlRenderer);

//    SDL_UpdateTexture(ctx->display->tex, NULL, ctx->display->pixels, 64*sizeof(uint32_t));

// printf("SDL_UpdateTexture\n");
    SDL_UpdateTexture(sdlTexture, NULL, textureBytes, width2  * sizeof(uint16_t));

    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);

    SDL_RenderPresent(sdlRenderer);
}         /* screen_draw */
