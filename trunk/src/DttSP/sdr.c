/* sdr.c

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006 by Frank Brickle, AB2KT and Bob McGwier, N4HY.

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

//========================================================================
/* initialization and termination */

void
reset_meters (unsigned int thread)
{
	if (uni[thread].meter.flag)
	{                           // reset metering completely
		int i, k;
		for (i = 0; i < RXMETERPTS; i++)
			for (k = 0; k < MAXRX; k++)
				uni[thread].meter.rx.val[k][i] = -200.0;
		for (i = 0; i < TXMETERPTS; i++)
			uni[thread].meter.tx.val[i] = -200.0;
	}
}

void
reset_spectrum (unsigned int thread)
{
	if (uni[thread].spec.flag)
		reinit_spectrum (&uni[thread].spec);
}

void
reset_counters (unsigned thread)
{
	int k;
	for (k = 0; k < uni[thread].multirx.nrx; k++)
		rx[thread][k].tick = 0;
	tx[thread].tick = 0;
}

//========================================================================

/* global and general info,
   not specifically attached to
   tx, rx, or scheduling */

PRIVATE void
setup_all (REAL rate, int buflen, SDRMODE mode, char *wisdom,
	int specsize, int numrecv, int cpdsize, unsigned int thread)
{
	uni[thread].samplerate = rate;
	uni[thread].buflen = buflen;
	uni[thread].mode.sdr = mode;
	if (thread != 1) uni[thread].mode.trx = RX;
	else uni[thread].mode.trx = TX;

	uni[thread].wisdom.path = wisdom;
	uni[thread].wisdom.bits = FFTW_ESTIMATE;
	{
		FILE *f = fopen (uni[thread].wisdom.path, "r");
		if (f)
		{
			char wisdomstring[32768];
			fread(wisdomstring,1,32768,f);
			if (fftwf_import_wisdom_from_string (wisdomstring) != 0)
				uni[thread].wisdom.bits = FFTW_MEASURE;
			fclose (f);
		}
	}

	if (uni[thread].meter.flag)
	{
		reset_meters (thread);
	}

	uni[thread].spec.rxk = 0;
	uni[thread].spec.buflen = uni[thread].buflen;
	uni[thread].spec.scale = SPEC_PWR;
	uni[thread].spec.type = SPEC_POST_FILT;
	uni[thread].spec.size = specsize;
	uni[thread].spec.planbits = uni[thread].wisdom.bits;
	init_spectrum (&uni[thread].spec);
	//fprintf(stderr,"Created spectrum\n"),fflush(stderr);

	// set which receiver is listening to commands
	uni[thread].multirx.lis = 0;
	uni[thread].multirx.nrx = numrecv;

	// set mixing of input from aux ports
	uni[thread].mix.rx.flag = uni[thread].mix.tx.flag = FALSE;
	uni[thread].mix.rx.gain = uni[thread].mix.tx.gain = 1.0;

	uni[thread].cpdlen = cpdsize;

	uni[thread].tick = uni[thread].oldtick = 0;
}

/* purely rx */

