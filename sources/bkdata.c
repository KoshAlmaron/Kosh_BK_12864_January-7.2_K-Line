#include <stdint.h>			// Коротние название int.

#include "uart.h"			// UART.
#include "bkdata.h"			// Свой заголовок.

// Инициализация структуры
BK_t BK = {
	.DataStatus = 0,
	.DataError = 0,

	.DistRide = 0,
	.FuelRide = 0,
	.DistDay = 0,
	.FuelDay = 0,
	.DistAll = 0,
	.FuelAll = 0,
	.RideTimer = 0,
	.FuelConsumptionRatio = 100,
	.BrightLCD = {127, 32, 200},
	.ScreenMode = 0,
	.ScreenChange = 0,
	.StartStop = 1,

	.AlarmBoxTimer = 0,
	.ConfigBoxTimer = 0

};