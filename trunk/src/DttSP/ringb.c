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

#include <stdlib.h>
#include <string.h>
#include <ringb.h>
#include <bufvec.h>

ringb_t *
ringb_create (size_t sz2)
{
	ringb_t *rb = (ringb_t *) safealloc (1, sizeof (ringb_t), "Ring creation");
	rb->buf = safealloc (1, sz2, "Ring buffer buf");
	rb->size = sz2;		// power-of-2-sized
	rb->mask = rb->size - 1;
	rb->wptr = rb->rptr = 0;
	return rb;
}

ringb_float_t *
ringb_float_create (size_t sz2)
{
	ringb_float_t *rb = (ringb_float_t *) safealloc (1, sizeof (ringb_float_t),
		"Float Ring creation");
	rb->buf =
		(float *) safealloc (1, sz2 * sizeof (float), "Ring buffer float buf");
	rb->size = sz2;		// power-of-2-sized
	rb->mask = rb->size - 1;
	rb->wptr = rb->rptr = 0;
	return rb;
}

ringb_t *
ringb_create_usemem (char *usemem, size_t sz2)
{
	ringb_t *rb = (ringb_t *) usemem;
	rb->buf = usemem + sizeof (ringb_t);
	rb->size = sz2;		// power-of-2-sized
	rb->mask = rb->size - 1;
	rb->wptr = rb->rptr = 0;
	return rb;
}


void
ringb_free (ringb_t * rb)
{
	safefree (rb->buf);
	safefree ((char *) rb);
}

void
ringb_float_free (ringb_float_t * rb)
{
	safefree ((void *) rb->buf);
	safefree ((char *) rb);
}

void
ringb_reset (ringb_t * rb)
{
	// NB not thread-safe
	rb->rptr = 0;
	rb->wptr = 0;
}

void
ringb_float_reset (ringb_float_t * rb)
{
	// NB not thread-safe
	rb->rptr = 0;
	rb->wptr = 0;
}

void
ringb_clear (ringb_t * rb, size_t nbytes)
{
	register size_t i;
	char zero = 0;
	for (i = 0; i < nbytes; i++)
		ringb_write (rb, &zero, 1);
}

void
ringb_float_clear (ringb_float_t * rb, size_t nfloats)
{
	register size_t i;
	float zero = 0.0f;
	for (i = 0; i < nfloats; i++)
		ringb_float_write (rb, &zero, 1);
}

void
ringb_restart (ringb_t * rb, size_t nbytes)
{
	ringb_reset (rb);
	ringb_clear (rb, nbytes);
}

void
ringb_float_restart (ringb_float_t * rb, size_t nfloats)
{
	ringb_float_reset (rb);
	ringb_float_clear (rb, nfloats);
}

size_t
ringb_read_space (const ringb_t * rb)
{
	size_t w = rb->wptr, r = rb->rptr;
	if (w > r)
		return w - r;
	else
		return (rb->size + w - r ) & rb->mask;
}

size_t
ringb_float_read_space (const ringb_float_t * rb)
{
	size_t w = rb->wptr, r = rb->rptr;
	if (w > r)
		return w - r;
	else
		return (rb->size + w - r ) & rb->mask;
}

size_t
ringb_write_space (const ringb_t * rb)
{
	size_t w = rb->wptr, r = rb->rptr;
	if (w > r)
		return ((rb->size + r - w) & rb->mask) - 1;
	else if (w < r)
		return r - w - 1;
	else
		return rb->size - 1;
}

size_t
ringb_float_write_space (const ringb_float_t * rb)
{
	size_t w = rb->wptr, r = rb->rptr;
	if (w > r)
		return ((rb->size + r - w) & rb->mask) - 1;
	else if (w < r)
		return r - w - 1;
	else
		return rb->size - 1;
}

size_t
ringb_read (ringb_t * rb, char *dest, size_t cnt)
{
	size_t volatile free_cnt, cnt2, to_read, n1, n2;
	if ((free_cnt = ringb_read_space (rb)) == 0)
		return 0;
	to_read = cnt > free_cnt ? free_cnt : cnt;
	if ((cnt2 = rb->rptr + to_read) > rb->size)
		n1 = rb->size - rb->rptr, n2 = cnt2 & rb->mask;
	else
		n1 = to_read, n2 = 0;
	memcpy (dest, &(rb->buf[rb->rptr]), n1);
	rb->rptr = (rb->rptr + n1) & rb->mask;
	if (n2)
	{
		memcpy (dest + n1, &(rb->buf[rb->rptr]), n2);
		rb->rptr = (rb->rptr + n2) & rb->mask;
	}
	return to_read;
}

size_t
ringb_float_read (ringb_float_t * rb, float *dest, size_t cnt)
{
	size_t volatile free_cnt, cnt2, to_read, n1, n2;
	if ((free_cnt = ringb_float_read_space (rb)) == 0)
		return 0;
	to_read = cnt > free_cnt ? free_cnt : cnt;
	if ((cnt2 = rb->rptr + to_read) > rb->size)
		n1 = rb->size - rb->rptr, n2 = cnt2 & rb->mask;
	else
		n1 = to_read, n2 = 0;
	memcpy (dest, &(rb->buf[rb->rptr]), n1 * sizeof (float));
	rb->rptr = (rb->rptr + n1) & rb->mask;
	if (n2)
	{
		memcpy (dest + n1, &(rb->buf[rb->rptr]), n2 * sizeof (float));
		rb->rptr = (rb->rptr + n2) & rb->mask;
	}
	return to_read;
}