PRIVATE void
setup_rx (int k, unsigned int thread)
{
	/* conditioning */
	if (thread == 0) {
		diversity.gain = 1.0;
		diversity.scalar = Cmplx(1.0,0);
	}
	rx[thread][k].iqfix = newCorrectIQ (0.0, 1.0, 0.000f);
	// Remove the next line
	rx[thread][k].iqfix->wbir_state = JustSayNo;
	// Remove the previous line
	rx[thread][k].filt.coef = newFIR_Bandpass_COMPLEX (150.0,
		2850.0,	uni[thread].samplerate, uni[thread].buflen + 1);
	rx[thread][k].filt.ovsv =
		newFiltOvSv (FIRcoef (rx[thread][k].filt.coef), FIRsize (rx[thread][k].filt.coef),
		uni[thread].wisdom.bits);
	rx[thread][k].resample.flag = FALSE;
	normalize_vec_COMPLEX (rx[thread][k].filt.ovsv->zfvec, rx[thread][k].filt.ovsv->fftlen, rx[thread][k].filt.ovsv->scale);

	rx[thread][k].output_gain=1.0f;

	// hack for EQ
	rx[thread][k].filt.save =
		newvec_COMPLEX (rx[thread][k].filt.ovsv->fftlen, "RX filter cache");
	memcpy ((char *) rx[thread][k].filt.save, (char *) rx[thread][k].filt.ovsv->zfvec,
		rx[thread][k].filt.ovsv->fftlen * sizeof (COMPLEX));

	/* buffers */
	/* note we overload the internal filter buffers
	we just created */
	rx[thread][k].buf.i = newCXB (FiltOvSv_fetchsize (rx[thread][k].filt.ovsv),
		FiltOvSv_fetchpoint (rx[thread][k].filt.ovsv),
		"init rx[thread][k].buf.i");

	rx[thread][k].buf.o = newCXB (FiltOvSv_storesize (rx[thread][k].filt.ovsv),
		FiltOvSv_storepoint (rx[thread][k].filt.ovsv),
		"init rx[thread][k].buf.o");

	rx[thread][k].dcb = newDCBlocker(DCB_SINGLE_POLE, rx[thread][k].buf.i);
	rx[thread][k].dcb->flag = FALSE;

	/* conversion */
	rx[thread][k].osc.freq = -9000.0;
	rx[thread][k].osc.phase = 0.0;
	rx[thread][k].osc.gen = newOSC (uni[thread].buflen,
		ComplexTone,
		rx[thread][k].osc.freq,
		rx[thread][k].osc.phase,
		uni[thread].samplerate, "SDR RX Oscillator");

	rx[thread][k].dttspagc.gen = newDttSPAgc (
		agcMED,							// mode kept around for control reasons alone
		CXBbase (rx[thread][k].buf.o),	// buffer pointer
		CXBsize (rx[thread][k].buf.o),	// buffer size
		1.0f,							// Target output
		2.0f,							// Attack time constant in ms
		250,							// Decay time constant in ms
		1.0,							// Slope
		250,							// Hangtime in ms
		uni[thread].samplerate,			// Sample rate
		31622.8f,						// Maximum gain as a multipler, linear not dB
		0.00001f,						// Minimum gain as a multipler, linear not dB
		1.0,							// Set the current gain
		"AGC");							// Set a tag for an error message if the memory allocation fails		

	rx[thread][k].dttspagc.flag = TRUE;

	rx[thread][k].grapheq.gen = new_EQ (rx[thread][k].buf.o, uni[thread].samplerate, uni[thread].wisdom.bits);
	rx[thread][k].grapheq.flag = FALSE;

	/* demods */
	rx[thread][k].am.gen = newAMD (
		uni[thread].samplerate,			// REAL samprate
		0.0,							// REAL f_initial
		-2000.0,						// REAL f_lobound,
		2000.0,							// REAL f_hibound,
		300.0,							// REAL f_bandwid,
		CXBsize (rx[thread][k].buf.o),	// int size,
		CXBbase (rx[thread][k].buf.o),	// COMPLEX *ivec,
		CXBbase (rx[thread][k].buf.o),	// COMPLEX *ovec,
		AMdet,							// AM Mode AMdet == rectifier,
		//         SAMdet == synchronous detector
		"AM detector blew");   // char *tag

	rx[thread][k].fm.gen = newFMD (
		uni[thread].samplerate,			// REAL samprate
		0.0,							// REAL f_initial
		-8000.0,						// REAL f_lobound
		8000.0,							// REAL f_hibound
		16000.0,						// REAL f_bandwid
		CXBsize (rx[thread][k].buf.o),	// int size
		CXBbase (rx[thread][k].buf.o),	// COMPLEX *ivec
		CXBbase (rx[thread][k].buf.o),	// COMPLEX *ovec
		"New FM Demod structure");		// char *error message;

	/* noise reduction */
	rx[thread][k].anf.gen = new_lmsr (
		rx[thread][k].buf.o,	// CXB signal,
		64,						// int delay,
		0.01f,					// REAL adaptation_rate,
		0.000001f,				// REAL leakage,
		45,						// int adaptive_filter_size,
		LMADF_INTERFERENCE);
	rx[thread][k].anf.flag = FALSE;

	rx[thread][k].banf.gen = new_blms(
		rx[thread][k].buf.o,    // CXB signal,
		0.01f,				// REAL adaptation_rate,
		0.00000f,				// REAL leakage,
		LMADF_INTERFERENCE,		// type
		uni->wisdom.bits);      // fftw wisdom
	rx[thread][k].banf.flag = FALSE;

	rx[thread][k].anr.gen = new_lmsr (
		rx[thread][k].buf.o,	// CXB signal,
		40,						// int delay,
		0.00015f,				// REAL adaptation_rate,
		0.000001f,				// REAL leakage,
		30,						// int adaptive_filter_size,
		LMADF_NOISE);
	rx[thread][k].anr.flag = FALSE;

	rx[thread][k].banr.gen = new_blms(
		rx[thread][k].buf.o,    // CXB signal,
		0.001f,					// REAL adaptation_rate,
		0.000001f,				// REAL leakage,
		LMADF_NOISE,			// type
		uni->wisdom.bits);      // fftw wisdom
	rx[thread][k].banr.flag = FALSE;


	rx[thread][k].nb.thresh = 3.3f;
	rx[thread][k].nb.gen = new_noiseblanker (rx[thread][k].buf.i, rx[thread][k].nb.thresh);
	rx[thread][k].nb.flag = FALSE;

	rx[thread][k].nb_sdrom.thresh = 2.5f;
	rx[thread][k].nb_sdrom.gen = new_noiseblanker (rx[thread][k].buf.i, rx[thread][k].nb_sdrom.thresh);
	rx[thread][k].nb_sdrom.flag = FALSE;

	rx[thread][k].spot.gen = newSpotToneGen (
		-12.0,						// gain
		700.0,						// freq
		5.0,						// ms rise
		5.0,						// ms fall
		uni[thread].buflen,			// length of spot tone buffer
		uni[thread].samplerate);	// sample rate

	memset ((char *) &rx[thread][k].squelch, 0, sizeof (rx[thread][k].squelch));
	rx[thread][k].squelch.thresh = -150.0;
	rx[thread][k].squelch.power = 0.0;
	rx[thread][k].squelch.flag = rx[thread][k].squelch.running = rx[thread][k].squelch.set = FALSE;
	rx[thread][k].squelch.num = uni[thread].buflen - 48;

	rx[thread][k].cpd.gen = newWSCompander (uni[thread].cpdlen, 0.0, rx[thread][k].buf.o);
	rx[thread][k].cpd.flag = FALSE;

	rx[thread][k].mode = uni[thread].mode.sdr;
	rx[thread][k].bin.flag = FALSE;

	{
		REAL pos = 0.5,             // 0 <= pos <= 1, left->right
		theta = (REAL) ((1.0 - pos) * M_PI / 2.0);
		rx[thread][k].azim = Cmplx ((REAL) cos (theta), (IMAG) sin (theta));
	}

	rx[thread][k].tick = 0;
}

/* purely tx */

