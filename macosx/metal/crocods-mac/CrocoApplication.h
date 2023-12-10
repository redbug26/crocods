//
//  CrocoApplication.h
//  CrocoDS
//
//  Created by Miguel Vanhove on 1/10/16.
//  Copyright © 2016 Miguel Vanhove. All rights reserved.
//

#import <Cocoa/Cocoa.h>

struct core_crocods_s;
typedef struct core_crocods_s core_crocods_t;


@protocol CrocoApplicationDelegate <NSObject>

@optional

- (void)appIsFinished:(NSString *)message;

@end

@interface CrocoApplication : NSObject {
    
}

@property (nonatomic, weak) id<CrocoApplicationDelegate> delegate;
@property (assign) core_crocods_t *core;

- (NSString*)getBuffer;
- (void)scriptConsoleTextViewKeyDown:(NSEvent *)theEvent;
- (void)start;
- (void)quit;

@end
