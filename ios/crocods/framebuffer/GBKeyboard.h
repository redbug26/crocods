//
//  GBKeyboard.h
//  crocods
//
//  Created by Miguel Vanhove on 06/03/2020.
//  Copyright © 2020 Miguel Vanhove. All rights reserved.
//

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@protocol GBKeyboardDelegate <NSObject>

@optional
- (void)characterDown:(char)ch;
@end

@interface GBKeyboard : UIView <UIKeyInput>

@property (nonatomic, assign) BOOL active;
@property (nonatomic, weak) id<GBKeyboardDelegate> delegate;

@end

NS_ASSUME_NONNULL_END
