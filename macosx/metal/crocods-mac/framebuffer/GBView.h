

#if TARGET_OS_IOS
#import <UIKit/UIKit.h>

@interface GBView : UIView
#elif TARGET_OS_OSX
@interface GBView : NSView
#endif

- (void) flip;
- (uint32_t *_Nullable) pixels;

#define FB_BIT(n)   (1 << (n))

enum FB_KEY_BITS {
    FB_KEY_A = FB_BIT(0),
    FB_KEY_B = FB_BIT(1),
    FB_KEY_SELECT = FB_BIT(2),
    FB_KEY_START = FB_BIT(3),
    FB_KEY_RIGHT = FB_BIT(4),
    FB_KEY_LEFT = FB_BIT(5),
    FB_KEY_UP = FB_BIT(6),
    FB_KEY_DOWN = FB_BIT(7),
    FB_KEY_R = FB_BIT(8),
    FB_KEY_L = FB_BIT(9),
    FB_KEY_X = FB_BIT(10),
    FB_KEY_Y = FB_BIT(11),
    FB_KEY_TOUCH = FB_BIT(12),
    FB_KEY_LID = FB_BIT(13)
};

@property (nonatomic, assign) bool isPaused;

@property (getter=isMouseHidingEnabled) BOOL mouseHidingEnabled;
@property bool isRewinding;
@property unsigned char current_buffer;

#if TARGET_OS_IOS
@property UIView * _Nullable internalView;
#elif TARGET_OS_OSX
@property NSView * _Nullable internalView;
#endif


- (void) createInternalView;
- (uint32_t *_Nullable)currentBuffer;
- (uint32_t *_Nullable)previousBuffer;
- (void)screenSizeChanged;
-(void)resize:(CGRect)frame;

@property int width, height;



- (void)changeSizeWithWidth:(int)width andHeight:(int)height;

- (void)setKeyHandler:(void (^ __nullable)(BOOL pressed, enum FB_KEY_BITS finished))completionBlock;


@end