size_t
ringb_peek (ringb_t * rb, char *dest, size_t cnt)
{
	size_t volatile free_cnt, cnt2, to_read, n1, n2, tmp_rptr;
	tmp_rptr = rb->rptr;
	if ((free_cnt = ringb_read_space (rb)) == 0)
		return 0;
	to_read = cnt > free_cnt ? free_cnt : cnt;
	if ((cnt2 = tmp_rptr + to_read) > rb->size)
		n1 = rb->size - tmp_rptr, n2 = cnt2 & rb->mask;
	else
		n1 = to_read, n2 = 0;
	memcpy (dest, &(rb->buf[tmp_rptr]), n1);
	tmp_rptr = (tmp_rptr + n1) & rb->mask;
	if (n2)
		memcpy (dest + n1, &(rb->buf[tmp_rptr]), n2);
	return to_read;
}

size_t
ringb_write (ringb_t * rb, const char *src, size_t cnt)
{
	size_t volatile free_cnt, cnt2, to_write, n1, n2;
	if ((free_cnt = ringb_write_space (rb)) == 0)
		return 0;
	to_write = cnt > free_cnt ? free_cnt : cnt;
	if ((cnt2 = rb->wptr + to_write) > rb->size)
		n1 = rb->size - rb->wptr, n2 = cnt2 & rb->mask;
	else
		n1 = to_write, n2 = 0;
	memcpy (&(rb->buf[rb->wptr]), src, n1);
	rb->wptr = (rb->wptr + n1) & rb->mask;
	if (n2)
	{
		memcpy (&(rb->buf[rb->wptr]), src + n1, n2);
		rb->wptr = (rb->wptr + n2) & rb->mask;
	}
	return to_write;
}

size_t
ringb_float_write (ringb_float_t * rb, const float *src, size_t cnt)
{
	size_t volatile free_cnt, cnt2, to_write, n1, n2;
	if ((free_cnt = ringb_float_write_space (rb)) == 0)
		return 0;
	to_write = cnt > free_cnt ? free_cnt : cnt;
	if ((cnt2 = rb->wptr + to_write) > rb->size)
		n1 = rb->size - rb->wptr, n2 = cnt2 & rb->mask;
	else
		n1 = to_write, n2 = 0;
	memcpy (&(rb->buf[rb->wptr]), src, n1 * sizeof (float));
	rb->wptr = (rb->wptr + n1) & rb->mask;
	if (n2)
	{
		memcpy (&(rb->buf[rb->wptr]), src + n1, n2 * sizeof (float));
		rb->wptr = (rb->wptr + n2) & rb->mask;
	}
	return to_write;
}

void
ringb_read_advance (ringb_t * rb, size_t cnt)
{
	rb->rptr = (rb->rptr + cnt) & rb->mask;
}

void
ringb_write_advance (ringb_t * rb, size_t cnt)
{
	rb->wptr = (rb->wptr + cnt) & rb->mask;
}

void
ringb_float_read_advance (ringb_float_t * rb, size_t cnt)
{
	rb->rptr = (rb->rptr + cnt) & rb->mask;
}

void
ringb_float_write_advance (ringb_float_t * rb, size_t cnt)
{
	rb->wptr = (rb->wptr + cnt) & rb->mask;
}

void
ringb_get_read_vector (const ringb_t * rb, ringb_data_t * vec)
{
	size_t volatile free_cnt, cnt2, w = rb->wptr, r = rb->rptr;
	if (w > r)
		free_cnt = w - r;
	else
		free_cnt = (w - r + rb->size) & rb->mask;
	if ((cnt2 = r + free_cnt) > rb->size)
	{
		vec[0].buf = &(rb->buf[r]), vec[0].len = rb->size - r;
		vec[1].buf = rb->buf, vec[1].len = cnt2 & rb->mask;
	}
	else
	{
		vec[0].buf = &(rb->buf[r]), vec[0].len = free_cnt;
		vec[1].len = 0;
	}
}

void
ringb_get_write_vector (const ringb_t * rb, ringb_data_t * vec)
{
	size_t volatile free_cnt, cnt2, w = rb->wptr, r = rb->rptr;
	if (w > r)
		free_cnt = ((r - w + rb->size) & rb->mask) - 1;
	else if (w < r)
		free_cnt = r - w - 1;
	else
		free_cnt = rb->size - 1;
	if ((cnt2 = w + free_cnt) > rb->size)
	{
		vec[0].buf = &(rb->buf[w]), vec[0].len = rb->size - w;
		vec[1].buf = rb->buf, vec[1].len = cnt2 & rb->mask;
	}
	else
	{
		vec[0].buf = &(rb->buf[w]), vec[0].len = free_cnt;
		vec[1].len = 0;
	}
}
