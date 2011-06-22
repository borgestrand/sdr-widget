/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package jmonitor;

/**
 *
 * @author john
 */
public class MonitorUpdateThread extends Thread {

    MonitorUpdateThread(Client client,MonitorUpdateListener listener) {
        this.client=client;
        this.listener=listener;
    }

    public void run() {
        System.err.println("MonitorUpdateThread.run");
        while(true) {
            client.getSpectrum(listener);
            try {
                sleep(1000/fps);
            } catch (InterruptedException e) {
                System.err.println("MonitorUpdateThread: InterruptedException: "+e.getMessage());
            }
        }
    }

    public void setFps(int fps) {
        this.fps=fps;
    }

    private Client client;
    private MonitorUpdateListener listener;
    private int fps=10;

}
