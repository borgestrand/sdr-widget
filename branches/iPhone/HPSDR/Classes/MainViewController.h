//
//  MainViewController.h
//  HPSDR
//
//  Created by John Melton on 07/07/2009.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import "FlipsideViewController.h"

@interface MainViewController : UIViewController <FlipsideViewControllerDelegate> {
	
	NSTimer *timer;
	
	IBOutlet UIButton *button160;
	IBOutlet UIButton *button80;
	IBOutlet UIButton *button60;
	IBOutlet UIButton *button40;
	IBOutlet UIButton *button30;
	IBOutlet UIButton *button20;
	IBOutlet UIButton *button17;
	IBOutlet UIButton *button15;
	IBOutlet UIButton *button12;
	IBOutlet UIButton *button10;
	IBOutlet UIButton *button6;
	IBOutlet UIButton *buttonGen;
	IBOutlet UIButton *buttonWWV;

	
}

- (IBAction)showInfo;
- (IBAction)band160;
- (IBAction)band80;
- (IBAction)band60;
- (IBAction)band40;
- (IBAction)band30;
- (IBAction)band20;
- (IBAction)band17;
- (IBAction)band15;
- (IBAction)band12;
- (IBAction)band10;
- (IBAction)band6;

- (IBAction)bandGen;
- (IBAction)bandWWV;

- (void)resetTimer;

- (void)setBandButton: (int) band;

- (void) resetBandButton: (int) band;

@end