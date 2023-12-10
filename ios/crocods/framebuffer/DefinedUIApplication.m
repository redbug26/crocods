//
//  DefinedUIApplication.m
//  crocods
//
//  Created by Miguel Vanhove on 24/03/2020.
//  Copyright © 2020 Miguel Vanhove. All rights reserved.
//

#import "DefinedUIApplication.h"

//#import "AppConfig.h"

@interface UIInternalEvent : UIEvent @end
@interface UIPhysicalButtonsEvent : UIInternalEvent @end

@interface UIPhysicalKeyboardEvent : UIPhysicalButtonsEvent
@property (nonatomic, readonly) BOOL _isKeyDown;
@property (nonatomic, readonly) long long _keyCode;
@end

@interface UIApplication ()
- (void)handleKeyUIEvent:(UIPhysicalKeyboardEvent *)event;
@end

@interface DefinedUIApplication ()
@property (retain, nonatomic) NSMutableArray *inputQueue;

@end

@implementation K_Event
@end

@implementation DefinedUIApplication

#pragma mark -

- (K_Event *)k_pollEvent {
    if ([self.inputQueue count] == 0) {
        return NULL;
    }

    K_Event *event = [self.inputQueue firstObject];

    [self.inputQueue removeObject:event];

    return event;
}

/*for ios 7 and above*/

// Arrow key doesn't work if full keyboard is set in accessibility :(

- (void)handleKeyUIEvent:(UIPhysicalKeyboardEvent *)event {
    [super handleKeyUIEvent:event];

    UIWindow *keyWindow = [[UIApplication sharedApplication] keyWindow];
    UIView *firstResponder = [keyWindow performSelector:@selector(firstResponder)];

    if (self.inputQueue == nil) {
        self.inputQueue = [[NSMutableArray alloc] initWithCapacity:5];
    }
//
//    NSLog(@"key char:%@ (%@)",event._unmodifiedInput, firstResponder);
//    NSLog(@"key char:%d (%d)",event._isKeyDown, event._keyCode);

    K_Event *kevent = [[K_Event alloc] init];
    kevent.isKeyDown = event._isKeyDown;
    kevent.keyCode = event._keyCode;

    [self.inputQueue addObject:kevent];

//    short keyCode=[Definition getKeyCodeByKeyCodeString:event._unmodifiedInput];
//     NSLog(@"key char:%@,short:%i",event._unmodifiedInput,keyCode);
//    NSDictionary *userInfo = [[NSDictionary alloc] initWithObjectsAndKeys:[NSNumber numberWithShort:keyCode], @"keycode", [NSNumber numberWithInt:0], @"eventFlags",[NSNumber numberWithBool:NO],@"shift",[NSNumber numberWithBool:NO],@"command",[NSNumber numberWithBool:NO],@"option",[NSNumber numberWithBool:NO],@"control", nil];
//    [[NSNotificationCenter defaultCenter] postNotificationName:UIEventGSEventKeyUpNotification object:nil userInfo:userInfo];
}

- (BOOL)isEditingText {
//#if (TARGET_IPHONE_SIMULATOR)
    // If you're writing text into a textfield, we shouldn't try run commands.
    UIWindow *keyWindow = [[UIApplication sharedApplication] keyWindow];
    UIView *firstResponder = [keyWindow performSelector:@selector(firstResponder)];
    if (firstResponder) return YES;
//#endif

    return NO;
}

@end
