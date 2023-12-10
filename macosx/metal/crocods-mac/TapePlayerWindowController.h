/**
 * @file
 * @author  Miguel Vanhove / Kyuran <crocods@kyuran.be>
 * @author  
 * @version 2.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * https://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * Tape Player
 */

#import <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>

@interface TapePlayerWindowController : NSWindowController <AVAudioPlayerDelegate> {
    NSTimer *nst;
}

@property (strong) AVAudioPlayer *mySound;
@property (strong) IBOutlet NSButton *buttonPlay;
@property (strong) IBOutlet NSButton *buttonStop;
@property (strong) IBOutlet NSSlider *positionSlider;


-(IBAction)playButton:(id)sender;
-(IBAction)stopButton:(id)sender;
-(IBAction)modifyPosition:(id)sender;

- (id)initWithURL:(NSURL*)url;

-(void)progression;

@end
