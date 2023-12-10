#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// #ifndef _WIN32
// #include <sys/time.h>
// #else
// #include <SDL2/SDL.h>
// #endif
#include <time.h>
#include <getopt.h> /* for getopt_long; POSIX standard getopt is in unistd.h */
#include "resources.h"

#include "shared.h"
#include "guest.h"
#include "game_input.h"
#include "multitools.h"

#define RGB565(R, G, B) ((((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))

extern const unsigned char icons_gif[];

#ifdef CLI
#define MAINFILENAME "crocodsCLI"
#else
#define MAINFILENAME "crocods"
#endif


void WsRun(void);
void WsInit(void);

core_crocods_t gb;
int bx, by;

// int samples; // to delete

u16 pixels[384 * 288 * 2]; // Verifier taille

#define maxByCycle 400 // 50 fois par frame

uint32_t m_Flag;
uint32_t interval;

gamecfg GameConf;
char gameName[512];

uint32_t nextTick, lastTick = 0, newTick, currentTick, firstTick;

uint32_t expectedTime = 0, ellapsedTime = 0;

char framebuf[128];

int frame = 0;

#ifdef EMSCRIPTEN

EMSCRIPTEN_KEEPALIVE void emsc_load_browsebuffer(const uint8_t *ptr, int size)
{
    apps_browser_init0(&gb, 0);

    apps_browser_usebuffer(&gb, ptr, size);
}

EMSCRIPTEN_KEEPALIVE void emsc_load_data(const char *path, const uint8_t *ptr, int size)
{
    // fs_load_mem(path, ptr, size);

    printf("emsc_load_data: %s (size: %d)\n", path, size);

    UseDatafile(&gb, ptr, size, path);

    ExecuteMenu(&gb, ID_PAUSE_EXIT, NULL);
    ExecuteMenu(&gb, ID_AUTORUN, NULL); // Do: open disk & autorun // TODO
}

EMSCRIPTEN_KEEPALIVE void emsc_press_pad(int pressed, int button)
{
    button_virtual[button] = pressed;
}

EMSCRIPTEN_KEEPALIVE void emsc_press_key(int pressed, int cpc_scancode)
{
    if (pressed) {
        CPC_SetScanCode(&gb, cpc_scancode);
    } else {
        CPC_ClearScanCode(&gb, cpc_scancode);
    }
}

EMSCRIPTEN_KEEPALIVE void emsc_execute_menu(int menu_id)
{
    ExecuteMenu(&gb, menu_id, NULL);
}

EM_JS(void, emsc_fs_init, (void), {
    console.log("fs.h: registering Module['ccall']");
    Module['ccall'] = ccall;

    crocodsAppIsReady();
});

EM_JS(void, emsc_load_file, (const char *path_cstr), {
    var path = UTF8ToString(path_cstr);
    var req = new XMLHttpRequest();
    req.open("GET", path);
    req.responseType = "arraybuffer";
    req.onload = function(e) {
        var uint8Array = new Uint8Array(req.response);
        var res = ccall('emsc_load_data',
                        'int',
                        ['string', 'array', 'number'],
                        [path, uint8Array, uint8Array.length]);
    };
    req.send();
});

#endif /* ifdef EMSCRIPTEN */

extern int sndbufend, sndbufbeg;
extern GB_sample_t *sndbuf;


