#import <Cocoa/Cocoa.h>


@class Document;

@interface AppDelegate : NSObject <NSApplicationDelegate> {

    
    
}

@property IBOutlet NSWindow *preferencesWindow;
@property IBOutlet NSWindow *debuggerWindow;

@property Document *currentDocument;


- (IBAction)showPreferences: (id) sender;
- (IBAction)toggleDeveloperMode:(id)sender;

- (IBAction)playTape:(id)sender;



@end

