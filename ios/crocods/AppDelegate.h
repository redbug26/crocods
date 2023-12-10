//
//  AppDelegate.h
//  crocods
//
//  Created by Miguel Vanhove on 04/03/2020.
//  Copyright © 2020 Miguel Vanhove. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) NSData *dataToLoad;
@property (strong, nonatomic) NSString *fileToLoad;

+ (AppDelegate *)sharedSession;

@end

