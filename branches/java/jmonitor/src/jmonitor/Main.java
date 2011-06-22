/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package jmonitor;

/**
 *
 * @author john
 */
public class Main {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {

        String server="192.168.1.58";
        int receiver=0;
        int limit=0;
        int i=0;
        while(i<args.length) {

            if(args[i].equalsIgnoreCase("--receiver")) {
                i++;
                if(i<args.length) {
                    try {
                        receiver=Integer.parseInt(args[i]);
                    } catch (NumberFormatException e) {
                        usage("Invalid receiver argument");
                    }
                } else {
                    usage("Missing receiver argument");
                }
            } else if(args[i].equalsIgnoreCase("--server")) {
                i++;
                if(i<args.length) {
                    server=args[i];
                } else {
                    usage("Missing server argument");
                }
            } else if (args[i].equalsIgnoreCase("--limit")) {
                i++;
                if (i < args.length) {
                    limit = Integer.parseInt(args[i]);
                } else {
                    usage("Missing limit argument");
                }
            }

            i++;
        }

        Audio audio = new Audio(server, receiver);

        Client client=new Client(server,receiver,audio,limit);
        client.start();
        client.setFrequency(7048000);
        client.setMode(0);
        client.setFilter(-2850,-150);
        client.setGain(30);
        
        MonitorFrame frame=new MonitorFrame(client);
        MonitorUpdateThread monitorUpdateThread=new MonitorUpdateThread(client,frame);
        
        frame.setVisible(true);

        monitorUpdateThread.start();
    }

    private static void usage(String error) {
        System.err.print("Error: "+error);
        System.err.print("Usage: java -jar jmonitor.jar [--server a.b.c.d] [--receiver r] [--limit s]");
        System.exit(1);
    }
}
