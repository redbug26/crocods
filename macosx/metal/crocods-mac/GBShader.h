#import <Foundation/Foundation.h>

#include "plateform.h"

@interface GBShader : NSObject


@property core_crocods_t *gb;

- (instancetype)initWithName:(NSString *) shaderName andCore:(core_crocods_t*)core;
- (void) renderBitmap: (void *)bitmap previous:(void*) previous inSize:(NSSize)size scale: (double) scale;
@end
