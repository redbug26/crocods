//
//  ViewController.m
//  crocods
//
//  Created by Miguel Vanhove on 04/03/2020.
//  Copyright © 2020 Miguel Vanhove. All rights reserved.
//

#import "AppDelegate.h"
#import "ViewController.h"
#import "DefinedUIApplication.h"

#import "GBView.h"
#import "GBAudioClient.h"

#define USEDBOOL

#include "plateform.h"
#include <sys/time.h>

static ViewController *sharedInstance;

#define RGB565toRGB(rgb565) ((uint32_t)(((uint8_t)((((rgb565) & 0xF800) >> 11) << 3) | ((uint32_t)((uint8_t)((((rgb565) & 0x07E0) >> 5) << 2)) << 8)) | (((uint32_t)(uint8_t)(((rgb565) & 0x001F) << 3)) << 16)))

@interface ViewController () <UIDropInteractionDelegate>
{
    BOOL running;
    core_crocods_t gb;
    int bufferWidth, bufferHeight;
    int screenWidth;
    int bx, by;
    u16 button_state[18];

    CGFloat ratio;

    int marge;

    NSMutableArray<UIDragItem *> *sessions;
}

@property (strong, nonatomic) GBAudioClient *audioClient;

@end

@implementation ViewController

+ (ViewController *) sharedInstance {
    return sharedInstance;
}

- (void) loadData:(NSData *)data withAutorun:(BOOL)autorun andFilename:(NSString *)filename {
    UseDatafile(&gb, data.bytes, (int)data.length, [filename UTF8String]);

    ExecuteMenu(&gb, ID_PAUSE_EXIT, NULL);
    ExecuteMenu(&gb, ID_AUTORUN, NULL);             // Do: open disk & autorun // TODO
}

- (void) loadFilename:(NSString *)filename withAutorun:(BOOL)autorun {
    strcpy(gb.openFilename, [filename UTF8String]);

    ExecuteMenu(&gb, ID_PAUSE_EXIT, NULL);
    ExecuteMenu(&gb, ID_AUTORUN, NULL);             // Do: open disk & autorun // TODO
}

- (void) viewDidLoad {
    sharedInstance = self;      // TODO: place it in init

    [super viewDidLoad];

    [self initCGB];

    [self start];

    UIDropInteraction *dropAction = [[UIDropInteraction alloc] initWithDelegate:self];

    [self.view addInteraction:dropAction];

    AppDelegate *appDelegate = [AppDelegate sharedSession];

    if (appDelegate.dataToLoad != nil) {
        [self loadData:appDelegate.dataToLoad withAutorun:true andFilename:appDelegate.fileToLoad];
    }
}

#pragma mark - Drop interraction

- (BOOL) dropInteraction:(UIDropInteraction *)interaction canHandleSession:(id<UIDropSession>)session {
    if (session.localDragSession != nil) { // ignore drag session started within app
        return false;
    }

    return true;

    BOOL canHandle = false;

    canHandle = [session canLoadObjectsOfClass:[UIImage class]];
    return canHandle;
}

- (void) dropInteraction:(UIDropInteraction *)interaction sessionDidEnter:(id<UIDropSession>)session {
}

- (void) dropInteraction:(UIDropInteraction *)interaction performDrop:(id<UIDropSession>)session {
    for (UIDragItem *item in [session items]) {
        [item.itemProvider loadDataRepresentationForTypeIdentifier:@"be.kyuran.crocods.dsk"
                                                 completionHandler:^(NSData *_Nullable data,
                                                                     NSError *_Nullable error) {
             dispatch_async(dispatch_get_main_queue(), ^{
                                [[ViewController sharedInstance] loadData:data withAutorun:true andFilename:@""];
                            });
         }];
    }
}

- (UIDropProposal *) dropInteraction:(UIDropInteraction *)interaction sessionDidUpdate:(id<UIDropSession>)session {
    return [[UIDropProposal alloc] initWithDropOperation:UIDropOperationCopy];
}

- (nullable UITargetedDragPreview *) dropInteraction:(UIDropInteraction *)interaction previewForDroppingItem:(UIDragItem *)item withDefault:(UITargetedDragPreview *)defaultPreview {
    NSLog(@"item: %@", item);

    return nil;
}

#pragma mark -

long nds_GetTicks(void)
{
    struct timeval current_time;

    gettimeofday(&current_time, NULL);

    return ((unsigned long long)current_time.tv_sec * 1000000LL + current_time.tv_usec);
}

int ReadConfig(void)
{
    return 1;
}

