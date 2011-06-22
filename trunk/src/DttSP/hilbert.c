/* hilbert.c

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006, 2007, 2008 by Frank Brickle, AB2KT and Bob McGwier, N4HY

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

The authors can be reached by email at

ab2kt@arrl.net
or
rwmcgwier@gmail.com

or by paper mail at

The DTTS Microwave Society
6 Kathleen Place
Bridgewater, NJ 08807
*/

#include <hilbert.h>

// cf "Musical Engineer's Handbook" by Bernie Hutchins

PRIVATE REAL
pole[12] = {
   0.3609f, 2.7412f, 11.1573f, 44.7581f, 179.6242f,  798.4578f,
   1.2524f, 5.5671f, 22.3423f, 89.6271f, 364.7914f, 2770.1114f
};

Hilbert
newHilbert(CXB ibuf, CXB obuf, REAL rate) {
  Hilbert h = (Hilbert) safealloc(1, sizeof(HilbertInfo), "Hilbert Transformer");
  h->size = CXBsize(ibuf);
  h->c  = newvec_REAL(12, "Hilbert Transformer c vector");
  h->x1 = newvec_REAL(12, "Hilbert Transformer x1 vector");
  h->y1 = newvec_REAL(12, "Hilbert Transformer y1 vector");
  {
    int i;
    for (i = 0; i < 12; i++) {
      REAL u = (REAL)(pole[i] * M_PI * 15.0 * rate);
      h->c[i] = (REAL)((u - 1.0) / (u + 1.0));
      h->x1[i] = h->y1[i] = 0.0;
    }
  }
  h->buf.i = newCXB(h->size, CXBbase(ibuf), "Hilbert Transformer input buffer");
  h->buf.o = newCXB(h->size, CXBbase(obuf), "Hilbert Transformer output buffer");
  return h;
}

Hilsim
newHilbertsim(CXB ibuf, CXB obuf)
{
  Hilsim h;
  h = (Hilsim) safealloc(1, sizeof(HilsimInfo), "Hilbert Transformer");
  h->size = CXBsize(ibuf);
  h->buf.i = newCXB(h->size, CXBbase(ibuf), "Hilbert Transformer input buffer");
  h->buf.o = newCXB(h->size, CXBbase(obuf), "Hilbert Transformer output buffer");
  memset(h->d,0,sizeof(REAL)*6);
  memset(h->y,0,sizeof(REAL)*6);
  memset(h->x,0,sizeof(REAL)*4);
  h->invert = TRUE;
  return h;
}
void
delHilbert(Hilbert h) {
  if (h) {
    delvec_REAL(h->c);
    delvec_REAL(h->x1);
    delvec_REAL(h->y1);
    delCXB(h->buf.i);
    delCXB(h->buf.o);
    safefree((char *) h);
  }
}

void delHilsim(Hilsim h)
{
	if (h) {
		delCXB(h->buf.i);
		delCXB(h->buf.o);
		safefree((char *)h);
	}
}

void
hilbert_transform(Hilbert h) {
  REAL xn1, xn2, yn1, yn2;
  int i;

  for (i = 0; i < h->size; i++) {
    int j;

    xn1 = xn2 = CXBreal(h->buf.i, i);

    for (j = 0; j < 6; j++) {
      yn1 = h->c[j] * (xn1 - h->y1[j]) + h->x1[j];
      h->x1[j] = xn1;
      h->y1[j] = yn1;
      xn1 = yn1;
    }
    
    for (j = 6; j < 12; j++) {
      yn2 = h->c[j] * (xn2 - h->y1[j]) + h->x1[j];
      h->x1[j] = xn2;
      h->y1[j] = yn2;
      xn2 = yn2;
    }
    
    CXBdata(h->buf.o, i) = Cmplx(yn2, yn1);
  }
}
 
void
hilsim_transform(Hilsim h) {
  REAL *x = h->x,
       *y = h->y,
       *d = h->d;
  int i;
  
  for (i = 0; i < h->size; i++)
  {
    REAL xin = CXBreal(h->buf.i, i);
    
    x[0] = d[1] - xin;
    x[1] = d[0] - x[0] * 0.00196f;
    x[2] = d[3] - x[1];
    x[3] = d[1] + x[2] * 0.737f;
    
    d[1] = x[1];
    d[3] = x[3];
    
    y[0] = d[2] - xin;
    y[1] = d[0] + y[0] * 0.924f;
    y[2] = d[4] - y[1];
    y[3] = d[2] + y[2] * 0.439f;
    y[4] = d[5] - y[3];
    y[5] = d[4] - y[4] * 0.586f;
    
    d[2] = y[1];
    d[4] = y[3];
    d[5] = y[5];
    
    d[0] = xin;
    
    CXBdata(h->buf.o, i) = Cmplx(x[3], y[5]);
  }
}
 
/*
(defstruct (hilfil (:conc-name hilf-)) x y d)

(defun new-hilf ()
  (make-hilfil :x (make-array 4 :initial-element 0.0)
	       :y (make-array 6 :initial-element 0.0)
	       :d (make-array 6 :initial-element 0.0)))

(defun hilfilt (xin hilf)
  (let ((x (hilf-x hilf))
	(y (hilf-y hilf))
	(d (hilf-d hilf)))
    (setf (aref x 0) (- (aref d 1) xin)
	  (aref x 1) (- (aref d 0) (* (aref x 0) 0.00196))
	  (aref x 2) (- (aref d 3) (aref x 1))
	  (aref x 3) (+ (aref d 1) (* (aref x 2) 0.737))
	  (aref d 1) (aref x 1)
	  (aref d 3) (aref x 3)
	  (aref y 0) (- (aref d 2) xin)
	  (aref y 1) (+ (aref d 0) (* (aref y 0) 0.924))
	  (aref y 2) (- (aref d 4) (aref y 1))
	  (aref y 3) (+ (aref d 2) (* (aref y 2) 0.439))
	  (aref y 4) (- (aref d 5) (aref y 3))
	  (aref y 5) (- (aref d 4) (* (aref y 4) 0.586))
	  (aref d 5) (aref y 5)
	  (aref d 4) (aref y 3)
	  (aref d 2) (aref y 1)
	  (aref d 0) xin)
    (values (aref x 3) (aref y 5))))

 */  
