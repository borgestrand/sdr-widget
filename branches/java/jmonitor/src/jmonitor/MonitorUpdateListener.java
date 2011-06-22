/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package jmonitor;

/**
 *
 * @author john
 */
public interface MonitorUpdateListener {

    public void updateSamples(float[] samples, int filterLow, int filterHigh, int sampleRate);
    public void updateStatus();

}
