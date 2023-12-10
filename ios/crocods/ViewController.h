//
//  ViewController.h
//  crocods
//
//  Created by Miguel Vanhove on 04/03/2020.
//  Copyright © 2020 Miguel Vanhove. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface ViewController : UIViewController

- (void)loadData:(NSData*)data withAutorun:(BOOL)autorun andFilename:(NSString*)filename;
- (void)loadFilename:(NSString*)filename withAutorun:(BOOL)autorun;
+ (ViewController*)sharedInstance;

@end

