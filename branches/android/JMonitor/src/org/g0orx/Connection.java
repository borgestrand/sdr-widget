package org.g0orx;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.net.Socket;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;

public class Connection extends Thread {

	public Connection(String server,int port) {
		//Log.i("Connection",server+":"+port);
		
		this.server=server;
		this.port=port;
		try {
		    socket=new Socket(server,port);
		    inputStream=socket.getInputStream();
		    outputStream=socket.getOutputStream();
		} catch (Exception e) {
			Log.e("Connection","Error creating socket for "+server+":"+port+"'"+e.getMessage()+"'");
		}
		
		// 2.1
		//audioTrack=new AudioTrack(AudioManager.STREAM_MUSIC,8000,AudioFormat.CHANNEL_OUT_MONO,AudioFormat.ENCODING_PCM_16BIT,BUFFER_SIZE*2,AudioTrack.MODE_STREAM);
		
		// 1.6
		audioTrack=new AudioTrack(AudioManager.STREAM_MUSIC,8000,AudioFormat.CHANNEL_CONFIGURATION_MONO,AudioFormat.ENCODING_PCM_16BIT,AUDIO_BUFFER_SIZE*2,AudioTrack.MODE_STREAM);	
		
		audioTrack.play();
		System.gc();
	}
	
	public void close() {
		running=false;
		if(socket!=null) {
			try {
				socket.close();
			} catch (Exception e) {
				
			}
		}
		if(audioTrack.getPlayState()!=AudioTrack.PLAYSTATE_PLAYING) {
        	audioTrack.stop();
        }
	}
	
	public void run() {
		int bytes;
		byte[] header=new byte[HEADER_SIZE];
		byte[] spectrumBuffer=new byte[BUFFER_SIZE];
		byte[] audioBuffer=new byte[AUDIO_BUFFER_SIZE];
		
		if(socket!=null) {
		    while(running) {
			    try {
			    	bytes=0;
			    	while(bytes!=HEADER_SIZE) {
			    	    bytes+=inputStream.read(header,bytes,HEADER_SIZE-bytes);
			    	}
			    	
			    	// start the audio once we are connected
			    	if(!connected) {
			    		sendCommand("startAudioStream "+AUDIO_BUFFER_SIZE);
			    		connected=true;
			    	}
			    	
			    	switch(header[0]) {
			    		case SPECTRUM_BUFFER:
			    			bytes=0;
					    	while(bytes!=BUFFER_SIZE) {
					    	    bytes+=inputStream.read(spectrumBuffer,bytes,BUFFER_SIZE-bytes);
					    	}
			    		    processSpectrumBuffer(header,spectrumBuffer);
			    			break;
			    		case AUDIO_BUFFER:
			    			bytes=0;
					    	while(bytes!=AUDIO_BUFFER_SIZE) {
					    	    bytes+=inputStream.read(audioBuffer,bytes,AUDIO_BUFFER_SIZE-bytes);
					    	}
			    		    processAudioBuffer(header,audioBuffer);
			    		    break;
			    		default:
			    			Log.e("header","Invalid type "+header[0]);
			    		    break;	
			        }
			    	
			    } catch (Exception e) {
			    	System.err.println("Connection.run: Exception reading socket: "+e.toString());
			    	e.printStackTrace();
			    }
	   	    }
		}
	}
	
	private void processSpectrumBuffer(byte[] header,byte[] buffer) {
		int j;
		String s;
		
		j=0;
		while(header[j+32]!='\0') {
			j++;
		}
		s=new String(header,32,j);
		sampleRate=Integer.parseInt(s);
		
		j=0;
		while(header[j+40]!='\0') {
			j++;
		}
		s=new String(header,40,j);
		meter=Integer.parseInt(s);
		
		for(int i=0;i<BUFFER_SIZE;i++) {
			samples[i]=-(buffer[i]&0xFF)-30;
		}
		
		if(spectrumView!=null) {
			spectrumView.plotSpectrum(samples,filterLow,filterHigh,sampleRate);
		}
	}
	
	private void processAudioBuffer(byte[] header,byte[] buffer) {
		
		// decode 8 bit aLaw to 16 bit linear
        for (int i=0; i < AUDIO_BUFFER_SIZE; i++) {
            decodedBuffer[i]=aLawDecode[buffer[i]&0xFF];
        }
        
        int waitingToSend=audioSampleCount-audioTrack.getPlaybackHeadPosition();
        //Log.d("AudioTrack","waitingToSend="+waitingToSend);
        if(waitingToSend<AUDIO_BUFFER_SIZE) {
            //audioTrack.pause();
	    	audioSampleCount+=audioTrack.write(decodedBuffer,0,AUDIO_BUFFER_SIZE);
        } else {
        	Log.d("AudioTrack","dropping buffer");
        }
        //audioTrack.play();
	}
	
