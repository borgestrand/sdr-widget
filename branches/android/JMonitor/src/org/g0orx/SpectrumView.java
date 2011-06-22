package org.g0orx;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.util.Log;

public class SpectrumView extends View implements OnTouchListener {
	
	
	public SpectrumView(Context context,Connection connection) {
		super(context);
		this.connection=connection;
		paint=new Paint();
		for(int x=0;x<WIDTH;x++) {
			for(int y=0;y<HEIGHT;y++) {
			    waterfall.setPixel(x, y, Color.BLACK);
			}
		}
		this.setOnTouchListener(this);
	}
	
	protected void onDraw(Canvas canvas) {
		if(connection.isConnected()) {
			
			// draw the filter
			paint.setColor(Color.GRAY);
	        canvas.drawRect(filterLeft,0,filterRight,HEIGHT,paint);

			// plot the spectrum levels
			paint.setColor(Color.GRAY);
			int V = spectrumHigh - spectrumLow;
	        int numSteps = V/20;
	        int pixelStepSize = HEIGHT/numSteps;
	        for(int i=1; i<numSteps; i++)
	        {
	            int num = spectrumHigh - i*20;
	            int y = (int)Math.floor((spectrumHigh - num)*HEIGHT/V);

	            paint.setColor(Color.YELLOW);
	            canvas.drawLine(0, y, WIDTH, y, paint);

	            paint.setColor(Color.GREEN);
	            canvas.drawText(Integer.toString(num),3,y+2,paint);
	        }
			
			// plot the cursor
			paint.setColor(Color.RED);
			canvas.drawLine(240, 0, 240, HEIGHT, paint);
			
			// display the frequency and mode
			paint.setColor(Color.GREEN);
			canvas.drawText(Long.toString(connection.getFrequency())+" "+connection.getStringMode(), 100, 10, paint);
			
			if(vfoLocked) {
				paint.setColor(Color.RED);
				canvas.drawText("LOCKED", 300, 10, paint);
			}
			
			// plot the spectrum
			paint.setColor(Color.WHITE);
			canvas.drawLines(points,paint);
			
			// draw the waterfall
			canvas.drawBitmap(waterfall, 1,HEIGHT, paint);
			
			String status=connection.getStatus();
			if(status!=null) {
				paint.setColor(Color.RED);
				canvas.drawText(status, 0, 10, paint);
			}
			
		} else {
		    paint.setColor(0xffffffff);
		    canvas.drawRect(0, 0, canvas.getWidth(), canvas.getHeight(), paint);
		    paint.setColor(Color.RED);
            canvas.drawText("Server is busy - please wait",20,canvas.getHeight()/2,paint);
		}
	}
	
	public void plotSpectrum(int[] samples,int filterLow,int filterHigh,int sampleRate) {

		// scroll the waterfall down
		
		waterfall.getPixels(pixels,0,WIDTH,0,0,WIDTH,HEIGHT-1);
		waterfall.setPixels(pixels, 0, WIDTH, 0, 1, WIDTH, HEIGHT-1);
		
		int p=0;
		float sample;
		float previous=0.0F;
		for(int i=0;i<WIDTH;i++) {
			sample=(float)Math.floor(((float)spectrumHigh-(float)samples[i])*(float)HEIGHT/(float)(spectrumHigh-spectrumLow));
			if(i==0) {
				points[p++]=(float)i;
				points[p++]=sample;
			} else {
				points[p++]=(float)i;
				points[p++]=previous;
			}
			
			points[p++]=(float)i;
	        points[p++]=sample;
	       
	        waterfall.setPixel(i,0,calculatePixel(samples[i]));
	        
	        previous=sample;
		}
		
		this.filterLow=filterLow;
		this.filterHigh=filterHigh;
		filterLeft=(filterLow-(-sampleRate/2))*WIDTH/sampleRate;
        filterRight=(filterHigh-(-sampleRate/2))*WIDTH/sampleRate;

		this.postInvalidate();
	}
	
	private int calculatePixel(float sample) {
        // simple gray scale
        int v=((int)sample-waterfallLow)*255/(waterfallHigh-waterfallLow);

        if(v<0) v=0;
        if(v>255) v=255;

        int pixel=(255<<24)+(v<<16)+(v<<8)+v;
        return pixel;
    }
	
	public void setVfoLock() {
	    vfoLocked=!vfoLocked;	
	}
	
	public void scroll(int step) {
		if(!vfoLocked) {
		    connection.setFrequency((long)(connection.getFrequency()+(step*(connection.getSampleRate()/WIDTH))));
		}
	}
	
	public boolean onTouch(View view,MotionEvent event) {
		if(!vfoLocked) {
		switch(event.getAction()) {
		    case MotionEvent.ACTION_CANCEL:
		    	//Log.i("onTouch","ACTION_CANCEL");
		    	break;
		    case MotionEvent.ACTION_DOWN:
		    	//Log.i("onTouch","ACTION_DOWN");
		    	if(connection.isConnected()) {
		    		//connection.setStatus("onTouch.ACTION_DOWN: "+event.getX());
		    		startX=event.getX();
		    	}
		    	break;
		    case MotionEvent.ACTION_MOVE:
		    	//Log.i("onTouch","ACTION_MOVE");
		    	if(connection.isConnected()) {
		    		//connection.setStatus("onTouch.ACTION_MOVE: "+(int)event.getX());
		            int increment=(int)(startX-event.getX());
		            connection.setFrequency((long)(connection.getFrequency()+(increment*(connection.getSampleRate()/WIDTH))));
		            startX=event.getX();
		        }
		    	break;
		    case MotionEvent.ACTION_OUTSIDE:
		    	//Log.i("onTouch","ACTION_OUTSIDE");
		    	break;
		    case MotionEvent.ACTION_UP:
		    	//Log.i("onTouch","ACTION_UP");
		    	if(connection.isConnected()) {
		    		int scrollAmount=(int)((event.getX() - (WIDTH / 2)) * (connection.getSampleRate() / WIDTH));
		  
		    		if(startX==event.getX()) {
		    			// move this frequency to center of filter
		    			if(filterHigh<0) {
	                        connection.setFrequency(connection.getFrequency()+(scrollAmount + ((filterHigh - filterLow) / 2)));
	                    } else {
	                        connection.setFrequency(connection.getFrequency()+(scrollAmount - ((filterHigh - filterLow) / 2)));
	                    }
		    		}
		    	}
		    	break;
		}
		}
		
		return true;
	}
	
	private Paint paint;
	
	private Connection connection;
	
	private int WIDTH=480;
	private int HEIGHT=160;
	
    private float[] points=new float[WIDTH*4];
    
    Bitmap waterfall=Bitmap.createBitmap(WIDTH,HEIGHT,Bitmap.Config.ARGB_8888);
    int[] pixels=new int[WIDTH*HEIGHT];
    
	private int spectrumHigh=-40;
	private int spectrumLow=-140;
	
	private int waterfallHigh=-75;
	private int waterfallLow=-115;
	
	private int filterLow;
	private int filterHigh;
	
	private int filterLeft;
	private int filterRight;
	
	private boolean vfoLocked=false;
	
	private float startX;
	
}
