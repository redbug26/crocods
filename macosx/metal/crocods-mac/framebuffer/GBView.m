#import <UIKit/UIKit.h>

#import "GBView.h"
#import "GBViewMetal.h"

#import <GameController/GameController.h>

#define JOYSTICK_HIGH 0x4000
#define JOYSTICK_LOW  0x3800

@implementation GBView
{
    uint32_t *image_buffers[3];
    unsigned char current_buffer;
    bool mouse_hidden;
//    NSTrackingArea *tracking_area;
//    bool _mouseHidingEnabled;     // TODO
    bool axisActive[2];
    bool underclockKeyDown;
    double clockMultiplier;
//    NSEventModifierFlags previousModifiers;

    int width, height;

    void (^ keyHandler)(BOOL, enum FB_KEY_BITS);

    GCController *myController;

// -- touch button

    UIView *m_oglView;      // View that contains the key button

    UIImageView *key_up;
    UIImageView *key_down;
    UIImageView *key_left;
    UIImageView *key_right;
    UIImageView *key_a;
    UIImageView *key_b;
    UIImageView *key_x;
    UIImageView *key_y;
    UIImageView *key_r;
    UIImageView *key_l;
    UIImageView *key_select;
    UIImageView *key_start;

    CGPoint centerLocation;
    unsigned int ipc_keys_pressed;
}

@synthesize current_buffer, width, height;

+ (instancetype)alloc
{
    return [self allocWithZone:NULL];
}

+ (instancetype)allocWithZone:(struct _NSZone *)zone
{
    if (self == [GBView class]) {
        if ([GBViewMetal isSupported]) {
            return [GBViewMetal allocWithZone:zone];
        }
        return nil;
    }
    return [super allocWithZone:zone];
}

- (void)createInternalView
{
    assert(false && "createInternalView must not be inherited");
}

- (void)_init
{
    // TODO: update width
    width = 384;
    height = 272;

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(ratioKeepingChanged) name:@"GBAspectChanged" object:nil];
//    tracking_area = [ [NSTrackingArea alloc] initWithRect:(NSRect) {}
//                                                  options:NSTrackingMouseEnteredAndExited | NSTrackingActiveAlways | NSTrackingInVisibleRect
//                                                    owner:self
//                                                 userInfo:nil];
//    [self addTrackingArea:tracking_area];
    clockMultiplier = 1.0;
    [self createInternalView];
    [self addSubview:self.internalView];

//    self.internalView.frame = self.frame;

    NSLog(@"frame: %f,%f %fx%f", self.frame.origin.x, self.frame.origin.y, self.frame.size.width, self.frame.size.height);

#if TARGET_OS_IOS
    self.internalView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
#elif TARGET_OS_OSX
    self.internalView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
#endif

    [self screenSizeChanged];

    [self setupControllers:self];
}

- (void)layoutSubviews {
    [super layoutSubviews];

    // init your parameters here, like set up fonts, colors, etc...

    [self touchButtonInit];
}

- (void)resize:(CGRect)frame {
    self.frame = frame;
    self.internalView.frame = frame;
}

- (void)changeSizeWithWidth:(int)width0 andHeight:(int)height0 {
    width = width0;
    height = height0;

    [self screenSizeChanged];
}

