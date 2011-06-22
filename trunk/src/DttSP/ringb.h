/*
  Memory-mapped ringbuffer
  Derived from jack/ringbuffer.h

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.
    
    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software 
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

    Original
    Copyright (C) 2000 Paul Davis
    Copyright (C) 2003 Rohan Drape

    Derived
    Copyright (C) 2004, 2005, 2006 by Frank Brickle, AB2KT and Bob McGwier, N4HY
*/

#ifndef _ringb_h
#define _ringb_h

#include <sys/types.h>

typedef struct
{
  char *buf;
  size_t len;
} ringb_data_t;

typedef struct
{
  char *buf;
  size_t wptr, rptr, size, mask;
} ringb_t;

typedef struct
{
  float *buf;
  size_t len;
} ringb_floatdata_t;

typedef struct
{
  float *buf;
  size_t wptr, rptr, size, mask;
} ringb_float_t;

/* Sets up a ringbuffer data structure of a specified size
 * in pre-allocated memory.
 * sz is requested ringbuffer size in bytes,
 * MUST be a power of 2.
 * pre-allocated memory must be large enough to
 * accommodate (ringb header + stipulated memory size).
 * return a pointer to a new ringb_t, if successful,
 * 0 otherwise.
 * Freeing is done by freeing the pointer.  */

extern ringb_t *ringb_create_usemem (char *usemem, size_t sz2);

/* Sets up a ringbuffer data structure of a specified size
 * and allocates the required memory.  
 * sz is requested ringbuffer size in bytes(floats),
 * MUST be a power of 2.
 * pre-allocated memory must be large enough to
 * accommodate (ringb header + stipulated memory size).
 * return a pointer to a new ringb_t, if successful,
 * 0 otherwise.
 * Freeing is done by calling ringb_free. */

extern ringb_t *ringb_create (size_t sz2);
extern ringb_float_t *ringb_float_create (size_t sz2);

/* free the ring buffers that are created without external
*  storage supplied */

extern void ringb_free (ringb_t * rb);
extern void ringb_float_free (ringb_float_t * rb);

/* Fill a data structure with a description of the current readable
 * data held in the ringbuffer.  This description is returned in a two
 * element array of ringb_data_t.  Two elements are needed
 * because the data to be read may be split across the end of the
 * ringbuffer.
 *
 * The first element will always contain a valid len field, which
 * may be zero or greater.  If the len field is non-zero, then data
 * can be read in a contiguous fashion using the address given in the
 * corresponding @a buf field.
 *
 * If the second element has a non-zero len field, then a second
 * contiguous stretch of data can be read from the address given in
 * its corresponding buf field.
 *
 * rb a pointer to the ringbuffer structure.
 * vec a pointer to a 2 element array of ringb_data_t. */

extern void ringb_get_read_vector (const ringb_t * rb, ringb_data_t * vec);

/* Fill a data structure with a description of the current writable
 * space in the ringbuffer.  The description is returned in a two
 * element array of ringb_data_t.  Two elements are needed
 * because the space available for writing may be split across the end
 * of the ringbuffer.
 * The first element will always contain a valid len field, which
 * may be zero or greater.  If the @a len field is non-zero, then data
 * can be written in a contiguous fashion using the address given in
 * the corresponding buf field.
 * If the second element has a non-zero len field, then a second
 * contiguous stretch of data can be written to the address given in
 * the corresponding buf field.
 * rb a pointer to the ringbuffer structure.
 * vec a pointer to a 2 element array of ringb_data_t. */

extern void ringb_get_write_vector (const ringb_t * rb, ringb_data_t * vec);

/*
 * Read data from the ringbuffer.
 * rb a pointer to the ringbuffer structure.
 * dest a pointer to a buffer where data read from the
 * ringbuffer will go.
 * cnt the number of bytes to read.
 *
 * return the number of bytes read, which may range from 0 to cnt. */

extern size_t ringb_read (ringb_t * rb, char *dest, size_t cnt);
extern size_t ringb_float_read (ringb_float_t * rb, float *dest, size_t cnt);

/* Read data from the ringbuffer. Opposed to ringb_read()
 * this function does not move the read pointer. Thus it's
 * a convenient way to inspect data in the ringbuffer in a
 * continous fashion. The price is that the data is copied
 * into a user provided buffer. For "raw" non-copy inspection
 * of the data in the ringbuffer use ringb_get_read_vector().
 * rb a pointer to the ringbuffer structure.
 * dest a pointer to a buffer where data read from the
 *   ringbuffer will go.
 * cnt the number of bytes to read.
 * return the number of bytes read, which may range from 0 to cnt.
 */

extern size_t ringb_peek (ringb_t * rb, char *dest, size_t cnt);

/* Advance the read pointer.
 * After data have been read from the ringbuffer using the pointers
 * returned by ringb_get_read_vector(), use this function to
 * advance the buffer pointers, making that space available for future
 * write operations.
 *
 * rb a pointer to the ringbuffer structure.
 * cnt the number of bytes read. */

extern void ringb_read_advance (ringb_t * rb, size_t cnt);
extern void ringb_float_read_advance (ringb_float_t * rb, size_t cnt);

/* Return the number of bytes available for reading.
 * rb a pointer to the ringbuffer structure.
 * return the number of bytes available to read. */

extern size_t ringb_read_space (const ringb_t * rb);
extern size_t ringb_float_read_space (const ringb_float_t * rb);

/* Reset the read and write pointers, making an empty buffer.
 * This is not thread safe. */

extern void ringb_reset (ringb_t * rb);
extern void ringb_float_reset (ringb_float_t * rb);

/* Write data into the ringbuffer.
 * rb a pointer to the ringbuffer structure.
 * src a pointer to the data to be written to the ringbuffer.
 * cnt the number of bytes(floats) to write.
 * return the number of bytes(floats) written, which may range from 0 to cnt */

extern size_t ringb_write (ringb_t * rb, const char *src, size_t cnt);
extern size_t ringb_float_write (ringb_float_t * rb, const float *src,
				 size_t cnt);

/* Advance the write pointer.
 * After data have been written the ringbuffer using the pointers
 * returned by ringb_get_write_vector(), use this function
 * to advance the buffer pointer, making the data available for future
 * read operations.
 * rb a pointer to the ringbuffer structure.
 * cnt the number of bytes written. */

extern void ringb_write_advance (ringb_t * rb, size_t cnt);
extern void ringb_float_write_advance (ringb_float_t * rb, size_t cnt);

/* Return the number of bytes(floats) available for writing.
 * rb a pointer to the ringbuffer structure.
 * return the amount of free space (in bytes) available for writing. */

extern size_t ringb_write_space (const ringb_t * rb);
extern size_t ringb_float_write_space (const ringb_float_t * rb);

/* Fill the ring buffer for nbytes at the beginning with zeros 
 * rb a pointer to the ring buffer structure
 * nbytes the number of bytes to be written */

extern void ringb_clear (ringb_t * rb, size_t nbytes);
extern void ringb_float_clear (ringb_float_t * rb, size_t nfloats);

/* Reset the read and write pointers, making an empty buffer.
 * This is not thread safe. 
 * Fill the ring buffer for nbytes at the beginning with zeros 
 * rb a pointer to the ring buffer structure
 * nbytes the number of bytes to be written */

extern void ringb_restart (ringb_t * rb, size_t nbytes);
extern void ringb_float_restart (ringb_float_t * rb, size_t nfloats);

#endif
