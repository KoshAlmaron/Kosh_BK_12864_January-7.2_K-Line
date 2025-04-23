#include <avr/interrupt.h>		// Прерывания.
#include <stdint.h>				// Коротние название int.

#include "macros.h"				// Макросы.
#include "configuration.h"		// Настройки.
#include "bkdata.h"				// Структура с данными.
#include "display.h"			// Экраны БК.
#include "adc.h"				// АЦП.
#include "spdsens.h"			// Свой заголовок.

// Минимальное сырое значения для фильтрации ошибочных значений.
#define MIN_RAW_VALUE 173 		// ~200км/ч.

// Кольцевые буферы для замера оборотов, прохождение 1 зуба шагах таймера (4 мкс).
#define BUFFER_SIZE 16 + 4
#define BUFFER_BITE_SHIFT 4

volatile uint16_t ImpulseArray[BUFFER_SIZE] = {0};		// Кольцевой буфер.
volatile uint8_t BufPos = 0;							// Текущая позиция.
volatile uint8_t BufferReady = 1;						// Признак готовности буфера для записи.

uint16_t AccelTimer[3] = {0};

// Расчет скорости авто.
uint16_t get_car_speed() {
	#ifdef DEBUG_MODE
		return get_adc_value() >> 2;
	#endif

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
	uint16_t Speed = 0;
	if (AVG < UINT16_MAX - 5) {
		Speed = MIN(255, (uint32_t) SPEED_CALC_COEF / AVG);
	}
	return Speed;
}

void acceleration_test(uint16_t Timer) {
	static uint8_t PrevSpeed = 0;
	
	if (BK.ScreenMode != DISPLAY_ACCELERATION) {
		if (BK.AccelMeterStatus != 5 && BK.AccelMeterStatus != 6) {return;}
	}

	uint8_t Speed = get_car_speed();
	switch (BK.AccelMeterStatus) {
		case 0:
			if (BK.ScreenMode == DISPLAY_ACCELERATION) {
				if (!Speed) {BK.AccelMeterStatus = 3;}	// Готов к замеру 0-100.
				else if (Speed > 0 && Speed <= 20) {BK.AccelMeterStatus = 1;}	// Ожидание остановки.
				else if(Speed > 20) { // Ожидание скорости < 50 км/ч.
					BK.AccelMeterStatus = 2;
					if (Speed < 55) {BK.AccelMeterStatus = 4;}
				}
				for (uint8_t i = 0; i < 3; i++) {AccelTimer[i] = 0;}	// Сброс таймеров.
			}
			break;
		case 1:
			if (!Speed) {BK.AccelMeterStatus = 3;}	// Готов к замеру 0-100.
			break;
		case 2:
			if (Speed < 55) {BK.AccelMeterStatus = 4;}	// Готов к замеру 60-100.
			break;
		case 3:
			if (Speed) {BK.AccelMeterStatus = 5;}	// Старт замера скорости 0-100.
			break;
		case 4:
			if (Speed > 60) {BK.AccelMeterStatus = 6;}	// Старт замера скорости 60-100.
			else if (Speed <= 20) {BK.AccelMeterStatus = 1;}
			break;
		case 5:
			if (PrevSpeed < 30 && Speed >= 30) {AccelTimer[0] = Timer;}
			if (PrevSpeed < 60 && Speed >= 60) {AccelTimer[1] = Timer;}
			if (PrevSpeed < 100 && Speed >= 100) {
				AccelTimer[2] = Timer;
				BK.AccelMeterStatus = 7;
			}
			break;
		case 6:
			if (PrevSpeed < 100 && Speed >= 100) {
				AccelTimer[2] = Timer;
				BK.AccelMeterStatus = 8;
			}
			break;
	}

	PrevSpeed = Speed;
	if (Timer > 30000) {BK.AccelMeterStatus = 64;}
}

uint16_t get_accel_time(uint8_t N) {
	// AccelTimer[0] = 1556;
	// AccelTimer[1] = 3577;
	// AccelTimer[2] = 7022;
	return AccelTimer[N];
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