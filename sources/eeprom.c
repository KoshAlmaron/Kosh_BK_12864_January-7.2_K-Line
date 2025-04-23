#include <avr/eeprom.h>

#include "eeprom.h"			// Свой заголовок.
#include "bkdata.h"			// Структура с данными.
#include "configuration.h"	// Настройки.

// Чтение EEPROM
void read_eeprom() {
	// Пробег в метрах, расход топлива в мл.
    
	BK.DistDay = eeprom_read_dword((uint32_t*) 0);		// Пробег суточный (0-3).
	BK.DistAll = eeprom_read_dword((uint32_t*) 4);		// Пробег общий (4-7).
	BK.FuelDay = eeprom_read_dword((uint32_t*) 8);		// Топливо суточный (8-11).
	BK.FuelAll = eeprom_read_dword((uint32_t*) 12);		// Топливо всего (12-15).

    // Подсветка OLED
	BK.BrightLCD[0] = eeprom_read_byte((uint8_t*) 16);
	BK.BrightLCD[1] = eeprom_read_byte((uint8_t*) 17);
	BK.BrightLCD[2] = eeprom_read_byte((uint8_t*) 18);
}

// Запись EEPROM
void update_eeprom(uint8_t OverWrite) {
	uint32_t DstDay = BK.DistDay + BK.DistRide;
	uint32_t DstAll = BK.DistAll + BK.DistRide;

	uint32_t FuelDay = BK.FuelDay + BK.FuelRide;
	uint32_t FuelAll = BK.FuelAll + BK.FuelRide;

	if (OverWrite) {
		#ifdef WRITE_DISTANCE_DAY
			DstDay = WRITE_DISTANCE_DAY;
		#endif
		#ifdef WRITE_DISTANCE_ALL
			DstAll = WRITE_DISTANCE_ALL;
		#endif
		#ifdef WRITE_FUEL_BURNED_DAY
			FuelDay = WRITE_FUEL_BURNED_DAY;
		#endif
		#ifdef WRITE_FUEL_BURNED_ALL
			FuelAll = WRITE_FUEL_BURNED_ALL;
		#endif
	}

    eeprom_update_dword((uint32_t*) 0, DstDay); // Пробег суточный (0-3).
    eeprom_update_dword((uint32_t*) 4, DstAll); // Пробег общий (4-7).
    eeprom_update_dword((uint32_t*) 8, FuelDay); // Топливо суточный (8-11).
    eeprom_update_dword((uint32_t*) 12, FuelAll); // Топливо всего (12-15).
 
    // Подсветка OLED
    eeprom_write_byte((uint8_t*) 16, BK.BrightLCD[0]);
    eeprom_write_byte((uint8_t*) 17, BK.BrightLCD[1]);
    eeprom_write_byte((uint8_t*) 18, BK.BrightLCD[2]);
}