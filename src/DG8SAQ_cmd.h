/*
 * DG8SAQ_cmd.h
 *
 *  Created on: 2010-06-13
 *      Author: Loftur Jonasson, TF3LJ
 */

#ifndef DG8SAQ_CMD_H_
#define DG8SAQ_CMD_H_

extern volatile uint32_t freq_from_usb;			// Pass frequency from USB input command
												// if 0, then nothing to pass
extern bool	FRQ_fromusb;						// Flag: New frequency from USB

#endif /* DG8SAQ_CMD_H_ */
