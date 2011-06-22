//
//  FlipsideView.m
//  HPSDR
//
//  Created by John Melton on 07/07/2009.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import "FlipsideView.h"
#import "connection.h"
#import "samples.h"

@implementation FlipsideView

- (id)initWithFrame:(CGRect)frame {
	fprintf(stderr,"FlipsideView: initWithFrame\n");
    if (self = [super initWithFrame:frame]) {
    }
    return self;
}


- (void)drawRect:(CGRect)rect {
    // Drawing code
}


- (void)dealloc {
    [super dealloc];
}

- (void)init {
	
	id keyboardImpl=[objc_getClass("UIKeyboardImpl")sharedInstance];
	[keyboardImpl setAlpha:0.5];
	
	fprintf(stderr,"FlipsideView.init\n");
	// Initialization code
	NSString *stringHost = [[NSString alloc] initWithUTF8String:host];
	ghpsdrHost.text = stringHost;
	char temp[16];
	sprintf(temp,"%d",port);
	NSString *stringPort = [[NSString alloc] initWithUTF8String:temp];
	ghpsdrPort.text = stringPort;
	
	sprintf(temp,"%d",fps);
	NSString *stringDisplayFPS = [[NSString alloc] initWithUTF8String:temp];
	displayFPS.text = stringDisplayFPS;
	
    sprintf(temp,"%d",specHigh);
	NSString *stringSpectrumHigh = [[NSString alloc] initWithUTF8String:temp];
	spectrumHigh.text = stringSpectrumHigh;
	
	sprintf(temp,"%d",specLow);
	NSString *stringSpectrumLow = [[NSString alloc] initWithUTF8String:temp];
	spectrumLow.text = stringSpectrumLow;
	
}

- (void)saveState {
	const char *newHost;
	const char *newPort;
	const char *newDisplayFPS;
	const char *newSpectrumHigh;
	const char *newSpectrumLow;
	
	fprintf(stderr,"saveState\n");
	
	NSString *strHost=ghpsdrHost.text;
	newHost=[strHost UTF8String];
		
	NSString *strPort=ghpsdrPort.text;
	newPort=[strPort UTF8String];
	
	NSString *strDisplayFPS=displayFPS.text;
	newDisplayFPS=[strDisplayFPS UTF8String];
	
	NSString *strSpectrumHigh=spectrumHigh.text;
	newSpectrumHigh=[strSpectrumHigh UTF8String];
	
	NSString *strSpectrumLow=spectrumLow.text;
	newSpectrumLow=[strSpectrumLow UTF8String];
	

	fprintf(stderr,"host=%s\n",newHost);
	fprintf(stderr,"port=%s\n",newPort);
	fprintf(stderr,"displayFPS=%s\n",newDisplayFPS);
	fprintf(stderr,"spectrumHigh=%s\n",newSpectrumHigh);
	fprintf(stderr,"spectrumLow=%s\n",newSpectrumLow);

	newConnection(newHost,newPort);
	
	fps=atoi(newDisplayFPS);
	specHigh=atoi(newSpectrumHigh);
	specLow=atoi(newSpectrumLow);

	// save to property list
	
}

@end
