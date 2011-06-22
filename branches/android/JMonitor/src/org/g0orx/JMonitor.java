package org.g0orx;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;

public class JMonitor extends Activity {
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        
        connection=new Connection(server,BASE_PORT+receiver);
        
        update=new Update(connection);
        
        spectrumView=new SpectrumView(this,connection);
        
        connection.setSpectrumView(spectrumView);
        
        setContentView(spectrumView);
        
    }
    
    public boolean onTrackballEvent(MotionEvent event) {
    	switch(event.getAction()) {
    case MotionEvent.ACTION_DOWN:
    	//Log.i("onTouch","ACTION_DOWN");
    	spectrumView.setVfoLock();
    	break;
    case MotionEvent.ACTION_MOVE:
    	//Log.i("onTrackballEvent","ACTION_MOVE");
    	spectrumView.scroll(-(int)(event.getX()*6.0));
    	break;
    	}
        return true;
    }
    
    public void onStart() {
    	super.onStart();
    	Log.i("Jmonitor","onStart");
    	
    	
    }
    
    public void onRestart() {
    	super.onRestart();
    	Log.i("Jmonitor","onRestart");
    	
    }
    
    public void onResume() {
    	super.onResume();
    	Log.i("Jmonitor","onResume");
    	connection.start();        
        connection.setFrequency(7048000);
        connection.setMode(0);
        connection.setFilter(-2850,-150);
        connection.setGain(30);
        connection.setAGC(AGC_LONG);
    	update.start();
    }
    
    public void onPause() {
    	super.onPause();
    	Log.i("Jmonitor","onPause");
    }
    
    public void onStop() {
    	super.onStop();
    	Log.i("Jmonitor","onStop");
    	update.close();
    	connection.close();
    }
    
    public void onDestroy() {
    	super.onDestroy();
    	Log.i("Jmonitor","onDestroy");
    }

    public boolean onCreateOptionsMenu(Menu menu) {
    	menu.add(0, MENU_BAND, 0, "Band");
    	menu.add(0, MENU_MODE, 0, "Mode");
    	menu.add(0, MENU_FILTER, 0, "FILTER");
    	menu.add(0, MENU_AGC, 0, "AGC");
    	menu.add(0, MENU_DSP, 0, "DSP");
    	menu.add(0, MENU_GAIN, 0, "GAIN");
    	menu.add(0, MENU_QUIT, 0, "Quit");
    	return true;
    }
    
    public boolean onOptionsItemSelected(MenuItem item) {
    	switch(item.getItemId()) {
    	    case MENU_QUIT:
    		    this.finish();
    		    break;
    	    case MENU_BAND:
                showDialog(MENU_BAND);
    	        break;
    	    case MENU_MODE:
                showDialog(MENU_MODE);
    	        break;
    	    case MENU_FILTER:
                showDialog(MENU_FILTER);
    	        break;
    	    case MENU_AGC:
                showDialog(MENU_AGC);
    	        break;
    	    case MENU_DSP:
                showDialog(MENU_DSP);
    	    	break;
    	    case MENU_GAIN:
    	    	showDialog(MENU_GAIN);
    	    	break;
 
    	}
    	return true;
    }
    
    protected Dialog onCreateDialog(int id) {
    	Dialog dialog;
    	AlertDialog.Builder builder;
    	
    	dialog=null;
    	switch(id) {
    	    case MENU_BAND:
    	    	builder=new AlertDialog.Builder(this);
    	    	builder.setTitle("Select a Band");
    	    	builder.setSingleChoiceItems(bands,band,new DialogInterface.OnClickListener() {
    	    		public void onClick(DialogInterface dialog,int item) {
    	    			//
    	    			band=item;
    	    			switch(item) {
    	    			case BAND_160:
    	    				connection.setMode(MODE_LSB);
    	    				connection.setFilter(-2850,-150);
    	    				connection.setFrequency(1850000);
    	    				break;
    	    			case BAND_80:
    	    				connection.setMode(MODE_LSB);
    	    				connection.setFilter(-2850,-150);
    	    				connection.setFrequency(3790000);
    	    				break;
    	    			case BAND_60:
    	    				connection.setMode(MODE_LSB);
    	    				connection.setFilter(-2850,-150);
    	    				connection.setFrequency(5371500);
    	    				break;
    	    			case BAND_40:
    	    				connection.setMode(MODE_LSB);
    	    				connection.setFilter(-2850,-150);
    	    				connection.setFrequency(7048000);
    	    				break;
    	    			case BAND_30:
    	    				connection.setMode(MODE_USB);
    	    				connection.setFilter(150,2850);
    	    				connection.setFrequency(10135600);
    	    				break;
    	    			case BAND_20:
    	    				connection.setMode(MODE_USB);
    	    				connection.setFilter(150,2850);
    	    				connection.setFrequency(14200000);
    	    				break;
    	    			case BAND_17:
    	    				connection.setMode(MODE_USB);
    	    				connection.setFilter(150,2850);
    	    				connection.setFrequency(18118900);
    	    				break;
    	    			case BAND_15:
    	    				connection.setMode(MODE_USB);
    	    				connection.setFilter(150,2850);
    	    				connection.setFrequency(21200000);
    	    				break;
    	    			case BAND_12:
    	    				connection.setMode(MODE_USB);
    	    				connection.setFilter(150,2850);
    	    				connection.setFrequency(24910000);
    	    				break;
    	    			case BAND_10:
    	    				connection.setMode(MODE_USB);
    	    				connection.setFilter(150,2850);
    	    				connection.setFrequency(28500000);
    	    				break;
    	    			case BAND_6:
    	    				connection.setMode(MODE_USB);
    	    				connection.setFilter(150,2850);
    	    				connection.setFrequency(50200000);
    	    				break;
    	    			case BAND_GEN:
    	    				connection.setMode(MODE_AM);
    	    				connection.setFilter(-4000,4000);
    	    				connection.setFrequency(909000);
    	    				break;
    	    			case BAND_WWV:
    	    				connection.setMode(MODE_USB);
    	    				connection.setFilter(-4000,4000);
    	    				connection.setFrequency(5000000);
    	    				break;
    	    			}
    	    			dialog.dismiss();
    	    		}
    	    	});
    	    	dialog=builder.create();
    		    break;
    	    case MENU_MODE:
    	    	builder=new AlertDialog.Builder(this);
    	    	builder.setTitle("Select a Mode");
    	    	builder.setSingleChoiceItems(modes,connection.getMode(),new DialogInterface.OnClickListener() {
    	    		public void onClick(DialogInterface dialog,int item) {
    	    			connection.setMode(item);
    	    			switch(item) {
    	    			case MODE_LSB:
    	    				connection.setFilter(-2850,-150);
    	    				break;
    	    			case MODE_USB:
    	    				connection.setFilter(150,2850);
    	    				break;
    	    			case MODE_DSB:
    	    				connection.setFilter(-2600,2600);
    	    				break;
    	    			case MODE_CWL:
    	    				connection.setFilter(-800,-400);
    	    				break;
    	    			case MODE_CWU:
    	    				connection.setFilter(400,800);
    	    				break;
    	    			case MODE_FMN:
    	    				connection.setFilter(-2600,2600);
    	    				break;
    	    			case MODE_AM:
    	    				connection.setFilter(-4000,4000);
    	    				break;
    	    			case MODE_DIGU:
    	    				connection.setFilter(150,3450);
    	    				break;
    	    			case MODE_SPEC:
    	    				connection.setFilter(-6000,6000);
    	    				break;
    	    			case MODE_DIGL:
    	    				connection.setFilter(-3450,-150);
    	    				break;
    	    			case MODE_SAM:
    	    				connection.setFilter(-4000,4000);
    	    				break;
    	    			case MODE_DRM:
    	    				connection.setFilter(-6000,6000);
    	    				break;
    	    			}
    	    			dialog.dismiss();
    	    		}
    	    	});
    	    	dialog=builder.create();
    		    break;
    	    case MENU_FILTER:
    	    	builder=new AlertDialog.Builder(this);
    	    	builder.setTitle("Select Filter");
    	    	CharSequence[] filters=null;
    	    	switch(connection.getMode()) {
    	    	case MODE_LSB:
				case MODE_USB:
				case MODE_DSB:
					filters=ssbFilters;
					break;
				case MODE_CWL:
				case MODE_CWU:
					filters=cwFilters;
					break;
				case MODE_FMN:
				case MODE_AM:
				case MODE_DIGU:
				case MODE_DIGL:
				case MODE_SAM:
					filters=amFilters;
					break;
				case MODE_SPEC:
				case MODE_DRM:
					filters=null;
					break;
    	    	}
    	    	if(filters!=null) {
    	    	    builder.setSingleChoiceItems(ssbFilters,FILTER_5,new DialogInterface.OnClickListener() {
    	    		    public void onClick(DialogInterface dialog,int item) {
    	    			    switch(item) {
    	    			    case FILTER_0:
    	    				    switch(connection.getMode()) {
    	    				    case MODE_LSB:
    	    				    	connection.setFilter(-5150,-150);
    	    					    break;
    	    				    case MODE_USB:
    	    				    	connection.setFilter(150,5150);
    	    					    break;
    	    				    case MODE_DSB:
    	    				    	connection.setFilter(5000,5000);
    	    					    break;
    	    				    case MODE_CWL:
    	    				    	connection.setFilter(-cwPitch-500,-cwPitch+500);
    	    					    break;
    	    				    case MODE_CWU:
    	    				    	connection.setFilter(cwPitch-500,cwPitch+500);
    	    					    break;
    	    				    case MODE_FMN:
    	    				    case MODE_AM:
    	    				    case MODE_DIGU:
    	    				    case MODE_DIGL:
    	    				    case MODE_SAM:
    	    				    	connection.setFilter(-8000,8000);
    	    					    break;
    	    					case MODE_SPEC:
    	    					    break;
    	    				    case MODE_DRM:
    	    					    break;
    	    				    }
    	    				    break;
    	    			    case FILTER_1:
    	    			    	switch(connection.getMode()) {
    	    				    case MODE_LSB:
    	    				    	connection.setFilter(-4550,-150);
    	    					    break;
    	    				    case MODE_USB:
    	    				    	connection.setFilter(150,4550);
    	    					    break;
    	    				    case MODE_DSB:
    	    				    	connection.setFilter(-4400,4400);
    	    					    break;
    	    				    case MODE_CWL:
    	    				    	connection.setFilter(-cwPitch-400,-cwPitch+400);
    	    					    break;
    	    				    case MODE_CWU:
    	    				    	connection.setFilter(cwPitch-400,cwPitch+400);
    	    					    break;
    	    				    case MODE_FMN:
    	    				    case MODE_AM:
    	    				    case MODE_DIGU:
    	    				    case MODE_DIGL:
    	    				    case MODE_SAM:
    	    				    	connection.setFilter(-6000,6000);
    	    					    break;
    	    					case MODE_SPEC:
    	    					    break;
    	    				    case MODE_DRM:
    	    					    break;
    	    				    }
    	    				    break;
    	    			    case FILTER_2:
    	    			    	switch(connection.getMode()) {
    	    				    case MODE_LSB:
    	    				    	connection.setFilter(-3950,-150);
    	    					    break;
    	    				    case MODE_USB:
    	    				    	connection.setFilter(150,3950);
    	    					    break;
    	    				    case MODE_DSB:
    	    				    	connection.setFilter(-3800,3800);
    	    					    break;
    	    				    case MODE_CWL:
    	    				    	connection.setFilter(-cwPitch-375,-cwPitch+375);
    	    					    break;
    	    				    case MODE_CWU:
    	    				    	connection.setFilter(cwPitch-375,cwPitch+375);
    	    					    break;
    	    				    case MODE_FMN:
    	    				    case MODE_AM:
    	    				    case MODE_DIGU:
    	    				    case MODE_DIGL:
    	    				    case MODE_SAM:
    	    				    	connection.setFilter(-5000,5000);
    	    					    break;
    	    					case MODE_SPEC:
    	    					    break;
    	    				    case MODE_DRM:
    	    					    break;
    	    				    }
    	    				    break;
    	    			    case FILTER_3:
    	    			    	switch(connection.getMode()) {
    	    				    case MODE_LSB:
    	    				    	connection.setFilter(-3450,-150);
    	    					    break;
    	    				    case MODE_USB:
    	    				    	connection.setFilter(150,3450);
    	    					    break;
    	    				    case MODE_DSB:
    	    				    	connection.setFilter(-3300,3300);
    	    					    break;
    	    				    case MODE_CWL:
    	    				    	connection.setFilter(-cwPitch-300,-cwPitch+300);
    	    					    break;
    	    				    case MODE_CWU:
    	    				    	connection.setFilter(cwPitch-300,cwPitch+300);
    	    					    break;
    	    				    case MODE_FMN:
    	    				    case MODE_AM:
    	    				    case MODE_DIGU:
    	    				    case MODE_DIGL:
    	    				    case MODE_SAM:
    	    				    	connection.setFilter(-4000,4000);
    	    					    break;
    	    					case MODE_SPEC:
    	    					    break;
    	    				    case MODE_DRM:
    	    					    break;
    	    				    }
    	    				    break;
    	    			    case FILTER_4:
    	    			    	switch(connection.getMode()) {
    	    				    case MODE_LSB:
    	    				    	connection.setFilter(-3050,-150);
    	    					    break;
    	    				    case MODE_USB:
    	    				    	connection.setFilter(150,3050);
    	    					    break;
    	    				    case MODE_DSB:
    	    				    	connection.setFilter(-2900,2900);
    	    					    break;
    	    				    case MODE_CWL:
    	    				    	connection.setFilter(-cwPitch-250,-cwPitch+250);
    	    					    break;
    	    				    case MODE_CWU:
    	    				    	connection.setFilter(cwPitch-250,cwPitch+250);
    	    					    break;
    	    				    case MODE_FMN:
    	    				    case MODE_AM:
    	    				    case MODE_DIGU:
    	    				    case MODE_DIGL:
    	    				    case MODE_SAM:
    	    				    	connection.setFilter(-3300,3300);
    	    					    break;
    	    					case MODE_SPEC:
    	    					    break;
    	    				    case MODE_DRM:
    	    					    break;
    	    				    }
    	    				    break;
    	    			    case FILTER_5:
    	    			    	switch(connection.getMode()) {
    	    				    case MODE_LSB:
    	    				    	connection.setFilter(-2850,-150);
    	    					    break;
    	    				    case MODE_USB:
    	    				    	connection.setFilter(150,2850);
    	    					    break;
    	    				    case MODE_DSB:
    	    				    	connection.setFilter(-2700,2700);
    	    					    break;
    	    				    case MODE_CWL:
    	    				    	connection.setFilter(-cwPitch-200,-cwPitch+200);
    	    					    break;
    	    				    case MODE_CWU:
    	    				    	connection.setFilter(cwPitch-200,cwPitch+200);
    	    					    break;
    	    				    case MODE_FMN:
    	    				    case MODE_AM:
    	    				    case MODE_DIGU:
    	    				    case MODE_DIGL:
    	    				    case MODE_SAM:
    	    				    	connection.setFilter(-2600,2600);
    	    					    break;
    	    					case MODE_SPEC:
    	    					    break;
    	    				    case MODE_DRM:
    	    					    break;
    	    				    }
    	    				    break;
    	    			    case FILTER_6:
    	    			    	switch(connection.getMode()) {
    	    				    case MODE_LSB:
    	    				    	connection.setFilter(-2550,-150);
    	    					    break;
    	    				    case MODE_USB:
    	    				    	connection.setFilter(150,2550);
    	    					    break;
    	    				    case MODE_DSB:
    	    				    	connection.setFilter(-2400,2400);
    	    					    break;
    	    				    case MODE_CWL:
    	    				    	connection.setFilter(-cwPitch-125,-cwPitch+125);
    	    					    break;
    	    				    case MODE_CWU:
    	    				    	connection.setFilter(cwPitch-125,cwPitch+125);
    	    					    break;
    	    				    case MODE_FMN:
    	    				    case MODE_AM:
    	    				    case MODE_DIGU:
    	    				    case MODE_DIGL:
    	    				    case MODE_SAM:
    	    				    	connection.setFilter(-2000,2000);
    	    					    break;
    	    					case MODE_SPEC:
    	    					    break;
    	    				    case MODE_DRM:
    	    					    break;
    	    				    }
    	    				    break;
    	    			    case FILTER_7:
    	    			    	switch(connection.getMode()) {
    	    				    case MODE_LSB:
    	    				    	connection.setFilter(-2250,-150);
    	    					    break;
    	    				    case MODE_USB:
    	    				    	connection.setFilter(150,2250);
    	    					    break;
    	    				    case MODE_DSB:
    	    				    	connection.setFilter(-2100,2100);
    	    					    break;
    	    				    case MODE_CWL:
    	    				    	connection.setFilter(-cwPitch-50,-cwPitch+50);
    	    					    break;
    	    				    case MODE_CWU:
    	    				    	connection.setFilter(cwPitch-50,cwPitch+50);
    	    					    break;
    	    				    case MODE_FMN:
    	    				    case MODE_AM:
    	    				    case MODE_DIGU:
    	    				    case MODE_DIGL:
    	    				    case MODE_SAM:
    	    				    	connection.setFilter(-1550,1550);
    	    					    break;
    	    					case MODE_SPEC:
    	    					    break;
    	    				    case MODE_DRM:
    	    					    break;
    	    				    }
    	    				    break;
    	    			    case FILTER_8:
    	    			    	switch(connection.getMode()) {
    	    				    case MODE_LSB:
    	    				    	connection.setFilter(-1950,-150);
    	    					    break;
    	    				    case MODE_USB:
    	    				    	connection.setFilter(150,1950);
    	    					    break;
    	    				    case MODE_DSB:
    	    				    	connection.setFilter(-1800,1800);
    	    					    break;
    	    				    case MODE_CWL:
    	    				    	connection.setFilter(-cwPitch-25,-cwPitch+25);
    	    					    break;
    	    				    case MODE_CWU:
    	    				    	connection.setFilter(cwPitch-25,cwPitch+25);
    	    					    break;
    	    				    case MODE_FMN:
    	    				    case MODE_AM:
    	    				    case MODE_DIGU:
    	    				    case MODE_DIGL:
    	    				    case MODE_SAM:
    	    				    	connection.setFilter(-1450,1450);
    	    					    break;
    	    					case MODE_SPEC:
    	    					    break;
    	    				    case MODE_DRM:
    	    					    break;
    	    				    }
    	    				    break;
    	    			    case FILTER_9:
    	    			    	switch(connection.getMode()) {
    	    				    case MODE_LSB:
    	    				    	connection.setFilter(-1150,-150);
    	    					    break;
    	    				    case MODE_USB:
    	    				    	connection.setFilter(150,1150);
    	    					    break;
    	    				    case MODE_DSB:
    	    				    	connection.setFilter(-1000,1000);
    	    					    break;
    	    				    case MODE_CWL:
    	    				    	connection.setFilter(-cwPitch-12,-cwPitch+12);
    	    					    break;
    	    				    case MODE_CWU:
    	    				    	connection.setFilter(cwPitch-12,cwPitch+12);
    	    					    break;
    	    				    case MODE_FMN:
    	    				    case MODE_AM:
    	    				    case MODE_DIGU:
    	    				    case MODE_DIGL:
    	    				    case MODE_SAM:
    	    				    	connection.setFilter(-1000,1000);
    	    					    break;
    	    					case MODE_SPEC:
    	    					    break;
    	    				    case MODE_DRM:
    	    					    break;
    	    				    }
    	    				    break;
    	    			    }
    	    		    	dialog.dismiss();
    	    		    }
    	    	    });
    	    	    dialog=builder.create();
    	    	}
    		    break;
    	    case MENU_AGC:
    	    	builder=new AlertDialog.Builder(this);
    	    	builder.setTitle("Select AGC");
    	    	builder.setSingleChoiceItems(agcs,agc,new DialogInterface.OnClickListener() {
    	    		public void onClick(DialogInterface dialog,int item) {
    	    			//
    	    			dialog.dismiss();
    	    		}
    	    	});
    	    	dialog=builder.create();
    		    break;
    	    case MENU_DSP:
    	    	builder=new AlertDialog.Builder(this);
    	    	builder.setTitle("Select DSP");
    	    	builder.setMultiChoiceItems(dsps,dsp_state,new DialogInterface.OnMultiChoiceClickListener() {
    	    		public void onClick(DialogInterface dialog,int item,boolean state) {
    	    			//
    	    			switch(item) {
    	    			case DSP_NR:
    	    				connection.setNR(state);
    	    				break;
    	    			case DSP_ANF:
    	    				connection.setANF(state);
    	    				break;
    	    			case DSP_NB:
    	    				connection.setNB(state);
    	    				break;
    	    			}
    	    			
    	    			dialog.dismiss();
    	    		}
    	    	});
    	    	dialog=builder.create();
    		    break;
    	    case MENU_GAIN:
    	    	builder=new AlertDialog.Builder(this);
    	    	builder.setTitle("Select Gain");
    	    	builder.setSingleChoiceItems(gains,GAIN_30,new DialogInterface.OnClickListener() {
    	    		public void onClick(DialogInterface dialog,int item) {
    	    			connection.setGain(item*10);
    	    			dialog.dismiss();
    	    		}
    	    	});
    	    	dialog=builder.create();
    		    break;
    		default:
    		    dialog=null;
    		    break;
    	}
    	return dialog;
    }
    
    private Connection connection;
    private SpectrumView spectrumView;
    private Update update;
    
    public static final int MENU_QUIT=0;
    public static final int MENU_BAND=1;
    public static final int MENU_MODE=2;
    public static final int MENU_FILTER=3;
    public static final int MENU_AGC=4;
    public static final int MENU_DSP=5;
    public static final int MENU_GAIN=6;
    
    public static final CharSequence[] bands={"160","80","60","40","30","20","17","15","12","10","6","GEN","WWV"};
    
    private int band=BAND_40;
    
    public static final int BAND_160=0;
    public static final int BAND_80=1;
    public static final int BAND_60=2;
    public static final int BAND_40=3;
    public static final int BAND_30=4;
    public static final int BAND_20=5;
    public static final int BAND_17=6;
    public static final int BAND_15=7;
    public static final int BAND_12=8;
    public static final int BAND_10=9;
    public static final int BAND_6=10;
    public static final int BAND_GEN=11;
    public static final int BAND_WWV=12;
    
    public static final CharSequence[] modes={"LSB","USB","DSB","CWL","CWU","FMN","AM","DIGU","SPEC","DIGL","SAM","DRM"};
    
    public static final int MODE_LSB=0;
    public static final int MODE_USB=1;
    public static final int MODE_DSB=2;
    public static final int MODE_CWL=3;
    public static final int MODE_CWU=4;
    public static final int MODE_FMN=5;
    public static final int MODE_AM=6;
    public static final int MODE_DIGU=7;
    public static final int MODE_SPEC=8;
    public static final int MODE_DIGL=9;
    public static final int MODE_SAM=10;
    public static final int MODE_DRM=11;
    
    public static final CharSequence[] agcs={"OFF","LONG","SLOW","MEDIUM","FAST"};
    private int agc=AGC_LONG;
    
    public static final int AGC_OFF=0;
    public static final int AGC_LONG=1;
    public static final int AGC_SLOW=2;
    public static final int AGC_MEDIUM=3;
    public static final int AGC_FAST=4;
    
    public static final CharSequence[] dsps={"NR","ANF","NB"};
    
    public static final int DSP_NR=0;
    public static final int DSP_ANF=1;
    public static final int DSP_NB=2;
    
    private boolean[] dsp_state={false,false,false};
    
    public static final CharSequence[] gains={"0","10","20","30","40","50","60","70","80","90","100"};
    
    public int gain=30;
    
    public static final int GAIN_0=0;
    public static final int GAIN_10=1;
    public static final int GAIN_20=2;
    public static final int GAIN_30=3;
    public static final int GAIN_40=4;
    public static final int GAIN_50=5;
    public static final int GAIN_60=6;
    public static final int GAIN_70=7;
    public static final int GAIN_80=8;
    public static final int GAIN_90=9;
    public static final int GAIN_100=10;
    
    public static final CharSequence[] ssbFilters={"5.0k","4.4k","3.8k","3.3k","2.9k","2.7k","2.4k","2.1k","1.8k","1.0k"};
    public static final CharSequence[] cwFilters={"1.0k","800","750","600","500","400","250","100","50","25"};
    public static final CharSequence[] amFilters={"16.0k","12.0k","10.0k","8.0k","6.6k","5.2k","4.0k","3.1k","2.9k","2.0k"};

    private int filter=FILTER_5;
    
    public static final int FILTER_0=0;
    public static final int FILTER_1=1;
    public static final int FILTER_2=2;
    public static final int FILTER_3=3;
    public static final int FILTER_4=4;
    public static final int FILTER_5=5;
    public static final int FILTER_6=6;
    public static final int FILTER_7=7;
    public static final int FILTER_8=8;
    public static final int FILTER_9=9;
    
    private int cwPitch=400;
    
    private String server="81.146.61.118";
    private static final int BASE_PORT=8000;
    private int port=8000;
    private int receiver=0;
    
   
}