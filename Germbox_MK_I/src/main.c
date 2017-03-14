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
#include "pwm.h"
#include "pio.h"
#include "pmc.h"
#include "adc.h"



int main (void)
{

	pwm_channel_t pwm_ch_instance;
	pwm_clock_t pwm_clk;
		
	
	sysclk_init();
	board_init();
	
	pmc_enable_periph_clk(ID_PWM);
	pwm_channel_disable(PWM, PWM_CHANNEL_0);
	pwm_clk.ul_clka = 1000 * 100;
	pwm_clk.ul_clkb = 0;
	pwm_clk.ul_mck = 32000000;
	pwm_init(PWM, &pwm_clk);
	
	pwm_ch_instance.ul_prescaler = PWM_CMR_CPRE_CLKA;
	pwm_ch_instance.ul_period = 100;
	pwm_ch_instance.ul_duty = 50;
	pwm_ch_instance.channel = PWM_CHANNEL_0;
	pwm_channel_init(PWM, &pwm_ch_instance);
	
	pwm_channel_enable(PWM, PWM_CHANNEL_0);
	
	while(1)
	{
		
	}
	
	

	/* Insert application code here, after the board has been initialized. */
}