void WsRun(void)
{
    frame++;

    if ((gb.cliStopAfter != 0) && (frame == gb.cliStopAfter)) {
        if (gb.cliSaveSnaphsot) {
            ExecuteMenu(&gb, ID_SAVESNAP, NULL);
        }
        if (gb.cliSaveImage) {
            saveScreen(&gb);
        }
        if (gb.cliSpeedTest) {
            float msperframe = (float)(guestGetMilliSeconds() - firstTick) / gb.cliStopAfter;
            ddlog(&gb, 0, "%f millisecondss / frame\n", msperframe);

            ddlog(&gb, 0, "%0.2fx quicker\n", (1000000 / 50) / msperframe);
        }

        ddlog(&gb, 1, "Stop after %d frames\n", gb.cliStopAfter);
        ExecuteMenu(&gb, ID_EXIT, NULL);
    }

    if (!gb.turbo) {

        currentTick = guestGetMilliSeconds();
        int32_t crwait = (nextTick - currentTick);

// printf("%d - %d = %d (%d)\n", nextTick, currentTick, crwait, interval);

        if (crwait > 0) {
            if (crwait < 1000000) {
                // guestSleep(crwait / 1000);
// printf("%d - %d = %d (%d)\n", nextTick, currentTick, crwait, interval);

            }
        } else {
            // printf("Quicker\n");
        }
        // nextTick = currentTick;
    }

    if (!gb.isPaused) { // Pause only the Z80
        long ticks = guestGetMilliSeconds();

        sndbufbeg = 0;
        sndbufend = 0;

        croco_cpu_doFrame(&gb);

        guest_queue_audio(sndbuf, sndbufend * 4);

        // int n;
        // char *out = (char *)sndbuf;
        // for (n = 0; n < sndbufend; n++) {
        //     printf("%02x", out[n]);
        // }
        // printf("\n");

        // ddlog(&gb, 2, "after croco_cpu_doFrame: have %d (%d %d)\n", (sndbufend - sndbufbeg + SNDBUFSIZE) % SNDBUFSIZE, sndbufbeg, sndbufend);

        expectedTime += 1000000 / 50; // (50 hz & 1000 ms)
        ellapsedTime += (guestGetMilliSeconds() - ticks);

#ifndef EMSCRIPTEN

        guestSleep(0);

        if (!gb.turbo) {
        #ifndef CLI
            while (waitSound(&gb)) guestSleep(2);      // Synchronize sound
        #endif
        }

#else
        if (!gb.turbo) {
            uint32_t ellapsedTime0 = ellapsedTime;

            do {
                ellapsedTime = ellapsedTime0 + (guestGetMilliSeconds() - ticks);
            } while (ellapsedTime < expectedTime);
        }
#endif
    }

    if (gb.dispframerate) {
        cpcprintOnScreen(&gb, framebuf);
    }

    u16 keys_pressed = WsInputGetState(&gb);

    if (gb.runApplication != NULL) {
        gb.runApplication(&gb, (gb.wait_key_released == 1) ? 0 : keys_pressed);
    }

    // printf("UpdateScreen\n");

    UpdateScreen(&gb);

    if (keys_pressed == 0) {
        gb.wait_key_released = 0;
    }

    if ((gb.runApplication == NULL) && (gb.wait_key_released == 0)) {
        gb.ipc.keys_pressed = keys_pressed;
    }

    // printf("Readkey\n");

    nds_ReadKey(&gb);

    // if (gb.inKeyboard)
    // {
    // gb.ipc.keys_pressed = 0;
    // memset(gb.clav, 0xFF, sizeof(gb.clav));
    // }

    // printf("guestScreenDraw\n");

    // myprintf("guestScreenDraw");

    guestScreenDraw(&gb);

    // printf("updateScreenBuffer\n");

    updateScreenBuffer(&gb, gb.MemBitmap, gb.screenBufferWidth);

    gb.overlayBitmap_width = 0;

    // Synchronisation de l'image ‡ 50 Hz
    // Pourcentage, temps espere, temps pris, temps z80, nombre de drawligne

    if (gb.dispframerate) {
        int percent = (int)(((float)expectedTime / (float)ellapsedTime) * 100);

        if (percent > 500) {
            sprintf(framebuf, "%4dx", percent / 100);
        } else {
            sprintf(framebuf, "%4d%%", percent);
        }


//                            NSLog(@"%s", framebuf);

        cpcprint16(&gb, gb.MemBitmap, gb.MemBitmap_width, 0, 0, framebuf, RGB565(0xFF, 0xFF, 0x00), RGB565(0x00, 0x00, 0xFF), 1, 0);
    }
    // printf("expect: %d, ellapsed: %d\n", expectedTime, ellapsedTime);


    if (gb.frameBeforeConsole > 0) {

        sprintf(gb.messageToDislay, "%d",  gb.frameBeforeConsole);
        gb.messageTimer = 2;

        printf("framebeforeconsole: %d - %d\n", gb.frameBeforeConsole, guestGetMilliSeconds());

        gb.frameBeforeConsole--;
        if (gb.frameBeforeConsole == 0) {
            apps_console_init(&gb, 1);
        }
    }

    // printf("end of wsrun\n");

    nextTick += interval;
} /* WsRun */



char autoString[256];

u8 *disk = NULL;
u32 diskLength;

u8 *snapshot = NULL;
u32 snapshotLength;