- (void) initCGB {
    memset(&gb, 0, sizeof(gb));

    // Get map layout

//        TISInputSourceRef current_source = TISCopyCurrentKeyboardInputSource();
//        NSString *s = (__bridge NSString *)(TISGetInputSourceProperty(current_source, kTISPropertyInputSourceID));
//        NSUInteger last_dot_num = [s rangeOfString:@"." options:NSBackwardsSearch].location;
//        NSString *substring = [s substringFromIndex:last_dot_num + 1];
//        printf("%s", [substring UTF8String]);
//
//        if ( ([substring isEqualToString:@"Belgian"]) ||
//             ([substring isEqualToString:@"French"]) ||
//             ([substring isEqualToString:@"French-numerical"]) ||
//             ([substring isEqualToString:@"SwissFrench"])
//             ) {
//            gb.keyboardLayout = 1;  // French
//        }
//
//        if ( ([substring isEqualToString:@"Spanish"]) ||
//             ([substring isEqualToString:@"Spanish-ISO"])
//             ) {
//            gb.keyboardLayout = 2;  // Spanish
//        }

    NSData *cpc6128 = [[NSData alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"cpc6128" ofType:@"bin"]];
    NSData *romdisc = [[NSData alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"romdisc" ofType:@"bin"]];

    /* home dir */

    gb.home_dir = malloc(2048);

    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSURL *documentDirectory = [[fileManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] lastObject];

    NSArray *contents = [fileManager contentsOfDirectoryAtURL:documentDirectory includingPropertiesForKeys:[NSArray arrayWithObject:NSURLNameKey] options:NSDirectoryEnumerationSkipsHiddenFiles error:nil];

    NSLog(@"dir %@", [fileManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask]);

    for (NSURL *fileName in contents) {
        NSLog(@"file: %@", fileName);
    }

    NSString *homeDir = [documentDirectory path];

    strcpy(gb.home_dir, [homeDir UTF8String]);

    /* system dir */

    gb.system_dir = malloc(2048);

    NSString *appFolderPath = [[NSBundle mainBundle] resourcePath];

    strcpy(gb.system_dir, [appFolderPath UTF8String]);
    NSLog(@"App Directory is: %@", appFolderPath);

//
    nds_initBorder(&gb, &bx, &by);
    nds_init(&gb);

    if (ReadConfig()) {
        strcpy(gb.file_dir, [homeDir UTF8String]); // Force home_dir (due to the path changed by iOS)

        NSLog(@"Read config: OK");
        if (InitMemCPC(&gb, (char *)[cpc6128 bytes], (char *)[romdisc bytes])) {
            NSLog(@"Init Memory CPC: OK");

            //            NSData *yancc = [[NSData alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"xmass08P" ofType:@"rom"]];
            //            AddRom(&gb, (char *)[yancc bytes], 7);

            //            NSData *boulder = [[NSData alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"boulder" ofType:@"rom"]];
            //            AddRom(&gb, (char *)[boulder bytes], 7);

            //            NSData *rompack =  [[NSData alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"rompack" ofType:@"rom"]];
            //            AddRom(&gb, (char *)[rompack bytes], 3);

            //            NSData *m4firm =  [[NSData alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"M4FIRM" ofType:@"BIN"]];
            //            AddRom(&gb, (char *)[m4firm bytes], 7);

            //            NSData *osMod =  [[NSData alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"os_mod" ofType:@"rom"]];
            //            AddRom(&gb, (char *)[osMod bytes], 6);

            InitPlateforme(&gb, gb.screenBufferWidth);

            NSLog(@"Begin emulation");

            SetPalette(&gb, -1);


            //            [self performSelectorInBackground:@selector(Go) withObject:nil];
            //            [self performSelectorInBackground:@selector(GoSound) withObject:nil];
            //
            //           EjectDiskUPD();
            // PauseSound();
        } else {
            NSLog(@"Roms du CPC non trouvÈes");
        }
    } else {
        NSLog(@"Fichier de configuration du CPC non trouvÈ.");
    }

    //    strcpy(gb.openFilename, )

    ExecuteMenu(&gb, ID_AUTORUN, NULL);             // Do: open disk & autorun // TODO

    //    GB_init_cgb(&gb);
    //    GB_load_boot_rom(&gb, [[[NSBundle mainBundle] pathForResource:@"cgb_boot" ofType:@"bin"] UTF8String]);
    //    GB_set_vblank_callback(&gb, (GB_vblank_callback_t) vblank);
    //    GB_set_log_callback(&gb, (GB_log_callback_t) consoleLog);
    //    GB_set_input_callback(&gb, (GB_input_callback_t) consoleInput);
    //    GB_set_async_input_callback(&gb, (GB_input_callback_t) asyncConsoleInput);
    //    GB_set_rgb_encode_callback(&gb, rgbEncode);
    //    gb.user_data = (__bridge void *)(self);
} /* initCGB */

