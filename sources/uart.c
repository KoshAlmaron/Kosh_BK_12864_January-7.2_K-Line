#include <stdint.h>			// Коротние название int.
#include <avr/interrupt.h>	// Прерывания.

#include "uart.h"			// Свой заголовок.

#define UART_BAUD_RATE 10400UL

#define RX_BUFFER_SIZE 128 + 6	// Размер буфера приема.
#define TX_BUFFER_SIZE 16		// Размер буфера отправки.

#define SET_UBRR ((F_CPU / (8UL * UART_BAUD_RATE)) - 1UL)	// 191
//#define SET_UBRR 220

static volatile uint8_t RxBuffer[RX_BUFFER_SIZE] = {0};	// Буфер приема.
static volatile uint8_t HeaderSize = 0;					// Длина заголовка.
static volatile uint8_t RxMsgSize = 0;						// Длина блока данных.
static volatile uint8_t RxBuffPos = 0;						// Позиция в буфере.
static volatile uint8_t RxEchoSize = 0;					// Длина эхо пакета.

static uint8_t RxMsgAnswer = 0x00;		// Код положительного ответ.
static uint8_t RxMsgType = 0x00;		// Код ожидаемого пакета.
static uint8_t RxDataType = 0x00;		// Код ожидаемого блока данных.

/*
	Статусы приема пакета:
		0 - готов к приему,
		1 - идет прием пакета,
		2 - пакет принят,
		3 - пакет проверен,

		16 - ожидание,

		20 - ошибка, не правильное начало пакета,
		21 - ошибка, не совпадает длина пакета,
		22 - ошибка, превышение максимальной длины пакета,

		30 - ошибка, чужой пакет,
		31 - ошибка, неизвестный отправитель,
		32 - ошибка, отрицательный ответ ЭБУ,
		33 - ошибка, не верный тип диагностических данных,
		34 - ошибка, контрольная сумма не совпадает.
*/

static volatile uint8_t RxDataStatus = 16;

static uint8_t	TxBuffer[TX_BUFFER_SIZE] = {0};		// Буфер отправки.
static uint16_t TxMsgSize = 0;						// Количество байт для отправки.
static volatile uint16_t TxBuffPos = 0;			// Позиция в буфере.
static volatile uint8_t TxReady = 1;				// UART готов к отправке.

void uart_init() {
	// Сброс регистров настроек, так как загрузчик Arduino может нагадить.
	UCSR0A = 0;
	UCSR0B = 0;
	UCSR0C = 0;				// Асинхронный режим, 1 стоповый бит, без проверки четности.

	UCSR0A |= (1 << U2X0);							// Двойная скорость передачи.
	UCSR0C |= (1 << UCSZ00) | ( 1 << UCSZ01);		// Размер пакета 8 бит.
	
	UBRR0H = (uint8_t) (SET_UBRR >> 8);				// Настройка скорости.
	UBRR0L = (uint8_t) SET_UBRR;
			
	UCSR0B |= (1 << RXEN0);		// Прием.
	UCSR0B |= (1 << TXEN0);		// Передача.
	UCSR0B |= (1 << RXCIE0);	// Прерывание по завершению приёма.
}

uint8_t get_rx_buffer(uint8_t i) {return RxBuffer[i];}
void set_rx_buffer(uint8_t i, uint8_t Value) {
	i += HeaderSize - 1;
	RxBuffer[i] = Value;
}

uint8_t uart_get_uint8(uint8_t i) {
	i += HeaderSize - 1;
	return RxBuffer[i];
}

int8_t uart_get_int8(uint8_t i) {
	i += HeaderSize - 1;
	int8_t Value = 0;
	uint8_t *pValue = (uint8_t*)&Value;
	*pValue = RxBuffer[i];
	return Value;
}

uint16_t uart_get_uint16(uint8_t i) { 
	i += HeaderSize - 1;
	uint16_t Value = 0;
	uint8_t *pValue = (uint8_t*)&Value;
	*pValue = RxBuffer[i + 1];  
	*(pValue + 1) = RxBuffer[i];
	return Value;
}

int16_t uart_get_int16(uint8_t i) {
	i += HeaderSize - 1;
	int16_t Value = 0;
	uint8_t *pValue = (uint8_t*)&Value;
	*pValue = RxBuffer[i + 1];  
	*(pValue + 1) = RxBuffer[i];
	return Value;
}

// Проверка принятого пакета.
uint8_t uart_test_rx_packet() {
	if (RxBuffer[1] != ADDRESS_BK) {
		RxDataStatus = 30;	// Чужой пакет.
		return RxDataStatus;
	}
	if (RxBuffer[2] != ADDRESS_ECU) {
		RxDataStatus = 31;	// Неизвестный отправитель.
		return RxDataStatus;
	}
	if (RxBuffer[HeaderSize] != RxMsgAnswer) {
		RxDataStatus = 32;	// Отрицательный ответ ЭБУ.
		return RxDataStatus;
	}
	if (RxMsgAnswer == 0x61 && RxBuffer[HeaderSize + 1] != RxDataType) {
		RxDataStatus = 33;	// Не верный тип диагностических данных.
		return RxDataStatus;
	}	

	uint8_t CRC = 0;
	for (uint8_t i = 0; i < RxBuffPos - 1; i++) {
		CRC += RxBuffer[i];
	}
	if (RxBuffer[RxBuffPos - 1] != CRC) {
		RxDataStatus = 34;	// Контрольная сумма не совпадает.
		return RxDataStatus;
	}
	return 3;	// Пакет проверен
}

