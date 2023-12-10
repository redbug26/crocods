//
//  GBKeyboard.m
//  crocods
//
//  Created by Miguel Vanhove on 06/03/2020.
//  Copyright © 2020 Miguel Vanhove. All rights reserved.
//

#import "GBKeyboard.h"

@interface GBKeyboard ()
{
    UIView *inputView;
}

@end

@implementation GBKeyboard

@synthesize active;

- (id)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    inputView = [[UIView alloc] initWithFrame:CGRectZero];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didEnterBackground) name:UIApplicationDidEnterBackgroundNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didBecomeActive) name:UIApplicationDidBecomeActiveNotification object:nil];

    return self;
}

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
    // Drawing code
}
*/

- (BOOL)canBecomeFirstResponder {
    return YES;
}

- (void)didEnterBackground {
    if (self.active) [self resignFirstResponder];
}

- (void)didBecomeActive {
    if (self.active) [self becomeFirstResponder];
}

- (void)setActive:(BOOL)value {
    if (active == value) return;

    active = value;
    if (active) {
        [self becomeFirstResponder];
    } else {
        [self resignFirstResponder];
    }
}

- (UIView *)inputView {
    return inputView;
}

- (BOOL)hasText {
    return NO;
}

- (void)insertText:(NSString *)text {
    char ch = [text characterAtIndex:0];

    [self.delegate characterDown:ch];

    /*
    if (_delegateFlags.characterDown) {
        [_delegate characterDown:ch];
    }

    char *p = strchr(ON_STATES, ch);
    bool stateChanged = false;
    if (p) {
        int index = p-ON_STATES;
        _iCadeState |= (1 << index);
        stateChanged = true;
        if (_delegateFlags.buttonDown) {
            [_delegate buttonDown:(1 << index)];
        }
    } else {
        p = strchr(OFF_STATES, ch);
        if (p) {
            int index = p-OFF_STATES;
            _iCadeState &= ~(1 << index);
            stateChanged = true;
            if (_delegateFlags.buttonUp) {
                [_delegate buttonUp:(1 << index)];
            }
        }
    }

    if (stateChanged && _delegateFlags.stateChanged) {
        [_delegate stateChanged:_iCadeState];
    }

    static int cycleResponder = 0;
    if (++cycleResponder > 20) {
        // necessary to clear a buffer that accumulates internally
        cycleResponder = 0;
        [self resignFirstResponder];
        [self becomeFirstResponder];
    }
     */
}

- (void)deleteBackward {
    [self.delegate characterDown:8];
}

- (NSArray *)keyCommands
{
    UIKeyCommand *upArrow = [UIKeyCommand keyCommandWithInput:UIKeyInputUpArrow modifierFlags:0 action:@selector(upArrow:)];
    UIKeyCommand *downArrow = [UIKeyCommand keyCommandWithInput:UIKeyInputDownArrow modifierFlags:0 action:@selector(downArrow:)];
    UIKeyCommand *leftArrow = [UIKeyCommand keyCommandWithInput:UIKeyInputLeftArrow modifierFlags:0 action:@selector(leftArrow:)];
    UIKeyCommand *rightArrow = [UIKeyCommand keyCommandWithInput:UIKeyInputRightArrow modifierFlags:0 action:@selector(rightArrow:)];
    return [[NSArray alloc] initWithObjects:upArrow, downArrow, leftArrow, rightArrow, nil];
}

/*
 { 28, CPC_CURSOR_DOWN, CPC_NIL } ,
 { 29, CPC_CURSOR_UP, CPC_NIL } ,
 { 30, CPC_CURSOR_LEFT, CPC_NIL } ,
 { 31, CPC_CURSOR_RIGHT, CPC_NIL } ,

 */

- (void)upArrow:(UIKeyCommand *)keyCommand
{
    [self.delegate characterDown:29];
}

- (void)downArrow:(UIKeyCommand *)keyCommand
{
    [self.delegate characterDown:28];
}

- (void)leftArrow:(UIKeyCommand *)keyCommand
{
    [self.delegate characterDown:30];
}

- (void)rightArrow:(UIKeyCommand *)keyCommand
{
    [self.delegate characterDown:31];
}

@end
