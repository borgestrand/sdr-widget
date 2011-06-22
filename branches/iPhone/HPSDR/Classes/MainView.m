//
//  MainView.m
//  HPSDR
//
//  Created by John Melton on 07/07/2009.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import "MainView.h"
#import "samples.h"
#import "connection.h"

void updateSamples(CGContextRef context);
void connectToServer();
void readCallBack (CFReadStreamRef stream, CFStreamEventType event, void *myPtr);

@implementation MainView

CGPoint lastLocation;
int touched=0;
int dragged=0;

char *modeStr[]={"LSB","USB","DSB","CWL","CWU","FMN","AM","DIGU","SPEC","DIGL","SAM","DRM"};

- (void)refresh {
    [self setNeedsDisplay];
}

- (void)drawRect:(CGRect)rect {
	// Drawing code
	// 480x320
	
	CGPoint points[2];
	
	char text[64];
	
	CGContextRef context = UIGraphicsGetCurrentContext();
	
	[super drawRect:rect];
	
	
	//fprintf(stderr,"drawRect: width=%f,height=%f\n",CGRectGetWidth(rect),CGRectGetHeight(rect));
	CGContextSetRGBStrokeColor(context,0.0,1.0,0.0,1.0);
	CGContextSetLineWidth(context,1.0);
	
	// draw the filter
	float hzPerPixel=(float)sampleRate/CGRectGetWidth(rect);
	float filterX1=240.0+((float)filterLow/hzPerPixel);
	float filterX2=240.0+((float)filterHigh/hzPerPixel);
	CGRect filterRect=CGRectMake(filterX1,20,filterX2-filterX1,140);
	CGContextSetRGBFillColor(context,0.5,0.5,0.5,1.0);
	CGContextFillRect(context,filterRect);
	
	// draw the dBm horizontal lines
	points[0]=CGPointMake(0.0,40.0);
	points[1]=CGPointMake(480.0,40.0);
	CGContextStrokeLineSegments(context,points,2);
	
	points[0]=CGPointMake(0.0,80.0);
	points[1]=CGPointMake(480.0,80.0);
	CGContextStrokeLineSegments(context,points,2);
	
	points[0]=CGPointMake(0.0,120.0);
	points[1]=CGPointMake(480.0,120.0);
	CGContextStrokeLineSegments(context,points,2);
	
    // draw the cursor
	CGContextSetRGBStrokeColor(context,1.0,0.0,0.0,1.0);
	points[0]=CGPointMake(240.0,20.0);
	points[1]=CGPointMake(240.0,160.0);
	CGContextStrokeLineSegments(context,points,2);
	
	CGContextSelectFont(context,"Helvetica",16.0,kCGEncodingMacRoman);
	CGContextSetTextDrawingMode(context,kCGTextFill);
	CGContextSetRGBFillColor(context,255,0,0,1);
	CGAffineTransform xform=CGAffineTransformMake(1.0,0.0,0.0,-1.0,0.0,0.0);
	CGContextSetTextMatrix(context,xform);
	
	int increment;
	increment=(specLow-specHigh)/4;
	int db=specHigh;
	char str[8];
	
	db+=increment;
	sprintf(str,"%d",db);	
	CGContextShowTextAtPoint(context,0.0,40.0,str,strlen(str));
	db+=increment;
	sprintf(str,"%d",db);
	CGContextShowTextAtPoint(context,0.0,80.0,str,strlen(str));
	db+=increment;
	sprintf(str,"%d",db);
	CGContextShowTextAtPoint(context,0.0,120.0,str,strlen(str));
	
	if(isConnected()) {
		if(samplesReceived) {
	        CGContextSetRGBFillColor(context,0,255,0,1);
	        CGContextSelectFont(context,"Helvetica",20.0,kCGEncodingMacRoman);
	        sprintf(text,"%ld %s",frequency,modeStr[mode]);
	        CGContextShowTextAtPoint(context,200,20,text,strlen(text));
			updateSamples(context);
	    } else {
			CGContextSetRGBFillColor(context,255,0,0,1);
			CGContextSelectFont(context,"Helvetica",20.0,kCGEncodingMacRoman);
			sprintf(text,"%s","Server is busy - please wait");
			CGContextShowTextAtPoint(context,200,20,text,strlen(text));
		}
	} else {
		CGContextSetRGBFillColor(context,255,0,0,1);
	    CGContextSelectFont(context,"Helvetica",20.0,kCGEncodingMacRoman);
	    sprintf(text,"%s","Not Connected");
	    CGContextShowTextAtPoint(context,200,20,text,strlen(text));
	}
	
}


- (void)dealloc {
    [super dealloc];
}

- (void) repaint {
	fprintf(stderr,"repaint\n");
	[self setNeedsDisplay];
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *) event {
	UITouch *touch = [touches anyObject];
	lastLocation = [touch locationInView:self];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	//char command[64];
	UITouch *touch = [touches anyObject];
	CGPoint touchLocation = [touch locationInView:self];
	dragged=dragged+(int)(lastLocation.x-touchLocation.x);
	touched=1;
	lastLocation=touchLocation;
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	//char command[64];
	UITouch *touch = [touches anyObject];
	CGPoint touchLocation = [touch locationInView:self];
	dragged=dragged+(int)(lastLocation.x-touchLocation.x);
	touched=1;
	lastLocation=touchLocation;
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
	fprintf(stderr,"touchesCancelled\n");
}

- (void)dragged {
	if(touched) {
		touched=0;
		setFrequency(frequency+(dragged*(sampleRate/480)));
		dragged=0;
    }
	
}


@end

void updateSamples(CGContextRef context) {
	CGPoint points[480];
	int i;
	
	for(i=0;i<480;i++) {
		points[i].x=(float)i;
		points[i].y=(floorf(((float)specHigh-samples[i])*160.0/(float)(specHigh-specLow)));
    }
	
	CGContextSetRGBStrokeColor(context,1.0,1.0,0.0,1.0);
	CGContextSetLineWidth(context,1.0);
	CGContextBeginPath(context);
	CGContextAddLines(context,points,480);
	CGContextStrokePath(context);
	
}

