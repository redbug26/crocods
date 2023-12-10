#import "AppDelegate.h"
#import "Document.h"
#import "TapePlayerWindowController.h"
#import "CrocoDebuggerPanel.h"
#import "ImporterDSK.h"
#import "ManageDSK.h"

@interface AppDelegate ()

@end

@implementation AppDelegate
{
    NSWindow *preferences_window;
    TapePlayerWindowController *tapePlayer;
    ImporterDSK *importerDSK;
    ManageDSK *manageDSK;

    CrocoDebuggerPanel *debuggerPanel;
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
//    [super applicationDidFinishLaunching:notification];

#define KEY(x) ({ unichar __x = x; [NSString stringWithCharacters:&(__x) length:1]; })
    [[NSUserDefaults standardUserDefaults] registerDefaults:@{
         @"GBRight": KEY(NSRightArrowFunctionKey),
         @"GBLeft": KEY(NSLeftArrowFunctionKey),
         @"GBUp": KEY(NSUpArrowFunctionKey),
         @"GBDown": KEY(NSDownArrowFunctionKey),

         @"GBA": @"x",
         @"GBB": @"z",
         @"GBSelect": @"\x7f",
         @"GBStart": @"\r",

         @"GBTurbo": @" ",

         @"GBFilter": @"NearestNeighbor",
    }];
#undef KEY

    //    [NSWindow setAllowsAutomaticWindowTabbing:TRUE];

//    [self setBetaWithExpirationDate:@"2022-11-15"];

    //    [self showDebugger:nil];
}

- (id)openDocumentWithContentsOfURL:(NSURL *)url display:(BOOL)isShown error:(NSError **)error
{
    //    NSLog(@"openDocumentWithContentsOfURL: %@", url);

    NSDocumentController *dc = [NSDocumentController sharedDocumentController];

    [dc openDocumentWithContentsOfURL:url
                              display:isShown
                    completionHandler:^void (NSDocument *theDoc, BOOL aBool, NSError *error) {}];

    return nil;
}

- (void)application:(NSApplication *)sender openFiles:(NSArray *)filenames
{
    //    NSLog(@"openFiles: %@", filenames);

    if ([[[NSProcessInfo processInfo] arguments] containsObject:@"-close"]) {
        NSDocumentController *dc = [NSDocumentController sharedDocumentController];
        //        for (Document *doc in dc.documents) {
        //            [doc close];
        //            [dc removeDocument:doc];
        //        }
        [dc closeAllDocumentsWithDelegate:nil didCloseAllSelector:NULL contextInfo:NULL];
    }

    for (NSString *filename in filenames) {
        [self openDocumentWithContentsOfURL:[NSURL fileURLWithPath:filename] display:true error:nil];
    }
}

- (IBAction)toggleDeveloperMode:(id)sender
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    [defaults setBool:![defaults boolForKey:@"DeveloperMode"] forKey:@"DeveloperMode"];
}

- (BOOL)validateMenuItem:(NSMenuItem *)anItem
{
    if ([anItem action] == @selector(toggleDeveloperMode:)) {
        [(NSMenuItem *)anItem setState:[[NSUserDefaults standardUserDefaults] boolForKey:@"DeveloperMode"]];
    }

    return true;
}

- (IBAction)showPreferences:(id)sender
{
    NSArray *objects;
    if (!_preferencesWindow) {
        [[NSBundle mainBundle] loadNibNamed:@"Preferences" owner:self topLevelObjects:&objects];
    }
    [_preferencesWindow makeKeyAndOrderFront:self];
}

- (IBAction)showDebugger:(id)sender
{
    debuggerPanel = [[CrocoDebuggerPanel alloc] init];

    [debuggerPanel showWindow:self];
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender;
{
    return YES;
}

//- (void)applicationDidBecomeActive:(NSNotification *)notification
//{
//    //    NSDocumentController *controller = [NSDocumentController sharedDocumentController];
//    //    if (![[controller documents] count]) {
//    //        [[NSDocumentController sharedDocumentController] openDocument:self];
//    //    }
//}

- (void)applicationDidBecomeActive:(NSNotification *)aNotification {
    NSEnumerator *documentEnum = [[[NSDocumentController sharedDocumentController] documents] objectEnumerator];
    Document *aDocument;

    while (aDocument = [documentEnum nextObject]) {
        NSURL *aURL = [aDocument diskURL];

        if (aURL) {
            NSDictionary *aDict = [[NSFileManager defaultManager] attributesOfItemAtPath:[aURL path] error:nil];

            if (aDict) {
                NSDate *aDate = [aDict objectForKey:NSFileModificationDate];

                if ([[aDocument fileModificationDate] compare:aDate] < 0) {
                    NSBeginAlertSheet(NSLocalizedString(@"File changed on disk", @"Alert title"),
                                      NSLocalizedString(@"Ignore", @"Button title"),
                                      NSLocalizedString(@"Reload", @"Button title"),
                                      nil,
                                      [[[aDocument windowControllers] lastObject] window],
                                      self,
                                      NULL,
                                      @selector(sheetDidDismiss:returnCode:contextInfo:),
                                      (__bridge void *)(aDocument),
                                      NSLocalizedString(@"File has been externally modified. You can reload it or keep running version, but you will need to save them, if you don't want to lose them.", @"Alert message"));
                }
            }
        }
    }
}

- (void)sheetDidDismiss:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo {
    Document *aDoc = (__bridge NSDocument *)contextInfo;

    [aDoc updateChangeCount:NSChangeReadOtherContents];
    if (returnCode == NSAlertDefaultReturn) {  // Ignore
        [aDoc performSelector:@selector(revertDocumentToSaved:) withObject:nil afterDelay:0];
    }
    if (returnCode == NSAlertAlternateReturn) { // Reload
        [aDoc reset:nil];
    }
}

- (IBAction)playTape:(id)sender {
    NSOpenPanel *openDlg = [NSOpenPanel openPanel];
    [openDlg setCanChooseFiles:YES];

    long int result = [openDlg runModal];

    if (result == NSModalResponseOK) {
        NSURL *selectURL = [openDlg URL];

        tapePlayer = [[TapePlayerWindowController alloc] initWithURL:selectURL];

        [tapePlayer showWindow:self];
    }
}

- (IBAction)btnImportDSK:(id)sender {
    importerDSK = [[ImporterDSK alloc] init];

    [importerDSK showWindow:self];
}

- (IBAction)btnManageDSK:(id)sender {
    manageDSK = [[ManageDSK alloc] init];

    [manageDSK showWindow:self];
}

@end
