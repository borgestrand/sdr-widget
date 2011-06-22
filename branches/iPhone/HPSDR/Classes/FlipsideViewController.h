//
//  FlipsideViewController.h
//  HPSDR
//
//  Created by John Melton on 07/07/2009.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

@protocol FlipsideViewControllerDelegate;



@interface FlipsideViewController : UIViewController {
	id <FlipsideViewControllerDelegate> delegate;
}

@property (nonatomic, assign) id <FlipsideViewControllerDelegate> delegate;


- (IBAction)done;

@end


@protocol FlipsideViewControllerDelegate
- (void)flipsideViewControllerDidFinish:(FlipsideViewController *)controller;
@end