PRIVATE void
setup_tx (unsigned int thread)
{
	/* conditioning */
	tx[thread].iqfix = newCorrectIQ (0.0, 1.0, 0.0);
	tx[thread].filt.coef = newFIR_Bandpass_COMPLEX (300.0, 3000.0, 
		uni[thread].samplerate, uni[thread].buflen + 1);
	tx[thread].filt.ovsv = newFiltOvSv (FIRcoef (tx[thread].filt.coef),
		FIRsize (tx[thread].filt.coef), uni[thread].wisdom.bits);
	tx[thread].filt.ovsv_pre = newFiltOvSv (FIRcoef (tx[thread].filt.coef),
		FIRsize (tx[thread].filt.coef), uni[thread].wisdom.bits);
	normalize_vec_COMPLEX (tx[thread].filt.ovsv->zfvec, tx[thread].filt.ovsv->fftlen,tx[thread].filt.ovsv->scale);

	// hack for EQ
	tx[thread].filt.save = newvec_COMPLEX (tx[thread].filt.ovsv->fftlen, "TX filter cache");
	memcpy ((char *) tx[thread].filt.save,
		(char *) tx[thread].filt.ovsv->zfvec,
		tx[thread].filt.ovsv->fftlen * sizeof (COMPLEX));

	/* buffers */
	tx[thread].buf.i = newCXB (FiltOvSv_fetchsize (tx[thread].filt.ovsv),
		FiltOvSv_fetchpoint (tx[thread].filt.ovsv), "init tx[thread].buf.i");
	tx[thread].buf.o = newCXB (FiltOvSv_storesize (tx[thread].filt.ovsv),
		FiltOvSv_storepoint (tx[thread].filt.ovsv), "init tx[thread].buf.o");
	tx[thread].buf.ic = newCXB (FiltOvSv_fetchsize (tx[thread].filt.ovsv_pre),
		FiltOvSv_fetchpoint (tx[thread].filt.ovsv_pre), "init tx[thread].buf.ic");
	tx[thread].buf.oc = newCXB (FiltOvSv_storesize (tx[thread].filt.ovsv_pre),
		FiltOvSv_storepoint (tx[thread].filt.ovsv_pre), "init tx[thread].buf.oc");

	tx[thread].dcb.flag = FALSE;
	tx[thread].dcb.gen = newDCBlocker (DCB_MED, tx[thread].buf.i);

	/* conversion */
	tx[thread].osc.freq = 0.0;
	tx[thread].osc.phase = 0.0;
	tx[thread].osc.gen = newOSC (uni[thread].buflen,
		ComplexTone,
		tx[thread].osc.freq,
		tx[thread].osc.phase, uni[thread].samplerate, "SDR TX Oscillator");

	tx[thread].am.carrier_level = 0.5f;
	tx[thread].fm.cvtmod2freq = (REAL) (3000.0 * TWOPI / uni[thread].samplerate); //3 kHz deviation

	tx[thread].leveler.gen = newDttSPAgc (
		1,							// mode kept around for control reasons
		CXBbase (tx[thread].buf.i),	// input buffer
		CXBsize (tx[thread].buf.i),	// output buffer
		1.1f,						// Target output
		2,							// Attack time constant in ms
		500,						// Decay time constant in ms
		1,							// Slope
		500,						//Hangtime in ms
		uni[thread].samplerate,		// Sample rate
		1.778f,						// Maximum gain as a multipler, linear not dB
		1.0,						// Minimum gain as a multipler, linear not dB
		1.0,						// Set the current gain
		"LVL");						// Set a tag for an error message if the memory allocation fails
	tx[thread].leveler.flag = TRUE;

	tx[thread].grapheq.gen = new_EQ (tx[thread].buf.i, uni[thread].samplerate, uni[thread].wisdom.bits);
	tx[thread].grapheq.flag = FALSE;

	memset ((char *) &tx[thread].squelch, 0, sizeof (tx[thread].squelch));
	tx[thread].squelch.thresh = -40.0;
	tx[thread].squelch.atten = 80.0;
	tx[thread].squelch.power = 0.0;
	tx[thread].squelch.flag = FALSE;
	tx[thread].squelch.running = tx[thread].squelch.set = FALSE;
	tx[thread].squelch.num = uni[thread].buflen - 48;

	tx[thread].alc.gen = newDttSPAgc (
		1,								// mode kept around for control reasons alone
		CXBbase (tx[thread].buf.i),		// input buffer
		CXBsize (tx[thread].buf.i),		// output buffer
		1.08f,							// Target output
		2,								// Attack time constant in ms
		10,								// Decay time constant in ms
		1,								// Slope
		500,							// Hangtime in ms
		uni[thread].samplerate, 1.0,	// Maximum gain as a multipler, linear not dB
		.000001f,						// Minimum gain as a multipler, linear not dB
		1.0,							// Set the current gain
		"ALC");							// Set a tag for an error message if the memory allocation fails
	tx[thread].alc.flag = TRUE;

	tx[thread].spr.gen =
		newSpeechProc (0.4f, 3.0, CXBbase (tx[thread].buf.i), CXBsize (tx[thread].buf.o));
	tx[thread].spr.flag = FALSE;

	tx[thread].cpd.gen = newWSCompander (uni[thread].cpdlen, (REAL)-0.1, tx[thread].buf.i);
	tx[thread].cpd.flag = FALSE;

	tx[thread].hlb.gen = newHilbertsim(tx[thread].buf.i, tx[thread].buf.i);
	tx[thread].hlb.flag = TRUE;

	//tx[thread].scl.dc = cxzero;

	tx[thread].mode = uni[thread].mode.sdr;

	tx[thread].tick = 0;
	/* not much else to do for TX */
}

/* how the outside world sees it */

void
setup_workspace (REAL rate, int buflen, SDRMODE mode,
                 char *wisdom, int specsize, int numrecv, int cpdsize, unsigned int thread)
{
	int k;

	setup_all (rate, buflen, mode, wisdom, specsize, numrecv, cpdsize, thread);

	for (k = 0; k < uni[thread].multirx.nrx; k++)
	{
		setup_rx (k, thread);
		uni[thread].multirx.act[k] = FALSE;
	}
	uni[thread].multirx.act[0] = TRUE;
	uni[thread].multirx.nac = 1;

	setup_tx (thread);
}

void
destroy_workspace (unsigned int thread)
{
	int k;


	/* TX */
	delHilsim(tx[thread].hlb.gen);
	delWSCompander (tx[thread].cpd.gen);
	delSpeechProc (tx[thread].spr.gen);
	delDttSPAgc (tx[thread].leveler.gen);
	delDttSPAgc (tx[thread].alc.gen);
	delOSC (tx[thread].osc.gen);
	delDCBlocker (tx[thread].dcb.gen);
	delvec_COMPLEX (tx[thread].filt.save);
	delFiltOvSv (tx[thread].filt.ovsv);
	delFIR_Bandpass_COMPLEX (tx[thread].filt.coef);
	delCorrectIQ (tx[thread].iqfix);
	delCXB (tx[thread].buf.o);
	delCXB (tx[thread].buf.i);

	/* RX */
	for (k = 0; k < uni[thread].multirx.nrx; k++)
	{
		delWSCompander (rx[thread][k].cpd.gen);
		delSpotToneGen (rx[thread][k].spot.gen);
		delDttSPAgc (rx[thread][k].dttspagc.gen);
		del_nb (rx[thread][k].nb_sdrom.gen);
		del_nb (rx[thread][k].nb.gen);
		del_lmsr (rx[thread][k].anf.gen);
		del_lmsr (rx[thread][k].anr.gen);
		delAMD (rx[thread][k].am.gen);
		delFMD (rx[thread][k].fm.gen);
		delOSC (rx[thread][k].osc.gen);
		delvec_COMPLEX (rx[thread][k].filt.save);
		delFiltOvSv (rx[thread][k].filt.ovsv);
		delFIR_Bandpass_COMPLEX (rx[thread][k].filt.coef);
		delCorrectIQ (rx[thread][k].iqfix);
		delCXB (rx[thread][k].buf.o);
		delCXB (rx[thread][k].buf.i);
	}

	/* all */
	finish_spectrum (&uni[thread].spec);
	//fprintf(stderr,"Destroyed spectrum\n"),fflush(stderr);
}

//////////////////////////////////////////////////////////////////////////
// execution
//////////////////////////////////////////////////////////////////////////

//========================================================================
// util

PRIVATE void
CXBscl (CXB buff, REAL scl)
{
	int i;
	for (i = 0; i < CXBhave (buff); i++)
		CXBdata (buff, i) = Cscl (CXBdata (buff, i), scl);
}

PRIVATE REAL
CXBnorm (CXB buff)
{
	int i;
	REAL sum = 0.0;
	for (i = 0; i < CXBhave (buff); i++)
		sum += Csqrmag (CXBdata (buff, i));
	return (REAL) sqrt (sum);
}

PRIVATE REAL
CXBnormsqr (CXB buff)
{
	int i;
	REAL sum = 0.0;
	for (i = 0; i < CXBhave (buff); i++)
		sum += Csqrmag (CXBdata (buff, i));
	return (REAL) (sum);
}

