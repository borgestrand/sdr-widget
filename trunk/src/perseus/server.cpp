/**
* @file server.c
* @brief HPSDR server application
* @author John Melton, G0ORX/N6LYT
* @version 0.1
* @date 2009-10-13
*/


/* Copyright (C)
* 2009 - John Melton, G0ORX/N6LYT
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#include <boost/program_options.hpp>
using namespace boost::program_options;
#include <iostream>
using namespace std;

#include "client.h"
#include "listener.h"
#include "perseus-sdr.h"
#include "receiver.h"


struct PerseusConfig perseusConfig;



int main(int argc,char* argv[]) {

	int num_perseus;
    int debug_level;
	//eeprom_prodid prodid;


	// Set debug info default dumped to stderr to the minimum verbose level
	perseus_set_debug(0);

    try {

        options_description desc("Allowed options");
        desc.add_options()
        // First parameter describes option name/short name
        // The second is parameter to option
        // The third is description
        ("help,h",                                                                     "print usage message")
        ("debug,d",      value<int>(&debug_level)->default_value(0),                   "debug level")

        ("receivers,r",  value<int>(&perseusConfig.nrx)->default_value(1),             "# of receivers")
        ("samplerate,s", value<int>(&perseusConfig.sample_rate)->default_value(95000), "samplerate (95000 | 125000 | 250000 )")

        ("dither,t",     value<bool>(&perseusConfig.dither)->default_value(false),     "dither on|off")
        ("preamp,p",     value<bool>(&perseusConfig.preamp)->default_value(false),     "preamplifier on|off")
        ("randomizer,z", value<bool>(&perseusConfig.random)->default_value(false),     "randomizer on|off")
        ;
    
        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);    

        if (vm.count("help")) {  
            cout << desc << "\n";
            return 0;
        }
        
        cout << "DEBUG:      " << debug_level               << "\n";
        cout << "SAMPLERATE: " << perseusConfig.sample_rate << "\n";
        cout << "RECEIVER:   " << perseusConfig.nrx         << "\n";
        cout << "DITHER:     " << perseusConfig.dither      << "\n";
        cout << "PREAMP:     " << perseusConfig.preamp      << "\n";
        cout << "RANDOMIZER: " << perseusConfig.random      << "\n";

        perseus_set_debug(debug_level);


        // Check how many Perseus receivers are connected to the system
        num_perseus = perseus_init();
        if (num_perseus==0) {
            cout << "No Perseus receiver(s) detected" << endl;
            perseus_exit();
            return 0;
        } else {
            cout << num_perseus << " Perseus receiver(s) found" << endl;
            if (num_perseus < perseusConfig.nrx) {

                return ~0;
            }
        }
        cout << "Press q <ENTER> to exit." << endl;
        init_receivers (&perseusConfig);

        create_listener_thread();

        //create_ozy_thread();

        char ch;
        while((ch = getc(stdin)) != EOF) {
            if (ch == 'q') {
                break;
            }
        }
        return 0;
    } 
    catch(exception& e) {
        cerr << e.what() << "\n";
    }
}




#if 0
void process_args(int argc,char* argv[], perseus_descr *pPd) {
    static struct option long_options[] = {
        {"receivers",optional_argument,  0,  0 },
        {"samplerate",required_argument, 0,  1 },
        {"dither",required_argument,     0,  2 },
        {"random",required_argument,     0,  3 },
        {"preamp",required_argument,     0,  4 },
        {"debug",required_argument,      0, 'd'},
    };
    static const char* short_options = "rs:d:";
    static int option_index;

    int i, debug_level;

    // set defaults
    //ozy_set_receivers(1);
    //ozy_set_sample_rate(96000);

    while((i=getopt_long(argc,argv,short_options,long_options,&option_index))!=EOF) {
        switch(i) {
            case 'd':
                if (sscanf(optarg, "%d", &debug_level) == 1 && (debug_level >= 0) && (debug_level <=3)) {
                    perseus_set_debug(debug_level);
                }
                break;
            case 0: // receivers
                //ozy_set_receivers(atoi(optarg));
                break;
            case 1: // sample rate
                perseusConfig.sample_rate = (atoi(optarg));
                printf("%d\n", perseusConfig.sample_rate);
                break;
            case 2: // dither
                if(strcmp(optarg,"off")==0) {
                    //ozy_set_dither(0);
                } else if(strcmp(optarg,"on")==0) {
                    //ozy_set_dither(1);
                } else {
                    fprintf(stderr,"invalid dither option\n");
                }
                break;
            case 3: // random
                if(strcmp(optarg,"off")==0) {
                    //ozy_set_random(0);
                } else if(strcmp(optarg,"on")==0) {
                    //ozy_set_random(1);
                } else {
                    fprintf(stderr,"invalid random option\n");
                }
                break;
            case 4: // preamp
                if(strcmp(optarg,"off")==0) {
                    //ozy_set_preamp(0);
                } else if(strcmp(optarg,"on")==0) {
                    //ozy_set_preamp(1);
                } else {
                    fprintf(stderr,"invalid preamp option\n");
                }
                break;
       //   case 5: // 10 MHz clock source
       //       if(strcmp(optarg,"atlas")==0) {
       //           ozy_set_10mhzsource(0);
       //       } else if(strcmp(optarg,"penelope")==0) {
       //           ozy_set_10mhzsource(1);
       //       } else if(strcmp(optarg,"mercury")==0) {
       //           ozy_set_10mhzsource(2);
       //       } else {
       //           fprintf(stderr,"invalid 10 MHz clock option\n");
       //       }
       //       break;
       //   case 6: // 122.88 MHz clock source
       //       if(strcmp(optarg,"penelope")==0) {
       //           ozy_set_122_88mhzsource(0);
       //       } else if(strcmp(optarg,"mercury")==0) {
       //           ozy_set_122_88mhzsource(1);
       //       } else {
       //           fprintf(stderr,"invalid 122.88 MHz clock option\n");
       //       }
       //       break;
       //   case 7: // mic source
       //       if(strcmp(optarg,"janus")==0) {
       //           ozy_set_micsource(0);
       //       } else if(strcmp(optarg,"penelope")==0) {
       //           ozy_set_micsource(1);
       //       } else {
       //           fprintf(stderr,"invalid mic source option\n");
       //       }
       //       break;
       //   case 8: // class
       //       if(strcmp(optarg,"other")==0) {
       //           ozy_set_class(0);
       //       } else if(strcmp(optarg,"E")==0) {
       //           ozy_set_class(1);
       //       } else {
       //           fprintf(stderr,"invalid class option\n");
       //       }
       //       break;
       //
       //   case 9: // timing
       //       ozy_set_timing(1);
       //       break;
       //
            default:
                fprintf(stderr,"Usage: \n");
                fprintf(stderr,"  server --receivers N --samplerate 48000|96000|192000\n");
                exit(~0);
               
        }
    }
}

#endif