- (void)screenSizeChanged
{
    if (image_buffers[0]) free(image_buffers[0]);
    if (image_buffers[1]) free(image_buffers[1]);
    if (image_buffers[2]) free(image_buffers[2]);

    size_t buffer_size = sizeof(image_buffers[0][0]) * width * height;

    printf("screenSizeChanged: %dx%d %lu\n", width, height, buffer_size);

    image_buffers[0] = malloc(buffer_size);
    image_buffers[1] = malloc(buffer_size);
    image_buffers[2] = malloc(buffer_size);

    NSInteger x, y, pos;

    if (1 == 0) {
        pos = 0;
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
//            NSLog(@"%d", ((rand() % 255) << 24) | ((rand() % 255) << 16) | ((rand() % 255) << 8) | ((rand() % 255) << 0));

                if ((x != 5) && (y != 5) && (x != 10) && (y != 10) && (x != 118) && (y != 54)) {
                    uint32_t *buf;

                    buf = image_buffers[0];
                    buf[pos] = ((rand() % 255) << 24) | ((rand() % 255) << 16) | ((rand() % 255) << 8) | ((rand() % 255) << 0);

                    buf = image_buffers[1];
                    buf[pos] = ((rand() % 255) << 24) | ((rand() % 255) << 16) | ((rand() % 255) << 8) | ((rand() % 255) << 0);

                    buf = image_buffers[2];
                    buf[pos] = ((rand() % 255) << 24) | ((rand() % 255) << 16) | ((rand() % 255) << 8) | ((rand() % 255) << 0);
                }
                pos++;
            }
        }
    }

    dispatch_async(dispatch_get_main_queue(), ^{
        [self setFrame:self.superview.frame];
    });
}

- (void)ratioKeepingChanged
{
    [self setFrame:self.superview.frame];
}

- (unsigned char)numberOfBuffers
{
    return 2;
}

- (void)dealloc
{
    free(image_buffers[0]);
    free(image_buffers[1]);
    free(image_buffers[2]);

    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    if (!(self = [super initWithCoder:coder])) {
        return self;
    }
    [self _init];
    return self;
}

- (instancetype)initWithFrame:(CGRect)frameRect
{
    if (!(self = [super initWithFrame:frameRect])) {
        return self;
    }

//    self.frame = frameRect;

    [self _init];
    return self;
}

- (void)setFrame:(CGRect)frame
{
    frame = self.superview.frame;

    /*
    if (_gb && ![[NSUserDefaults standardUserDefaults] boolForKey:@"GBAspectRatioUnkept"]) {
        double ratio = frame.size.width / frame.size.height;
        double width = GB_get_screen_width(_gb);
        double height = GB_get_screen_height(_gb);
        if (ratio >= width / height) {
            double new_width = round(frame.size.height / height * width);
            frame.origin.x = floor((frame.size.width - new_width) / 2);
            frame.size.width = new_width;
            frame.origin.y = 0;
        } else {
            double new_height = round(frame.size.width / width * height);
            frame.origin.y = floor((frame.size.height - new_height) / 2);
            frame.size.height = new_height;
            frame.origin.x = 0;
        }
    }
     */

    [super setFrame:frame];
}

- (void)flip
{
//    if (underclockKeyDown && clockMultiplier > 0.5) {
//        clockMultiplier -= 1.0 / 16;
//        GB_set_clock_multiplier(_gb, clockMultiplier);
//    }
//    if (!underclockKeyDown && clockMultiplier < 1.0) {
//        clockMultiplier += 1.0 / 16;
//        GB_set_clock_multiplier(_gb, clockMultiplier);
//    }
    current_buffer = (current_buffer + 1) % self.numberOfBuffers;
}

- (uint32_t *)pixels
{
    return image_buffers[(current_buffer + 1) % self.numberOfBuffers];
}

/*
 - (void)flagsChanged:(NSEvent *)event
{
    if (event.modifierFlags > previousModifiers) {
        [self keyDown:event];
    } else {
        [self keyUp:event];
    }

    previousModifiers = event.modifierFlags;
}
 */

- (uint32_t *)currentBuffer
{
    return image_buffers[current_buffer];
}

- (uint32_t *)previousBuffer
{
    return image_buffers[(current_buffer + 2) % self.numberOfBuffers];
}

#pragma mark - Key

#define KEY(x) ({ unichar __x = x; [NSString stringWithCharacters:&(__x) length:1]; })

#if TARGET_OS_IOS

