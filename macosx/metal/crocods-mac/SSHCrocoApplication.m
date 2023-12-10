//
//  SSHCrocoApplication.m
//  CrocoDS
//
//  Created by Miguel Vanhove on 28/09/16.
//  Copyright © 2016 Miguel Vanhove. All rights reserved.
//

#import "SSHCrocoApplication.h"
#import "plateform.h"

@interface SSHCrocoApplication () {
    NMSSHSession *session;
    
    NSString *login;
    NSString *host;
    
    NSFont *font;
    
    NSMutableString *buffer;
}

@property (nonatomic, strong) dispatch_queue_t sshQueue;

@end

@implementation SSHCrocoApplication

- (id)init {
    self = [super init];
    if (self) {
        
        session = nil;
        
//        [[NMSSHLogger sharedLogger] setLogLevel:NMSSHLogLevelError];
        
        self.sshQueue = dispatch_queue_create("NMSSH.queue", DISPATCH_QUEUE_SERIAL);
        
        buffer = [[NSMutableString alloc] initWithString:@""];
    }
    
    return self;
}

- (void)start {
    
    NSLog(@"start application");
    
    
    [buffer appendString:@"Wait while connecting\n"];
    
    [self connectToSSH];
    
}


- (void)scriptConsoleTextViewKeyDown:(NSEvent *)theEvent {
    
    NSString* character = [theEvent characters];
    
    if ([character length]>0) {
        switch ([character characterAtIndex:0])
        {
            case NSUpArrowFunctionKey:
                character = @"\x1b[A";
                break;
            case NSDownArrowFunctionKey:
                character = @"\x1b[B";
                break;
            case NSRightArrowFunctionKey:
                character = @"\x1b[C";
                break;
            case NSLeftArrowFunctionKey:
                character = @"\x1b[D";
                break;
                
                
            case NSHomeFunctionKey:
                character = @"\x1b[H";
                break;
                
            case NSF3FunctionKey:
                character = @"\x1b?";  // M-? dans midnight commander
                break;
                
            case NSF4FunctionKey:
                character = @"\x1b[4~";
                break;
            case NSF5FunctionKey:
                character = @"\x1bOK";
                break;
            case NSF6FunctionKey:
                character = @"\x1b?w";
                break;
                
                
            case NSEndFunctionKey:
                character = @"\x1bK"; // non
                break;
                
        }
    }
    
    [self writeChar:character];
}

- (void)writeChar:(NSString*)character {
    dispatch_async(self.sshQueue, ^{
        
        NSError *error;
        
        BOOL result = [session.channel write:character error:&error timeout:@5]; // 5 secondes
        
        
        if (!result) {
            [self connectToSSH];
        }
    });
}

- (void)appendText:(NSString*)string {
    
    
    [buffer appendString:string];
    
    //    NSLog(@"%@", string); //
    
    
}

- (NSString*)getBuffer {
    
    NSString *outputBuffer = [[NSString alloc] initWithString:buffer];
    [buffer setString:@""];
    
    return outputBuffer;
}


- (BOOL)windowShouldClose:(id)sender {
    
    dispatch_async(self.sshQueue, ^{
        
        if (session!=nil) {
            [session disconnect];
        }
    });
    
    return true;
    
}


- (IBAction)run:(id)sender {
    
    //    [self execScript:scriptEntry.comment];
}

- (void)connectToSSH {
    NSString *password = @"";
    
    if (self.core->runParam[0][0]==0) {
        host = @"localhost";
    } else {
        host = [[NSString alloc] initWithUTF8String:self.core->runParam[0]];
    }
    
    if (self.core->runParam[1][0]==0) {
        login = @"Miguel Vanhove";
    } else {
        login = [[NSString alloc] initWithUTF8String:self.core->runParam[1]];
    }
    
    if (self.core->runParam[2][0]!=0) {
        password = [[NSString alloc] initWithUTF8String:self.core->runParam[2]];
    } else {
        password = @"";
        
    }
    
    dispatch_async(self.sshQueue, ^{
        
        session = [NMSSHSession connectToHost:host withUsername:login];
        
        if (session.isConnected) {
            
            [session authenticateByPassword:password];
            
            if (!session.isAuthorized) {
                [session authenticateByKeyboardInteractiveUsingBlock:^NSString*(NSString *request) {
                    return password;
                }];
            }
            
            if (!session.isAuthorized) {
                [self appendText:@"ERROR: Not authorized on ssh server.\n"];
                [self quit];
                return;
            }
        } else {
            [self appendText:@"ERROR: Couldn't connect to ssh server.\n"];
            [self quit];
            return;
        }
        
        NSLog(@"Authentication succeeded");
        
        NSError *error;
        
        session.channel.delegate = self;
        
        [session.channel setRequestPty:YES];
        [session.channel setPtyTerminalType:NMSSHChannelPtyTerminalAnsi];
        [session.channel startShell:&error];
        
        int width=40;
        
        if (self.core->lastMode==2) {
            width=80;
        }
        
        int height=25;
        
        [session.channel requestSizeWidth:width height:height];
        
        NSLog(@"startShell");
        
        
    });
    
}

