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

#include "asf.h""

#include "pid.h"
#include "stimer.h"
#include "display.h"
#include "heater.h"
#include "thermo.h"
#include "pump.h"
#include "encoder.h"


#define PID_TIMER		0 // stimer channel number
#define PRINT_TIMER		1 // timer for printing things over usb
#define MIN_TIMER		2 // timer used for minute counting
#define WATERING_TIMER	3 // timer used for measuring 


#define KP	35//55
#define KI	0.011//0.06
#define KD	100//15

#define  I_MAX 400

#define DEFAULT_SET_T			29.0	//default set temperature  in �C
#define DEFAULT_SET_HUMID		3		//defeaut set humidity
#define DEFAULT_WATERING_TIME	10		//default duration of watering in seconds

#define WATTERING_INTERVAL		120 //watering interval in minutes
#define SHORT_WATERING			15 //watering duration in seconds
#define LONG_WATERING			25 //watering duration in seconds
#define HUMID_TRESHOLD			4 // treshild between short and long wattering duration


//#define PRINT_OVER_USB //uncoment if you want controller to prnt data out over USB

/* best so far (PI)
#define KP	90
#define KI	0.01
#define KD	0
*/

/* old settings (PID)

#define KP	150
#define KI	0.2
#define KD	6
*/


/* Underdamped (PID)

#define KP	25
#define KI	0.01
#define KD	250
*/

/* kind of ok (PID)

#define KP	25
#define KI	0.05
#define KD	250
*/



volatile uint32_t time_expired = 0, watering_expired = 0; // indicator flags
volatile uint32_t ttp = 0;		// time to print

volatile uint32_t expired_time = 0;
volatile uint32_t water_timer = 0;

void min_count (void)
{
	water_timer++;
	expired_time++;
	
	if(water_timer == WATTERING_INTERVAL)
	{
		water_timer = 0;
		watering_expired = 1;
	}
}

static void water_off (void)
{
	pump_off;
}


static void init_min_counter (void)
{
	stimer_set_time(MIN_TIMER, 60000, 1);
	stimer_register_callback(MIN_TIMER, min_count);
	stimer_start(MIN_TIMER);
}

static void init_watering_timer (void)
{
	stimer_set_time(WATERING_TIMER, DEFAULT_WATERING_TIME * 100, 0);
	stimer_register_callback(WATERING_TIMER, water_off);
}

void water (uint8_t amount)
{
	stimer_set_time(WATERING_TIMER, amount * 1000, 0);
	pump_on; // pump will be turned off in callback routine striggered automaticaly by timer
	stimer_start(WATERING_TIMER);
	
}

void float_split (float input, uint32_t* wh, uint32_t* dec)
{
	uint32_t temp;
	temp = (uint32_t)input;
	*dec =((input -(float)temp) * 10);
	*wh = (uint32_t)temp;
}

void pid_timeout(void)
{
	time_expired = 1;
}

void time_to_print(void)
{
	ttp = 1;
}