#elif TARGET_OS_OSX
- (BOOL)becomeFirstResponder
{
    /* Non-Roman keyboard layouts breaks user input. */
    TSMDocumentID document = TSMGetActiveDocument();

    CFArrayRef inpu_sources = TISCreateASCIICapableInputSourceList();
    TSMSetDocumentProperty(document, kTSMDocumentEnabledInputSourcesPropertyTag,
                           sizeof(CFArrayRef), &inpu_sources);
    CFRelease(inpu_sources);

    return [super becomeFirstResponder];
}

- (BOOL)resignFirstResponder
{
    TSMDocumentID document = TSMGetActiveDocument();
    TSMRemoveDocumentProperty(document, kTSMDocumentEnabledInputSourcesPropertyTag);

    return [super resignFirstResponder];
}

#endif

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)setKeyHandler:(void (^)(BOOL, enum FB_KEY_BITS))completionBlock {
    keyHandler = completionBlock;
}

#if TARGET_OS_OSX

- (void)keyDown:(NSEvent *)theEvent
{
    if ([theEvent modifierFlags] & NSEventModifierFlagCommand) {
        return [super keyUp:theEvent];
    }

    NSString *key = theEvent.charactersIgnoringModifiers;

    [self useKey:key pressed:true];
}

- (void)keyUp:(NSEvent *)theEvent
{
    if ([theEvent modifierFlags] & NSEventModifierFlagCommand) {
        return [super keyUp:theEvent];
    }

    NSString *key = theEvent.charactersIgnoringModifiers;

    [self useKey:key pressed:false];
}

- (void)useKey:(NSString *)key pressed:(BOOL)pressed {
    if ([key isEqualToString:@"\x7f"]) {
        keyHandler(pressed, FB_KEY_R);
    }
    if ([key isEqualToString:@"\t"]) {
        keyHandler(pressed, FB_KEY_L);
    }

    if ([key isEqualToString:KEY(NSRightArrowFunctionKey)]) {
        keyHandler(pressed, FB_KEY_RIGHT);
    }
    if ([key isEqualToString:KEY(NSUpArrowFunctionKey)]) {
        keyHandler(pressed, FB_KEY_UP);
    }
    if ([key isEqualToString:KEY(NSDownArrowFunctionKey)]) {
        keyHandler(pressed, FB_KEY_DOWN);
    }
    if ([key isEqualToString:KEY(NSLeftArrowFunctionKey)]) {
        keyHandler(pressed, FB_KEY_LEFT);
    }
    if ([key isEqualToString:@"x"]) {
        keyHandler(pressed, FB_KEY_A);
    }
    if ([key isEqualToString:@"z"]) {
        keyHandler(pressed, FB_KEY_B);
    }
    if ([key isEqualToString:@"\r"]) {
        keyHandler(pressed, FB_KEY_START);
    }
    if ([key isEqualToString:KEY(NSF12FunctionKey)]) {
        keyHandler(pressed, FB_KEY_SELECT);
    }
}

#endif

#pragma mark - Gamecontroller support

- (void)setupControllers:(id)sender {
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];

    NSLog(@"center: %@", center);

    [center addObserver:self
               selector:@selector(controllerDidConnectNotification:)
                   name:GCControllerDidConnectNotification
                 object:nil];

    [center addObserver:self
               selector:@selector(controllerDidConnectNotification:)
                   name:GCControllerDidDisconnectNotification
                 object:nil];

    [GCController startWirelessControllerDiscoveryWithCompletionHandler:^(void) {
        NSLog(@"end of startWirelessControllerDiscoveryWithCompletionHandler");
        [self controllerDidConnectNotification:self];
    }];

    [self controllerDidConnectNotification:self];
}

