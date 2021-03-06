/*
 * heater.c
 *
 * Created: 15. 03. 2017 02:12:48
 *  Author: matja
 */ 


#include "pwm.h"
#include "pio.h"
#include "pmc.h"

#include "hardware_conf.h"

#include <stdint.h>

pwm_channel_t pwm_ch_instance;

void heater_init(void)
{
	
	pwm_clock_t pwm_clk;
	
	pmc_enable_periph_clk(ID_PIOA);
	pio_set_peripheral(HEATER_PORT, HEATER_PIN_MUX, HEATER_PIN);
	
	pmc_enable_periph_clk(ID_PWM);
	pwm_channel_disable(PWM, HEATER_PWM_CH);
	pwm_clk.ul_clka = HEATER_PWM_FREQUENCY * 100;
	pwm_clk.ul_clkb = 0;
	pwm_clk.ul_mck = 32000000;
	pwm_init(PWM, &pwm_clk);
	
	pwm_ch_instance.ul_prescaler = PWM_CMR_CPRE_CLKA;
	pwm_ch_instance.ul_period = 100;
	pwm_ch_instance.ul_duty = 3;
	pwm_ch_instance.channel = HEATER_PWM_CH;
	pwm_ch_instance.alignment = PWM_ALIGN_LEFT;
	pwm_ch_instance.polarity = PWM_LOW;
	pwm_channel_init(PWM, &pwm_ch_instance);
	pwm_channel_enable(PWM, HEATER_PWM_CH);
	
}

volatile void heater_set (uint8_t heat)
{
	if(heat > 100)
	{
		heat = 100;
	}
	pwm_channel_update_duty(PWM, &pwm_ch_instance, (uint32_t)heat);
}