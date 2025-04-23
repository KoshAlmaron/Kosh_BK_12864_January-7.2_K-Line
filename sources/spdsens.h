// Датчики скорости валов.

#ifndef _SPDSENS_H_
	#define _SPDSENS_H_
	
	uint16_t get_car_speed();

	void acceleration_test(uint16_t Timer);
	uint16_t get_accel_time(uint8_t N);

#endif