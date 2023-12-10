//
//  CrocoDebuggerPanel.m
//
//
//  Created by Miguel Vanhove on 14/10/16.
//
//

#import "CrocoDebuggerPanel.h"
#import "plateform.h"
#import "Document.h"
#import "AppDelegate.h"

#include "dissz80p.h"


@interface CrocoDebuggerPanel () {
    IBOutlet NSTextField *regPC;
    IBOutlet NSTextView *memoryBank;
    IBOutlet NSTextView *z80Dump;

}

@end

@implementation CrocoDebuggerPanel

- (id)init {
    self = [super initWithWindowNibName:@"CrocoDebuggerPanel" owner:self];
    if (self != nil) {
        NSNotificationCenter* theCenter = [NSNotificationCenter defaultCenter];
        [theCenter addObserver:self selector:@selector(windowLostFocus:) name:NSWindowDidResignKeyNotification object:self.window];
        [theCenter addObserver:self selector:@selector(windowLostFocus:) name:NSWindowDidResignMainNotification object:self.window];
        [theCenter addObserver:self selector:@selector(windowHasFocus:) name:NSWindowDidBecomeKeyNotification object:self.window];
        [theCenter addObserver:self selector:@selector(windowHasFocus:) name:NSWindowDidBecomeMainNotification object:self.window];
    }
    return self;
}

- (void)dealloc
{
    
    
    NSNotificationCenter* theCenter = [NSNotificationCenter defaultCenter];
    [theCenter removeObserver:self name:NSWindowDidResignKeyNotification object:self.window];
    [theCenter removeObserver:self name:NSWindowDidResignMainNotification object:self.window];
    [theCenter removeObserver:self name:NSWindowDidBecomeKeyNotification object:self.window];
    [theCenter removeObserver:self name:NSWindowDidBecomeMainNotification object:self.window];
    
    //  GB_free(&gb);
}

- (void)windowDidLoad {
    [super windowDidLoad];
    
    [[z80Dump textStorage] setFont:[NSFont fontWithName:@"Courier" size:10]];

    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}


- (void)windowLostFocus:(id)sender {
//    NSNotification *notification = sender;
    
}

- (void)windowHasFocus:(id)sender {
//    NSNotification *notification = sender;
    
    [self dump];
}

#define   ROMINF_OFF        0x04
#define   ROMSUP_OFF        0x08
#define   MASK_14BIT      0x3FFF

