#import "TapePlayerWindowController.h"
#include "playtzx.h"


#pragma mark - TZXConverter

@interface TZXConverter : NSObject

+ (NSData*)dataWithTZXFile:(NSString*)file;
+ (void)convertSoundFile:(NSString*)inputFile toMP3:(NSString*)outputFile;

@end

@implementation TZXConverter

+ (NSData*)dataWithTZXFile:(NSString *)file {
    
    playtzx_main([file UTF8String], "/tmp/toto.tmp");
    
    return [NSData dataWithContentsOfFile:@"/tmp/toto.tmp"];
}

+ (void)convertSoundFile:(NSString*)inputFile toMP3:(NSString*)outputFile {
    
    AudioStreamBasicDescription	inputFormat;
    AudioFileTypeID				outputFileType;
    AudioStreamBasicDescription	outputFormat;
    ExtAudioFileRef infile, outfile;
    
    NSURL *inputFileURL = [NSURL fileURLWithPath:inputFile];
    NSURL *outputFileURL = [NSURL fileURLWithPath:outputFile];
    
    // first open the input file
    OSStatus err = ExtAudioFileOpenURL ((__bridge CFURLRef)inputFileURL, &infile);
    
    // if outputBitRate is specified, this can change the sample rate of the output file
    // so we let this "take care of itself"
    
    outputFileType=kAudioFileMP3Type;
    
    outputFormat.mFormatID = kAudioFormatMPEGLayer3;
    outputFormat.mSampleRate = 44100.0;
    outputFormat.mChannelsPerFrame = 1;
    outputFormat.mBytesPerFrame = 2;
    outputFormat.mBitsPerChannel = 16;
    
    {
        AudioFileID infile;
        err = AudioFileOpenURL((__bridge CFURLRef)inputFileURL, kAudioFileReadPermission, 0, &infile);
        
        UInt32 size;
        AudioFileGetPropertyInfo(infile, kAudioFilePropertyFormatList, &size, NULL);
        UInt32 numFormats = size / sizeof(AudioFormatListItem);
        AudioFormatListItem *formatList = (AudioFormatListItem*)malloc(size);
        
        AudioFileGetProperty(infile,kAudioFilePropertyFormatList, &size, formatList);
        numFormats = size / sizeof(AudioFormatListItem); // we need to reassess the actual number of formats when we get it
        if (numFormats == 1) {
            inputFormat = formatList[0].mASBD;
        }
        AudioFileClose (infile);
        free(formatList);
    }
    
    
    // use AudioFormat API to fill out the rest.
    UInt32 size = sizeof(outputFormat);
    err = AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &size, &outputFormat);
    
    // create the output file (this will erase an exsiting file)
    err = ExtAudioFileCreateWithURL ((__bridge CFURLRef)outputFileURL, outputFileType, &outputFormat, NULL, kAudioFileFlags_EraseFile, &outfile);
    
    size = sizeof(inputFormat);
    err = ExtAudioFileSetProperty(infile, kExtAudioFileProperty_ClientDataFormat, size, &inputFormat);
    
    size = sizeof(inputFormat);
    err = ExtAudioFileSetProperty(outfile, kExtAudioFileProperty_ClientDataFormat, size, &inputFormat);
    
    
    // set up buffers
    char srcBuffer[32768];
    
    // do the read and write - the conversion is done on and by the write call
    while (1)
    {
        AudioBufferList fillBufList;
        fillBufList.mNumberBuffers = 1;
        fillBufList.mBuffers[0].mNumberChannels = inputFormat.mChannelsPerFrame;
        fillBufList.mBuffers[0].mDataByteSize = 32768;
        fillBufList.mBuffers[0].mData = srcBuffer;
        
        // client format is always linear PCM - so here we determine how many frames of lpcm
        // we can read/write given our buffer size
        UInt32 numFrames = (32768 / inputFormat.mBytesPerFrame);
        
        // printf("test %d\n", numFrames);
        
        err = ExtAudioFileRead (infile, &numFrames, &fillBufList);
        if (!numFrames) {
            // this is our termination condition
            break;
        }
        
        err = ExtAudioFileWrite(outfile, numFrames, &fillBufList);
    }
    
    // close
    ExtAudioFileDispose(outfile);
    ExtAudioFileDispose(infile);
}

@end


#pragma mark - TapePlayerWindowController

@interface TapePlayerWindowController () {
    NSURL *url;
}

@end


@implementation TapePlayerWindowController

@synthesize mySound, buttonPlay, buttonStop, positionSlider;

- (void)windowDidLoad {
    [super windowDidLoad];
    
    NSImage *myImage = [NSImage imageNamed:@"tape"];
    
    [[self.window contentView] setWantsLayer:YES];
    [[self.window contentView] layer].contents = myImage;
    
    self.window.title = [[[url path] lastPathComponent] stringByDeletingPathExtension];
    self.window.movableByWindowBackground = true;
    self.window.opaque = false;
    self.window.backgroundColor = [NSColor clearColor];
}

- (IBAction)showWindow:(id)sender {
    [super showWindow:sender];
    
    [self.window setAlphaValue:1.0];
}

- (id)initWithURL:(NSURL*)url0 {
    self = [super initWithWindowNibName:@"TapePlayerWindowController" owner:self];
    if (self != nil) {
        nst = [NSTimer scheduledTimerWithTimeInterval:0.2 target:self selector:@selector(progression:) userInfo:nil repeats:YES];
        
        url = url0;
        /*
         NSTimer *nst = [NSTimer timerWithTimeInterval:1.0
         target:self
         selector:@selector(progression)
         userInfo:nil
         repeats:YES];
         */
        
        //   [[NSRunLoop currentRunLoop] addTimer:nst forMode:NSDefaultRunLoopMode];
        NSLog(@"song current time: %2f", [mySound currentTime]);
    }
    return self;
}

- (IBAction)playButton:(id)sender {
    NSString *myPath = [[NSString alloc] initWithString:[url path]];
    NSLog(@"path = %@", myPath);
    
    NSData *data = [TZXConverter dataWithTZXFile:myPath];
    
    mySound = [[AVAudioPlayer alloc] initWithData:data error:nil];
    
    mySound.delegate = self;
    mySound.volume = 1.0;
    mySound.numberOfLoops = -1;
    [mySound play];
    
    [mySound setMeteringEnabled:YES];
    [mySound updateMeters];
        
    if (mySound) {
        NSLog(@"Playing music");
        buttonPlay.enabled = NO;
        buttonStop.enabled = YES;
    }
    
    [positionSlider setMaxValue:[mySound duration]];
}

- (IBAction)okButton:(id)sender {
    
    if ([self windowShouldClose:self]) {
        [self.window close];
    }
}

- (BOOL)windowShouldClose:(id)sender {
    [self stopButton:nil];

    return true;
}

- (IBAction)stopButton:(id)sender {
    [mySound stop];
    mySound = nil;
    
    NSLog(@"Music stoped");
    
    buttonPlay.enabled = YES;
    buttonStop.enabled = NO;
}

- (void)progression:(id)sender {
    if (mySound != nil) {
        [mySound updateMeters];
        
//        for (int i = 0; i < mySound.numberOfChannels; i++) {
//            NSLog(@"Channel %d, Level: %f dB - %f dB", i, [mySound averagePowerForChannel:i], [mySound peakPowerForChannel:i]);
//        }
        
        [positionSlider setDoubleValue:[mySound currentTime]];
    }
}

- (IBAction)modifyPosition:(id)sender {
    [mySound setCurrentTime:[positionSlider intValue]];
}

- (void)progression {
}



@end