- (void)controllerDidConnectNotification:(id)sender {
    NSLog(@"controllers: %@", [GCController controllers]);

    for (GCController *controller in [GCController controllers]) {
        myController = controller;

        NSInteger playerIndex = controller.playerIndex;
        if (playerIndex != GCControllerPlayerIndexUnset) {
            continue;
        }

        myController.extendedGamepad.dpad.left.valueChangedHandler =  ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            if (button == self->myController.extendedGamepad.dpad.left) {
                self->keyHandler(pressed, FB_KEY_LEFT);
            }
        };
        myController.extendedGamepad.dpad.right.valueChangedHandler =  ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            if (button == self->myController.extendedGamepad.dpad.right) {
                self->keyHandler(pressed, FB_KEY_RIGHT);
            }
        };
        myController.extendedGamepad.dpad.up.valueChangedHandler =  ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            self->keyHandler(pressed, FB_KEY_UP);
        };
        myController.extendedGamepad.dpad.down.valueChangedHandler =  ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            self->keyHandler(pressed, FB_KEY_DOWN);
        };
        myController.extendedGamepad.buttonA.valueChangedHandler =  ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            self->keyHandler(pressed, FB_KEY_A);
        };
        myController.extendedGamepad.buttonB.valueChangedHandler =  ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            self->keyHandler(pressed, FB_KEY_B);
        };
        myController.extendedGamepad.buttonX.valueChangedHandler =  ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            self->keyHandler(pressed, FB_KEY_X);
        };
        myController.extendedGamepad.buttonY.valueChangedHandler =  ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            self->keyHandler(pressed, FB_KEY_Y);
        };
        myController.extendedGamepad.leftShoulder.valueChangedHandler =  ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            self->keyHandler(pressed, FB_KEY_L);
        };
        myController.extendedGamepad.rightShoulder.valueChangedHandler =  ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            self->keyHandler(pressed, FB_KEY_R);
        };
        myController.extendedGamepad.rightShoulder.valueChangedHandler =  ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            self->keyHandler(pressed, FB_KEY_R);
        };

        myController.extendedGamepad.buttonMenu.valueChangedHandler =  ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            self->keyHandler(pressed, FB_KEY_SELECT);
        };

        NSLog(@"controller found");
    }
}

#pragma mark - Init touch button

#define RAYON 20

#define     BIT(n) (1 << (n))

enum      KEYPAD_BITS {
    KEY_A     = BIT(0), KEY_B = BIT(1), KEY_SELECT = BIT(2), KEY_START = BIT(3),
    KEY_RIGHT = BIT(4), KEY_LEFT = BIT(5), KEY_UP = BIT(6), KEY_DOWN = BIT(7),
    KEY_R     = BIT(8), KEY_L = BIT(9), KEY_X = BIT(10), KEY_Y = BIT(11),
    KEY_TOUCH = BIT(12), KEY_LID = BIT(13)
};

