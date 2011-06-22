//
//  FlipsideViewController.m
//  HPSDR
//
//  Created by John Melton on 07/07/2009.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import "FlipsideViewController.h"


@implementation FlipsideViewController

@synthesize delegate;

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor viewFlipsideBackgroundColor];      
	[self.view init];
}


- (IBAction)done {
	fprintf(stderr,"FlipsideViewController: Done\n");
	[self.view saveState];
	[self.delegate flipsideViewControllerDidFinish:self];	
}



- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
return (interfaceOrientation == UIInterfaceOrientationLandscapeLeft);
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
	fprintf(stderr,"FlipsideViewController: textFieldShouldReturn\n");
	[textField resignFirstResponder];
	return YES;
}

@end