- (void)channel:(NMSSHChannel *)channel didReadData:(NSString *)message {
    
    //    NSLog(@"didReadData: %ld", [message length]);
    
    [self performSelectorOnMainThread:@selector(appendText:) withObject:message waitUntilDone:TRUE];
    
    //	    [self appendText:message];
    
}

- (void)channel:(NMSSHChannel *)channel didReadError:(NSString *)error {
    
    //    NSLog(@"didReadData: %ld", [error length]);
    
    [self performSelectorOnMainThread:@selector(appendText:) withObject:[NSString stringWithFormat:@"%@", error] waitUntilDone:TRUE];
    
    //	    [self appendText:message];
    
}

- (void)quit {
    [self.delegate appIsFinished:@""];
    
    
    [self performSelectorOnMainThread:@selector(appendText:) withObject:@"Disconnected..." waitUntilDone:FALSE];
    
}

- (void)channelShellDidClose:(NMSSHChannel *)channel {
    dispatch_async(dispatch_get_main_queue(), ^{
        
        [self quit];
        
        
    });
}


- (void)controlTextDidChange:(NSNotification *)aNotification {
    //  NSTextField *fieldValue = [aNotification object];
    
    //    [runButton setKeyEquivalent:@""];
    //    [runButton setNeedsDisplay:YES];
}

- (IBAction)sendBreak:(id)sender {
    
    dispatch_async(self.sshQueue, ^{
        
        NSError *error;
        [session.channel write:[NSString stringWithFormat:@"%c", 3] error:&error timeout:@5]; // 5 secondes
    });
}

- (void)execScript:(NSString*)cmd {
    
    //    NSMenuItem *menu = [list selectedItem];
    //    KdbEntry *selectedSSH = [menu representedObject];
    //
    //    for(NSInteger j=0; j<[entries count]; j++) {
    //
    //        KdbEntry *entry = entries[j];
    //
    //        for(NSInteger i=0; i<[entry getNumberOfCustomAttributes]; i++) {
    //
    //            if (j==0) {
    //                NSString *key = [entry getCustomAttributeName:i];
    //                NSString *value = [entry getCustomAttributeValue:key];
    //
    //                cmd = [cmd stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"{%@}", key] withString:value];
    //            }
    //
    //            NSString *key = [entry getCustomAttributeName:i];
    //            NSString *value = [entry getCustomAttributeValue:key];
    //
    //            cmd = [cmd stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"{%@:%ld}", key, j] withString:value];
    //        }
    //    }
    //
    //    if (selectedSSH != connectedSSH) { // Need to reconnect
    //
    //        if ((session!=nil) && (session.isConnected)) {
    //            [session disconnect];
    //        }
    //
    //        [self connectToSSH];
    //    }
    
    //    [self appendText:[NSString stringWithFormat:@"%@@%@:~$ %@\n", login, host, cmd]]; // Pseudo prompt
    
    
    dispatch_async(self.sshQueue, ^{
        
        if ((session.isConnected) && (session.isAuthorized)) {
            
            
            NSLog(@"run %@", cmd);
            
            NSError *error = nil;
            [session.channel write:[NSString stringWithFormat:@"%@\n", cmd] error:&error timeout:@5]; // 5 secondes
            
        }
    });
    
    
    //    [[self window] makeFirstResponder:resultField];
    
    
    // BOOL success = [session.channel uploadFile:@"~/index.html" to:@"/var/www/9muses.se/"];
}

- (NSString *)session:(NMSSHSession *)session keyboardInteractiveRequest:(NSString *)request {
    //    __block NSString *string;
    //    dispatch_sync(dispatch_get_main_queue(), ^() {
    //        string = [[FileTransferManager sharedInstance] transferrableFile:self
    //                                               keyboardInteractivePrompt:request];
    //    });
    //    return string;
    
        return @"";
}

@end