- (void)touchButtonInit {
    int w, h;

    m_oglView = self.internalView;

    for (UIView *view in m_oglView.subviews) {
        if ([view isKindOfClass:[UIImageView class]]) {
            [view removeFromSuperview];
        }
    }

    NSLog(@"view size: %fx%f", m_oglView.frame.size.width, m_oglView.frame.size.height);

    w = m_oglView.frame.size.width / 6;
    h = m_oglView.frame.size.height / 4;

    UIImage *key_up_img = [UIImage imageNamed:@"key_up.png"];

    key_up = [[UIImageView alloc] initWithImage:key_up_img];
    key_up.frame = CGRectMake(w * 1,  h * 1, 48, 48);
    key_up.autoresizingMask = UIViewAutoresizingFlexibleTopMargin;
    [m_oglView addSubview:key_up];

    key_down = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"key_down.png"]];
    key_down.frame = CGRectMake(w * 1,  h * 3, 48, 48);
    key_down.autoresizingMask = UIViewAutoresizingFlexibleTopMargin;
    [m_oglView addSubview:key_down];

    key_left = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"key_left.png"]];
    key_left.frame = CGRectMake(w * 0,  h * 2, 48, 48);
    key_left.autoresizingMask = UIViewAutoresizingFlexibleTopMargin;
    [m_oglView addSubview:key_left];

    key_right = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"key_right.png"]];
    key_right.frame = CGRectMake(w * 2,  h * 2, 48, 48);
    key_right.autoresizingMask = UIViewAutoresizingFlexibleTopMargin;
    [m_oglView addSubview:key_right];

    centerLocation = CGPointMake(w * 1,  h * 3);
    key_left.center = CGPointMake(centerLocation.x - RAYON * 2, centerLocation.y);
    key_right.center = CGPointMake(centerLocation.x + RAYON * 2, centerLocation.y);
    key_up.center = CGPointMake(centerLocation.x, centerLocation.y - RAYON * 2);
    key_down.center = CGPointMake(centerLocation.x, centerLocation.y + RAYON * 2);

    key_a = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"key_a.png"]];
    key_a.frame = CGRectMake(w * 4,  h * 3, 48, 48);
    key_a.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleTopMargin;
    [m_oglView addSubview:key_a];

    key_b = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"key_b.png"]];
    key_b.frame = CGRectMake(w * 5,  h * 3, 48, 48);
    key_b.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleTopMargin;
    [m_oglView addSubview:key_b];

    key_select = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"key_select.png"]];
    key_select.frame = CGRectMake(w * 4,  h * 1, 48, 48);
    key_select.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleBottomMargin;
    [m_oglView addSubview:key_select];

    key_start = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"key_start.png"]];
    key_start.frame = CGRectMake(w * 5,  h * 1, 48, 48);
    key_start.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleBottomMargin;
    [m_oglView addSubview:key_start];

    key_l = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"key_l.png"]];
    key_l.frame = CGRectMake(w * 1,  h * 0, 48, 48);
    key_l.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleTopMargin;
    [m_oglView addSubview:key_l];

    key_r = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"key_r.png"]];
    key_r.frame = CGRectMake(w * 5,  h * 0, 48, 48);
    key_r.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleTopMargin;
    [m_oglView addSubview:key_r];

    key_x = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"key_x.png"]];
    key_x.frame = CGRectMake(w * 4,  h * 2, 48, 48);
    key_x.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleTopMargin;
    [m_oglView addSubview:key_x];

    key_y = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"key_y.png"]];
    key_y.frame = CGRectMake(w * 5,  h * 2, 48, 48);
    key_y.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleTopMargin;
    [m_oglView addSubview:key_y];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    UITouch *touch = [[touches allObjects] objectAtIndex:0];

    CGPoint location = [touch locationInView:m_oglView];
    if (location.y < m_oglView.frame.size.height) {       // Click dans l'ecran
        UIView *viewEffectX, *viewEffectY;

        if (location.x < m_oglView.frame.size.width / 2) {
            if (location.x - centerLocation.x > RAYON) {
                ipc_keys_pressed |= KEY_RIGHT;
                viewEffectX = key_right;
                keyHandler(1, FB_KEY_RIGHT);
            } else if (centerLocation.x - location.x > RAYON) {
                ipc_keys_pressed |= KEY_LEFT;
                keyHandler(1, FB_KEY_LEFT);
                viewEffectX = key_left;
            } else {
                ipc_keys_pressed &= (~KEY_LEFT);
                ipc_keys_pressed &= (~KEY_RIGHT);
                keyHandler(0, FB_KEY_RIGHT);
                keyHandler(0, FB_KEY_LEFT);
                viewEffectX = nil;
                [key_right setAlpha:(1.0)];
                [key_left setAlpha:(1.0)];
            }
            if (viewEffectX != nil) {
                [UIView beginAnimations:@"" context:NULL];
                [viewEffectX setAlpha:(0.0)];
                [UIView setAnimationDuration:0.5];
                [UIView commitAnimations];
            }
            if (location.y - centerLocation.y > RAYON) {
                ipc_keys_pressed |= KEY_DOWN;

                keyHandler(1, FB_KEY_DOWN);
                viewEffectY = key_down;
            } else if (centerLocation.y - location.y > RAYON) {
                ipc_keys_pressed |= KEY_UP;

                keyHandler(1, FB_KEY_UP);
                viewEffectY = key_up;
            } else {
                ipc_keys_pressed &= (~KEY_UP);
                ipc_keys_pressed &= (~KEY_DOWN);

                keyHandler(0, FB_KEY_DOWN);
                keyHandler(0, FB_KEY_UP);
                viewEffectY = nil;
                [key_up setAlpha:(1.0)];
                [key_down setAlpha:(1.0)];
            }
            if (viewEffectY != nil) {
                [UIView beginAnimations:@"" context:NULL];
                [viewEffectY setAlpha:(0.0)];
                [UIView setAnimationDuration:0.5];
                [UIView commitAnimations];
            }
        }
    }
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    UITouch *touch = [[touches allObjects] objectAtIndex:0];

    CGPoint location = [touch locationInView:m_oglView];
    if (location.y < m_oglView.frame.size.height) {       // Click dans l'ecran
        UIView *viewEffect = nil;

        if (CGRectContainsPoint(key_left.frame, location)) {
            ipc_keys_pressed |= KEY_LEFT;

            keyHandler(1, FB_KEY_LEFT);
            viewEffect = key_left;
        } else if (CGRectContainsPoint(key_right.frame, location)) {
            ipc_keys_pressed |= KEY_RIGHT;

            keyHandler(1, FB_KEY_RIGHT);
            viewEffect = key_right;
        } else if (CGRectContainsPoint(key_up.frame, location)) {
            ipc_keys_pressed |= KEY_UP;

            keyHandler(1, FB_KEY_UP);
            viewEffect = key_up;
        } else if (CGRectContainsPoint(key_down.frame, location)) {
            ipc_keys_pressed |= KEY_DOWN;

            keyHandler(1, FB_KEY_DOWN);
            viewEffect = key_down;
        } else if (CGRectContainsPoint(key_a.frame, location)) {
            ipc_keys_pressed |= KEY_A;

            keyHandler(1, FB_KEY_A);
            viewEffect = key_a;
        } else if (CGRectContainsPoint(key_b.frame, location)) {
            ipc_keys_pressed |= KEY_B;

            keyHandler(1, FB_KEY_B);
            viewEffect = key_b;
        } else if (CGRectContainsPoint(key_x.frame, location)) {
            ipc_keys_pressed |= KEY_X;

            keyHandler(1, FB_KEY_X);
            viewEffect = key_x;
        } else if (CGRectContainsPoint(key_y.frame, location)) {
            ipc_keys_pressed |= KEY_Y;

            keyHandler(1, FB_KEY_Y);
            viewEffect = key_y;
        } else if (CGRectContainsPoint(key_l.frame, location)) {
            ipc_keys_pressed |= KEY_L;

            keyHandler(1, FB_KEY_L);
            viewEffect = key_l;
        } else if (CGRectContainsPoint(key_r.frame, location)) {
            ipc_keys_pressed |= KEY_R;

            keyHandler(1, FB_KEY_R);
            viewEffect = key_r;
        } else if (CGRectContainsPoint(key_select.frame, location)) {
            ipc_keys_pressed |= KEY_SELECT;

            keyHandler(1, FB_KEY_SELECT);
            viewEffect = key_select;
        } else if (CGRectContainsPoint(key_start.frame, location)) {
            ipc_keys_pressed |= KEY_START;

            keyHandler(1, FB_KEY_START);
            viewEffect = key_start;
        } else if (location.x < m_oglView.frame.size.width / 2) {
            centerLocation = location;
            key_left.center = CGPointMake(centerLocation.x - RAYON * 2, centerLocation.y);
            key_right.center = CGPointMake(centerLocation.x + RAYON * 2, centerLocation.y);
            key_up.center = CGPointMake(centerLocation.x, centerLocation.y - RAYON * 2);
            key_down.center = CGPointMake(centerLocation.x, centerLocation.y + RAYON * 2);
        }

        if (viewEffect != nil) {
            [UIView beginAnimations:@"" context:NULL];
            [viewEffect setAlpha:(0.0)];
            [UIView setAnimationDuration:0.5];
            [UIView commitAnimations];
        }

        return;
    }

