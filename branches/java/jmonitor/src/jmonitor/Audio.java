/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package jmonitor;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.SourceDataLine;

/**
 *
 * @author john
 */
public class Audio {

    public Audio(String server,int receiver) {
        af = new AudioFormat((float) sampleRate, 16, 1, true, true);
        info = new DataLine.Info(SourceDataLine.class, af);
        try {
            source=(SourceDataLine) AudioSystem.getLine(info);
            source.open(af);
            source.start();
        } catch (Exception e) {
            System.err.println("Audio: "+e.toString());
        }
    }

    public void playAudioBuffer(byte[] buffer) {
        byte[] decodedBuffer=new byte[AUDIO_BUFFER_SIZE*2];
        aLawDecode(buffer,decodedBuffer);
        source.write(decodedBuffer,0,AUDIO_BUFFER_SIZE*2);
    }

    public void close() {
        source.close();
    }

    private void aLawDecode(byte[] buffer,byte[] decodedBuffer) {
        int i;
        short v;
        for (int inIx=48, outIx=0; inIx < buffer.length; inIx++) {
            i=buffer[inIx]&0xFF;
            v=decodetable[i];
            // assumes BIGENDIAN
            decodedBuffer[outIx++]=(byte)((v>>8)&0xFF);
            decodedBuffer[outIx++]=(byte)(v&0xFF);
        }
    }

    private static final int AUDIO_BUFFER_SIZE=480;

    private static final int sampleRate=8000;

    private AudioFormat af;
    private DataLine.Info info;
    private SourceDataLine source;
    
    private static short[] decodetable=new short[256];

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
            decodetable[i] = (short) value;
        }

    }

}
