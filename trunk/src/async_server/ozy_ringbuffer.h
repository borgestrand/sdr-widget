/** 
* @file ozy_ringbuffer.h
* @brief Header files for the Ozy ringbuffer functions
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-01-05
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

#ifndef _OZY_RINGBUFFER_H
#define	_OZY_RINGBUFFER_H

#ifdef	__cplusplus
extern "C" {
#endif

/* --------------------------------------------------------------------------*/
/** 
* @brief ozy_ringbuffer
*/
struct ozy_ringbuffer {
        int size;
        int entries;
        unsigned char* buffer;
        int insert_index;
        int remove_index;
    };

    extern struct ozy_ringbuffer* ozy_output_buffer;

    extern struct ozy_ringbuffer* new_ozy_ringbuffer(int n);
    extern int ozy_ringbuffer_put(struct ozy_ringbuffer* buffer,unsigned char* c,int n);
    extern int ozy_ringbuffer_get(struct ozy_ringbuffer* buffer,unsigned char* c,int n);
    extern int ozy_ringbuffer_entries(struct ozy_ringbuffer* buffer) ;
    extern void create_ozy_ringbuffer(int n);

#ifdef	__cplusplus
}
#endif

#endif	/* _OZY_RINGBUFFER_H */

