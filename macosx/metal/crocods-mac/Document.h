#import <Cocoa/Cocoa.h>

#include "GBView.h"
#import "CrocoApplication.h"

@class SSHCrocoApplication;

@interface Document : NSDocument <NSWindowDelegate, CrocoApplicationDelegate> {
    
}

@property (strong) IBOutlet GBView *view;
@property (strong) IBOutlet NSTextView *consoleOutput;
@property (strong) IBOutlet NSPanel *consoleWindow;
@property (strong) IBOutlet NSTextField *consoleInput;
@property (strong) IBOutlet NSWindow *mainWindow;
@property (strong) IBOutlet NSView *memoryView;
@property (strong) IBOutlet NSPanel *memoryWindow;
@property (strong) IBOutlet NSTextField *memoryBankInput;
@property (strong) IBOutlet NSToolbarItem *memoryBankItem;

@property (strong) IBOutlet NSURL *diskURL;


@property (strong) SSHCrocoApplication *crocoApplication;

@property (readonly) core_crocods_t gb;

@property (assign) NSEventModifierFlags modifierFlags;


-(void) performAtomicBlock: (void (^)())block;

- (IBAction)btnCaptureScreen:(id)sender;
- (void)documentWasModified:(id)sender;

- (IBAction)reset:(id)sender;

@end
