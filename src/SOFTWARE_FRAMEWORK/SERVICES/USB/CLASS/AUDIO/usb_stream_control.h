/* This header file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3-1.5.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file ******************************************************************
 *
 * \brief Management of the USB host mouse HID task.
 *
 * This file manages the USB host mouse HID task.
 *
 * - Compiler:           IAR EWAVR32 and GNU GCC for AVR32
 * - Supported devices:  All AVR32 devices with a USB module can be used.
 * - AppNote:
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 ***************************************************************************/

/* Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 * AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 */

#ifndef _USB_STREAM_CONTROL_H_
#define _USB_STREAM_CONTROL_H_


//_____ I N C L U D E S ____________________________________________________

#if( USB_STREAM_BUFFER_NUMBER<2 )
#  warning USB Audio stream error: Please initialize USB_STREAM_BUFFER_NUMBER for at least 2 buffers
#endif

#if( USB_STREAM_IDLE_BUFFER_NUMBER>USB_STREAM_BUFFER_NUMBER )
#  warning USB Audio stream error: You can not have more Idle buffers than the total number of buffers.
#endif

#if( USB_STREAM_BUFFER_NUMBER!=2 )  \
&& ( USB_STREAM_BUFFER_NUMBER!=4 )  \
&& ( USB_STREAM_BUFFER_NUMBER!=8 )  \
&& ( USB_STREAM_BUFFER_NUMBER!=16 ) \
&& ( USB_STREAM_BUFFER_NUMBER!=32 ) \
&& ( USB_STREAM_BUFFER_NUMBER!=64 ) \
&& ( USB_STREAM_BUFFER_NUMBER!=128 )\
&& ( USB_STREAM_BUFFER_NUMBER!=256 )
#  warning USB Audio stream error: Please initialize USB_STREAM_BUFFER_NUMBER to a power of 2 numbers (2, 4, 8, 16, ..)
#endif

//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________

//! @defgroup usb_stream_status USB stream status
//! Defines the various possible status returned by the USB Stream Control functions.
//! @{
#define USB_STREAM_STATUS_BUFFER_OVERFLOW    (-4) //!< Fatal error: the number of incoming bytes exceeds the buffer size.
#define USB_STREAM_STATUS_SLOW_DOWN          (-3) //!< Needs to delete samples to keep the stream synchronized.
#define USB_STREAM_STATUS_SPEED_UP           (-2) //!< Needs to add samples to keep the stream synchronized.
#define USB_STREAM_STATUS_NOT_SYNCHRONIZED   (-1) //!< Stream not synchronized.
#define USB_STREAM_STATUS_OK                  (0) //!< Stream synchronized.
//! @}

//! Reader buffer id
extern volatile U8   usb_stream_rd_id;

//! Writer buffer id
extern volatile U8   usb_stream_wr_id;

//! Holds the data size in each buffer.
extern volatile U16  usb_stream_audio_buffer_size[USB_STREAM_BUFFER_NUMBER];

//! Real size of the buffer, i.e. taking into account the possible data 'expension'
#define  USB_STREAM_REAL_BUFFER_SIZE     ( Align_up(USB_STREAM_BUFFER_SIZE + USB_STREAM_BUFFER_SIZE*USB_STREAM_MAX_EXCURSION/1000, 4) )

//! USB stream audio FIFO, composed of buffers
extern volatile S8   usb_stream_audio_buffer[     USB_STREAM_BUFFER_NUMBER][USB_STREAM_REAL_BUFFER_SIZE];


/*! \brief This function returns the number of used buffers in the USB stream FIFO.
 *
 * TODO: First release of the FIFO management does not support that 100% of the buffer are filled.
 *
 * \return \c number of used buffers.
 */
#if (defined __GNUC__)
__attribute__((__always_inline__))
#endif
extern __inline__ int usb_stream_fifo_get_used_room( void )
{
  return (usb_stream_wr_id - usb_stream_rd_id) & (USB_STREAM_BUFFER_NUMBER-1);
}


/*! \brief This function returns the number of free buffers in the USB stream FIFO.
 *
 * \return \c number of free buffers.
 */
#if (defined __GNUC__)
__attribute__((__always_inline__))
#endif
extern __inline__ int usb_stream_fifo_get_free_room( void )
{
  return USB_STREAM_BUFFER_NUMBER - usb_stream_fifo_get_used_room() - 1;
}


/*! \brief This function returns the pointer on a buffer according to its index
 *
 * \param index      Index of the buffer
 *
 * \return \c pointer on buffer
 */
#if (defined __GNUC__)
__attribute__((__always_inline__))
#endif
extern __inline__ S8* usb_stream_fifo_get_buffer( U8 index )
{
  return (S8*)usb_stream_audio_buffer[index];
}


/*! \brief This function put into the audio stream FIFO a new buffer
 * and increases the write index of the FIFO.
 *
 * \param size       Size of the buffer.
 */
#if (defined __GNUC__)
__attribute__((__always_inline__))
#endif
extern __inline__ void usb_stream_fifo_push( U16 size )
{
  // Prepare current buffer. Buffer content has been filled before.
  usb_stream_audio_buffer_size[usb_stream_wr_id] = size;

  // Validate current buffer which is ready.
  usb_stream_wr_id = (usb_stream_wr_id+1) & (USB_STREAM_BUFFER_NUMBER-1);
}

/*! \brief This function gets from the audio stream FIFO the next buffer to read.
 *
 *  Note that it does not increase the read index of the FIFO.
 *
 * \param pp_buffer  (return parameter) Pointer to the buffer address to update.
 * \param p_size     (return parameter) Pointer to the size of the buffer.
 */
#if (defined __GNUC__)
__attribute__((__always_inline__))
#endif
extern __inline__ void usb_stream_fifo_get( void** pp_buffer, U16* p_size )
{
  // Gives current buffer to PWM DAC driver.
  *pp_buffer = (void*)usb_stream_audio_buffer[usb_stream_rd_id];
  *p_size    = usb_stream_audio_buffer_size[  usb_stream_rd_id];
  usb_stream_audio_buffer_size[usb_stream_rd_id]= 0;
}


/*! \brief This function increases the read index of the FIFO.
 */
#if (defined __GNUC__)
__attribute__((__always_inline__))
#endif
extern __inline__ void usb_stream_fifo_pull( void )
{
  usb_stream_rd_id = (usb_stream_rd_id+1) & (USB_STREAM_BUFFER_NUMBER-1);
}


//_____ D E C L A R A T I O N S ____________________________________________

extern void usb_stream_init(
   U32 sample_rate_hz
,  U8 num_channels
,  U8 bits_per_sample
,  Bool swap_channels
,  U32 pba_hz);

extern int usb_stream_input(U8 pipe_in, U16 byte_count, U32* pFifoCount);


#endif  // _USB_STREAM_CONTROL_H_
