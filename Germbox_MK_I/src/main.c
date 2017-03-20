/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>
#include <stdint.h>
#include <stdio.h>


#include "heater.h"
#include "stimer.h"
#include "pump.h"
#include "display.h"
#include "thermo.h"
#include "encoder.h"

#include "adc.h"
#include "wdt.h"


int main (void)
{

	
	sysclk_init();
	board_init();
	heater_init();
	pump_init();
	stimer_init();
	thermo_init();

	display_init();
	encoder_init();

	while(1)
	{

	}
	
}
