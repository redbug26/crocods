#include <CoreAudio/CoreAudio.h>
#include "GBAudioClient.h"
#include "Document.h"
#include "AppDelegate.h"

#import <Carbon/Carbon.h>

#import "SSHCrocoApplication.h"

#include <sys/time.h>

#include "plateform.h"
#include "crtc.h"
#include "vga.h"
#include "z80.h"
#include "spatial_color_quant.h"

#define pperday   10
#define PI        3.1415926535
#define CX        ((320 - (pperday * 31)) / 2)
#define SIZEX     320
#define SIZEY     460

#define MAXARGLEN 128

// r = (((color >> 11) & 0x1f) << 3)
// g = (((color >> 5) & 0x1f) << 3)
// b = ((color & 0x1F) << 3)
// return (r << 0) | (g << 8) | (b << 16) | 0xFF000000;

#define RGB15TORGB32(color) (((((color >> 11) & 0x1f) << 3) << 0) | ((((color >> 5) & 0x1f) << 3) << 8) | (((color & 0x1F) << 3) << 16) | 0xFF000000)

#define RGB565toRGB(rgb565) ((uint32_t)(((uint8_t)((((rgb565) & 0xF800) >> 11) << 3) | ((uint32_t)((uint8_t)((((rgb565) & 0x07E0) >> 5) << 2)) << 8)) | (((uint32_t)(uint8_t)(((rgb565) & 0x001F) << 3)) << 16)))

#define RGB565(R, G, B)     ((((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))

@interface Document ()
{
    /* NSTextViews freeze the entire app if they're modified too often and too quickly.
     * We use this bool to tune down the write speed. Let me know if there's a more
     * reasonable alternative to this. */
    unsigned long pendingLogLines;
    bool tooMuchLogs;
    bool fullScreen;
    bool in_sync_input;

    NSString *lastConsoleInput;
    int bufferWidth, bufferHeight;

    int bx, by;
    int borderX, borderY;

    char autoString[256];

    u8 *disk;
    u32 diskLength;

    u16 button_state[18];

    u8 *snapshot;
    u32 snapshotLength;

    //    u8 *rom;
    //    u32 romLength;

    // NCurses

    NSInteger maxHeight;

    NSInteger curX, curY;   // 1 based

    NSColor *curAttrTextColor;
    NSColor *curAttrBackgroundColor;

    NSInteger tabSpaces;
    NSInteger savX, savY, isSaved;
    char *ansiTerminators;

    NSInteger maxX, maxY;          // size of ansi output window

    // Bundle information

    NSFileWrapper *documentFileWrapper;

    NSURL *currentBundle;

    NSMutableDictionary *screenshot;

    bool updateChangeCountWasSent;
    bool captureScreen;
}

@property GBAudioClient *audioClient;
- (void) vblank;
@end

long nds_GetTicks(void)
{
    struct timeval current_time;

    gettimeofday(&current_time, NULL);

    return ((unsigned long long)current_time.tv_sec * 1000000LL + current_time.tv_usec);
}

@implementation Document
{
    volatile bool running;
    volatile bool stopping;
    NSConditionLock *has_debugger_input;
    NSMutableArray *debugger_input_queue;
    volatile bool is_inited;
}

@synthesize gb;

- (instancetype) init
{
    self = [super init];
    if (self) {
        NSArray *documents = [[NSDocumentController sharedDocumentController] documents];
        NSLog(@"count: %lu", (unsigned long)[documents count]);

        for (NSDocument *doc in documents) {
            [doc close];
        }

        has_debugger_input = [[NSConditionLock alloc] initWithCondition:0];
        debugger_input_queue = [[NSMutableArray alloc] init];

        disk = nil;
        snapshot = nil;

        NSNotificationCenter *theCenter = [NSNotificationCenter defaultCenter];
        [theCenter addObserver:self selector:@selector(windowLostFocus:) name:NSWindowDidResignKeyNotification object:self.mainWindow];
        [theCenter addObserver:self selector:@selector(windowLostFocus:) name:NSWindowDidResignMainNotification object:self.mainWindow];
        [theCenter addObserver:self selector:@selector(windowHasFocus:) name:NSWindowDidBecomeKeyNotification object:self.mainWindow];
        [theCenter addObserver:self selector:@selector(windowHasFocus:) name:NSWindowDidBecomeMainNotification object:self.mainWindow];

        curX = 1;
        curY = 1;

        captureScreen = false;

        maxHeight = 0;

        maxX = 80, maxY = 25;          // size of ansi output window
        tabSpaces = 8;
        savX = savY = isSaved = 0;
        ansiTerminators = "HFABCDnsuJKLmpPM@";

        curAttrBackgroundColor = [NSColor blackColor];
        curAttrTextColor = [NSColor whiteColor];

        screenshot = [[NSMutableDictionary alloc] initWithCapacity:5];

        updateChangeCountWasSent = false;
    }
    return self;
} // init

#pragma mark - Open & Save

- (instancetype) initWithType:(NSString *)typeName error:(NSError **)outError
{
    self = [self init];

    NSData *dsk = [[NSData alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"empty" ofType:@"dsk"]];

    diskLength = [dsk length];

    disk = (u8 *)malloc(diskLength);
    memcpy(disk, [dsk bytes], diskLength);

    //    rom = (u8*)[dsk bytes];
    //    romLength = [dsk length];

    return self;
}

// Save data to file.

- (NSData *) dataOfType:(NSString *)typeName error:(NSError **)outError
{
    NSMutableData *dskData = [[NSMutableData alloc] initWithLength:0];

    [dskData appendBytes:&gb.Infos length:sizeof(gb.Infos)];
    [dskData appendBytes:gb.ImgDsk length:gb.LongFic];

    return dskData;
}

