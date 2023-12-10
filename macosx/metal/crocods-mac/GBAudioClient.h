#import <Foundation/Foundation.h>



@interface GBAudioClient : NSObject
@property (strong) void (^renderBlock)(UInt32 sampleRate, UInt32 nFrames, void *buffer);
@property (readonly) UInt32 rate;
@property (readonly, getter=isPlaying) bool playing;
-(void) start;
-(void) stop;
-(id) initWithRendererBlock:(void (^)(UInt32 sampleRate, UInt32 nFrames, void *buffer)) block
              andSampleRate:(UInt32) rate;
@end
