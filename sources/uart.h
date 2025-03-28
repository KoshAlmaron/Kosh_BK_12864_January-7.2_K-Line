// UART.

#ifndef _UART_H_
	#define _UART_H_

	// Комманды для ЭБУ.
	#define COMMAND_STC 0				// Начало обмена.
	#define COMMAND_CDI 1				// Стереть ошибки.
	#define COMMAND_RDTCBS 2			// Считать ошибки.
	#define COMMAND_RDBLI_RLI_ASS 3		// Получить параметры двигателя.
	#define COMMAND_RDBLI_RLI_FT 4		// Получить значения АЦП.

	#define ADDRESS_ECU 0x10
	#define ADDRESS_BK 	0xF1

	void uart_init();

	uint8_t get_rx_buffer(uint8_t i);
	void set_rx_buffer(uint8_t i, uint8_t Value);

	uint8_t uart_get_uint8(uint8_t i);
	int8_t uart_get_int8(uint8_t i); 
	
	uint16_t uart_get_uint16(uint8_t i);
	int16_t uart_get_int16(uint8_t i);
	
	uint8_t uart_test_rx_packet();

	uint8_t uart_get_rx_status();
	uint8_t uart_tx_ready();

	void uart_send_command(uint8_t Command);
	void uart_add_tx_crc();

	void uart_send_array();
	
#endif

/*
	Приемник включается только после запроса, так как, во-первых,
	при работе по K-line обмен данными идет последовательно, запрос - ответ.
	Во-вторых, это исключит получени эхо пакетов от преобрахователя UART - K-Line.
*/

