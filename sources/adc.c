#include <stdint.h>			// Коротние название int.
#include <avr/io.h>			// Номера бит в регистрах.
#include <avr/interrupt.h>		// Прерывания.

#include "macros.h"			// Макросы.
#include "pinout.h"			// Назначение выводов контроллера.
#include "adc.h"			// Свой заголовок.

// Размер буфера и размер битового сдвига для деления.
#define ADC_BUFFER_SIZE 8
#define ADC_BUFFER_SHIFT 3

#ifdef AUTO_BRIGHT_ADC_CHANNEL

// Измеренные значения с буфером усреднения.
volatile static uint16_t ADCValues[ADC_BUFFER_SIZE] = {0};
// Текущая позиция в буфере.s
volatile static uint8_t BufPos = 0;

// Инициализация АЦП.
void adc_init() {
	ADMUX = 0;
	ADCSRA = 0;

	ADMUX |= (1 << REFS0);					// Опорное напряжение 5В.
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1);	// Предделитель 64.
	ADMUX |= AUTO_BRIGHT_ADC_CHANNEL;		// Установка текущего канала.
	ADCSRA |= (1 << ADIE);					// Прерывание по окончании измерения.

	ADCSRA |= (1 << ADEN);					// Включаем АЦП.
	ADCSRA |= (1 << ADSC);					// Запуск первого измерения.

	for (uint8_t i = 0; i < ADC_BUFFER_SIZE; i++) {ADCValues[i] = 1024;}
}

uint16_t get_adc_value() {
	// Находим среднее значение.
	uint32_t AVG = 0;
	cli();
	for (uint8_t i = 0; i < ADC_BUFFER_SIZE; i++) {AVG += ADCValues[i];}
	sei();

	return 1024 - (AVG >> ADC_BUFFER_SHIFT);
}

// Прерывание по окончанию измерения.
ISR (ADC_vect) {
	ADCValues[BufPos++] = ADCL | (ADCH << 8);
	if (BufPos >= ADC_BUFFER_SIZE) {BufPos = 0;}
	ADCSRA |= (1 << ADSC);		// Запуск следующего измерения.
}

#endif