//
//  FlipsideView.h
//  HPSDR
//
//  Created by John Melton on 07/07/2009.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

@interface FlipsideView : UIView {
	IBOutlet UITextField *ghpsdrHost;
	IBOutlet UITextField *ghpsdrPort;
	IBOutlet UITextField *displayFPS;
	IBOutlet UITextField *spectrumHigh;
	IBOutlet UITextField *spectrumLow;
}

- (void) saveState;
- (void)init;
@end