int main (void)
{

	float temp, power, set_temp = DEFAULT_SET_T, old_set_temp = 0;
	uint8_t str_pos = 0, humid;
	uint8_t chars = 0, old_pb = 1, pb = 0;
	uint8_t days, hrs, mins;
	int8_t set_humid = DEFAULT_SET_HUMID;
	volatile uint8_t selector = 0;  // selector 0 -> seting temperature, selector -> setting humidity
	uint32_t whole, decimal;
	int32_t pulses;
	uint8_t str[20];
	pidstruct_t tPid;
	
	sysclk_init();
	board_init();
	init_min_counter(); // start minute timer
	init_watering_timer();
	
	#ifdef PRINT_OVER_USB	
		udc_start();
	#endif
	
	//init PID
	pid_init(TYPE_PID, KP, KI, KD, &tPid);
	pid_set_max_integral(I_MAX, &tPid);
	pid_set_limit(100.0, 0, &tPid);
	pid_anty_windup_enable(&tPid);
	tPid.cycyles_count = 0;
	
	
	//init timer that will trigger PID controller
	stimer_set_time(PID_TIMER, 200, 1);
	stimer_register_callback(PID_TIMER, pid_timeout);
	stimer_start(PID_TIMER);
	
	stimer_set_time(PRINT_TIMER, 1000, 1);
	stimer_register_callback(PRINT_TIMER, time_to_print);
	stimer_start(PRINT_TIMER);

	display_clear();
	water(10);
	
	while(1)
	{
		//get temperature
		temp = thermo_get_temp();
		humid = thermo_get_moisture();
		if(time_expired) // time to execut pid controll loop
		{
			power = pid_execute(&tPid, (set_temp - temp));
			heater_set((uint8_t)power);
			time_expired = 0;
			
		}
		if(watering_expired)  // time to check if wattering is needed
		{
			if(set_humid)
			{
				if(humid < set_humid)
				{
					if((int8_t)(set_humid - humid) < HUMID_TRESHOLD)
					{
						water(SHORT_WATERING);
					}
					else
					{
						water(LONG_WATERING);
					}
				}
			}
			watering_expired = 0;	
		}
		
		#ifdef PRINT_OVER_USB
		if(ttp) // time to print out things over USB
		{
			
			
			float_split(set_temp, &whole, &decimal);
			str_pos = sprintf(str, "$%2u.%1u", whole, decimal);
			float_split(temp, &whole, &decimal);
			str_pos += sprintf(str + (str_pos), ", %2u.%1u;", whole, decimal );
			udi_cdc_write_buf(str, str_pos);
			
			/*
			float_split(set_temp, &whole, &decimal);
			str_pos = sprintf(str, "$%2u.%1u", whole, decimal);
			float_split(temp, &whole, &decimal);
			str_pos += sprintf(str + (str_pos), ", %2u.%1u", whole, decimal );
			str_pos += sprintf(str + (str_pos), ", %d;", (uint32_t)tPid.sum );
			udi_cdc_write_buf(str, str_pos);
			*/
			ttp = 0;
		}
		#endif
		//dispaly handling
		days = expired_time / (24*60);
		hrs = (expired_time - (60 * 24 * days)) / 60;
		mins = (expired_time - (60 * 24 * days)) % 60;
		
		float_split(set_temp, &whole, &decimal);
		chars = sprintf(str, "SET T:%2u.%1u C ", whole, decimal);
		if(selector)
		{
			sprintf(str + chars, " ");
		}
		else
		{
			sprintf(str + chars, "<");
		}
		display_write_string(0, 0, str);
		
		chars = sprintf(str, "SET H:  %2u", set_humid);
		if(selector)
		{
			sprintf(str + chars, "   <");
		}
		else
		{
			sprintf(str + chars, "    ");
		}
		display_write_string(1, 0, str);
		
		float_split(temp, &whole, &decimal);
		sprintf(str, "TMP:%2u.%1u C, H:%2u", whole, decimal, humid);
		display_write_string(2, 0, str);
		
		sprintf(str, "PWR:%3u ", (uint32_t)power);
		display_write_string(3, 0, str);
		
		sprintf(str, "DURATION:%2ud:%02uh:%02um", days, hrs, mins);
		display_write_string(4, 0, str);
		
		sprintf(str, "NEXT:%2uh:%02um", (WATTERING_INTERVAL - water_timer) / 60,  (WATTERING_INTERVAL - water_timer) % 60);
		display_write_string(5, 0, str);
		
		//encoder handling
		pulses = encoder_get();
		if(selector)
		{
			set_humid += pulses;
			if(set_humid > 10)
			{
				set_humid = 10;
			}
			else if (set_humid < 0)
			{
				set_humid = 0;
			}
		}
		else
		{
			set_temp += (pulses * 0.5);
			if(set_temp > 40.0)
			{
				set_temp = 40.0;
			}
			else if (set_temp < 10.0)
			{
				set_temp = 10.0;
			}
		}
		
		if(pulses && (!selector))
		{
			pid_reset_int(&tPid);
		}
		
		pb = encoder_get_pb();
		if(pb != old_pb)
		{
			old_pb = pb;
			if(!pb)
			{
				selector++;
				if(selector > 1)
				{
					selector = 0;
				}	
			}
			
		}
	}
	
}