PRIVATE REAL
CXBpeak (CXB buff)
{
	int i;
	REAL maxsam = 0.0;
	for (i = 0; i < CXBhave (buff); i++)
		maxsam = max (Cmag (CXBdata (buff, i)), maxsam);
	return maxsam;
}

PRIVATE REAL peakl(CXB buff)
{
	int i;
	REAL maxpwr = 0.0;
	for(i=0; i<CXBhave(buff); i++)
		maxpwr = max(CXBreal(buff, i), maxpwr);
	return maxpwr;
}

PRIVATE REAL peakr(CXB buff)
{
	int i;
	REAL maxpwr = 0.0;
	for(i=0; i<CXBhave(buff); i++)
		maxpwr = max(CXBimag(buff, i), maxpwr);
	return maxpwr;
}

PRIVATE REAL
CXBpeakpwr (CXB buff)
{
	int i;
	REAL maxpwr = 0.0;
	for (i = 0; i < CXBhave (buff); i++)
		maxpwr = max (Csqrmag (CXBdata (buff, i)), maxpwr);
	return maxpwr;
}

//========================================================================
/* all */

// unfortunate duplication here, due to
// multirx vs monotx

PRIVATE void
do_rx_meter (int k, unsigned int thread, CXB buf, int tap)
{
	COMPLEX *vec = CXBbase (buf);
	int i, len = CXBhave (buf);
	REAL tmp;

	switch (tap)
	{
		case RXMETER_PRE_CONV:
			tmp = -10000.0f;
			for (i = 0; i < len; i++)
				tmp = (REAL) max (fabs (vec[i].re), tmp);
			//fprintf(stderr, "adc_r max: %f\n", uni[thread].meter.rx.val[k][ADC_REAL]), fflush(stderr);
			uni[thread].meter.rx.val[k][ADC_REAL] = (REAL) (20.0 * log10 (tmp + 1e-10));
			tmp = -10000.0f;
			for (i = 0; i < len; i++)
				tmp = (REAL) max (fabs (vec[i].im), tmp);
			uni[thread].meter.rx.val[k][ADC_IMAG] = (REAL) (20.0 * log10 (tmp + 1e-10));
			break;
		case RXMETER_POST_FILT:
			tmp = 0;
			for (i = 0; i < len; i++)
				tmp += Csqrmag (vec[i]);
			rx[thread][k].norm = tmp / (REAL) len;
			uni[thread].meter.rx.val[k][SIGNAL_STRENGTH] =
				(REAL) (10.0 * log10 (tmp + 1e-20));
			if (uni[thread].meter.rx.mode[k] == SIGNAL_STRENGTH)
				uni[thread].meter.rx.val[k][AVG_SIGNAL_STRENGTH] = uni[thread].meter.rx.val[k][SIGNAL_STRENGTH];
			tmp = uni[thread].meter.rx.val[k][AVG_SIGNAL_STRENGTH];
			uni[thread].meter.rx.val[k][AVG_SIGNAL_STRENGTH] =
				(REAL) (0.95 * tmp +
				0.05 *uni[thread].meter.rx.val[k][SIGNAL_STRENGTH]);
			break;
		case RXMETER_POST_AGC:
			uni[thread].meter.rx.val[k][AGC_GAIN] =
				(REAL) (20.0 * log10 (rx[thread][k].dttspagc.gen->gain.now + 1e-10));
			//fprintf(stdout, "rx gain: %15.12f\n", uni[thread].meter.rx.val[k][AGC_GAIN]);
			//fflush(stdout);
			break;
		default:
			break;
	}
}


PRIVATE void
do_rx_spectrum (int k, unsigned int thread, CXB buf, int type)
{
	if (uni[thread].spec.flag && k == uni[thread].spec.rxk && type == uni[thread].spec.type)
	{
		if ((uni[thread].spec.type == SPEC_POST_DET) && (!rx[thread][k].bin.flag)) 
		{
			int i;
			for (i=0; i<CXBhave(rx[thread][k].buf.o);i++)
				CXBdata(uni[thread].spec.accum, uni[thread].spec.fill+i) = Cmplx(CXBreal(rx[thread][k].buf.o, i)*1.414f, 0.0);
		}
		else
		{
			memcpy ((char *) &CXBdata (uni[thread].spec.accum, uni[thread].spec.fill),
				(char *) CXBbase (buf), CXBsize (buf) * sizeof (COMPLEX));
		}
		uni[thread].spec.fill = (uni[thread].spec.fill + CXBsize (buf)) & uni[thread].spec.mask;
	}
}

PRIVATE void
do_tx_spectrum (unsigned int thread, CXB buf)
{
	if (uni[thread].spec.type == SPEC_PREMOD) 
	{
		int i;
		for (i=0; i<CXBhave(tx[thread].buf.i);i++)
			CXBdata(uni[thread].spec.accum, uni[thread].spec.fill+i) = Cmplx(CXBreal(tx[thread].buf.i, i), 0.0);
	}
	else
	{
		memcpy ((char *) &CXBdata (uni[thread].spec.accum, uni[thread].spec.fill),
			(char *) CXBbase (buf), CXBsize (buf) * sizeof (COMPLEX));
	}
	uni[thread].spec.fill = (uni[thread].spec.fill + CXBsize (buf)) & uni[thread].spec.mask;
}

//========================================================================
/* RX processing */

PRIVATE void
should_do_rx_squelch (int k, unsigned int thread)
{
	if (rx[thread][k].squelch.flag)
	{
		int i, n = CXBhave (rx[thread][k].buf.o);
		rx[thread][k].squelch.power = 0.0;

		for (i = 0; i < n; i++)
			rx[thread][k].squelch.power += Csqrmag (CXBdata (rx[thread][k].buf.o, i));

		if(10.0 * log10 (rx[thread][k].squelch.power + 1e-17) < rx[thread][k].squelch.thresh)
			rx[thread][k].squelch.set = TRUE;
		else
			rx[thread][k].squelch.set = FALSE;
	}
	else
	{
		rx[thread][k].squelch.set = FALSE;
	}
}

PRIVATE void
should_do_tx_squelch (unsigned int thread)
{
	if (tx[thread].squelch.flag)
	{
		int i, n = CXBsize (tx[thread].buf.i);
		tx[thread].squelch.power = 0.0;

		for (i = 0; i < n; i++)
			tx[thread].squelch.power += Csqrmag (CXBdata (tx[thread].buf.i, i));

		if((-30 + 10.0 * log10 (tx[thread].squelch.power + 1e-17)) < tx[thread].squelch.thresh)
			tx[thread].squelch.set = TRUE;
		else
			tx[thread].squelch.set = FALSE;

	}
	else
	{
		tx[thread].squelch.set = FALSE;
	}
}

// apply squelch
// slew into silence first time

