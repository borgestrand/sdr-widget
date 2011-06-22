//
//  HPSDRAppDelegate.h
//  HPSDR
//
//  Created by John Melton on 07/07/2009.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

@class MainViewController;

@interface HPSDRAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    MainViewController *mainViewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) MainViewController *mainViewController;

@end