void WsInit(void)
{
    // char *savedir = NULL;
    // int i;

    // environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &savedir);

    // char oldOpenFilename[MAX_PATH];

    // strcpy(oldOpenFilename, gb.openFilename);

    // memset(&gb, 0, sizeof(gb));

    // strcpy(gb.openFilename, oldOpenFilename);

    // Get map layout (initialized in guestInit)

    // gb.keyboardLayout = 1; // French
    // gb.keyboardLayout = 0; // Default

    // Init nds

    nds_initBorder(&gb, &bx, &by);
    nds_init(&gb);

#ifndef CLI
    console_init(&gb);
    // if (gb.useWebServer) {      // TODO: add configuration in in
    //     web_init(&gb);
    // }
#endif

    strcpy(gb.snap_dir, "."); // Use current directory

    ddlog(&gb, 2, "Snapdir: %s (%p)", gb.snap_dir, gb.snap_dir);

    // ExecuteMenu(&gb, ID_SCREEN_320, NULL);
    // ExecuteMenu(&gb, ID_KEY_JOYSTICK, NULL);

    // ExecuteMenu(&gb, ID_KEY_KEYPAD, NULL);

    AutoType_Init(&gb);
    HardResetCPC(&gb);

    initSound(&gb, 44100);


#ifdef EMSCRIPTEN
    emsc_fs_init();
#endif
} /* WsInit */

#define KRED "\x1B[31m"
#define KNRM "\x1B[0m"


void help(void)
{
    printf("\n");
    printf("%s%s%s %s\n", KRED, MAINFILENAME, KNRM, CROCOVERSION);
    printf("build: %s %s\n", __DATE__, __TIME__);
    printf("\n");
    printf("Usage: %s [OPTIONS]\n", MAINFILENAME);
    printf("\n");
    printf("A swissknife for the Amstrad CPC\n");
    printf("\n");
    printf("Options:\n");
    printf(" -B, --loadBasic file     Load a file and save it to snapshot. (eq to --stopAfter 500 --saveSnaphsot --autoLoad 1)\n");
    printf("     --sna2gif file       Convert a snapshot to a gif. (eq to --stopAfter 1 --saveImage)\n");
    printf(" -i, --getInfo file       Get information of a file\n");
    printf("     --speedTest          Do a speedtest (time do run 5000 frames).\n");
    printf("Advanced:\n");
    printf("     --stopAfter frame    Stop execution after x frames.\n");
    printf("     --autoLoad <0|1>     Load file in place of run when selecting file in the catalog.\n");
    printf("     --autoRun <0|1>      Autorun first file in catalog. (true by default in cli mode)\n");
    printf("     --saveSnaphsot       Save snapshot when quit.\n");
    printf("     --saveImage          Save image when quit.\n");
    printf(" -c, --command            Send the command key by key to the emulator\n");
#ifndef CLI
    printf("Emulator:\n");
    printf(" -f,  --fullScreen        Start the emulator in fullscreen.\n");
    printf(" -w,  --warp              Start the emuator in warp mode.\n");
#endif
    printf("More:\n");
    printf(" -v, --verbose            Cause %s to be verbose.\n", MAINFILENAME);
    printf(" -d, --debug              Cause %s to display debug messages.\n", MAINFILENAME);
    printf(" -h, --help               This help.\n");
    printf("\n");
    printf("\n");
} /* help */