PRIVATE void
do_squelch (int k, unsigned int thread)
{
	if (!rx[thread][k].squelch.running)
	{
		int i, m = rx[thread][k].squelch.num, n = CXBhave (rx[thread][k].buf.o) - m;

		for (i = 0; i < m; i++)
		{
			CXBdata (rx[thread][k].buf.o, i) =
				Cscl (CXBdata (rx[thread][k].buf.o, i), (REAL) (1.0 - (REAL) i / m));
		}

		memset ((void *) (CXBbase (rx[thread][k].buf.o) + m), 0, n * sizeof (COMPLEX));
		rx[thread][k].squelch.running = TRUE;
	}
	else
	{
		memset ((void *) CXBbase (rx[thread][k].buf.o),
			0, CXBhave (rx[thread][k].buf.o) * sizeof (COMPLEX));
	}
}

PRIVATE void
do_tx_squelch (unsigned int thread)
{
	int i, m = tx[thread].squelch.num, n = CXBhave (tx[thread].buf.i);
	int l = ((int)tx[thread].squelch.atten * m) / 100;

	if (!tx[thread].squelch.running)
	{
		for (i = 0; i < n; i++)
		{
			REAL scale = (REAL) (1.0 - (REAL) (i < l ? i : l) / m);
			CXBdata (tx[thread].buf.i, i) =
				Cscl (CXBdata (tx[thread].buf.i, i), scale);
		}
		tx[thread].squelch.running = TRUE;
	}
	else if (l != m)
	{
		REAL scale = (REAL) (1.0 - (REAL) l / m);
		for (i = 0; i < n; i++)
		{
			CXBdata (tx[thread].buf.i, i) =
				Cscl (CXBdata (tx[thread].buf.i, i), scale);
		}
	}
	else
	{
		memset ((void *) CXBbase (tx[thread].buf.i),
			0, CXBhave (tx[thread].buf.i) * sizeof (COMPLEX));
	}
}

// lift squelch
// slew out from silence to full scale

PRIVATE void
no_squelch (int k, unsigned int thread)
{
	if (rx[thread][k].squelch.running)
	{
		int i, m = rx[thread][k].squelch.num;

		for (i = 0; i < m; i++)
		{
			CXBdata (rx[thread][k].buf.o, i) =
				Cscl (CXBdata (rx[thread][k].buf.o, i), (REAL) i / m);
		}
		rx[thread][k].squelch.running = FALSE;
	}
}

PRIVATE void
no_tx_squelch (unsigned int thread)
{
	int i, m = tx[thread].squelch.num, n = CXBhave (tx[thread].buf.i);
	int l = (((int) tx[thread].squelch.atten) * m) / 100;

	if (tx[thread].squelch.running)
	{
		for (i = 0; i < m; i++)
		{
			REAL scale = (REAL) (i < l ? l : i) / m;
			CXBdata (tx[thread].buf.i, i) =
				Cscl (CXBdata (tx[thread].buf.i, i), scale);
		}
		tx[thread].squelch.running = FALSE;
	}
}
/* Routine to do the actual adding of buffers through the complex linear combination required */

#if 0
void
do_rx_diversity_combine()
{
	int i, n=CXBhave (rx[0][0].buf.i);
	for (i=0;i<n;i++)
	{
		CXBdata(rx[0][0].buf.i,i) = Cscl(Cadd(CXBdata(rx[0][0].buf.i,i),Cmul(CXBdata(rx[2][0].buf.i,i),diversity.scalar)),diversity.gain);
	}
}
#endif
/* pre-condition for (nearly) all RX modes */
PRIVATE void
do_rx_pre (int k, unsigned int thread)
{
	int i, n = min (CXBhave (rx[thread][k].buf.i), uni[thread].buflen);

	// metering for uncorrected values here
	do_rx_meter (k, thread, rx[thread][k].buf.i, RXMETER_PRE_CONV);	

	if (rx[thread][k].dcb->flag) DCBlock(rx[thread][k].dcb);

	if (rx[thread][k].nb.flag)
		noiseblanker (rx[thread][k].nb.gen);
	if (rx[thread][k].nb_sdrom.flag)
		SDROMnoiseblanker (rx[thread][k].nb_sdrom.gen);			

	correctIQ (rx[thread][k].buf.i, rx[thread][k].iqfix, FALSE, k);

	/* 2nd IF conversion happens here */
	if (rx[thread][k].osc.gen->Frequency != 0.0)
	{
		ComplexOSC (rx[thread][k].osc.gen);
		for (i = 0; i < n; i++)
			CXBdata (rx[thread][k].buf.i, i) = Cmul (CXBdata (rx[thread][k].buf.i, i),
			OSCCdata (rx[thread][k].osc.gen, i));
	}

	/* filtering, metering, spectrum, squelch, & AGC */

	//do_rx_meter (k, rx[thread][k].buf.i, RXMETER_PRE_FILT);
	do_rx_spectrum (k, thread, rx[thread][k].buf.i, SPEC_PRE_FILT);
	
	if (rx[thread][k].mode != SPEC)
	{
		if (rx[thread][k].resample.flag) {
			PolyPhaseFIRF(rx[thread][k].resample.gen1r);
			PolyPhaseFIRF(rx[thread][k].resample.gen1i);
		}
		if (rx[thread][k].tick == 0)
			reset_OvSv (rx[thread][k].filt.ovsv);

		filter_OvSv (rx[thread][k].filt.ovsv);
	}
	else
	{
		memcpy (CXBbase (rx[thread][k].buf.o), CXBbase (rx[thread][k].buf.i),
			sizeof (COMPLEX) * CXBhave (rx[thread][k].buf.i));
	}
    
	CXBhave (rx[thread][k].buf.o) = CXBhave (rx[thread][k].buf.i);

	do_rx_meter (k, thread, rx[thread][k].buf.o, RXMETER_POST_FILT);
	do_rx_spectrum (k, thread, rx[thread][k].buf.o, SPEC_POST_FILT);

	if (rx[thread][k].cpd.flag)
		WSCompand (rx[thread][k].cpd.gen);

	should_do_rx_squelch (k, thread);

}

