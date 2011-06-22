/* resample.c

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006 by Frank Brickle, AB2KT and Bob McGwier, N4HY

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
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
rwmcgwier@comcast.net

or by paper mail at

The DTTS Microwave Society
6 Kathleen Place
Bridgewater, NJ 08807
*/



#include <common.h>

DttSP_EXP ResSt
newPolyPhaseFIR (int filterMemoryBuffLength,
		 int indexfiltMemBuf,
		 int interpFactor, int filterPhaseNum, int deciFactor)
{
  ResSt tmp;
  tmp = (ResSt) safealloc (1, sizeof (resampler), "PF Resampler");
  tmp->indexfiltMemBuf = indexfiltMemBuf;
  tmp->interpFactor = interpFactor;
  tmp->filterPhaseNum = filterPhaseNum;
  tmp->deciFactor = deciFactor;
  tmp->numFiltTaps = 31 * deciFactor;
  tmp->filterMemoryBuffLength =
    nblock2 (max (filterMemoryBuffLength, tmp->numFiltTaps));
  tmp->MASK = tmp->filterMemoryBuffLength - 1;
  tmp->filterMemoryBuff =
    (COMPLEX *) safealloc (tmp->filterMemoryBuffLength, sizeof (COMPLEX),
			   "Filter buff: resampler");
  tmp->filter =
    newFIR_Lowpass_REAL (0.45f / (REAL) deciFactor, (REAL) interpFactor,
			 31 * interpFactor);
  return tmp;
}

void
delPolyPhaseFIR (ResSt resst)
{
  if (resst)
    {
      delFIR_Lowpass_REAL (resst->filter);
      safefree ((char *) resst->filterMemoryBuff);
      safefree ((char *) resst);
    }
}

DttSP_EXP void
PolyPhaseFIR (ResSt resst)
/******************************************************************************
* CALLING PARAMETERS:
* Name          Use    Description
* ____          ___    ___________
* *input               pointer to input COMPLEX data array
* *output              pointer to output COMPLEX data array
* RealFIR filter           pointer to filter coefficients array
* *filterMemoryBuff    pointer to buffer used as filter memory. Initialized
*                      all data to 0 before 1st call.  length is calculated
*                      from numFiltTaps
* filterMemoryBuffLength length of filterMemoryBuff
* inputArrayLength     length of input array :note that "output" may differ
*                      in length
* numFiltTaps          number of filter taps in array "filtcoeff": <filterMemoryBuffLength
* indexfiltMemBuf     index to where next input sample is to be stored in
*                      "filterMemoryBuff",initalized 0 to before first call
* interpFactor         interpolation factor: output rate = input rate *
*                      "interpFactor" / "deciFactor".
* filterPhaseNum      filter phase number (index), initialized to 0 before
*                      first call
* deciFactor           decimation factor:
*                      output rate = (input rate * "interpFactor"/"deciFactor")
* numOutputSamples    number of output samples placed in array "output"
*
* CALLED BY:
*
* RETURN VALUE:
* Name      Description
* ____      ___________
* none
*
* DESCRIPTION: This function is used to change the sampling rate of the data.
*              The input is first upsampled to a multiple of the desired
*              sampling rate and then down sampled to the desired sampling rate.
*
*              Ex. If we desire a 7200 Hz sampling rate for a signal that has
*                  been sampled at 8000 Hz the signal can first be upsampled
*                  by a factor of 9 which brings it up to 72000 Hz and then
*                  down sampled by a factor of 10 which results in a sampling
*                  rate of 7200 Hz.
*
* NOTES:
*        Also, the "*filterMemoryBuff" MUST be 2^N REALs long. This
*        routine uses circular addressing on this buffer assuming that
*        it is 2^N REALs in length.
*
******************************************************************************/
{
/******************************************************************************
*               LOCAL VARIABLE DECLARATIONS
*******************************************************************************
* Type              Name                 Description
* ____              ____                 ___________                         */
  int i, j, k, jj;		/* counter variables */
  COMPLEX *outptr;

  resst->numOutputSamples = 0;


  for (i = 0; i < resst->inputArrayLength; i++)
    {

      /*
       * save data in circular buffer
       */

      resst->filterMemoryBuff[resst->indexfiltMemBuf] = resst->input[i];
      j = resst->indexfiltMemBuf;
      jj = j;


      /*
       * circular addressing
       */

      resst->indexfiltMemBuf = (resst->indexfiltMemBuf + 1) & resst->MASK;

      /*
       * loop through each filter phase: interpolate then decimate
       */

      while (resst->filterPhaseNum < resst->interpFactor)
	{
	  j = jj;
	  /*         output[*numOutputSamples] = 0.0; */
	  outptr = resst->output + resst->numOutputSamples;
	  *outptr = cxzero;

	  /*
	   * perform convolution
	   */

	  for (k = resst->filterPhaseNum; k < resst->numFiltTaps;
	       k += resst->interpFactor)
	    {
	      /*            output[*numOutputSamples] += filtcoeff[k]*filterMemoryBuff[j]; */
	      //*outptr += resst->filtcoeff[k]* resst->filterMemoryBuff[j];
	      *outptr =
		Cadd (*outptr,
		      Cscl (resst->filterMemoryBuff[j],
			    FIRtap (resst->filter, k)));


	      /*
	       * circular adressing
	       */

	      j = (j + resst->MASK) & resst->MASK;
	    }

	  /*
	   * scale the data
	   */

	  /*         output[*numOutputSamples]*=(REAL)interpFactor; */
	  *outptr = Cscl (*outptr, (REAL) resst->interpFactor);
	  resst->numOutputSamples += 1;

	  /*
	   * increment interpolation phase # by decimation factor
	   */

	  resst->filterPhaseNum += (resst->deciFactor);

	}			/* end while *filterPhaseNum < interpFactor */

      resst->filterPhaseNum -= resst->interpFactor;

    }				/* end for inputArrayLength */
}				/* end PolyPhaseFir */
