package org.g0orx;

import android.util.Log;

public class Update extends Thread {

	public Update(Connection connection) {
	    this.connection=connection;	
	}
	
	public void run() {
		//Log.i("Update","run");
		while(running) {
			connection.getSpectrum();
			try {
			    sleep(1000/fps);
			} catch(Exception e) {
				
			}
		}
	}
	
	public void close() {
		running=false;
	}
	
	private Connection connection;
	private boolean running=true;
	private int fps=8;
	
}
