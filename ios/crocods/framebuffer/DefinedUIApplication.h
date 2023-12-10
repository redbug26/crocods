//
//  DefinedUIApplication.h
//  crocods
//
//  Created by Miguel Vanhove on 24/03/2020.
//  Copyright © 2020 Miguel Vanhove. All rights reserved.
//

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface K_Event : NSObject
    
@property (nonatomic) BOOL isKeyDown;
@property (nonatomic) long long keyCode;

@end


@interface DefinedUIApplication : UIApplication

- (K_Event*)k_pollEvent;


@end

NS_ASSUME_NONNULL_END