PRIVATE void
do_rx_post (int k, unsigned int thread)
{
	int i, n = CXBhave (rx[thread][k].buf.o);

	if(rx[thread][k].squelch.set)
	{
		do_squelch (k, thread);
	}
	else no_squelch (k, thread);

	if (rx[thread][k].grapheq.flag)
	{
		switch(rx[thread][k].mode)
		{
			case DRM:
			case DIGL:
			case DIGU: // do nothing in digital modes
				break;
			default:
				graphiceq (rx[thread][k].grapheq.gen);
				break;
		}
	}

	do_rx_spectrum(k, thread, rx[thread][k].buf.o, SPEC_POST_DET);
	// not binaural?
	// position in stereo field


	if (rx[thread][k].anf.flag)
	{
		switch(rx[thread][k].mode)
		{
			case DRM:
			case DIGL:
			case DIGU:
			case CWL:
			case CWU: // do nothing
				break;
			default:
				lmsr_adapt (rx[thread][k].anf.gen);
				//blms_adapt(rx[thread][k].banf.gen);
				break;
		}
	}

	if (rx[thread][k].anr.flag)
		lmsr_adapt (rx[thread][k].anr.gen);
		//blms_adapt(rx[thread][k].banr.gen);

	/*if(thread == 0 && k == 0)
		fprintf(stdout, "before: %15f12  ", CXBpeak(rx[thread][k].buf.i));*/
#if 0
	if (diversity.flag && (k==0) && (thread==2))
		for (i = 0; i < n; i++) CXBdata(rx[thread][k].buf.o,i) = cxzero;
	else 
#endif
		DttSPAgc (rx[thread][k].dttspagc.gen, rx[thread][k].tick);
	
	/*if(thread == 0 && k == 0)
	{
		fprintf(stdout, "after: %15f12\n", CXBpeak(rx[thread][k].buf.o));
		fflush(stdout);
	}*/

	do_rx_meter(k, thread, rx[thread][k].buf.o, RXMETER_POST_AGC);
	do_rx_spectrum (k, thread, rx[thread][k].buf.o, SPEC_POST_AGC);

	if (!rx[thread][k].bin.flag)
		for (i = 0; i < CXBhave (rx[thread][k].buf.o); i++)
			CXBimag (rx[thread][k].buf.o, i) = CXBreal (rx[thread][k].buf.o, i);

	if(uni[thread].multirx.nac == 1)
	{
		for (i = 0; i < n; i++)
				CXBdata(rx[thread][k].buf.o, i) = Cscl(Cmplx(rx[thread][k].azim.re*CXBreal(rx[thread][k].buf.o, i),
														rx[thread][k].azim.im*CXBimag(rx[thread][k].buf.o, i)),1.414f);
	}
	else
	{
		for (i = 0; i < n; i++)
			CXBdata(rx[thread][k].buf.o, i) = Cmplx(rx[thread][k].azim.re*CXBreal(rx[thread][k].buf.o, i),
													rx[thread][k].azim.im*CXBimag(rx[thread][k].buf.o, i));
	} 

	if ((thread == 2)  && (diversity.flag))
		for (i=0;i< n; i++) CXBdata(rx[thread][k].buf.o,i) = cxzero;
	else
		if (rx[thread][k].output_gain != 1.0)
			for (i = 0; i < n; i++) CXBdata(rx[thread][k].buf.o,i) = Cscl(CXBdata(rx[thread][k].buf.o,i),rx[thread][k].output_gain);
	if (rx[thread][k].resample.flag) {
		PolyPhaseFIRF(rx[thread][k].resample.gen2r);
		PolyPhaseFIRF(rx[thread][k].resample.gen2i);
	}

}

/* demod processing */

PRIVATE void
do_rx_SBCW (int k, unsigned int thread)
{

}

PRIVATE void
do_rx_AM (int k, unsigned int thread)
{
	AMDemod (rx[thread][k].am.gen);
}

PRIVATE void
do_rx_FM (int k, unsigned int thread)
{
	FMDemod (rx[thread][k].fm.gen);
}

PRIVATE void
do_rx_DRM (int k, unsigned int thread)
{

}

PRIVATE void
do_rx_SPEC (int k, unsigned int thread)
{

}

PRIVATE void
do_rx_NIL (int k, unsigned int thread)
{
	int i, n = min (CXBhave (rx[thread][k].buf.i), uni[thread].buflen);
	for (i = 0; i < n; i++)
		CXBdata (rx[thread][k].buf.o, i) = cxzero;
}

/* overall dispatch for RX processing */

PRIVATE void
do_rx (int k, unsigned int thread)
{
	do_rx_pre (k, thread);
	switch (rx[thread][k].mode)
	{
		case DIGU:
		case DIGL:
		case USB:
		case LSB:
		case CWU:
		case CWL:
		case DSB:
			do_rx_SBCW (k, thread);
			break;
		case AM:
		case SAM:
			do_rx_AM (k, thread);
			break;
		case FMN:
			do_rx_FM (k, thread);
			break;
		case DRM:
			do_rx_DRM (k, thread);
			break;
		case SPEC:
		default:
			do_rx_SPEC (k, thread);
			break;
	}
	do_rx_post (k, thread);
}

//==============================================================
/* TX processing */
PRIVATE REAL mic_avg = 0.0f, mic_pk = 0.0f,
	alc_avg = 0.0f, alc_pk = 0.0f,
	lev_avg = 0.0f, lev_pk = 0.0f,
	eq_avg = 0.0f, eq_pk = 0.0f,
	comp_avg = 0.0f, comp_pk = 0.0f,
	cpdr_avg = 0.0f, cpdr_pk = 0.0f;