- (NSFileWrapper *) fileWrapperOfType:(NSString *)typeName error:(NSError **)outError
{
    BOOL haveIcon = false;

    updateChangeCountWasSent = false;

    if (documentFileWrapper == nil) {
        documentFileWrapper = [[NSFileWrapper alloc] initDirectoryWithFileWrappers:nil];
    }

    //    NSDictionary *fileWrappers = [documentFileWrapper fileWrappers];
    //    NSFileWrapper *diskFileWrapper = [fileWrappers objectForKey:@"disk.dsk"];
    //    NSFileWrapper *videoFileWrapper = [fileWrappers objectForKey:@"video.mp4"];

    NSMutableData *dskData = [[NSMutableData alloc] initWithLength:0];

    [dskData appendBytes:&gb.Infos length:sizeof(gb.Infos)];
    [dskData appendBytes:gb.ImgDsk length:gb.LongFic];

    [documentFileWrapper addRegularFileWithContents:dskData preferredFilename:@"disk.dsk"];

    for (NSString *filename in [screenshot allKeys]) {
        NSData *screenshotData = [screenshot objectForKey:filename];

        [documentFileWrapper addRegularFileWithContents:screenshotData preferredFilename:filename];

        if (!haveIcon) {
            haveIcon = true;
            NSImage *background = [NSImage imageNamed:@"Floppy"];
            NSImage *icon = [[NSImage alloc] initWithData:screenshotData];

            NSImage *newImage = [[NSImage alloc] initWithSize:[background size]];
            [newImage lockFocus];

            CGRect newImageRect = CGRectZero;
            newImageRect.size = [newImage size];

            [background drawInRect:newImageRect];
            [icon drawInRect:CGRectMake(190, 140, 320, 200)];

            [newImage unlockFocus];

            CGImageRef newImageRef = [newImage CGImageForProposedRect:NULL context:nil hints:nil];

            NSImage *finalImage = [[NSImage alloc] initWithCGImage:newImageRef size:background.size];

            [[NSWorkspace sharedWorkspace] setIcon:finalImage forFile:self.fileURL.path options:0];
        }
    }

    int len;
    char *buffer = getSnapshot(&gb, &len);

    if (buffer != NULL) {
        NSData *snapshot0 = [[NSData alloc] initWithBytes:buffer length:len];
        [documentFileWrapper addRegularFileWithContents:snapshot0 preferredFilename:@"snapshot.sna"];
    }

    return documentFileWrapper;
} // fileWrapperOfType

- (NSString *) defaultDraftName
{
    if ([[currentBundle lastPathComponent] length] == 0) {
        return @"Unknown";
    }

    return [currentBundle lastPathComponent];
}

- (BOOL) readFromFile:(NSString *)fileName ofType:(NSString *)type
{
    if (is_inited++) {
        return YES;
    }

    BOOL isDir;

    if (![[NSFileManager defaultManager] fileExistsAtPath:fileName isDirectory:&isDir]) {
        return NO;
    }

    if ([fileName.pathExtension isEqualToString:@"crocods"]) {
        currentBundle = [NSURL fileURLWithPath:fileName];
    } else {
        currentBundle = [[[NSURL fileURLWithPath:fileName] URLByDeletingPathExtension] URLByAppendingPathExtension:@"crocods"];
        self.diskURL = self.fileURL;
        self.fileURL = nil;

        //        self.fileType = @"be.kyuran.crocods.bundle";
    }

    if (isDir) {  // .crocods bundle ?
        NSString *dskFile = [fileName stringByAppendingPathComponent:@"disk.dsk"];
        NSData *dsk = [[NSData alloc] initWithContentsOfFile:dskFile];
        if (dsk != nil) {
            strcpy(gb.openFilename, [dskFile UTF8String]);

            diskLength = (u32)[dsk length];

            disk = (u8 *)malloc(diskLength);
            memcpy(disk, [dsk bytes], diskLength);
        } else {
            // Load empty disk
        }

        NSString *snapFile = [fileName stringByAppendingPathComponent:@"snapshot.sna"];
        NSData *snap = [[NSData alloc] initWithContentsOfFile:snapFile];
        if (snap != nil) {
            snapshotLength = (u32)[snap length];

            snapshot = (u8 *)malloc(snapshotLength);
            memcpy(snapshot, [snap bytes], snapshotLength);
        }

        // Load image

        NSDirectoryEnumerator *dirEnum = [[NSFileManager defaultManager] enumeratorAtPath:fileName];

        NSString *key;
        while ((key = [dirEnum nextObject])) {
            if ([[key pathExtension] isEqualToString:@"png"]) {
                NSString *fullName = [fileName stringByAppendingPathComponent:key];
                NSData *data = [NSData dataWithContentsOfFile:fullName];

                [screenshot setObject:data forKey:key];
            }

            // Do something with the file name
        }

        return YES;
    }

    // We have a .dsk or .sna

    strcpy(gb.openFilename, [fileName UTF8String]);
    return [self loadDiskOrSnapshot:fileName];

    //    GB_load_rom(&gb, [fileName UTF8String]);
    //    GB_load_battery(&gb, [[[fileName stringByDeletingPathExtension] stringByAppendingPathExtension:@"sav"] UTF8String]);
    //    GB_debugger_load_symbol_file(&gb, [[[fileName stringByDeletingPathExtension] stringByAppendingPathExtension:@"sym"] UTF8String]);
} // readFromFile

- (BOOL) loadDiskOrSnapshot:(NSString *)fileName {
    char header[32];
    NSData *dsk = [[NSData alloc] initWithContentsOfFile:fileName];

    if ([dsk length] < 32) {
        return NO;
    }

    memcpy(header, [dsk bytes], 32);

    if ( (!memcmp(header, "MV - CPC", 8)) || (!memcmp(header, "EXTENDED", 8)) ) {
        diskLength = (u32)[dsk length];

        disk = (u8 *)malloc(diskLength);
        memcpy(disk, [dsk bytes], diskLength);
    }

    if (!memcmp(header, "MV - SNA", 8)) {
        snapshotLength = (u32)[dsk length];

        snapshot = (u8 *)malloc(snapshotLength);
        memcpy(snapshot, [dsk bytes], snapshotLength);
    }

    return YES;
} /* loadDiskOrSnapshot */

#pragma mark -

- (IBAction) btnCaptureScreen:(id)sender
{
    captureScreen = true;
}

- (void) captureScreen
{
    // convert 565 to 8888
    u32 *buffer = (u32 *)malloc(gb.screenBufferWidth * gb.screenBufferHeight * sizeof(u32));
    int x, y;

    for (x = 0; x < gb.screenBufferWidth; x++) {
        for (y = 0; y < gb.screenBufferHeight; y++) {
            u16 aPixel = gb.MemBitmap[x + y * gb.screenBufferWidth];

            int b = (((aPixel) << 3) & 0xF8);
            int g = (((aPixel) >> 3) & 0xFC);
            int r = (((aPixel) >> 8) & 0xF8);
            buffer[x + y * gb.screenBufferWidth] = (r) | (g << 8) | (b << 16);
        }
    }

    // create the bitmap context:
    const size_t ComponentsPerPixel = 4;
    const size_t BitsPerComponent = 8;
    const size_t BytesPerRow = ((BitsPerComponent * gb.screenBufferWidth) / 8) * ComponentsPerPixel;
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef gtx = CGBitmapContextCreate(buffer, gb.screenBufferWidth, gb.screenBufferHeight, BitsPerComponent, BytesPerRow, colorSpace, kCGImageAlphaNoneSkipLast);

    // create the image:
    CGImageRef toCGImage = CGBitmapContextCreateImage(gtx);

    NSBitmapImageRep *newRep = [[NSBitmapImageRep alloc] initWithCGImage:toCGImage];

    [newRep setSize:NSMakeSize(gb.screenBufferWidth, gb.screenBufferHeight)];
    NSData *gifData = [newRep representationUsingType:NSGIFFileType properties:@{}];

    NSString *imageName;
    int i = 0;

    do {
        if (i == 0) {
            imageName = [NSString stringWithFormat:@"capture.gif"];
        } else {
            imageName = [NSString stringWithFormat:@"capture_%d.gif", i];
        }
        i++;
    } while ([screenshot objectForKey:imageName] != nil);

    [screenshot setObject:gifData forKey:imageName];

    CGColorSpaceRelease(colorSpace);
    CGContextRelease(gtx);
} // captureScreen

