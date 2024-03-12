#include "light_control.h"
#include "motion.h"
extern struct time curTime;
extern struct time_set timeSetting;
uint8_t getDutyCycle_Manual(struct time_set foo)
{
	/* Illuminance 0% */
	if(foo.h_start_0 < foo.h_stop_0)
	{
		if(foo.h_start_0 <= curTime.hour && foo.h_stop_0 >= curTime.hour && foo.m_start_0 <= curTime.minutes && foo.m_stop_0 >= curTime.minutes)
			return 0;
	}
	else if(foo.h_start_0 > foo.h_stop_0)
	{
		if(( (foo.h_start_0 <= curTime.hour) && (foo.m_start_0 <= curTime.minutes) ) || ( (foo.h_stop_0 >= curTime.hour) && (foo.m_stop_0 >= curTime.minutes) ))
			return 0;
	}
	else
	{
		if(foo.m_start_0 < foo.m_stop_0)
		{
			if(foo.h_start_0 <= curTime.hour && foo.h_stop_0 >= curTime.hour && foo.m_start_0 <= curTime.minutes && foo.m_stop_0 >= curTime.minutes)
				return 0;
		}
		else
		{
			if(( (foo.h_start_0 <= curTime.hour) && (foo.m_start_0 <= curTime.minutes) ) || ( (foo.h_stop_0 >= curTime.hour) && (foo.m_stop_0 >= curTime.minutes) ))
			return 0;
		}
	}
	
	/* Illuminance 50% */
	if(foo.h_start_50 < foo.h_stop_50)
	{
		if(foo.h_start_50 <= curTime.hour && foo.h_stop_50 >= curTime.hour && foo.m_start_50 <= curTime.minutes && foo.m_stop_50 >= curTime.minutes)
			return 50;
	}
	else if(foo.h_start_50 > foo.h_stop_50)
	{
		if(( (foo.h_start_50 <= curTime.hour) && (foo.m_start_50 <= curTime.minutes) ) || ( (foo.h_stop_50 >= curTime.hour) && (foo.m_stop_50 >= curTime.minutes) ))
			return 50;
	}
	else
	{
		if(foo.m_start_50 < foo.m_stop_50)
		{
			if(foo.h_start_50 <= curTime.hour && foo.h_stop_50 >= curTime.hour && foo.m_start_50 <= curTime.minutes && foo.m_stop_50 >= curTime.minutes)
				return 50;
		}
		else
		{
			if(( (foo.h_start_50 <= curTime.hour) && (foo.m_start_50 <= curTime.minutes) ) || ( (foo.h_stop_50 >= curTime.hour) && (foo.m_stop_50 >= curTime.minutes) ))
			return 50;
		}
	}
	
	/* Illuminance 75% */
	if(foo.h_start_75 < foo.h_stop_75)
	{
		if(foo.h_start_75 <= curTime.hour && foo.h_stop_75 >= curTime.hour && foo.m_start_75 <= curTime.minutes && foo.m_stop_75 >= curTime.minutes)
			return 75;
	}
	else if(foo.h_start_75 > foo.h_stop_75)
	{
		if(( (foo.h_start_75 <= curTime.hour) && (foo.m_start_75 <= curTime.minutes) ) || ( (foo.h_stop_75 >= curTime.hour) && (foo.m_stop_75 >= curTime.minutes) ))
			return 75;
	}
	else
	{
		if(foo.m_start_75 < foo.m_stop_75)
		{
			if(foo.h_start_75 <= curTime.hour && foo.h_stop_75 >= curTime.hour && foo.m_start_75 <= curTime.minutes && foo.m_stop_75 >= curTime.minutes)
				return 75;
		}
		else
		{
			if(( (foo.h_start_75 <= curTime.hour) && (foo.m_start_75 <= curTime.minutes) ) || ( (foo.h_stop_75 >= curTime.hour) && (foo.m_stop_75 >= curTime.minutes) ))
				return 75;
		}
	}
	
	/* Illuminance 100% */
	if(foo.h_start_100 < foo.h_stop_100)
	{
		if(foo.h_start_100 <= curTime.hour && foo.h_stop_100 >= curTime.hour && foo.m_start_100 <= curTime.minutes && foo.m_stop_100 >= curTime.minutes)
			return 100;
	}
	else if(foo.h_start_100 > foo.h_stop_100)
	{
		if(( (foo.h_start_100 <= curTime.hour) && (foo.m_start_100 <= curTime.minutes) ) || ( (foo.h_stop_100 >= curTime.hour) && (foo.m_stop_100 >= curTime.minutes) ))
			return 100;
	}
	else
	{
		if(foo.m_start_100 < foo.m_stop_100)
		{
			if(foo.h_start_100 <= curTime.hour && foo.h_stop_100 >= curTime.hour && foo.m_start_100 <= curTime.minutes && foo.m_stop_100 >= curTime.minutes)
				return 100;
		}
		else
		{
			if(( (foo.h_start_100 <= curTime.hour) && (foo.m_start_100 <= curTime.minutes) ) || ( (foo.h_stop_100 >= curTime.hour) && (foo.m_stop_100 >= curTime.minutes) ))
				return 100;
		}
	}
	return 0;
}
uint8_t getDutyCycle_Auto(uint32_t light_sensor)
{
	if(light_sensor <= 500)
	{
		if(get_motion_sensor_value())
			return 100;
		else
			return 20;
	}
	else
		return 0;
}