- (CPC_SCANCODE) scancodeToCPC:(long long)keyCode {
    if (keyCode == 4) return CPC_A;
    if (keyCode == 5) return CPC_B;
    if (keyCode == 6) return CPC_C;
    if (keyCode == 7) return CPC_D;
    if (keyCode == 8) return CPC_E;
    if (keyCode == 9) return CPC_F;
    if (keyCode == 10) return CPC_G;
    if (keyCode == 11) return CPC_H;
    if (keyCode == 12) return CPC_I;
    if (keyCode == 13) return CPC_J;
    if (keyCode == 14) return CPC_K;
    if (keyCode == 15) return CPC_L;
    if (keyCode == 16) return CPC_M;
    if (keyCode == 17) return CPC_N;
    if (keyCode == 18) return CPC_O;
    if (keyCode == 19) return CPC_P;
    if (keyCode == 20) return CPC_Q;
    if (keyCode == 21) return CPC_R;
    if (keyCode == 22) return CPC_S;
    if (keyCode == 23) return CPC_T;
    if (keyCode == 24) return CPC_U;
    if (keyCode == 25) return CPC_V;
    if (keyCode == 26) return CPC_W;
    if (keyCode == 27) return CPC_X;
    if (keyCode == 28) return CPC_Y;
    if (keyCode == 29) return CPC_Z;

    if (keyCode == 30) return CPC_1;
    if (keyCode == 31) return CPC_2;
    if (keyCode == 32) return CPC_3;
    if (keyCode == 33) return CPC_4;
    if (keyCode == 34) return CPC_5;
    if (keyCode == 35) return CPC_6;
    if (keyCode == 36) return CPC_7;
    if (keyCode == 37) return CPC_8;
    if (keyCode == 38) return CPC_9;
    if (keyCode == 39) return CPC_ZERO;

    if (keyCode == 43) return CPC_TAB;
    if (keyCode == 44) return CPC_SPACE;
//        if (keyCode==53) return ORTildeKey;
    if (keyCode == 42) return CPC_DEL;
    if (keyCode == 40) return CPC_RETURN;
    if (keyCode == 41) return CPC_ESC;
    if (keyCode == 100) return CPC_ESC;     // # on iPad keyboard

    //        if (keyCode==54) return ORCommaKey;

    if (keyCode == 82) return CPC_CURSOR_UP;
    if (keyCode == 81) return CPC_CURSOR_DOWN;
    if (keyCode == 80) return CPC_CURSOR_LEFT;
    if (keyCode == 79) return CPC_CURSOR_RIGHT;

    if (keyCode == 224) return CPC_CONTROL;
    if (keyCode == 225) return CPC_SHIFT;  // LEFT SHIFT
    if (keyCode == 229) return CPC_SHIFT;  // RIGHT SHIFT

    if (keyCode == 45) return CPC_MINUS;
    if (keyCode == 46) return CPC_HAT;

    if (keyCode == 47) return CPC_AT;
    if (keyCode == 48) return CPC_OPEN_SQUARE_BRACKET;

    if (keyCode == 51) return CPC_COLON;
    if (keyCode == 52) return CPC_SEMICOLON;
    if (keyCode == 49) return CPC_CLOSE_SQUARE_BRACKET;

    if (keyCode == 54) return CPC_COMMA;
    if (keyCode == 55) return CPC_DOT;        // point
    if (keyCode == 56) return CPC_BACKSLASH;
    if (keyCode == 53) return CPC_FORWARD_SLASH;

    if (keyCode == 230) return CPC_COPY;      // right option
    if (keyCode == 57) return CPC_CAPS_LOCK;  // caps lock

//    226 // left option -> ????

    NSLog(@"unknown code: %d", keyCode);

    return CPC_NIL;
} /* scancodeToCPC */