- (void) dealloc
{
    NSNotificationCenter *theCenter = [NSNotificationCenter defaultCenter];

    [theCenter removeObserver:self name:NSWindowDidResignKeyNotification object:self.mainWindow];
    [theCenter removeObserver:self name:NSWindowDidResignMainNotification object:self.mainWindow];
    [theCenter removeObserver:self name:NSWindowDidBecomeKeyNotification object:self.mainWindow];
    [theCenter removeObserver:self name:NSWindowDidBecomeMainNotification object:self.mainWindow];

    //  GB_free(&gb);
}

- (BOOL) isDocumentEdited {
    // return [super isDocumentEdited];
    return false;
}

- (void) windowLostFocus:(id)sender
{
    NSNotification *notification = sender;

    if (notification.object == self.mainWindow) {
        ExecuteMenu(&gb, ID_PAUSE_ENTER, NULL);
    }
}

- (void) windowHasFocus:(id)sender
{
    NSNotification *notification = sender;

    if (notification.object == self.mainWindow) {
        ExecuteMenu(&gb, ID_PAUSE_EXIT, NULL);
    }

    AppDelegate *appDelegate = [[NSApplication sharedApplication] delegate];

    appDelegate.currentDocument = self;
}

- (void) initCGB
{
    char oldOpenFilename[PATH_MAX];

    strcpy(oldOpenFilename, gb.openFilename);

    memset(&gb, 0, sizeof(gb));

    strcpy(gb.openFilename, oldOpenFilename);

    // Get map layout

    TISInputSourceRef current_source = TISCopyCurrentKeyboardInputSource();
    NSString *s = (__bridge NSString *)(TISGetInputSourceProperty(current_source, kTISPropertyInputSourceID));
    NSUInteger last_dot_num = [s rangeOfString:@"." options:NSBackwardsSearch].location;
    NSString *substring = [s substringFromIndex:last_dot_num + 1];

    printf("%s", [substring UTF8String]);

    if ( ([substring isEqualToString:@"Belgian"]) ||
         ([substring isEqualToString:@"French"]) ||
         ([substring isEqualToString:@"French-numerical"]) ||
         ([substring isEqualToString:@"SwissFrench"])
         ) {
        gb.keyboardLayout = 1;  // French
    }

    if ( ([substring isEqualToString:@"Spanish"]) ||
         ([substring isEqualToString:@"Spanish-ISO"])
         ) {
        gb.keyboardLayout = 2;  // Spanish
    }

    NSData *cpc6128 = [[NSData alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"cpc6128" ofType:@"bin"]];
    NSData *romdisc = [[NSData alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"romdisc" ofType:@"bin"]];

    /* home dir */

    gb.home_dir = malloc(2048);

    NSFileManager *fileMgr = [NSFileManager defaultManager];

    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString *appSupportPath = [paths objectAtIndex:0];
    NSString *appName = [[NSRunningApplication currentApplication] localizedName];
    NSString *homeDir = [appSupportPath stringByAppendingPathComponent:appName];

    if (![fileMgr fileExistsAtPath:homeDir]) {
        [fileMgr createDirectoryAtPath:homeDir withIntermediateDirectories:NO attributes:nil error:nil];
    }

    strcpy(gb.home_dir, [homeDir UTF8String]);

    nds_initBorder(&gb, &bx, &by);
    nds_init(&gb);

    if (ReadConfig()) {
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

    if (!(self.modifierFlags & NSEventModifierFlagShift)) {
        ExecuteMenu(&gb, ID_AUTORUN, NULL);     // Do: open disk & autorun // TODO
    }

    //    GB_init_cgb(&gb);
    //    GB_load_boot_rom(&gb, [[[NSBundle mainBundle] pathForResource:@"cgb_boot" ofType:@"bin"] UTF8String]);
    //    GB_set_vblank_callback(&gb, (GB_vblank_callback_t) vblank);
    //    GB_set_log_callback(&gb, (GB_log_callback_t) consoleLog);
    //    GB_set_input_callback(&gb, (GB_input_callback_t) consoleInput);
    //    GB_set_async_input_callback(&gb, (GB_input_callback_t) asyncConsoleInput);
    //    GB_set_rgb_encode_callback(&gb, rgbEncode);
    //    gb.user_data = (__bridge void *)(self);
} // initCGB

- (void) vblank
{
    int ignoreRow;

    if (gb.screenIsOptimized) {
        ignoreRow = 0;
    } else {
        ignoreRow = 3; // only in caprice32 actually and not in 320x200.. weird
    }

    [self screen_draw:gb.MemBitmap + 384 * 2 * ignoreRow to:self.view.pixels];

//    char framebuf[32];
//    sprintf(framebuf, "%d", self.view.current_buffer);
//
//    cpcprint(&gb, 0, 192 - 8, framebuf, 1);
//
//    NSLog(@"current_buffer: %d", self.view.current_buffer);

//    dispatch_async(dispatch_get_main_queue(), ^{
//        self.view.mouseHidingEnabled = (self.mainWindow.styleMask & NSWindowStyleMaskFullScreen) != 0;
    [self.view flip];
//    });

    updateScreenBuffer(&gb, NULL, gb.screenBufferWidth);      // Prepare for the next

    //  GB_set_pixels_output(&gb, self.view.pixels);
} /* vblank */

- (void) saveImage:(NSImage *)image
{
    // printf("Usage: spatial_color_quant <source image.rgb> <width> <height> <desired palette size> <output image.rgb> [dithering level] [filter size (1/3/5)]\n");

    char *paletteCPC = (char *)malloc(32 * 3);

    const int width = image.size.width;
    const int height = image.size.height;

    int i;

    for (i = 0; i < 32; i++) {
        u16 aPixel = gb.BG_PALETTE[i];

        int b = (((aPixel) << 3) & 0xF8);
        int g = (((aPixel) >> 3) & 0xFC);
        int r = (((aPixel) >> 8) & 0xF8);

        paletteCPC[i * 3 + 0] = r;
        paletteCPC[i * 3 + 1] = g;
        paletteCPC[i * 3 + 2] = b;
    }

    int num_colors = 32;

    char *bufferOut = (char *)malloc(width * height * 4);
    char *bufferIn = (char *)malloc(width * height * 4);

    CGImageSourceRef source = CGImageSourceCreateWithData((CFDataRef)[image TIFFRepresentation], NULL);
    CGImageRef cgImage =  CGImageSourceCreateImageAtIndex(source, 0, NULL);

    CGContextRef context = CGBitmapContextCreate(bufferIn, width, height, 8, 4 * width, CGImageGetColorSpace(cgImage), kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);

    CGRect rect = CGRectMake(0.0, 0.0, width, height);

    CGContextTranslateCTM(context, 0, 0);
    CGContextClearRect(context, rect);
    CGContextDrawImage(context, rect, cgImage);
    CGContextRelease(context);


    convertSpatialColorQuant(width, height, paletteCPC, num_colors, bufferIn, bufferOut);

    // Convert bufferOut to PNG
    const size_t ComponentsPerPixel = 4;
    const size_t BitsPerComponent = 8;
    const size_t BytesPerRow = ((BitsPerComponent * gb.screenBufferWidth) / 8) * ComponentsPerPixel;
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef gtx = CGBitmapContextCreate(bufferOut, width, height, BitsPerComponent, BytesPerRow, colorSpace, kCGImageAlphaNoneSkipLast);

    CGImageRef toCGImage = CGBitmapContextCreateImage(gtx);

    NSBitmapImageRep *newRep = [[NSBitmapImageRep alloc] initWithCGImage:toCGImage];

    [newRep setSize:NSMakeSize(width, height)];
    NSData *pngData = [newRep representationUsingType:NSPNGFileType properties:@{}];

    NSString *imageName;

    i = 0;

    do {
        if (i == 0) {
            imageName = [NSString stringWithFormat:@"capture.png"];
        } else {
            imageName = [NSString stringWithFormat:@"capture_%d.png", i];
        }
        i++;
    } while ([screenshot objectForKey:imageName] != nil);

    [screenshot setObject:pngData forKey:imageName];

    CGColorSpaceRelease(colorSpace);
    CGContextRelease(gtx);

    free(bufferIn);
    free(bufferOut);
    free(paletteCPC);
} // saveImage

- (IBAction) paste:sender         // Materiel: http://cpcrulez.fr/coding_menu-src-magazines.htm?t=ncKl6NLWlM7g6N7Wjc7p
{
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    NSArray *classArray;
    BOOL ok;
    NSDictionary *options = [NSDictionary dictionary];

    classArray = [NSArray arrayWithObject:[NSImage class]];

    ok = [pasteboard canReadObjectForClasses:classArray options:options];
    if (ok) {
        NSArray *images = [pasteboard readObjectsForClasses:classArray options:options];

        if ([images count] > 0) {
            NSImage *image = images[0];

            [self saveImage:image];

            NSLog(@"image: %@", image);
            return;
        }
    }

    classArray = [NSArray arrayWithObject:[NSString class]];

    ok = [pasteboard canReadObjectForClasses:classArray options:options];
    if (ok) {
        NSArray *strings = [pasteboard readObjectsForClasses:classArray options:options];

        if ([strings count] > 0) {
            NSString *string = strings[0];
            AutoType_SetString(&gb, [string UTF8String], false);
            return;
        }
    }
}

#pragma mark - NCurses

- (void) sendOutput:(NSString *)string
{
    NSLog(@"Display string: '%@' (%lu)", string, (unsigned long)[string length]);

    for (NSInteger i = 0; i < [string length]; i++) {
        unichar car = [string characterAtIndex:i];

        [self ansiOut:car];
    }
}

- (void) appIsFinished:(NSString *)string
{
    self.crocoApplication = nil;

    [self sendOutput:@"\n\n*Break*\n"];
}

- (void) nextLine
{
    curY++;

    //    [self locate:(int)curX andY:(int)curY];

    //    [self ansiAffChr:'\r' atX:curX andY:curY];
    //    [self ansiAffChr:'\n' atX:curX andY:curY];

    //	NSLog(@"nextLine: %ld", curY);

    if (curY > maxY) {
        [self ansiAffChr:' ' atX:1 andY:(int)maxY + 1];

        //        [[self textStorage] deleteCharactersInRange:NSMakeRange(0, maxX + 1)];
        curY--;
        maxHeight--;
    }

    [self locate:(int)curX andY:(int)curY];
} // nextLine

- (void) getCursor
{
    SRegs z80;

    char z80Code[4] = "\xCD\x78\xBB";

    ExecZ80Code(&gb, z80Code, 3, &z80);

    curX = z80.HL.Byte.High;
    curY = z80.HL.Byte.Low;
}

- (void) locate:(int)x andY:(int)y
{
    //    NSLog(@"locate %d,%d", x,y);

    char z80Code[8] = "\x26\x02\x2E\x02\xCD\x75\xBB"; // Locate

    z80Code[1] = x;
    z80Code[3] = y;

    ExecZ80Code(&gb, z80Code, 7, NULL);
}

- (void) setPos:(char *)argbuf withLen:(NSInteger)arglen
{
    NSInteger y, x;
    char *p;

    if (!*argbuf || !arglen) {
        curX = 1;
        curY = 1;
    }

    y = atoi(argbuf);
    p = strchr(argbuf, ';');

    if ((y >= 1) & (p != NULL)) {
        if (y > maxY) {
            y = maxY;
        }

        x = atoi(p + 1);
        if (x >= 1) {
            curX = x;
            curY = y;
        }
    }
} // setPos

- (void) clearToEnd:(char *)argbuf withLen:(NSInteger)arglen
{
    [self unHandledNCurses:__PRETTY_FUNCTION__ atLine:__LINE__];

    int type;

    if (arglen == 0) {
        type = 0;
    } else {
        type = atoi(argbuf);
    }

    if (type == 0) {
        for (NSInteger x = curX; x < maxX; x++) {
            [self ansiAffChr:' ' atX:x andY:curY];
            [self ansiAffCol:curAttrTextColor withBackground:curAttrBackgroundColor atX:x andY:curY];
        }
    }
}

- (void) unHandledNCurses:(const char *)function atLine:(int)line
{
    NSLog((@"%s [Line %d]: unhandled"), function, line);
}

- (void) goUp:(char *)argbuf
{
    [self unHandledNCurses:__PRETTY_FUNCTION__ atLine:__LINE__];

    int x;

    x = atoi(argbuf);

    do {
        if (curY == 1) break;

        curY--;
        x--;
    } while (x > 0);

    //    [self locate:(int)curX andY:(int)curY];
}

- (void) goDown:(char *)argbuf
{
    [self unHandledNCurses:__PRETTY_FUNCTION__ atLine:__LINE__];

    int x;

    x = atoi(argbuf);

    do {
        //	  if (cury == maxy - 1)        break;
        curY++;
        x--;
    } while (x > 0);

    //    [self locate:(int)curX andY:(int)curY];
}

- (void) goLeft:(char *)argbuf
{
    [self unHandledNCurses:__PRETTY_FUNCTION__ atLine:__LINE__];

    int x;

    //    [self ansiAffChr:8 atX:curX andY:curY];

    x = atoi(argbuf);

    do {
        if (curX == 1) break;
        curX--;
        x--;
    } while (x > 0);

    //    [self locate:(int)curX andY:(int)curY];
} // goLeft

- (void) goRight:(char *)argbuf
{
    [self unHandledNCurses:__PRETTY_FUNCTION__ atLine:__LINE__];

    int x;

    x = atoi(argbuf);

    do {
        if (curX > maxX) break;
        curX++;
        x--;
    } while (x > 0);

    //    [self locate:(int)curX andY:(int)curY];
}

- (void) backspace:(char *)argbuf
{
    //    int x;

    int x;

    x = atoi(argbuf);

    do {
        [self ansiAffChr:127 atX:curX andY:curY];
        [self ansiAffChr:'@' atX:curX andY:curY];

        x--;
    } while (x > 0);

    //
    //    if (*argbuf==0) {
    //        x=1;
    //    } else {
    //        x = atoi(argbuf);
    //    }
    //
    //    [self ansiAffChr:' ' atX:curX andY:curY];
    //
    //
    //    NSInteger pos = curX + curY*(maxX+1);
} // backspace

// -( void)backspace:(char*)argbuf
// {
//    int x;
//
//    if (*argbuf==0) {
//        x=1;
//    } else {
//        x = atoi(argbuf);
//    }
//
//    NSInteger pos = curX + curY*(maxX+1);
//
//    [[self textStorage] deleteCharactersInRange:NSMakeRange(pos, x)];
//
//    NSDictionary *attrs = @{ NSForegroundColorAttributeName : [NSColor whiteColor],
//                             NSFontAttributeName: [NSFont fontWithName:@"Monaco" size:13.0]};
//
//    NSString *string = [@"" stringByPaddingToLength:x withString:@" " startingAtIndex:0];
//
//    NSMutableAttributedString* attrString = [[NSMutableAttributedString alloc] initWithString:string attributes:attrs];
//
//    [[self textStorage] insertAttributedString:attrString atIndex:(curY+1)*(maxX+1)-x-1];
// }

- (void) savePos
{
    savX = curX;
    savY = curY;
    isSaved = 1;
}

- (void) restorePos
{
    if (isSaved) {
        [self unHandledNCurses:__PRETTY_FUNCTION__ atLine:__LINE__];

        curX = savX;
        curX = savY;
        isSaved = 0;
    }
}

- (void) fillLineToY:(NSInteger)y
{
    while (y >= maxHeight) {
        [self unHandledNCurses:__PRETTY_FUNCTION__ atLine:__LINE__];

        //        NSDictionary *attrs = @{ NSForegroundColorAttributeName : [NSColor whiteColor],
        //                                 NSFontAttributeName: [NSFont fontWithName:@"Monaco" size:13.0]};
        //
        //        NSString *string = [@"" stringByPaddingToLength:maxX withString:@" " startingAtIndex:0];
        //        string = [string stringByAppendingString:@"\n"];
        //
        //        NSMutableAttributedString* attr = [[NSMutableAttributedString alloc] initWithString:string attributes:attrs];
        //
        //        [[self textStorage] appendAttributedString:attr];
        maxHeight++;
    }
}

- (void) clearScreen
{
    char z80Code[4] = "\xCD\x6C\xBB";

    ExecZ80Code(&gb, z80Code, 3, NULL);  // print car

    curX = 1;
    curY = 1;

    //    [self getCursor];
    //    NSLog(@"curx: %ld, cury: %ld", (long)curX, (long)curY);
}

- (void) ansiAffChr:(unichar)car atX:(NSInteger)x andY:(NSInteger)y
{
    [self locate:(int)x andY:(int)y];

    //    [self fillLineToY:y]; // A remettre ?

    switch (car) {
        //        case 196: car=0x91;break;
        //        case 196: car=0x92;break;
        case 192: car = 0x93; break;
        //        case 196: car=0x94;break;
        case 179: car = 0x95; break;
        case 218: car = 0x96; break;
        case 195: car = 0x97; break;
        //        case 196: car=0x98;break;
        case 217: car = 0x99; break;
        case 196: car = 0x9A; break;
        case 193: car = 0x9B; break;
        case 191: car = 0x9C; break;
        case 180: car = 0x9D; break;
        case 194: car = 0x9E; break;
        case 197: car = 0x9F; break;
    }

    //    NSLog(@"print %c", car);

    char z80Code[6] = "\x3E\x44\xCD\x5A\xBB";

    z80Code[1] = car;
    ExecZ80Code(&gb, z80Code, 5, NULL);  // print car

    //	[[self textStorage] replaceCharactersInRange:NSMakeRange(x + y*(maxX+1), 1) withString:string]; // Tres lent !

    //    [self replaceCharactersInRange:NSMakeRange(x + y*(maxX+1), 1) withString:string];

    //    [self locate:(int)curX andY:(int)curY];
} // ansiAffChr

- (void) ansiAffCol:(NSColor *)textColor withBackground:(NSColor *)backgroundColor atX:(NSInteger)x andY:(NSInteger)y
{
    //    NSRange range = NSMakeRange(x + y*(maxX+1), 1);

    //    [self fillLineToY:y];

    //    [self setTextColor:textColor range:range];

    //    NSMutableAttributedString *attr = [self textStorage];

    //    [attr addAttribute:NSBackgroundColorAttributeName value:backgroundColor range:range]; // Tres lent !
}

- (NSInteger) ansiOut:(unichar)b
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
                 * curx = 0;
                 * break;
                 *
                 * case '\n':
                 * cury++;
                 * break;
                 */

                case '\n':
                    curX = 1;

                    [self nextLine];

                    break;

                case '\r':
                    curX = 1;

                    break;

                case '\t': {
                    NSInteger toDo;

                    toDo = tabSpaces - (curX % tabSpaces);

                    for (x = 0; x < toDo; x++) {
                        [self ansiAffChr:' ' atX:curX andY:curY];
                        [self ansiAffCol:curAttrTextColor withBackground:curAttrBackgroundColor atX:curX andY:curY];
                        curX++;
                        if (curX > maxX) {
                            curX = 1;
                            [self nextLine];
                        }

                        [self locate:(int)curX andY:(int)curY];
                    }
                    break;
                }
                case '\b':
                    if (curX > 1) curX--;
                    break;

                case '\07':                  // The beep -------------------
                    // putchar('\07');
                    [self ansiAffChr:7 atX:curX andY:curY];

                    break;

                default:

                    [self ansiAffChr:b atX:curX andY:curY];
                    [self ansiAffCol:curAttrTextColor withBackground:curAttrBackgroundColor atX:curX andY:curY];

                    curX++;
                    if (curX > maxX) {
                        curX = 1;
                        [self nextLine];
                    }

                    //			NSLog(@"cur at: %ldx%ld", curX, curY);
                    break;
            } // switch
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
            if (strchr(ansiTerminators, (int)b)) {
                //			NSLog(@"set ansi: [%c %s", b, argbuf);

                switch ((int)b) {
                    case 'H':         // --- set cursor position --------------------
                    case 'F':
                        [self setPos:argbuf withLen:arglen];
                        break;

                    case 'A':         // --- up -------------------------------------
                        [self goUp:argbuf];
                        break;

                    case 'B':         // --- down -----------------------------------
                        [self goDown:argbuf];
                        break;

                    case 'C':         // --- right ----------------------------------
                        [self goRight:argbuf];
                        break;

                    case 'D':         // --- left -----------------------------------
                        [self goLeft:argbuf];
                        break;

                    case 'n':         // --- device statusreport pos ----------------
                        break;

                    case 's':         // --- save pos -------------------------------
                        [self savePos];
                        break;

                    case 'u':         // --- restore pos ----------------------------
                        [self restorePos];
                        break;

                    case 'J':         // --- clear screen ---------------------------
                        [self clearScreen];
                        break;

                    case 'K':         // --- delete to eol --------------------------
                        [self clearToEnd:argbuf withLen:arglen];

                        // ChrWin(curx,cury,maxx-1,cury,32);
                        // ColWin(curx,cury,maxx-1,cury,curattr);
                        break;

                    case 'M':         // --- set video attribs ----------------------
                        //                        [self deleteLines:argbuf withLen:arglen];
                        break;

                    case 'L':         // --- set video attribs ----------------------
                        //                        [self insertLines:argbuf withLen:arglen];
                        break;

                    case 'm':         // --- set video attribs ----------------------
                        //                        [self setColors:argbuf withLen:arglen];
                        break;

                    case 'p':         // --- keyboard redef -------------------------
                        break;

                    case 'P':         // --- backspace ----------------------
                        [self backspace:argbuf];
                        break;

                    default:          // --- unsupported ----------------------------
                        NSLog(@"ERROR !");
                        break;
                } // switch
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
            NSLog(@"Error of ANSI code");
            break;
    } // switch

    return curY;
} // ansiOut