//    location = [touch locationInView:mykeyboard];
//
//    int x,y;
//
//    x=(location.x*256)/mykeyboard.frame.size.width;
//    y=(location.y*192)/mykeyboard.frame.size.height;
//
//    //    NSLog(@"location: %f,%f", location.x, location.y);
//
//    if (y<64) {
//        [self showSettingsView];
//        /*
//         if (location.x<160) {
//         char autofile[256];
//
//         NSData *dsk=[[NSData alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"bombjack" ofType:@"dsk"]];
//         LireDiskMem((u8*)[dsk bytes], [dsk length], autofile);
//
//         if (autofile[0]!=0) {
//         char buffer[256];
//         sprintf(buffer,"run\"%s\n", autofile);
//         AutoType_SetString(buffer, TRUE);
//         }
//         } else {
//         NSData *rom=[[NSData alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"boulder" ofType:@"sna"]];
//         LireSnapshotMem((u8*)[rom bytes]);
//         }
//         */
//        return;
//    }
//
//
//    ipc.touchXpx=x;
//    ipc.touchYpx=y;
//    ipc.touchDown=1;
//
//    NSLog(@"keyboard: %d,%d", ipc.touchXpx, ipc.touchYpx);
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    UITouch *touch = [[touches allObjects] objectAtIndex:0];

    CGPoint location = [touch locationInView:m_oglView];

    if (ipc_keys_pressed != 0) {
        UIView *viewEffect = nil;

        if ((ipc_keys_pressed | KEY_LEFT) == KEY_LEFT) {
            viewEffect = key_left;
            keyHandler(0, FB_KEY_LEFT);
        } else if ((ipc_keys_pressed | KEY_RIGHT) == KEY_RIGHT) {
            viewEffect = key_right;
            keyHandler(0, FB_KEY_RIGHT);
        } else if ((ipc_keys_pressed | KEY_UP) == KEY_UP) {
            viewEffect = key_up;
            keyHandler(0, FB_KEY_UP);
        } else if ((ipc_keys_pressed | KEY_DOWN) == KEY_DOWN) {
            viewEffect = key_down;
            keyHandler(0, FB_KEY_DOWN);
        } else if ((ipc_keys_pressed | KEY_A) == KEY_A) {
            viewEffect = key_a;
            keyHandler(0, FB_KEY_A);
        } else if ((ipc_keys_pressed | KEY_B) == KEY_B) {
            viewEffect = key_b;
            keyHandler(0, FB_KEY_B);
        } else if ((ipc_keys_pressed | KEY_X) == KEY_X) {
            viewEffect = key_x;
            keyHandler(0, FB_KEY_X);
        } else if ((ipc_keys_pressed | KEY_Y) == KEY_Y) {
            viewEffect = key_y;
            keyHandler(0, FB_KEY_Y);
        } else if ((ipc_keys_pressed | KEY_L) == KEY_L) {
            viewEffect = key_l;
            keyHandler(0, FB_KEY_L);
        } else if ((ipc_keys_pressed | KEY_R) == KEY_R) {
            viewEffect = key_r;
            keyHandler(0, FB_KEY_R);
        } else if ((ipc_keys_pressed | KEY_SELECT) == KEY_SELECT) {
            viewEffect = key_select;
            keyHandler(0, FB_KEY_SELECT);
        } else if ((ipc_keys_pressed | KEY_START) == KEY_START) {
            viewEffect = key_start;
            keyHandler(0, FB_KEY_START);
        }

        if (viewEffect != nil) {
            [UIView beginAnimations:@"" context:NULL];
            [viewEffect setAlpha:(1.0)];
            [UIView setAnimationDuration:0.5];
            [UIView commitAnimations];
        }
    }

    [key_up setAlpha:(1.0)];
    [key_down setAlpha:(1.0)];
    [key_left setAlpha:(1.0)];
    [key_right setAlpha:(1.0)];

    ipc_keys_pressed = 0;

    if (location.y < m_oglView.frame.size.height) {       // Click dans l'ecran
        return;
    }
}

@end
