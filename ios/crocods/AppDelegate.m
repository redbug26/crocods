//
//  AppDelegate.m
//  crocods
//
//  Created by Miguel Vanhove on 04/03/2020.
//  Copyright © 2020 Miguel Vanhove. All rights reserved.
//

#import "AppDelegate.h"
#import "ViewController.h"

//@interface UIInternalEvent : UIEvent @end
//@interface UIPhysicalButtonsEvent : UIInternalEvent @end
//
//@interface UIPhysicalKeyboardEvent : UIPhysicalButtonsEvent
//@property (retain, nonatomic) NSString *_unmodifiedInput;
//@end
//
//@interface UIResponder ()
//- (void)handleKeyUIEvent:(UIPhysicalKeyboardEvent *)event;
//@end

@interface AppDelegate ()

@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    return YES;
}

- (BOOL)application:(UIApplication *)app openURL:(NSURL *)url options:(NSDictionary<UIApplicationOpenURLOptionsKey,id> *)options {
    
    
    
    NSLog(@"load %@", url);
    
    
    return YES;
}

+ (AppDelegate *)sharedSession
{
    return (AppDelegate *)[[UIApplication sharedApplication] delegate];
}


#pragma mark - UISceneSession lifecycle

- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)connectingSceneSession options:(UISceneConnectionOptions *)options {
    // Called when a new scene session is being created.
    // Use this method to select a configuration to create the new scene with.
    return [[UISceneConfiguration alloc] initWithName:@"Default Configuration" sessionRole:connectingSceneSession.role];
}

- (void)application:(UIApplication *)application didDiscardSceneSessions:(NSSet<UISceneSession *> *)sceneSessions {
    // Called when the user discards a scene session.
    // If any sessions were discarded while the application was not running, this will be called shortly after application:didFinishLaunchingWithOptions.
    // Use this method to release any resources that were specific to the discarded scenes, as they will not return.
}

@end