/* pre-condition for (nearly) all TX modes */
PRIVATE REAL peaksmooth = 0.0;
PRIVATE void
do_tx_meter (unsigned int thread, CXB buf, TXMETERTYPE mt)
{
	COMPLEX *vec = CXBbase (buf);
	int i, len = CXBhave (buf);
	REAL tmp = 0.0f;

	switch (mt)
	{
		case TX_MIC:
			for (i = 0; i < CXBhave (tx[thread].buf.i); i++) // calculate avg Mic
				mic_avg = (REAL) (0.9995 * mic_avg +
				0.0005 * Csqrmag (CXBdata (tx[thread].buf.i, i)));
			uni[thread].meter.tx.val[TX_MIC] = (REAL) (-10.0 * log10 (mic_avg + 1e-16));

			mic_pk = CXBpeak(tx[thread].buf.i);		// calculate peak mic                 
			uni[thread].meter.tx.val[TX_MIC_PK] = (REAL) (-20.0 * log10 (mic_pk + 1e-16));
			break;

		case TX_PWR:
			for (i = 0, tmp = 0.0000001f;
				i < CXBhave (tx[thread].buf.o); i++)
				tmp += Csqrmag (CXBdata (tx[thread].buf.o, i));
			uni[thread].meter.tx.val[TX_PWR] = tmp/(REAL) len;
			break;

		case TX_ALC:
			for (i = 0; i < CXBhave (tx[thread].buf.i); i++)
				alc_avg = (REAL) (0.9995 * alc_avg +
				0.0005 * Csqrmag (CXBdata (tx[thread].buf.i, i)));
			uni[thread].meter.tx.val[TX_ALC] = (REAL) (-10.0 * log10 (alc_avg + 1e-16));

			alc_pk = CXBpeak(tx[thread].buf.i);
			uni[thread].meter.tx.val[TX_ALC_PK] = (REAL) (-20.0 * log10 (alc_pk+ 1e-16));
			uni[thread].meter.tx.val[TX_ALC_G] = (REAL)(20.0*log10(tx[thread].alc.gen->gain.now+1e-16));
			//fprintf(stdout, "pk: %15.12f  comp: %15.12f\n", uni[thread].meter.tx.val[TX_ALC_PK], uni[thread].meter.tx.val[TX_ALC_G]);
			//fflush(stdout);
			break;

		case TX_EQ:
			for (i = 0; i < CXBhave (tx[thread].buf.i); i++)
				eq_avg = (REAL) (0.9995 * eq_avg +
				0.0005 * Csqrmag (CXBdata (tx[thread].buf.i, i)));
			uni[thread].meter.tx.val[TX_EQ] = (REAL) (-10.0 * log10 (eq_avg + 1e-16));

			eq_pk = CXBpeak(tx[thread].buf.i);
			uni[thread].meter.tx.val[TX_EQ_PK] = (REAL) (-20.0 * log10 (eq_pk + 1e-16));
			break;

		case TX_LVL:
			for (i = 0; i < CXBhave (tx[thread].buf.i); i++)
				lev_avg = (REAL) (0.9995 * lev_avg +
				0.0005 * Csqrmag (CXBdata (tx[thread].buf.i, i)));
			uni[thread].meter.tx.val[TX_LVL] = (REAL) (-10.0 * log10 (lev_avg + 1e-16));

			lev_pk = CXBpeak(tx[thread].buf.i);
			uni[thread].meter.tx.val[TX_LVL_PK] = (REAL) (-20.0 * log10 (lev_pk + 1e-16));
			uni[thread].meter.tx.val[TX_LVL_G] = (REAL)(20.0*log10(tx[thread].leveler.gen->gain.now + 1e-16));
			break;

		case TX_COMP:
			for (i = 0; i < CXBhave (tx[thread].buf.i); i++)
				comp_avg = (REAL) (0.9995 * comp_avg +
				0.0005 * Csqrmag (CXBdata (tx[thread].buf.i, i)));
			uni[thread].meter.tx.val[TX_COMP] = (REAL) (-10.0 * log10 (comp_avg + 1e-16));

			comp_pk = CXBpeak(tx[thread].buf.i);
			uni[thread].meter.tx.val[TX_COMP_PK] = (REAL) (-20.0 * log10 (comp_pk + 1e-16));
			break;

		case TX_CPDR:
			for (i = 0; i < CXBhave (tx[thread].buf.i); i++)
				cpdr_avg = (REAL) (0.9995 * cpdr_avg +
				0.0005 * Csqrmag (CXBdata (tx[thread].buf.i, i)));
			uni[thread].meter.tx.val[TX_CPDR] = (REAL) (-10.0 * log10 (cpdr_avg + 1e-16));

			cpdr_pk = CXBpeak(tx[thread].buf.i);
			uni[thread].meter.tx.val[TX_CPDR_PK] = (REAL) (-20.0 * log10 (cpdr_pk + 1e-16));
			break;

		default:
			break;
	}
}

PRIVATE void
do_tx_pre (unsigned int thread)
{
	int i, n = CXBhave (tx[thread].buf.i);
	for (i = 0; i < n; i++)
		CXBdata (tx[thread].buf.i, i) = Cmplx (CXBimag (tx[thread].buf.i, i), 0.0);
	//hilsim_transform(tx[thread].hlb.gen);
//	fprintf(stderr,"Peak value = %f\n",CXBpeakpwr(tx[thread].buf.i));
	if (tx[thread].dcb.flag)
		DCBlock (tx[thread].dcb.gen);

	do_tx_meter (thread, tx[thread].buf.i, TX_MIC);
	//fprintf(stderr,"[%.2f,%.2f]  ", peakl(tx[thread].buf.i), peakr(tx[thread].buf.i));

	should_do_tx_squelch(thread);
	if(tx[thread].squelch.set)
		do_tx_squelch (thread);
	else //if (!tx[thread].squelch.set)
		no_tx_squelch (thread);

	switch(tx[thread].mode)
	{
		case DIGU:
		case DIGL:
		case DRM:
			do_tx_meter (thread, tx[thread].buf.i, TX_EQ);
			do_tx_meter (thread, tx[thread].buf.i, TX_LVL);
			do_tx_meter (thread, tx[thread].buf.i, TX_COMP);

			if (tx[thread].alc.flag)
				DttSPAgc (tx[thread].alc.gen, tx[thread].tick);
			do_tx_meter (thread, tx[thread].buf.i, TX_ALC);

			do_tx_meter (thread, tx[thread].buf.i, TX_CPDR);
			break;
		default:
			if (tx[thread].grapheq.flag)
				graphiceq (tx[thread].grapheq.gen);
			do_tx_meter (thread, tx[thread].buf.i, TX_EQ);
			//fprintf(stderr,"[%.2f,%.2f]  ", peakl(tx[thread].buf.i), peakr(tx[thread].buf.i));

			if (tx[thread].leveler.flag)
				DttSPAgc (tx[thread].leveler.gen, tx[thread].tick);						
			do_tx_meter (thread, tx[thread].buf.i, TX_LVL);
			//fprintf(stderr,"[%.2f,%.2f]  ", peakl(tx[thread].buf.i), peakr(tx[thread].buf.i));

			if (tx[thread].alc.flag)
				DttSPAgc (tx[thread].alc.gen, tx[thread].tick);
			do_tx_meter (thread, tx[thread].buf.i, TX_ALC);
			//fprintf(stderr,"[%.2f,%.2f]  ", peakl(tx[thread].buf.i), peakr(tx[thread].buf.i));

			if (tx[thread].spr.flag)
				SpeechProcessor (tx[thread].spr.gen);						
			do_tx_meter (thread, tx[thread].buf.i, TX_COMP);
			//fprintf(stderr,"[%.2f,%.2f]  ", peakl(tx[thread].buf.i), peakr(tx[thread].buf.i));

			if (tx[thread].cpd.flag)
				WSCompand (tx[thread].cpd.gen);						
			do_tx_meter (thread, tx[thread].buf.i, TX_CPDR);
			//fprintf(stderr,"[%.2f,%.2f]  ", peakl(tx[thread].buf.i), peakr(tx[thread].buf.i));

			break;						
	}
}

