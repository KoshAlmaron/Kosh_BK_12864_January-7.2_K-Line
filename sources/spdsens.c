#include <avr/interrupt.h>		// Прерывания.
#include <stdint.h>				// Коротние название int.

#include "macros.h"				// Макросы.
#include "configuration.h"		// Настройки.
#include "bkdata.h"				// Структура с данными.
#include "spdsens.h"			// Свой заголовок.

// Минимальное сырое значения для фильтрации ошибочных значений.
#define MIN_RAW_VALUE 830 		// ~180км/ч.

// Кольцевые буферы для замера оборотов, прохождение 1 зуба шагах таймера (4 мкс).
#define BUFFER_SIZE 16 + 4
#define BUFFER_BITE_SHIFT 4

volatile uint16_t ImpulseArray[BUFFER_SIZE] = {0};		// Кольцевой буфер.
volatile uint8_t BufPos = 0;							// Текущая позиция.
volatile uint8_t BufferReady = 1;						// Признак готовности буфера для записи.

// Расчет скорости авто.
uint16_t get_car_speed() {
	// Считается по количеству импульсов на 1 км.
	// Скорость = 1000000 / ([Шаг таймера, мкс] * [Кол-во шагов]) * 3600 / [Импульсов на 1 км].
	// Скорость = (1000000 / (4 * Period)) * (3600 / 25959).
	#define SPEED_CALC_COEF 34670LU

	BufferReady = 0;	// Запрещаем обновление буфера в прерываниях.

	// Переменные для хранения двух крайних значений.
	uint16_t MaxValue = ImpulseArray[0];
	uint16_t MaxValuePrev = ImpulseArray[0];
	
	uint16_t MinValue = ImpulseArray[0];
	uint16_t MinValuePrev = ImpulseArray[0];
	
	uint32_t AVG = 0;
	// Суммируем среднее и ищем крайние значения.
	for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
		AVG += ImpulseArray[i];

		if (ImpulseArray[i] < MinValue) {
			MinValuePrev = MinValue;
			MinValue = ImpulseArray[i];
		}
		if (ImpulseArray[i] > MaxValue) {
			MaxValuePrev = MaxValue;
			MaxValue = ImpulseArray[i];
		}
	}
	BufferReady = 1;	// Разрешаем обновление буфера в прерываниях.

	// Исключаем два самых больших и два самых маленьких значения.
	AVG -= (MaxValue + MaxValuePrev + MinValue + MinValuePrev);
	// Находим среднее значение.
	AVG = AVG >> BUFFER_BITE_SHIFT;

	// Рассчитываем скорость.
	uint8_t Speed = 0;
	if (AVG < UINT16_MAX - 5) {
		Speed = MIN(255, (uint32_t) (SPEED_CALC_COEF << 1) / AVG);
	}
	return Speed;
}

// Прерывание по захвату сигнала таймером 1.
ISR (TIMER1_CAPT_vect) {
	// Обнулить счётный регистр.
	TCNT1 = 0;

	// Фильтр на значения выше 180 км/ч.
	if (BufferReady && ICR1 > MIN_RAW_VALUE) {
		// Записываем значение в кольцевой буфер.	
		ImpulseArray[BufPos] = ICR1;
		BufPos++;
		if (BufPos >= BUFFER_SIZE) {BufPos = 0;}
	}

	// Подсчет пробега, ImpulsePerMeter импульсов метр.
	static uint8_t MCounter = 0;
	MCounter++;
	if (MCounter >= IMPULSES_PER_METER) {
		BK.DistRide++;
		MCounter = 0;
	}
}

// Прерывание по переполнению таймера 1.
ISR (TIMER1_OVF_vect) {
	for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
		ImpulseArray[i] = UINT16_MAX;
	}
}