- (int32_t) WsInputGetState {
    int32_t button = 0;

    /* Button A    */
    button |= button_state[4] ? (1 << 0) : 0;
    /* Button B    */
    button |= button_state[5] ? (1 << 1) : 0;

    /* SELECT BUTTON */
    button |= button_state[11] ? (1 << 2) : 0;
    /* START BUTTON */
    button |= button_state[10] ? (1 << 3) : 0;

    /* RIGHT -> X1    */
    button |= button_state[1] ? (1 << 4) : 0;
    /* LEFT -> X1    */
    button |= button_state[0] ? (1 << 5) : 0;
    /* UP -> X1        */
    button |= button_state[2] ? (1 << 6) : 0;
    /* DOWN -> X1    */
    button |= button_state[3] ? (1 << 7) : 0;

    /* R1    */
    button |= button_state[9] ? (1 << 8) : 0;
    /* L1    */
    button |= button_state[8] ? (1 << 9) : 0;

    /* DefinedUIApplication */

    DefinedUIApplication *app = (DefinedUIApplication *)[UIApplication sharedApplication];
    K_Event *event;

    do {
        event = [app k_pollEvent];
        if (event != NULL) { // Handle event
            CPC_SCANCODE cpcScanCode = CPC_NIL;

            NSLog(@"received: %d,%lld", event.isKeyDown, event.keyCode);

// Called from plateform.h !!
/*
 *          if ((event.isKeyDown) && ((event.keyCode == 231) || (event.keyCode == 227))) {     // Right or Left CMD
 *              ExecuteMenu(&gb, ID_MENU_ENTER, NULL);
 *          }
 */

            if (event.keyCode == 79) {   // FB_KEY_RIGHT
                self->button_state[1] = event.isKeyDown ? 1 : 0;
            } else if (event.keyCode == 80) {                    // FB_KEY_LEFT
                self->button_state[0] = event.isKeyDown ? 1 : 0;
            } else if (event.keyCode ==  82) {                   // FB_KEY_UP
                self->button_state[2] = event.isKeyDown ? 1 : 0;
            } else if (event.keyCode == 81) {                    // FB_KEY_DOWN
                self->button_state[3] = event.isKeyDown ? 1 : 0;
            } else if (event.keyCode == 224) {                   // FB_KEY_A
                self->button_state[4] = event.isKeyDown ? 1 : 0;
            } else if (event.keyCode == 226) {                   // FB_KEY_B
                self->button_state[5] = event.isKeyDown ? 1 : 0;
            } else if (event.keyCode == 40) {                    // FB_KEY_START
                self->button_state[10] = event.isKeyDown ? 1 : 0;
            }

            if (gb.keyEmul == 3) {
                if (event.keyCode == 42) {   // R1 - Delete
                    self->button_state[9] = event.isKeyDown ? 1 : 0;
                } else if (event.keyCode == 43) {                // L1 - Tab
                    self->button_state[8] = event.isKeyDown ? 1 : 0;
                } else if (event.keyCode ==  27) {               // X
                    self->button_state[6] = event.isKeyDown ? 1 : 0;
                } else if (event.keyCode == 28) {                // Y
                    self->button_state[7] = event.isKeyDown ? 1 : 0;
                } else if (event.keyCode == 0) {                 // R2
                    self->button_state[14] = event.isKeyDown ? 1 : 0;
                } else if (event.keyCode == 0) {                 // L2
                    self->button_state[13] = event.isKeyDown ? 1 : 0;
                }
            }

            if (gb.keyEmul == 2) { // Utilisation du clavier hardware -- ExecuteMenu(&gb, ID_KEY_KEYBOARD, NULL);
                cpcScanCode = [self scancodeToCPC:event.keyCode];

                if (cpcScanCode != CPC_NIL) {
                    if (event.isKeyDown) {
                        CPC_SetScanCode(&gb, cpcScanCode);
                    } else {
                        CPC_ClearScanCode(&gb, cpcScanCode);
                    }
                }
            }
        }
    } while (event != NULL);

    return button;
} /* WsInputGetState */