- (void)dump {
    
    AppDelegate *appDelegate = [[NSApplication sharedApplication] delegate];
    Document *currentDoc = appDelegate.currentDocument;
    
    //currentDoc->gb
    
    core_crocods_t core = [currentDoc gb];
    
    
    regPC.stringValue = [NSString stringWithFormat:@"%04X", core.Z80.PC.Word];
    
    NSLog(@"running: %@ - %d", currentDoc, core.Z80.PC.Word);
    
    
    
    int AdjRam[ 8 ][ 4 ][ 8 ] =
    {
        // C0       C1       C2       C3       C4       C5       C6       C7
        0x00000, 0x00000, 0x10000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
        0x04000, 0x04000, 0x14000, 0x0C000, 0x10000, 0x14000, 0x18000, 0x1C000,
        0x08000, 0x08000, 0x18000, 0x08000, 0x08000, 0x08000, 0x08000, 0x08000,
        0x0C000, 0x1C000, 0x1C000, 0x1C000, 0x0C000, 0x0C000, 0x0C000, 0x0C000,
        
        // C8       C9       CA       CB       CC       CD       CE       CF
        0x00000, 0x00000, 0x20000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
        0x04000, 0x04000, 0x24000, 0x0C000, 0x20000, 0x24000, 0x28000, 0x2C000,
        0x08000, 0x08000, 0x28000, 0x08000, 0x08000, 0x08000, 0x08000, 0x08000,
        0x0C000, 0x2C000, 0x2C000, 0x2C000, 0x0C000, 0x0C000, 0x0C000, 0x0C000,
        
        // D0       D1       D2       D3       D4       D5       D6       D7
        0x00000, 0x00000, 0x30000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
        0x04000, 0x04000, 0x34000, 0x0C000, 0x30000, 0x34000, 0x38000, 0x3C000,
        0x08000, 0x08000, 0x38000, 0x08000, 0x08000, 0x08000, 0x08000, 0x08000,
        0x0C000, 0x3C000, 0x3C000, 0x3C000, 0x0C000, 0x0C000, 0x0C000, 0x0C000,
        
        // D8       D9       DA       DB       DC       DD       DE       DF
        0x00000, 0x00000, 0x40000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
        0x04000, 0x04000, 0x44000, 0x0C000, 0x40000, 0x44000, 0x48000, 0x4C000,
        0x08000, 0x08000, 0x48000, 0x08000, 0x08000, 0x08000, 0x08000, 0x08000,
        0x0C000, 0x4C000, 0x4C000, 0x4C000, 0x0C000, 0x0C000, 0x0C000, 0x0C000,
        
        // E0       E1       E2       E3       E4       E5       E6       E7
        0x00000, 0x00000, 0x50000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
        0x04000, 0x04000, 0x54000, 0x0C000, 0x50000, 0x54000, 0x58000, 0x5C000,
        0x08000, 0x08000, 0x58000, 0x08000, 0x08000, 0x08000, 0x08000, 0x08000,
        0x0C000, 0x5C000, 0x5C000, 0x5C000, 0x0C000, 0x0C000, 0x0C000, 0x0C000,
        
        // E8       E9       EA       EB       EC       ED       EE       EF
        0x00000, 0x00000, 0x60000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
        0x04000, 0x04000, 0x64000, 0x0C000, 0x60000, 0x64000, 0x68000, 0x6C000,
        0x08000, 0x08000, 0x68000, 0x08000, 0x08000, 0x08000, 0x08000, 0x08000,
        0x0C000, 0x6C000, 0x6C000, 0x6C000, 0x0C000, 0x0C000, 0x0C000, 0x0C000,
        
        // F0       F1       F2       F3       F4       F5       F6       F7
        0x00000, 0x00000, 0x70000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
        0x04000, 0x04000, 0x74000, 0x0C000, 0x70000, 0x74000, 0x78000, 0x7C000,
        0x08000, 0x08000, 0x78000, 0x08000, 0x08000, 0x08000, 0x08000, 0x08000,
        0x0C000, 0x7C000, 0x7C000, 0x7C000, 0x0C000, 0x0C000, 0x0C000, 0x0C000,
        
        // F8       F9       FA       FB       FC       FD       FE       FF
        0x00000, 0x00000, 0x80000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
        0x04000, 0x04000, 0x84000, 0x0C000, 0x80000, 0x84000, 0x88000, 0x8C000,
        0x08000, 0x08000, 0x88000, 0x08000, 0x08000, 0x08000, 0x08000, 0x08000,
        0x0C000, 0x8C000, 0x8C000, 0x8C000, 0x0C000, 0x0C000, 0x0C000, 0x0C000,
        
        // 0123     0127     4567     0327     0423     0523     0623     0723
    };
    
    
    NSMutableString *bankString = [[NSMutableString alloc] initWithString:@""];
    
    [bankString appendString:[NSString stringWithFormat:@"Core Poke 0 : mem %04X\n", AdjRam[ core.Bloc ][ 0 ][ core.RamSelect ]]];
    if ( core.DecodeurAdresse & ROMINF_OFF ) {
         [bankString appendString:[NSString stringWithFormat:@"Core Peek 0 : mem %04X\n", AdjRam[ core.Bloc ][ 0 ][ core.RamSelect ]]];
    } else {
         [bankString appendString:[NSString stringWithFormat:@"Core Peek 0 : Rominf\n"]];
    }

     [bankString appendString:[NSString stringWithFormat:@"Core Poke 1 : mem %04X\n", AdjRam[ core.Bloc ][ 1 ][ core.RamSelect ]]];
     [bankString appendString:[NSString stringWithFormat:@"Core Peek 1 : mem %04X\n", AdjRam[ core.Bloc ][ 1 ][ core.RamSelect ]]];

     [bankString appendString:[NSString stringWithFormat:@"Core Poke 2 : mem %04X\n", AdjRam[ core.Bloc ][ 2 ][ core.RamSelect ]]];
     [bankString appendString:[NSString stringWithFormat:@"Core Peek 2 : mem %04X\n", AdjRam[ core.Bloc ][ 2 ][ core.RamSelect ]]];
    
     [bankString appendString:[NSString stringWithFormat:@"Core Poke 3 : mem %04X\n", AdjRam[ core.Bloc ][ 3 ][ core.RamSelect ]]];
    
    if ( core.DecodeurAdresse & ROMSUP_OFF ) {
         [bankString appendString:[NSString stringWithFormat:@"Core Peek 3 : mem %04X\n", AdjRam[ core.Bloc ][ 3 ][ core.RamSelect ]]];
    } else {
        [bankString appendString:[NSString stringWithFormat:@"Core Peek 3: Romext %d", core.NumRomExt]];
    }

    memoryBank.string = bankString;

    
    // Debugger
    
    NSMutableString *z80String = [[NSMutableString alloc] initWithString:@""];

    
    DISZ80	*d;             /* Pointer to the Disassembly structure */
    int		line, err;		/* line count */
    WORD	dAddr;			/* Disassembly address */
    
    d = malloc(sizeof(DISZ80));
    memset(d, 0, sizeof(DISZ80));
    
    dAddr = core.Z80.PC.Word;
    
    dZ80_SetDefaultOptions(d);
    d->cpuType = DCPU_Z80;
    d->mem0Start = &core.TabPEEK[ dAddr >> 14 ][ dAddr & MASK_14BIT ];
    
    dAddr = dAddr & MASK_14BIT;
    
    d->flags |= DISFLAG_SINGLE;
    
    /* And we're off! Let's disassemble 20 instructions from dAddr */
    for(line=0; line < 20; line++)
    {
        /* Set the disassembly address */
        d->start = d->end = dAddr;
        
        err = dZ80_Disassemble(d);
        if (err != DERR_NONE)
        {
            printf("**** dZ80 error:  %s\n", dZ80_GetErrorText(err));
            break;
        }
        
        /* Display the disassembled line, using the hex dump and disassembly buffers in the DISZ80 structure */
        [z80String appendString:[NSString stringWithFormat:@"%04x: %10s  %s\n", dAddr, d->hexDisBuf, d->disBuf]];
        
        /* Point to the next instruction (bytesProcessed holds the number of bytes for the last instruction disassembled) */
        dAddr += (WORD)d->bytesProcessed;
    }
    
    z80Dump.string = z80String;
}

- (IBAction)oneStep:(id)sender {
    
    
    AppDelegate *appDelegate = [[NSApplication sharedApplication] delegate];
    Document *currentDoc = appDelegate.currentDocument;
    
    //currentDoc->gb
    
    core_crocods_t core = [currentDoc gb];
    
    core.debugOneStep=1;
    
    [self performSelector:@selector(dump) withObject:nil afterDelay:1.0];
}


@end
