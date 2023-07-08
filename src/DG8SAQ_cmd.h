/*
 * DG8SAQ_cmd.h
 *
 *  Created on: 2010-06-13
 *      Author: Loftur Jonasson, TF3LJ
 */

#ifndef DG8SAQ_CMD_H_
#define DG8SAQ_CMD_H_

//
// Host to Device transmissions
//
#define DG8SAQ_SET_FREQ_MUL_ADD	0x31			// takes 8 bytes, frequency subtract and multiply
#define DG8SAQ_SET_BAND_MUL_ADD 0x31			// takes 8 bytes, band specific frequency subtract and multiply
#define DG8SAQ_SET_FREQ 0x32					// takes 4 bytes, set frequency
#define DG8SAQ_SET_XTAL 0x33					// takes 4 bytes, write new crystal frequency to EEPROM and use it
#define DG8SAQ_SET_START_F 0x34					// takes 4 bytes, write new startup frequency to eeprom
#define DG8SAQ_SET_SMOOTH 0x35					// takes 2 bytes, write new smooth tune to eeprom and use it.

//
// USB query commands
//
#define DG8SAQ_GET_VERSION 0x00					//
// LEGACY_PORT_CMD
#define DG8SAQ_SET_PORTD_DIR 0x01				//
#define DG8SAQ_READ_PORTD 0x02					//
#define DG8SAQ_READ_PORTD_STATE 0x03			//
#define DG8SAQ_SET_PORTD 0x04					//

#define DG8SAQ_WDT_REBOOT 0x0f					// reboot by watchdog

#define DG8SAQ_SET_PORT_MASKED 0x15				//
#define DG8SAQ_GET_PORT_BITS 0x16				//

#define DG8SAQ_FILTER_TABLE 0x17				// read and write band pass filter cross over tables
#define DG8SAQ_FILTER_TABLE_SET 0x18			// scrambled filters set
#define DG8SAQ_FILTER_TABLE_GET 0x19			// scrambled filters get
#define DG8SAQ_LPF_TABLE_SET 0x1a				// scrambled low pass filters set
#define DG8SAQ_LPF_TABLE_GET 0x1b				// scrambled low pass filters get

#define DG8SAQ_GET_FREQ 0x3a					//
#define DG8SAQ_GET_SMOOTH 0x3b					//
#define DG8SAQ_GET_START_F 0x3c					//
#define DG8SAQ_GET_XTAL 0x3d					//

#define DG8SAQ_GET_I2C_STATUS 0x40				//
// 0x41
// 0x50
// 0x51
// 0x52
// 0x61
// 0x64
// 0x65
// 0x66
// 0x67
// 0x68
// 0x6e
// 0x6f

#define DG8SAQ_SDR_CTL 0x71					//
#define DG8SAQ_SDR_CTL_SET_SR 0x00			//

#endif /* DG8SAQ_CMD_H_ */