- (void) screen_draw:(u16 *)pixels to:(u32 *)destPixels {
    int x, y;
    uint32_t *buffer_scr;

    if (gb.resize == 2) {  // 320X200
        buffer_scr = destPixels;

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

                    buffer_scr[x + y * bufferWidth] = RGB565toRGB(color);
                }
            }
        }
    }

    if (gb.resize == 1) {   // TODO: improve resize
        // ID_SCREEN_AUTO

        buffer_scr = destPixels;

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
                for (x = 0; x < bufferWidth; x++) {
                    u16 color = pixels[x  + (gb.x0 * 2) + (y + gb.y0) * 768];

                    buffer_scr[x + y * bufferWidth] = RGB565toRGB(color);
                }
            }
        }
    }

    #define PG_LBMASK 0xFEFEFEFE

    #define AlphaBlendFast(pixel, backpixel) (((((pixel) & PG_LBMASK) >> 1) | (((backpixel) & PG_LBMASK) >> 1)))

    if (gb.resize == 4) {       // Overscan (ok mode 2)
        buffer_scr = destPixels;

        for (y = 0; y < bufferHeight; y++) {
            for (x = 0; x < bufferWidth; x++) {
                u16 color = pixels[x + y * bufferWidth];

                buffer_scr[x + y * bufferWidth] = RGB565toRGB(color); //  AlphaBlendFast(RGB565toRGB(color),buffer_scr[x + y * bufferWidth]) | 0xFF000000;
            }
        }
    }

    // Draw icon

    if (gb.iconTimer > 0) {
        int x, y;
        int dispiconX = gb.iconToDislay / 16;
        int dispiconY = gb.iconToDislay % 16;

        buffer_scr = destPixels + (8 * bufferWidth) + 8;

        for (y = 0; y < 32; y++) {
            for (x = 0; x < 32; x++) {
                u16 car;
                car = gb.icons[(x + dispiconX * 32) + (y + dispiconY * 32) * 448];
                if (car != 33840) {
                    *buffer_scr = RGB565toRGB(car);
                }
                buffer_scr++;
            }

            buffer_scr += (bufferWidth - 32);
        }

        gb.iconTimer--;
    }

    if (gb.overlayBitmap_width != 0) {
        int dbl = 2;
        if (gb.screenIsOptimized) {
            dbl = (gb.lastMode == 2) ? 2 : 1;
        }
        buffer_scr = destPixels;

        for (y = 0; y < gb.overlayBitmap_height; y++) {
            u32 *dest = buffer_scr;
            if (gb.overlayBitmap_center != 1) {
                dest += bufferWidth * gb.overlayBitmap_posy + gb.overlayBitmap_posx + y * bufferWidth;
            } else {
                dest += bufferWidth * ((bufferHeight -  gb.overlayBitmap_height) / 2) + ((bufferWidth -  gb.overlayBitmap_width * dbl) / 2) + y * bufferWidth;
            }
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

#pragma mark - main loop

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

    return button;
} /* WsInputGetState */

- (void) run
{
    running = true;

    [self.view setKeyHandler:^(BOOL pressed, enum FB_KEY_BITS key) {
         if (key == FB_KEY_RIGHT) {
             button_state[1] = pressed ? 1 : 0;
         } else if (key == FB_KEY_LEFT) {
             button_state[0] = pressed ? 1 : 0;
         } else if (key == FB_KEY_UP) {
             button_state[2] = pressed ? 1 : 0;
         } else if (key == FB_KEY_DOWN) {
             button_state[3] = pressed ? 1 : 0;
         } else if (key == FB_KEY_A) {
             button_state[4] = pressed ? 1 : 0;
         } else if (key == FB_KEY_B) {
             button_state[5] = pressed ? 1 : 0;
         } else if (key == FB_KEY_SELECT) {
             button_state[11] = pressed ? 1 : 0;
         } else if (key == FB_KEY_START) {
             button_state[10] = pressed ? 1 : 0;
         } else if (key == FB_KEY_L) {
             button_state[8] = pressed ? 1 : 0;
         } else if (key == FB_KEY_R) {
             button_state[9] = pressed ? 1 : 0;
         }
     }];

//    self.view.gb = &gb;
//    self.view.document = self;

    initSound(&gb, 44100);

    self.audioClient = [[GBAudioClient alloc] initWithRendererBlock:^(UInt32 sampleRate, UInt32 nFrames, GB_sample_t *buffer) {
                            if (gb.isPaused) {
                                memset(buffer, 0, nFrames * 4);
                                return;
                            }

                            crocods_copy_sound_buffer(&gb, buffer, nFrames);
                        } andSampleRate:44100];

//    self.view.mouseHidingEnabled = (self.mainWindow.styleMask & NSWindowStyleMaskFullScreen) != 0;

    if (![[NSUserDefaults standardUserDefaults] boolForKey:@"Mute"]) {
        [self.audioClient start];
    }

    unsigned long expectedTime = 0, ellapsedTime = 0;

    @autoreleasepool {
        [NSThread setThreadPriority:1.0];

        unsigned long TimeOut = 0, OldTime = 0;
        char framebuf[128];

        framebuf[0] = 0;

        do {
            if (!gb.isPaused) {       // Pause only the Z80
                long ticks = nds_GetTicks();

                TimeOut = croco_cpu_doFrame(&gb);

                expectedTime += 1000000 / 50;  // (50 hz & 1000 ms)
                ellapsedTime += (nds_GetTicks() - ticks);
            }

            // End of frame

//            if (!CalcCRTCLine(&gb))
            {     // Arrive en fin d'ecran ?
                  // Rafraichissement de l'ecran...

//                if ((!gb.isPaused) && (!updateChangeCountWasSent)) {
//                    updateChangeCountWasSent = true;
//
//                    [self performSelectorOnMainThread:@selector(updateChangeCount:) withObject:NSChangeDone waitUntilDone:FALSE];
//
//                    //                    [self updateChangeCount:NSChangeDone];
//                }

                if (gb.runStartApp > 0) {
                    if (gb.runStartApp == 1) {
                        self.crocoApplication = [[SSHCrocoApplication alloc] init];
                        self.crocoApplication.delegate = self;
                        self.crocoApplication.core = &gb;
                    }

                    if (gb.runStartApp == 10) {   // start after 30 frames
                        [self getCursor];

                        [self.crocoApplication start];
                    }

                    gb.runStartApp++;
                }

                if (self.crocoApplication != nil) {
                    NSString *string = [self.crocoApplication getBuffer];

                    if ([string length] > 0) {
                        [self sendOutput:string];

                        [self locate:(int)curX andY:(int)curY];
                    }
                }

                if (captureScreen) {
                    captureScreen = false;

                    [self captureScreen];
                }

                if (gb.dispframerate) {
                    cpcprint16(&gb, gb.MemBitmap, gb.MemBitmap_width, 0, 0, framebuf, RGB565(0xFF, 0xFF, 0x00), RGB565(0x00, 0x00, 0xFF), 1, 0);
                }

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

                    if (gb.resize == 1) {                // TODO: improve resize
                        width = gb.screenBufferWidth;
                        height = gb.screenBufferHeight;

                        if ((width < 256) || (height < 128)) {      // Game use splits or others video hack
                            ExecuteMenu(&gb, ID_SCREEN_OVERSCAN, NULL);
                        }
                    }

                    if (gb.resize == 4) {                // Overscan
                        width = 384;
                        height = 272;
                    }

                    if ((!gb.screenIsOptimized) || (gb.lastMode == 2)) {
                        width = width * 2;
                    }

                    if (bufferWidth != width) {
                        bufferWidth = width;
                        bufferHeight = height;

                        NSLog(@"changeSize %dx%d", width, height);

                        [self.view changeSizeWithWidth:width andHeight:height];
                    }
                }

                [self vblank];

                gb.overlayBitmap_width = 0;

//                NSLog(@"update screen: %p (%d,%d)", self.view.pixels, gb.MonitorScanLineCount, gb.LineCounter);

                // Synchronisation de l'image ‡ 50 Hz
                // Pourcentage, temps espere, temps pris, temps z80, nombre de drawligne

//                BOOL doResync = gb.DoResync;

//                doResync = true;

//                if (doResync) {
                //                    NSInteger Time;

                borderX = bx;
                borderY = by;

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

                if ((!gb.turbo)  && ((gb.AutoType.nFlags & (AUTOTYPE_ACTIVE | AUTOTYPE_WAITING)) == 0) ) {      // && (gb.SoundBusy!=0)
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
} // run

/*
 * - (void) run
 * {
 * running = true;
 * GB_set_pixels_output(&gb, self.view.pixels);
 * self.view.gb = &gb;
 * GB_set_sample_rate(&gb, 96000);
 * self.audioClient = [[GBAudioClient alloc] initWithRendererBlock:^(UInt32 sampleRate, UInt32 nFrames, GB_sample_t *buffer) {
 * GB_apu_copy_buffer(&gb, buffer, nFrames);
 * } andSampleRate:96000];
 * self.view.mouseHidingEnabled = (self.mainWindow.styleMask & NSWindowStyleMaskFullScreen) != 0;
 * if (![[NSUserDefaults standardUserDefaults] boolForKey:@"Mute"]) {
 * [self.audioClient start];
 * }
 *
 * while (running) {
 * GB_run(&gb);
 * }
 *
 * [self.audioClient stop];
 * self.audioClient = nil;
 * self.view.mouseHidingEnabled = NO;
 * GB_save_battery(&gb, [[[self.fileName stringByDeletingPathExtension] stringByAppendingPathExtension:@"sav"] UTF8String]);   // fileURL ?
 * stopping = false;
 * }
 */

- (void) start
{
    if (running) return;
    [[[NSThread alloc] initWithTarget:self selector:@selector(run) object:nil] start];
}

- (void) stop
{
    if (!running) return;

    while (self.audioClient != nil);

    running = false;
}

- (void) documentWasModified:(id)sender
{
    if ((!gb.isPaused) && (!updateChangeCountWasSent)) {
        updateChangeCountWasSent = true;

        [self performSelectorOnMainThread:@selector(updateChangeCount:) withObject:NSChangeDone waitUntilDone:FALSE];

        //                    [self updateChangeCount:NSChangeDone];
    }
}

- (IBAction) reset:(id)sender
{
    [self stop];
    is_inited = false;

    if (disk != nil) {
        free(disk);
        disk = nil;
    }
    [self loadDiskOrSnapshot:[NSString stringWithUTF8String:gb.openFilename]];

    //    GB_free(&gb);
    [self initCGB];

    //    [self readFromFile:[NSString stringWithUTF8String:gb.openFilename] ofType:@"gb"];
    [self start];
}

- (IBAction) togglePause:(id)sender
{
    if (running) {
        [self stop];
    } else {
        [self start];
    }
}

- (void) windowControllerDidLoadNib:(NSWindowController *)aController
{
    [super windowControllerDidLoadNib:aController];

    [self initCGB];

    bufferWidth = 384;
    bufferHeight = 272;

    self.consoleOutput.textContainerInset = NSMakeSize(4, 4);
    [self.view becomeFirstResponder];

    CGRect window_frame = self.mainWindow.frame;

    window_frame.size.width = MAX([[NSUserDefaults standardUserDefaults] integerForKey:@"LastWindowWidth"],
                                  window_frame.size.width);
    window_frame.size.height = MAX([[NSUserDefaults standardUserDefaults] integerForKey:@"LastWindowHeight"],
                                   window_frame.size.height);
    [self.mainWindow setFrame:window_frame display:YES];
    [self start];

    //    [self updateChangeCount:NSChangeDone];
} /* windowControllerDidLoadNib */

+ (BOOL) autosavesInPlace
{
    return NO;
}

- (NSString *) windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"Document";
}

- (void) close
{
    ExecuteMenu(&gb, ID_SAVE_SETTINGS, NULL);

    [[NSUserDefaults standardUserDefaults] setInteger:self.mainWindow.frame.size.width forKey:@"LastWindowWidth"];
    [[NSUserDefaults standardUserDefaults] setInteger:self.mainWindow.frame.size.height forKey:@"LastWindowHeight"];
    [self stop];
    [self.consoleWindow close];
    [super close];
}

- (IBAction) interrupt:(id)sender
{
    gb.debug_stopped = true;
    if (!running) {
        [self start];
    }
    [self.consoleInput becomeFirstResponder];
}

- (IBAction) mute:(id)sender
{
    if (self.audioClient.isPlaying) {
        [self.audioClient stop];
    } else {
        [self.audioClient start];
    }
    [[NSUserDefaults standardUserDefaults] setBool:!self.audioClient.isPlaying forKey:@"Mute"];
}

- (BOOL) validateUserInterfaceItem:(id<NSValidatedUserInterfaceItem>)anItem
{
    if ([anItem action] == @selector(mute:)) {
        [(NSMenuItem *)anItem setState:!self.audioClient.isPlaying];
    } else if ([anItem action] == @selector(reset:) && anItem.tag != 0) {
        //        [(NSMenuItem*)anItem setState:(anItem.tag == 1 && !gb.is_cgb) || (anItem.tag == 2 && gb.is_cgb)];
    } else if ([anItem action] == @selector(interrupt:)) {
        if (![[NSUserDefaults standardUserDefaults] boolForKey:@"DeveloperMode"]) {
            return false;
        }
    }
    return [super validateUserInterfaceItem:anItem];
}

- (void) windowWillEnterFullScreen:(NSNotification *)notification
{
    fullScreen = true;
    self.view.mouseHidingEnabled = running;
}

- (void) windowWillExitFullScreen:(NSNotification *)notification
{
    fullScreen = false;
    self.view.mouseHidingEnabled = NO;
}

- (NSRect) windowWillUseStandardFrame:(NSWindow *)window defaultFrame:(NSRect)newFrame
{
    if (fullScreen) {
        return newFrame;
    }
    NSRect rect = window.contentView.frame;

    int titlebarSize = window.contentView.superview.frame.size.height - rect.size.height;
    int step = 160 / [[window screen] backingScaleFactor];

    rect.size.width = floor(rect.size.width / step) * step + step;
    rect.size.height = rect.size.width / 10 * 9 + titlebarSize;

    if (rect.size.width > newFrame.size.width) {
        rect.size.width = 384;
        rect.size.height = 272 + titlebarSize;
    } else if (rect.size.height > newFrame.size.height) {
        rect.size.width = 384;
        rect.size.height = 272 + titlebarSize;
    }

    rect.origin = window.frame.origin;
    rect.origin.y -= rect.size.height - window.frame.size.height;

    return rect;
} // windowWillUseStandardFrame

- (IBAction) showConsoleWindow:(id)sender
{
    [self.consoleWindow orderBack:nil];
}

- (IBAction) saveState:(id)sender
{
    bool was_running = running;

    if (!gb.debug_stopped) {
        [self stop];
    }

    ExecuteMenu(&gb, ID_SAVESNAP, NULL);

    //    GB_save_state(&gb, [[[self.fileName stringByDeletingPathExtension] stringByAppendingPathExtension:[NSString stringWithFormat:@"s%ld", (long)[sender tag] ]] UTF8String]);
    if (was_running) {
        [self start];
    }
}

- (IBAction) loadState:(id)sender
{
    bool was_running = running;

    if (!gb.debug_stopped) {
        [self stop];
    }

    //    ExecuteMenu(&gb, ID_, NULL);

    //    GB_load_state(&gb, [[[self.fileName stringByDeletingPathExtension] stringByAppendingPathExtension:[NSString stringWithFormat:@"s%ld", (long)[sender tag] ]] UTF8String]);
    if (was_running) {
        [self start];
    }
}

- (IBAction) clearConsole:(id)sender
{
    [self.consoleOutput setString:@""];
}

- (IBAction) performExecuteMenu:(id)sender
{
    if ([sender isKindOfClass:[NSMenuItem class]]) {
        NSMenuItem *menuItem = sender;
        ExecuteMenu(&gb, (int)menuItem.tag, NULL);
    }
}

- (void) performAtomicBlock:(void (^)())block
{
    while (!is_inited);
    bool was_running = running && !gb.debug_stopped;

    if (was_running) {
        [self stop];
    }
    block();
    if (was_running) {
        [self start];
    }
}

void guestExit(void)
{

}

void guestGetAllKeyPressed(core_crocods_t *core, char *string)
{
    string[0] = 0;
}

void guestGetJoystick(core_crocods_t *core, char *string)
{
    string[0] = 0;
}

@end