- (void) start
{
    if (running) return;

    if (gb.standalone) {
        [((GBView *)self.view) setMouseHandler:^(char pressed, int x, int y) {
             static int count = 0;

             if (count < 4) {
                 self->button_state[4] = pressed ? 1 : 0;
                 count++;
             } else {
                 if (x > 192) {
                     self->button_state[1] = pressed ? 1 : 0;
                 } else {
                     self->button_state[0] = pressed ? 1 : 0;
                 }
             }
         }];
    } else {
        [((GBView *)self.view) setMouseHandler:^(char pressed, int x, int y) {
             gb.ipc.touchDown = pressed;

             gb.ipc.touchXpx = (x / 2 - marge); // (int)((float)(e.button.x * core->screenBufferWidth) / w);
             gb.ipc.touchYpx = y; // (int)((float)(e.button.y * core->screenBufferHeight) / h);

             printf("tt %dx%d\n", gb.ipc.touchXpx, gb.ipc.touchYpx);
//
//            gb.ipc.touchXpx = x; // (int)((float)(e.button.x * core->screenBufferWidth) / w);
//            gb.ipc.touchYpx = y; // (int)((float)(e.button.y * core->screenBufferHeight) / h);
         }];

        [((GBView *)self.view) setCharHandler:^(char car) {
//        char txt[2];
//        txt[0] = car;
//        txt[1] = 0;
//        AutoType_SetString(&gb, txt, 0);
         }];

        [((GBView *)self.view) setKeyHandler:^(BOOL pressed, enum FB_KEY_BITS key) {
             NSLog(@"key: %d (pressed: %d)", key, pressed
                   );

             if (key == FB_KEY_RIGHT) {
                 self->button_state[1] = pressed ? 1 : 0;
             } else if (key == FB_KEY_LEFT) {
                 self->button_state[0] = pressed ? 1 : 0;
             } else if (key == FB_KEY_UP) {
                 self->button_state[2] = pressed ? 1 : 0;
             } else if (key == FB_KEY_DOWN) {
                 self->button_state[3] = pressed ? 1 : 0;
             } else if (key == FB_KEY_A) {
                 self->button_state[4] = pressed ? 1 : 0;
             } else if (key == FB_KEY_B) {
                 self->button_state[5] = pressed ? 1 : 0;
             } else if (key == FB_KEY_SELECT) {
                 self->button_state[11] = pressed ? 1 : 0;
             } else if (key == FB_KEY_START) {
                 self->button_state[10] = pressed ? 1 : 0;
             } else if (key == FB_KEY_L) {
                 self->button_state[8] = pressed ? 1 : 0;
             } else if (key == FB_KEY_R) {
                 self->button_state[9] = pressed ? 1 : 0;
             }
         }];
    }

    [[[NSThread alloc] initWithTarget:self selector:@selector(run) object:nil] start];
} /* start */

- (void) stop
{
    if (!running) return;

//    while (self.audioClient != nil);

    running = false;
}