int main(int argc, char **argv)
{

    int c;

    static struct option long_options[] = {
        {"loadBasic",    no_argument,               NULL,                         'B'},
        {"sna2gif",      no_argument,               NULL,                         0},
        {"stopAfter",    required_argument,         NULL,                         0},
        {"verbose",      no_argument,               NULL,                         'v',                       },
        {"debug",        no_argument,               NULL,                         'd',                       },
        {"help",         no_argument,               NULL,                         'h'},
        {"getInfo",      no_argument,               NULL,                         'i'},
        {"file",         required_argument,         NULL,                         0},
        {"saveSnaphsot", no_argument,               NULL,                         0},
        {"saveImage",    no_argument,               NULL,                         0},
        {"speedTest",    no_argument,               NULL,                         0},
        {"autoLoad",     required_argument,         NULL,                         0},
        {"autoRun",      required_argument,         NULL,                         0},
        {"fullScreen",   no_argument,               NULL,                         'f'},
        {"warp",         no_argument,               NULL,                         'w'},
        {"command",      required_argument,         NULL,                         'c'},
        {NULL,           0,                         NULL,                         0}
    };
    int option_index = 0;

    memset(&gb, 0, sizeof(gb));


#ifdef NOROMLOADER
    if (argc < 2) return 0;
#endif


#ifdef CLI
    gb.cliMode = 1;
#endif

    if (gb.cliMode) {
        gb.cliAutorun = 1;
    }

    while ((c = getopt_long(argc, argv, "h?BZ:vdifwc:", long_options, &option_index)) != -1) {
        // int this_option_optind = optind ? optind : 1;
        switch (c) {
            case 0:
                ddlog(&gb, 2, "option %s", long_options[option_index].name);
                if (optarg) {
                    ddlog(&gb, 2, " with arg %s", optarg);
                }
                ddlog(&gb, 2, "\n");

                if (!strcmp(long_options[option_index].name, "stopAfter")) {
                    gb.cliStopAfter = atol(optarg);
                    ddlog(&gb, 2, "Set stopAfter to %d\n", gb.cliStopAfter);
                } else if (!strcmp(long_options[option_index].name, "autoLoad")) {
                    gb.cliAutoload = atol(optarg);
                    ddlog(&gb, 2, "Set loadFile to %d\n", gb.cliStopAfter);
                } else if (!strcmp(long_options[option_index].name, "autoRun")) {
                    gb.cliAutorun = atol(optarg);
                    ddlog(&gb, 2, "Set autoRun to %d\n", gb.cliStopAfter);
                } else if (!strcmp(long_options[option_index].name, "saveSnaphsot")) {
                    gb.cliSaveSnaphsot = 1;
                } else if (!strcmp(long_options[option_index].name, "saveImage")) {
                    gb.cliSaveImage = 1;
                } else if (!strcmp(long_options[option_index].name, "speedTest")) {
                    gb.cliSpeedTest = 1;
                    gb.cliStopAfter = 500;
                } else if (!strcmp(long_options[option_index].name, "sna2gif")) {
                    gb.cliSaveImage = 1;
                    gb.cliStopAfter = 1;
                }

                break;
            case 'c':
                gb.cliCommand = (char *)malloc(strlen(optarg) + 1);
                strcpy(gb.cliCommand, optarg);
                break;
            case 'f':
                gb.cliFullScreen = 1;
                break;
            case 'i':
                gb.cliGetInfo = 1;
                break;
            case 'v':
                gb.cliVerbose = 1;
                break;
            case 'w':
                gb.cliWarp = 1;
                break;
            case 'd':
                gb.cliVerbose = 2;
                break;
            case 'B':
                gb.cliAutoload = 1;
                gb.cliStopAfter = 500;
                gb.cliSaveSnaphsot = 1;
                break;
            case '?':
            case 'h':
                help();
                exit(EXIT_SUCCESS);
                break;
            default:
                printf("?? getopt returned character code 0%o ??\n", c);
        } /* switch */
    }

    strcpy(gameName, ""); // Empty game

    if (optind < argc) {
        while (optind < argc) {
            if (gameName[0] == 0) { // First argument is game filename
                char fullpath[512];

                current_workpath(fullpath);
                ddlog(&gb, 2, "fullpath: %s\n", fullpath);
                apps_disk_path2Abs(fullpath, argv[optind++]);
                ddlog(&gb, 2, "fullpath: %s\n", fullpath);

                strcpy(gameName, fullpath);
            }
        }
    }

    if ((argc == 1) && (gb.cliMode)) {
        help();
        exit(EXIT_SUCCESS);
    }

    loadIniGuest(&gb); // Force loading ini to read width & height for guest
    guestInit();


    m_Flag = GF_GAMEINIT;
    WsInit();

    if (gb.cliFullScreen) {
        ExecuteMenu(&gb, ID_FULLSCREEN_ON, NULL);
    }
    if (gb.cliWarp) {
        ExecuteMenu(&gb, ID_ENABLE_TURBO, NULL);
    }
    if (gb.cliCommand != NULL) {
        AutoType_SetString(&gb,  gb.cliCommand, 1);
    }

    while (m_Flag != GF_GAMEQUIT) {
        switch (m_Flag) {
            case GF_GAMEINIT:

                firstTick = guestGetMilliSeconds();

                m_Flag = GF_GAMERUNNING;
                /* Init timing */
                interval = (1.0 / 50) * 1000000;
                ddlog(&gb, 2, "interval: %d\n", interval);
                nextTick = guestGetMilliSeconds() + interval;

                strcpy(gb.openFilename, gameName);

                if (gb.cliCommand == NULL) {
                    ExecuteMenu(&gb, ID_AUTORUN, NULL);
                } else {
                    ExecuteMenu(&gb, ID_INSERTDISK, NULL);
                }
                // if (gb.soundEnabled) {
                guestStartAudio();
                // }
                // mydebug(&gb, "sample: %d\n", samples);

                if (!gb.standalone) {
                    // ExecuteMenu(&gb, ID_SHOW_INFOPANEL_SHORTCUTS, NULL);
                }
                break;

            case GF_GAMERUNNING: {
                #ifdef EMSCRIPTEN
                emscripten_set_main_loop(WsRun, 0, 0);
                emscripten_exit_with_live_runtime();
#else
                WsRun();
#endif
            }
            break;
        } /* switch */
    }

    guestExit();
    return 0;
} /* main */
