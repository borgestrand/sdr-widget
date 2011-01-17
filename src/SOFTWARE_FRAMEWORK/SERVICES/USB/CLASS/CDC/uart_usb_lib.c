/* This source file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3-1.5.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file ******************************************************************
 *
 * \brief This file controls the UART USB functions.
 *
 * These functions allow to use en USB endpoint as we would do using an UART.
 * This is particurly well suited for USB CDC class.
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

//_____  I N C L U D E S ___________________________________________________
#include "compiler.h"
#include "conf_usb.h"
#include "usb_drv.h"
#include "uart_usb_lib.h"
#include "composite_widget.h"

//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________

U8    dev_rx_cnt;
U8    dev_tx_cnt;

//_____ D E C L A R A T I O N S ____________________________________________


void uart_usb_init(void)
{
  dev_rx_cnt=0 ;
  dev_tx_cnt=0 ;
}

// Functions that manage characters input through USB
//

Bool uart_usb_test_hit(void)
{
  if( dev_rx_cnt==0 )
  {
    if( Is_usb_out_received(EP_CDC_RX) )
    {
      dev_rx_cnt = Usb_byte_count(EP_CDC_RX);
      Usb_reset_endpoint_fifo_access(EP_CDC_RX);
      if( dev_rx_cnt==0 )
      {
        Usb_ack_out_received_free(EP_CDC_RX);
        return FALSE;
      }
      else
        return TRUE;
    }
    else
      return FALSE;
  }
  else
    return TRUE;
}

char uart_usb_getchar(void)
{
  register char data_rx;

  while( !uart_usb_test_hit() );

  data_rx=Usb_read_endpoint_data(EP_CDC_RX, 8);
  dev_rx_cnt--;
  if( dev_rx_cnt==0 ) Usb_ack_out_received_free(EP_CDC_RX);

  return data_rx;
}



// Functions that manage characters output through USB
//

Bool uart_usb_tx_ready(void)
{
  if( !Is_usb_write_enabled(EP_CDC_TX) )
    return FALSE;

  return TRUE;
}

int uart_usb_putchar(int data_to_send)
{
  while( !uart_usb_tx_ready() ); // Wait Endpoint ready
  if( dev_tx_cnt==0 )
  {
    Usb_reset_endpoint_fifo_access(EP_CDC_TX);
  }
  Usb_write_endpoint_data(EP_CDC_TX, 8, data_to_send);
  dev_tx_cnt++;
  if( !uart_usb_tx_ready() ) //If Endpoint full -> flush
  {
    uart_usb_flush();
  }
  return data_to_send;
}

void uart_usb_flush (void)
{
  Bool zlp=FALSE;
  if( dev_tx_cnt!=0 )
  {
    if(!Is_usb_write_enabled(EP_CDC_TX))              // Endpoint full, need ZLP
       zlp=TRUE;

    Usb_ack_in_ready_send(EP_CDC_TX);

    if( zlp==TRUE )
    {
       while( !Is_usb_write_enabled(EP_CDC_TX) );     // Wait Endpoint ready...
       Usb_ack_in_ready_send(EP_CDC_TX);              // ...and Send ZLP
    }
    dev_tx_cnt = 0;
  }
}