- (void) run
{
    running = true;

    initSound(&gb, 44100);

    /*
     * typedef struct
     * {
     *   unsigned short left;
     *   unsigned short right;
     * } GB_sample_t;
     */

    self.audioClient = [[GBAudioClient alloc] initWithRendererBlock:^(UInt32 sampleRate, UInt32 nFrames, void *buffer) {
                            if (self->gb.isPaused) {
                                memset(buffer, 0, nFrames * 4);
                                return;
                            }

                            crocods_copy_sound_buffer(&self->gb, buffer, nFrames);
                        } andSampleRate:44100];

//        if (![[NSUserDefaults standardUserDefaults] boolForKey:@"Mute"]) {
    [self.audioClient start];
//        }

    unsigned long expectedTime = 0, ellapsedTime = 0;
    unsigned long frame = 0;

    @autoreleasepool {
        [NSThread setThreadPriority:1.0];

        unsigned long TimeOut = 0, OldTime = 0;
        char framebuf[128];

        framebuf[0] = 0;

        do {
            frame++;

            if (!gb.isPaused) {           // Pause only the Z80
                long ticks = nds_GetTicks();

                TimeOut = croco_cpu_doFrame(&gb);

                expectedTime += 1000000 / 50;      // (50 hz & 1000 ms)
                ellapsedTime += (nds_GetTicks() - ticks);
            }

            // End of frame

            //            if (!CalcCRTCLine(&gb))
            {         // Arrive en fin d'ecran ?
                // Rafraichissement de l'ecran...

                //                if ((!gb.isPaused) && (!updateChangeCountWasSent)) {
                //                    updateChangeCountWasSent = true;
                //
                //                    [self performSelectorOnMainThread:@selector(updateChangeCount:) withObject:NSChangeDone waitUntilDone:FALSE];
                //
                //                    //                    [self updateChangeCount:NSChangeDone];
                //                }

//                    if (captureScreen) {
//                        captureScreen = false;
//
//                        [self captureScreen];
//                    }

                if (gb.changeFilter != 0) {
                    gb.changeFilter = 0;
                }

                if (gb.SoundBusy > 0) {
                }

                u16 keys_pressed = [self WsInputGetState];

                if (gb.runApplication != NULL) {
                    gb.runApplication(&gb, (gb.wait_key_released == 1) ? 0 : keys_pressed);
                }

                UpdateScreen(&gb);

                if (keys_pressed == 0) {
                    gb.wait_key_released = 0;
                }

                if ((gb.runApplication == NULL)  && (gb.wait_key_released == 0)) {
                    gb.ipc.keys_pressed = keys_pressed;
                }

                nds_ReadKey(&gb);

                if (1 == 1) {
                    int width = 0, height = 0;

                    if (gb.resize == 2) {
                        width = 320;
                        height = 200;
                    }

                    if (gb.resize == 1) {                      // TODO: improve resize
                        width = gb.screenBufferWidth;
                        height = gb.screenBufferHeight;

                        if ((width < 256) || (height < 48)) { // Game use splits or others video hack
//                            ExecuteMenu(&gb, ID_SCREEN_OVERSCAN, NULL);
                        }
                    }

                    if (gb.resize == 4) {                      // Overscan
                        width = 384;
                        height = 272;
                    }

                    if ((!gb.screenIsOptimized) || (gb.lastMode == 2)) {
                        width = width * 2;
                    }

                    screenWidth = width;
                    width = width * ratio;

                    if (bufferWidth != width) {
                        bufferWidth = width;
                        bufferHeight = height;

                        NSLog(@"changeSize %dx%d", width, height);

                        dispatch_sync(dispatch_get_main_queue(), ^{
                            [((GBView *)self.view) changeSizeWithWidth:bufferWidth andHeight:bufferHeight];
                        });
                    }

                    if ((frame & 1) == 1) {
//                    saveScreen(&gb);
                    }
                }

                [self performSelectorOnMainThread:@selector(vblank) withObject:nil waitUntilDone:true];

//                [self vblank];

                gb.overlayBitmap_width = 0;

                //                NSLog(@"update screen: %p (%d,%d)", self.view.pixels, gb.MonitorScanLineCount, gb.LineCounter);

                // Synchronisation de l'image ‡ 50 Hz
                // Pourcentage, temps espere, temps pris, temps z80, nombre de drawligne

                //                BOOL doResync = gb.DoResync;

                //                doResync = true;

                //                if (doResync) {
                //                    NSInteger Time;

//                    borderX = bx;
//                    borderY = by;

                if (gb.dispframerate) {
                    int percent = (int)(((float)expectedTime / (float)ellapsedTime) * 100);

                    if (percent > 500) {
                        sprintf(framebuf, "%4dx", percent / 100);
                    } else {
                        sprintf(framebuf, "%4d%%", percent);
                    }
                    //                            NSLog(@"%s", framebuf);

                    // sprintf(framebuf, " %08X %02X %02X %02X", IPC2->psgbuf, IPC2->psgbuf[0], IPC2->psgbuf[1], IPC2->psgbuf[2]);
                }

                if ((!gb.turbo)  && ((gb.AutoType.nFlags & (AUTOTYPE_ACTIVE | AUTOTYPE_WAITING)) == 0) ) {          // && (gb.SoundBusy!=0)
                    static int frame = 0;

                    frame++;
                    // Wait Timeout - (Time - OldTime) milliseconds

                    float waitTime = TimeOut - (nds_GetTicks() - OldTime);
                    waitTime = waitTime / 1000000;

                    //                        NSLog(@"W%f-%d-%ld\n", waitTime, frame, nds_GetTicks());

                    if (waitTime < 1) {
                        //                        NSLog(@"%f", waitTime);

                        [NSThread sleepForTimeInterval:waitTime];
                    }

                    //                        Time = nds_GetTicks();
                    //                        while (Time - OldTime < TimeOut) {
                    //                            Time = nds_GetTicks();
                    //                        }
                    //                    }

                    //                    tz80 = 0;

                    OldTime = nds_GetTicks();
                    TimeOut = 0;
                }
            }
        } while (1);
    }

    [self.audioClient stop];
    self.audioClient = nil;

    running = false;
} /* run */

