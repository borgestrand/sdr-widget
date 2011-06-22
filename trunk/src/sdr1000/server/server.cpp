#include <stdlib.h>
#include <getopt.h>
#include <sys/io.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>


#include "client.h"
#include "listener.h"
#include "receiver.h"
#include "sdr1000.h"

#include "hw_sdr1000.h"

static struct option long_options[] = {
    {"rfe",required_argument, 0, 0},
    {"pa",required_argument, 0, 1},
    {"lptaddr",required_argument, 0, 2},
    {"usb",required_argument, 0, 3},
    {"samplerate",required_argument, 0, 4},
    {"device",required_argument, 0, 5},
    {"input",required_argument, 0, 6},
    {"output",required_argument, 0, 7},
    {0,0, 0, 0},
};

static char* short_options="rplusdio";
static int option_index;

static int rfe=0;
static int pa=0;
static unsigned long lpt_addr=0x0378;
static int usb=0;

static SDR1000* sdr1000;

extern "C" void SDR1000_set_frequency(long frequency);

int process_args(int argc,char* argv[]) {
    int i;
    while((i=getopt_long(argc,argv,short_options,long_options,&option_index))!=EOF) {
        switch(i) {
            case 0: //rfe
                rfe=atoi(optarg);
                break;
            case 1: //pa
                pa=atoi(optarg);
                break;
            case 2: //lpt_addr
                lpt_addr=strtol(optarg,NULL,16);
                break;
            case 3: //usb
                usb=atoi(optarg);
                break;
            case 4: //samplerate
                sdr1000_set_sample_rate(atoi(optarg));
                break;
            case 5: //device
                sdr1000_set_device(optarg);
                break;
            case 6: //input
                sdr1000_set_input(optarg);
                break;
            case 7: //output
                sdr1000_set_output(optarg);
                break;
            default:
                fprintf(stderr,"Usage:\n");
                fprintf(stderr,"    sdr1000server [--rfe 0|1] [--pa 0|1] [--lpt_addr <hex addess>] [--usb 0|1]\n");
                return -1;
                break;
        }
    }
    return 0;
}

int main(int argc,char* argv[]) {
    int rc;
    rc=process_args(argc,argv);
    if(rc<0) exit(1);

    sdr1000=new SDR1000("sdr1000",rfe,pa,usb,lpt_addr);
    sdr1000->PowerOn();
    sdr1000->SetFreq(7.058);

    init_receivers();
    create_listener_thread();
    create_sdr1000_thread();

    while(1) {
        sleep(1000);
    }

}

void SDR1000_set_frequency(long frequency) {
    sdr1000->SetFreq((float)frequency/1000000.0);
}