uint8_t uart_get_rx_status() {return RxDataStatus;}

// Возвращает готовность интерфейса.
uint8_t uart_tx_ready() {
	//if (RxDataStatus == 1) {return 0;}	// Идет прием.
	return TxReady;
}

void uart_send_command(uint8_t Command) {
	TxBuffPos = 0;
	TxBuffer[TxBuffPos++] = 0x80;
	TxBuffer[TxBuffPos++] = ADDRESS_ECU;	// Адрес ЭБУ.
	TxBuffer[TxBuffPos++] = ADDRESS_BK;		// Адрес БК.

	switch (Command) {
		case COMMAND_STC:			// Начало обмена.
			RxMsgType = 0x81;		// Код запроса.
			RxMsgAnswer = 0xC1;		// Код положительного ответа.
			TxBuffer[TxBuffPos++] = RxMsgType;
			break;
		case COMMAND_CDI:			// Стереть ошибки.
			RxMsgType = 0x14;
			RxMsgAnswer = 0x54;
			TxBuffer[TxBuffPos++] = RxMsgType;
			TxBuffer[TxBuffPos++] = 0x00;	// Все коды вместе с их статусом.
			TxBuffer[TxBuffPos++] = 0x00;	// Все группы ошибок.
			break;
		case COMMAND_RDTCBS:		// Считать ошибки.
			RxMsgType = 0x18;
			RxMsgAnswer = 0x58;		
			TxBuffer[TxBuffPos++] = RxMsgType;
			TxBuffer[TxBuffPos++] = 0x00;	// Получить все коды вместе с их статусом.
			TxBuffer[TxBuffPos++] = 0xFF;	// Получить все группы ошибок.
			TxBuffer[TxBuffPos++] = 0x00;	// Получить все группы ошибок.
			break;
		case COMMAND_RDBLI_RLI_ASS:	// Получить параметры двигателя.
			RxMsgType = 0x21;
			RxMsgAnswer = 0x61;	
			RxDataType = 0x01;
			TxBuffer[TxBuffPos++] = RxMsgType;
			TxBuffer[TxBuffPos++] = RxDataType;	// Параметры двигателя.
			break;
		case COMMAND_RDBLI_RLI_FT:	// Получить значения АЦП.
			RxMsgType = 0x21;
			RxMsgAnswer = 0x61;
			RxDataType = 0x03;
			TxBuffer[TxBuffPos++] = RxMsgType;
			TxBuffer[TxBuffPos++] = RxDataType;	// Значения АЦП.
			break;
	}

	TxBuffer[0] += TxBuffPos - 3;	// Запись длины пакета.
	uart_add_tx_crc();				// Расчет контрольной суммы.

	// Сброс переменных по приему.
	RxBuffPos = 0; 		
	HeaderSize = 0;
	RxMsgSize = 0;
	
	uart_send_array();
}

void uart_add_tx_crc() {
	uint8_t CRC = 0;
	for (uint8_t i = 0; i < TxBuffPos; i++) {
		CRC += TxBuffer[i];
	}
	TxBuffer[TxBuffPos++] = CRC;
}

// Отправить массив в UART.
void uart_send_array() {
	TxMsgSize = TxBuffPos;		// Количество байт на отправку.
	RxEchoSize = TxBuffPos;		// Длина эхо пакета для отсечения на приеме.
	TxBuffPos = 0;				// Сбрасываем позицию в буфере передачи.
	TxReady = 0;				// UART занят.
	UCSR0B |= (1 << UDRIE0);	// Включаем прерывание по опустошению буфера.
	RxDataStatus = 0;			// Включаем прием.
}

// Прерывание по опустошению буфера.
ISR (USART_UDRE_vect) {
	UDR0 = TxBuffer[TxBuffPos++];	// Загружаем очередной байт.
	if (TxBuffPos >= TxMsgSize) {	// Все данные загружены в буфер отправки.
		UCSR0B &= ~(1 << UDRIE0);	// Отключаеи передатчик.
		TxReady = 1;
	}
}

// Прерывание по окончании приема.
ISR (USART_RX_vect) {
	uint8_t OneByte = UDR0;	// Байт надо всегда забирать, иначе будет петля.

	if (RxDataStatus > 1) {return;}

	if (RxEchoSize) {	// Пропускаем эхо пакет.
		RxEchoSize--;
		return;
	}

	uint8_t Error = 0x00;
	RxBuffer[RxBuffPos] = OneByte;

	// Проверка пакета.
	switch (RxBuffPos) {
		case 0:			// Длина пакета № 1.
			if (RxBuffer[RxBuffPos] < 0x80) {	// Не правильное начало пакета.
				Error = 20;
				break;
			}
			else if (RxBuffer[RxBuffPos] > 0x80) {
				HeaderSize = 3;
				RxMsgSize = RxBuffer[RxBuffPos] - 0x80;
			}
			RxDataStatus = 1;
			break;
		case 3:			// Длина пакета № 2.
			if (!RxMsgSize) {
				HeaderSize = 4;
				RxMsgSize = RxBuffer[RxBuffPos];
			} 
			break;
	}
	RxBuffPos++;

	// Конец пакета.
	if (RxMsgSize) {
		if (RxBuffPos == RxMsgSize + HeaderSize + 1) {
			RxDataStatus = 2;
			return;
		}
		if (RxBuffPos > RxMsgSize + HeaderSize + 1) {Error = 21;}
	}

	if (RxBuffPos > 128 + 4)  {Error = 22;}	// Максимальная длина для протокола.
	if (Error) {RxDataStatus = Error;}		// Остановка приема при ошибке.
}