- (void) screen_draw:(u16 *)pixels to:(u32 *)destPixels {
    int x, y;
    uint32_t *buffer_scr;

    marge = (bufferWidth - screenWidth) / 2; // (bufferHeight * ratio * 2) -  768);

    u16 color = pixels[0];

    buffer_scr = destPixels;

    for (y = 0; y < bufferHeight; y++) {
        for (x = 0; x < marge; x++) {
            buffer_scr[x + y * bufferWidth] = RGB565toRGB(color);       //  AlphaBlendFast(RGB565toRGB(color),buffer_scr[x + y * bufferWidth]) | 0xFF000000;
            buffer_scr[x + marge + screenWidth + y * bufferWidth] = RGB565toRGB(color);       //  AlphaBlendFast(RGB565toRGB(color),buffer_scr[x + y * bufferWidth]) | 0xFF000000;
        }
    }

    if (gb.resize == 2) {  // 320X200
        if (gb.screenIsOptimized) {
            int dbl = (gb.lastMode == 2) ? 2 : 1;

            for (y = 0; y < 200; y++) {
                for (x = 0; x < 320 * dbl; x++) {
                    u16 color = pixels[x + y * gb.screenBufferWidth * dbl];

                    buffer_scr[x + y * bufferWidth] = RGB565toRGB(color);
                }
            }
        } else {
            for (y = 0; y < 200; y++) {
                for (x = 0; x < 640; x++) {
                    u16 color = pixels[x  + (8 * 8) + (y + 40) * 768];

                    buffer_scr[x + marge +  y * bufferWidth] = RGB565toRGB(color);
                }
            }
        }
    }

    if (gb.resize == 1) {   // TODO: improve resize
        // ID_SCREEN_AUTO

        int dbl = 1;
        if (gb.screenIsOptimized) {
            dbl = (gb.lastMode == 2) ? 2 : 1;

            for (y = 0; y < bufferHeight; y++) {
                for (x = 0; x < bufferWidth; x++) {
                    int pos = (x * gb.screenBufferWidth * dbl) / bufferWidth + ((y * gb.screenBufferHeight) / bufferHeight) * gb.screenBufferWidth * dbl;

                    *buffer_scr = RGB565toRGB(gb.MemBitmap[pos]);
                    buffer_scr++;
                }
            }
        } else {
            for (y = 0; y < bufferHeight; y++) {
                for (x = 0; x < screenWidth; x++) {
                    u16 color = pixels[x  + (gb.x0 * 2) + (y + gb.y0) * 768];

                    buffer_scr[x + marge +  y * bufferWidth] = RGB565toRGB(color);
                }
            }
        }
    }

    #define PG_LBMASK 0xFEFEFEFE

    #define AlphaBlendFast(pixel, backpixel) (((((pixel) & PG_LBMASK) >> 1) | (((backpixel) & PG_LBMASK) >> 1)))

    if (gb.resize == 4) {       // Overscan (ok mode 2)
        for (y = 0; y < bufferHeight; y++) {
            for (x = 0; x < screenWidth; x++) {
                u16 color = pixels[x + y *  768];

                buffer_scr[(x + marge) + y * bufferWidth] = RGB565toRGB(color); //  AlphaBlendFast(RGB565toRGB(color),buffer_scr[x + y * bufferWidth]) | 0xFF000000;
            }
        }
    }

    // Draw icon

    if (gb.iconTimer > 0) {
        int x, y;
        int y0;

        int dbl_x = 2;
        int dbl_y = 1;

        int dispiconX = gb.iconToDislay / 16;
        int dispiconY = gb.iconToDislay % 16;

        buffer_scr = destPixels + (8 * bufferWidth) + 8;

        for (y = 0; y < 32; y++) {
            for (y0 = 0; y0 < dbl_y; y0++) {
                int step_x = 0;
                u16 *src = gb.icons + (dispiconX * 32) + (y + dispiconY * 32) * 448;

                for (x = 0; x < 32 * dbl_x; x++) {
                    u16 car;
                    car = *src;
                    if (car != 33840) {
                        *buffer_scr = RGB565toRGB(car);
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
            }
            buffer_scr += (bufferWidth - 32 * dbl_x);
        }

        gb.iconTimer--;
    }

    if (gb.overlayBitmap_width != 0) {
        int dbl = 2;
        if (gb.screenIsOptimized) { // Only in the wincpc & arnold core
            dbl = (gb.lastMode == 2) ? 2 : 1;
        }
        buffer_scr = destPixels;

        if (gb.overlayBitmap_center == 1) {
            gb.overlayBitmap_posx = ((screenWidth -  gb.overlayBitmap_width * dbl) / 2) / dbl;
            gb.overlayBitmap_posy = ((bufferHeight - gb.overlayBitmap_height) / 2);
        }

        for (y = 0; y < gb.overlayBitmap_height; y++) {
            u32 *dest = buffer_scr;
            dest += (gb.overlayBitmap_posx * dbl + marge) + (gb.overlayBitmap_posy + y) * bufferWidth;

            u16 *src = gb.overlayBitmap + y * 320;
            for (x = 0; x < gb.overlayBitmap_width * dbl; x++) {
                u16 car = *src;
                if (car != 63519) {                  // RGB565(255,0,255)
                    *dest = RGB565toRGB(car);
                }
                dest++;
                if (((x & 1) == 1) || (dbl == 1)) {
                    src++;
                }
            }
        }
    }
} /* screen_draw */

- (void) vblank
{
    int ignoreRow;

    CGFloat ratio0 = (self.view.frame.size.width / self.view.frame.size.height);

    ratio0 = (2 + ratio0) / 3;       // increase but not too much ;)

    if (ratio != ratio0) {
        ratio = ratio0;
        NSLog(@"new ratio: %f %fx%f", ratio, self.view.frame.size.width, self.view.frame.size.height);
    }

    if (gb.screenIsOptimized) {
        ignoreRow = 0;
    } else {
        ignoreRow = 3; // only in caprice32 actually and not in 320x200.. weird
    }

    [self screen_draw:self->gb.MemBitmap + 384 * 2 * ignoreRow to:((GBView *)(self.view)).pixels];
    [(GBView *)self.view flip];

    // Display border

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

    int i = gb.TabCoul[ 16 ];

    int r = (RgbCPCdef[ i ] >> 16) & 0xFF;
    int g = (RgbCPCdef[ i ] >> 8) & 0xFF;
    int b = (RgbCPCdef[ i ] >> 0) & 0xFF;

    self.view.backgroundColor = [UIColor colorWithRed:(float)r / 255 green:(float)g / 255 blue:(float)b / 255 alpha:1.0];

//    char framebuf[32];
//    sprintf(framebuf, "%d", self.view.current_buffer);
//
//    cpcprint(&gb, 0, 192 - 8, framebuf, 1);
//
//    NSLog(@"current_buffer: %d", self.view.current_buffer);

//    dispatch_async(dispatch_get_main_queue(), ^{
//        self.view.mouseHidingEnabled = (self.mainWindow.styleMask & NSWindowStyleMaskFullScreen) != 0;
//    });

    updateScreenBuffer(&gb, NULL, gb.screenBufferWidth);      // Prepare for the next

    //  GB_set_pixels_output(&gb, self.view.pixels);
} /* vblank */

void guestGetAllKeyPressed(core_crocods_t *core, char *string)
{
    string[0] = 0;
}

void guestPrintchar(core_crocods_t *core, char car) // In guest.h
{
    printf("%c", car);
}

void guestFullscreen(core_crocods_t *core, char on)
{

}

char guestIsFullscreen(core_crocods_t *core)
{
    return 1;
}

void guestGetJoystick(core_crocods_t *core, char *string)
{
    string[0] = 0;

    GBView *gbview = [GBView sharedSession];
    NSInteger hardKey = gbview.hardKey;

    if ((hardKey & FB_KEY_A) != 0) {
        strcat(string, "A ");
    }
    if ((hardKey & FB_KEY_B) != 0) {
        strcat(string, "B ");
    }
    if ((hardKey & FB_KEY_X) != 0) {
        strcat(string, "X ");
    }
    if ((hardKey & FB_KEY_Y) != 0) {
        strcat(string, "Y ");
    }
    if ((hardKey & FB_KEY_L) != 0) {
        strcat(string, "L ");
    }
    if ((hardKey & FB_KEY_R) != 0) {
        strcat(string, "R ");
    }
    if ((hardKey & FB_KEY_START) != 0) {
        strcat(string, "START ");
    }
    if ((hardKey & FB_KEY_SELECT) != 0) {
        strcat(string, "SELECT ");
    }
    if ((hardKey & FB_KEY_UP) != 0) {
        strcat(string, "UP ");
    }
    if ((hardKey & FB_KEY_DOWN) != 0) {
        strcat(string, "DOWN ");
    }
    if ((hardKey & FB_KEY_LEFT) != 0) {
        strcat(string, "LEFT ");
    }
    if ((hardKey & FB_KEY_RIGHT) != 0) {
        strcat(string, "RIGHT ");
    }

    //    n = SDL_JoystickNumButtons(joy);
    //    for (i = 0; i < n; i++) {
    //        char buf[32];
    //        sprintf(buf, "%d", SDL_JoystickGetButton(joy, i));
    //        strcat(string, buf);
    //    }
    //
    //    char buf[32];
    //    sprintf(buf, " %c%c%c%c", SDL_JoystickGetAxis(joy, 0) > 2048 ? 'R' : '-', SDL_JoystickGetAxis(joy, 0) < -2048 ? 'L' : '-', SDL_JoystickGetAxis(joy, 1) > 2048 ? 'D' : '-', SDL_JoystickGetAxis(joy, 1) < -2048 ? 'U' : '-');
    //    strcat(string, buf);
} /* guestGetJoystick */

void guestExit(core_crocods_t *core)
{
}

@end
