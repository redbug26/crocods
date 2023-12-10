//
//  main.m
//  crocods
//
//  Created by Miguel Vanhove on 04/03/2020.
//  Copyright © 2020 Miguel Vanhove. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#import "DefinedUIApplication.h"

int main(int argc, char * argv[]) {
    NSString * appDelegateClassName;
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        appDelegateClassName = NSStringFromClass([AppDelegate class]);
    }
    return UIApplicationMain(argc, argv, NSStringFromClass([DefinedUIApplication class]), appDelegateClassName);
}
