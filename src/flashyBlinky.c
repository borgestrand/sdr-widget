/* -*- mode: c; tab-width: 4; c-basic-offset: 4 -*- */
#include "compiler.h"
#include "gpio.h"
#include "board.h"
//
#include "conf_isp.h"

#include "flashyBlinky.h"

/*! \name Pin Configuration
 */
//! @{


//! @}



/*! \brief This is an example of how to access the gpio.c driver to set, clear, toggle... the pin 
 */
int flashyBlinky(void)
{

  U32 i;


  while (1)
  {



    // Poll push button value.
    for (i = 0; i < 200; i++)
    {
    }

  }
}
