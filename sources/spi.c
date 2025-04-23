#include <avr/io.h>			// Названия регистров и номера бит.
#include <stdint.h>			// Коротние название int.
#include "util/delay.h"		// Задержки.

#include "macros.h"			// Макросы.
#include "pinout.h"			// Начначение выводов контроллера.
#include "spi.h"			// Свой заголовок.

#ifdef SPI_CS_PIN

#define SPI_SCL_PIN B, 5
#define SPI_MOSI_PIN B, 3
#define SPI_MISO_PIN B, 4

// Настройка интерфейса.
void spi_init() {
	SPCR = 0;
	SPSR = 0;

	// Порты SCL, MOSI и CS как выходы.
	SET_PIN_MODE_OUTPUT(SPI_SCL_PIN);
	SET_PIN_MODE_OUTPUT(SPI_MOSI_PIN);
	SET_PIN_MODE_OUTPUT(SPI_CS_PIN);
	SET_PIN_HIGH(SPI_CS_PIN);
	SET_PIN_MODE_OUTPUT(SPI_DC_PIN);

	// Сброс дисплея на старте.
	SET_PIN_MODE_OUTPUT(SPI_RESET_PIN);
	SET_PIN_LOW(SPI_RESET_PIN);
	_delay_ms(200);
	SET_PIN_HIGH(SPI_RESET_PIN);

	SPCR |= (1 << MSTR);		// SPI мастер.
	SPCR |= (1 << SPE);			// SPI включен.
	SPSR |= (1 << SPI2X);		// Двойная скорость передачи.
	// Итоговый делитель скорости x2.
}

// Отправка данных, необходимо передать указатель на массив, размер 
// и тип данных: 0 - комманда, 1 - данные.
void spi_send_array(uint8_t *Array, uint16_t ArrSize, uint8_t DataType) {
	if (DataType) {SET_PIN_HIGH(SPI_DC_PIN);}
	else {SET_PIN_LOW(SPI_DC_PIN);}

	SET_PIN_LOW(SPI_CS_PIN);	// Активируем получателя.
	while(ArrSize--) {
		SPDR = *Array++;
		while(!(SPSR & (1<<SPIF)));		
	}
	SET_PIN_HIGH(SPI_CS_PIN);
}

#endif		//	SPI_CS_PIN