PRIVATE void
do_tx_post (unsigned int thread)
{
	CXBhave (tx[thread].buf.o) = CXBhave (tx[thread].buf.i);

	if (tx[thread].tick == 0)
		reset_OvSv (tx[thread].filt.ovsv);

	//fprintf(stderr,"[%.2f,%.2f]  ", peakl(tx[thread].buf.i), peakr(tx[thread].buf.i));
	filter_OvSv (tx[thread].filt.ovsv);
	if (uni[thread].spec.flag)
		do_tx_spectrum (thread, tx[thread].buf.o);
	//fprintf(stderr,"[%.2f,%.2f]  ", peakl(tx[thread].buf.o), peakr(tx[thread].buf.o));

	// meter modulated signal

	if (tx[thread].osc.gen->Frequency != 0.0)
	{
		int i;
		ComplexOSC (tx[thread].osc.gen);
		for (i = 0; i < CXBhave (tx[thread].buf.o); i++)
		{
			CXBdata (tx[thread].buf.o, i) =
				Cmul (CXBdata (tx[thread].buf.o, i), OSCCdata (tx[thread].osc.gen, i));
		}
	}
	correctIQ (tx[thread].buf.o, tx[thread].iqfix, TRUE,0);
	do_tx_meter (thread, tx[thread].buf.o, TX_PWR);
	//fprintf(stderr,"[%.2f,%.2f]  ", peakl(tx[thread].buf.o), peakr(tx[thread].buf.o));
	//fprintf(stderr,"\n");
	//fflush(stderr);
}

/* modulator processing */

PRIVATE void
do_tx_SBCW (unsigned int thread)
{
	int n = min (CXBhave (tx[thread].buf.i), uni[thread].buflen);

	//fprintf(stderr,"[%.2f,%.2f]  ", peakl(tx[thread].buf.i), peakr(tx[thread].buf.i));
	if (tx[thread].mode != DSB)
		CXBscl (tx[thread].buf.i, 2.0f);
}

PRIVATE void
do_tx_AM (unsigned int thread)
{
	int i, n = min (CXBhave (tx[thread].buf.i), uni[thread].buflen);
	//fprintf(stderr,"[%.2f,%.2f]  ", peakl(tx[thread].buf.i), peakr(tx[thread].buf.i));

	for (i = 0; i < n; i++)
	{
		CXBdata (tx[thread].buf.i, i) = Cmplx ((REAL)
			(tx[thread].am.carrier_level +
			(1.0f - tx[thread].am.carrier_level) * CXBreal (tx[thread].buf.i, i)), 0.0);
	}
}

PRIVATE void
do_tx_FM (unsigned int thread)
{
	int i, n = min (CXBhave (tx[thread].buf.i), uni[thread].buflen);
	//fprintf(stderr,"[%.2f,%.2f]  ", peakl(tx[thread].buf.i), peakr(tx[thread].buf.i));

	for (i = 0; i < n; i++)
	{
		tx[thread].osc.phase += CXBreal (tx[thread].buf.i, i) * tx[thread].fm.cvtmod2freq;
		CXBdata (tx[thread].buf.i, i) =
			Cmplx ((REAL) cos (tx[thread].osc.phase), (IMAG) sin (tx[thread].osc.phase));
	}
}

PRIVATE void
do_tx_NIL (thread)
{
	int i, n = min (CXBhave (tx[thread].buf.i), uni[thread].buflen);
	for (i = 0; i < n; i++)
		CXBdata (tx[thread].buf.i, i) = cxzero;
}

/* general TX processing dispatch */

PRIVATE void
do_tx (unsigned int thread)
{
	do_tx_pre (thread);
	switch (tx[thread].mode)
	{
		case USB:
		case LSB:
		case CWU:
		case CWL:
		case DIGU:
		case DIGL:
		case DRM:
		case DSB:
			do_tx_SBCW (thread);
			break;
		case AM:
		case SAM:
			do_tx_AM (thread);
			break;
		case FMN:
			do_tx_FM (thread);
			break;
		case SPEC:
		default:
			do_tx_NIL (thread);
			break;
	}
	do_tx_post (thread);
	//fprintf(stderr,"%f\n",Cmag(CXBdata(tx[thread].buf.o,0))),fflush(stderr);
}

//========================================================================
/* overall buffer processing;
   come here when there are buffers to work on */

void
process_samples (float *bufl, float *bufr, float *auxl, float *auxr, int n, unsigned int thread)
{
	int i, k;

	switch (uni[thread].mode.trx)
	{
		case RX:

			// make copies of the input for all receivers
			for (k = 0; k < uni[thread].multirx.nrx; k++)
			{
				BOOLEAN kdone=FALSE;
				int kone = -1;
				if (uni[thread].multirx.act[k])
				{
					if (!kdone) {
						kdone = TRUE;
						kone = k;
						for (i = 0; i < n; i++)
						{
							CXBimag (rx[thread][k].buf.i, i) =
								bufl[i], CXBreal (rx[thread][k].buf.i, i) = bufr[i];
						}
						CXBhave (rx[thread][k].buf.i) = n;
					} else memcpy(rx[thread][k].buf.i,rx[thread][kone].buf.i,CXBhave(rx[thread][kdone].buf.i)*sizeof(COMPLEX));
				}
			}
			
			// prepare buffers for mixing
			memset ((char *) bufl, 0, n * sizeof (float));
			memset ((char *) bufr, 0, n * sizeof (float));

			// run all receivers
			for (k = 0; k < uni[thread].multirx.nrx; k++)
			{
                if (uni[thread].multirx.act[k])
				{
					do_rx (k, thread), rx[thread][k].tick++;
					// mix
					for (i = 0; i < n; i++)
					{
						bufl[i] += (float) CXBimag (rx[thread][k].buf.o, i);
						bufr[i] += (float) CXBreal (rx[thread][k].buf.o, i);
					}
					CXBhave (rx[thread][k].buf.o) = n;
				}
			}

			// late mixing of aux buffers
#if 0
			if (uni[thread].mix.rx.flag)
			{
				for (i = 0; i < n; i++)
				{
					bufl[i] += (float) (auxl[i] * uni[thread].mix.rx.gain),
						bufr[i] += (float) (auxr[i] * uni[thread].mix.rx.gain);
				}
			}
#endif
			break;

		case TX:
#if 0
			// early mixing of aux buffers
			if (uni[thread].mix.tx.flag)
			{
				for (i = 0; i < n; i++)
				{
					bufl[i] += (float) (auxl[i] * uni[thread].mix.tx.gain),
						bufr[i] += (float) (auxr[i] * uni[thread].mix.tx.gain);
				}
			}
#endif
			for (i = 0; i < n; i++)
			{
				CXBimag (tx[thread].buf.i, i) = bufl[i];
				CXBreal (tx[thread].buf.i, i) = bufr[i];
			}

			CXBhave (tx[thread].buf.i) = n;
			tx[thread].norm = CXBpeak (tx[thread].buf.i);

			do_tx (thread), tx[thread].tick++;

			for (i = 0; i < n; i++)
				bufl[i] = (float) CXBimag (tx[thread].buf.o, i),
				bufr[i] = (float) CXBreal (tx[thread].buf.o, i);
			CXBhave (tx[thread].buf.o) = n;

			break;
	}

	uni[thread].tick++;
}
