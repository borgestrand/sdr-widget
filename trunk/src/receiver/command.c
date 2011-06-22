/** 
* @file command.c
* @brief Write commands to DttSP
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-11
*/

/* Copyright (C) 
* 2009 - John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
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


//
// command.c
//
//

#include <gtk/gtk.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --------------------------------------------------------------------------*/
/** 
* @brief Update to Dttsp 
* 
* @param command
* @param log
* 
* @return 
*/
extern void do_update(char* command,FILE* log);

/* --------------------------------------------------------------------------*/
/** 
* @brief Write commands to DttSP
* 
* @param command
*/
void writeCommand(char *command) {
    char s[80];
//fprintf(stderr,"writeCommand: %s\n",command);
//    sprintf(s,"%s\n",command);
//    if(command!=NULL) {
//        do_update(s,NULL);
//    }
}
