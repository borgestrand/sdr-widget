/** 
* @file spectrum_buffers.h
* @brief Header files for the spectrum buffer functions
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-1-12
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

#ifndef _SPECTRUM_BUFFERS_H
#define	_SPECTRUM_BUFFERS_H

#ifdef	__cplusplus
extern "C" {
#endif
    #include <semaphore.h>

    #define SPECTRUM_BUFFER_SIZE 8192

/* --------------------------------------------------------------------------*/
/** 
* @brief spectrum_buffer
*/
    struct spectrum_buffer {
        struct spectrum_buffer* next;
        int sequence;
        int size;
        char buffer[SPECTRUM_BUFFER_SIZE];
    };

    void put_spectrum_input_buffer(struct spectrum_buffer* buffer);
    struct spectrum_buffer* get_spectrum_input_buffer(void);

    struct spectrum_buffer* new_spectrum_buffer();
    void free_spectrum_buffer(struct spectrum_buffer* buffer);

    void create_spectrum_buffers(int n);
    struct spectrum_buffer* get_spectrum_free_buffer(void);

    sem_t* spectrum_input_buffer_sem;

#ifdef	__cplusplus
}
#endif

#endif	/* _SPECTRUM_BUFFERS_H */