	public synchronized void sendCommand(String command) {
		
		// Log.i("sendCommand",command);
		byte[] commandBytes=command.getBytes();
		for(int i=0;i<32;i++) {
			if(i<commandBytes.length) {
				commandBuffer[i]=commandBytes[i];
			} else {
				commandBuffer[i]=0;
			}
		}
		
		if(socket!=null) {
			try {
				outputStream.write(commandBuffer);
				outputStream.flush();
			} catch (IOException e) {
				System.err.println("Connection.sendCommand: IOException: "+e.getMessage());
			}
		}
	}
	
	public void setFrequency(long frequency) {
        this.frequency=frequency;
        sendCommand("setFrequency "+frequency);
    }

	public long getFrequency() {
		return frequency;
	}
	
    public void setFilter(int filterLow,int filterHigh) {
        this.filterLow=filterLow;
        this.filterHigh=filterHigh;
        sendCommand("setFilter "+filterLow+" "+filterHigh);
    }

    public void setMode(int mode) {
        this.mode=mode;
        sendCommand("setMode "+mode);
    }
    
    public int getMode() {
    	return mode;
    }
    
    public String getStringMode() {
    	return modes[mode];
    }
    
    public void setAGC(int agc) {
        this.agc=agc;
        sendCommand("setRXAGC  "+agc);
    }

    public void setNR(boolean state) {
        sendCommand("setNR "+state);
    }
    
    public void setANF(boolean state) {
        sendCommand("setANF "+state);
    }
    
    public void setNB(boolean state) {
        sendCommand("setNB "+state);
    }
    
    public void setGain(int gain) {
        sendCommand("SetRXOutputGain "+gain);
    }
    
    public void getSpectrum() {
    	sendCommand("getSpectrum "+BUFFER_SIZE);
    }

    public int[] getSamples() {
    	return samples;
    }
    
    public int getMeter() {
    	return meter;
    }
    
    public int getSampleRate() {
    	return sampleRate;
    }
    
    public boolean isConnected() {
    	return connected;
    }
    
	public void setSpectrumView(SpectrumView spectrumView) {
		this.spectrumView=spectrumView;
	}
	
	public void setStatus(String message) {
		status=message;
	}
	
	public String getStatus() {
		return status;
	}
	
	private SpectrumView spectrumView;
	
	private static final int HEADER_SIZE=48;
	private static final int BUFFER_SIZE=480;
	private static final int AUDIO_BUFFER_SIZE=2000;
	
	private static final int SPECTRUM_BUFFER=0;
	private static final int AUDIO_BUFFER=1;
	
	private String server;
	private int port;
	private Socket socket;
	private InputStream inputStream;
	private OutputStream outputStream;
	private boolean running=true;
	private boolean connected=false;
	
	private long frequency;
    private int filterLow;
    private int filterHigh;
    private int mode;
    private int sampleRate;
    private int band;
    private int meter;
    private int agc;
    
    private int cwPitch=600;
    
    byte[] commandBuffer=new byte[32];
    
    private int[] samples=new int[BUFFER_SIZE];
    private int audioSampleCount;
    short[] decodedBuffer=new short[AUDIO_BUFFER_SIZE];
    
	private AudioTrack audioTrack;
	private String status=null;
	
	public static final int modeLSB=0;
    public static final int modeUSB=1;
    public static final int modeDSB=2;
    public static final int modeCWL=3;
    public static final int modeCWU=4;
    public static final int modeFMN=5;
    public static final int modeAM=6;
    public static final int modeDIGU=7;
    public static final int modeSPEC=8;
    public static final int modeDIGL=9;
    public static final int modeSAM=10;
    public static final int modeDRM=11;

	private String[] modes={"LSB","USB","DSB","CWL","CWU","FMN","AM","DIGU","SPEC","DIGL","SAM","DRM"};
	
    private static short[] aLawDecode=new short[256];
	
	static {

        for (int i = 0; i < 256; i++) {
            int input = i ^ 85;
            int mantissa = (input & 15) << 4;
            int segment = (input & 112) >> 4;
            int value = mantissa + 8;
            if (segment >= 1) {
                value += 256;
            }
            if (segment > 1) {
                value <<= (segment - 1);
            }
            if ((input & 128) == 0) {
                value = -value;
            }
            aLawDecode[i] = (short) value;
        }

    }
	
	
}
