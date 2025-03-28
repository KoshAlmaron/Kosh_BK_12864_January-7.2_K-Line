
#include <avr/pgmspace.h>		// Работа с PROGMEM.

#include "configuration.h"		// Настройки.
#include "macros.h"				// Макросы.
#include "pinout.h"				// Начначение выводов контроллера.

#ifdef SPI_CS_PIN
	#include "spi.h"			// SPI.
#else
	#include "i2c.h"			// I2C (TWI).
#endif

#include "oled.h"				// Свой заголовок.

#define OLED_WIDTH	128
#define OLED_HEIGHT	64

#define DATA_BUFFER_SIZE OLED_WIDTH * OLED_HEIGHT / 8
#define SEND_BUFFER_SIZE DATA_BUFFER_SIZE + 2
#define COMMAND_BUFFER_SIZE 10

// Массив данных экрана (128 * 64 / 8 + адрес + команда). 
static uint8_t SendBuffer[SEND_BUFFER_SIZE];
// Указатель на третий элемент массива - первый элемент данных.
static uint8_t *DataBuffer = (SendBuffer + 2);

// Массив для отправки команд.
static uint8_t CommandBuffer[COMMAND_BUFFER_SIZE];

// Режим отрисовки пикселя, 0 - номальный, 1 - инверсия (XOR), 2 - стирание.
static uint8_t DrawMode = 0;

// Ограничения для отрисовки пикселей.
static uint8_t Xmin = 0;
static uint8_t Ymin = 0;
static uint8_t Xmax = OLED_WIDTH - 1;
static uint8_t Ymax = OLED_HEIGHT - 1;

// Параметры выбранного шрифта.
static const uint8_t* Font = 0;			// Указатель на массив символов.
static uint8_t FontFirstSymbol = 0;		// Код первого символа в массиве.
static uint8_t FontSymbolsCount = 0;	// Количество символов.
static uint8_t FontHeight = 0;			// Высота шрифта.
static uint8_t FontSpace = 0;			// Расстояние между символами.

static const uint8_t OledBaseConfig[] PROGMEM = {
	0xAE,		// Выключить дисплей.
	0x20,		// Режим адресации памяти,
		0x00,		// Значение - горизонтальный.
	0xA1, 	// Отображение по горизонтали, A1 - нормально, A0 - зеркально.
	0xC8,	// Отображение по вертикали, С8 - нормально, С0 - зеркально.
	0x2E, 	// Отключение прокрутки.
	0xA4, 	// Отображение содержимого RAM.
	0xA6,   // Нормальное отображение, 0xA7 - инверсное.
	0x22,	// Установка диапазона страниц (строк):
		0x00,					// первая,
		(OLED_HEIGHT >> 3) - 1,	// последняя.
	#if OLED_HEIGHT == 32
		0xA8,	// Установка коэффициента мультиплексирования.
			0x1F,	 // 128x32 - 0x1F.
		0xDA,	// Конфигурацию выводов COM-сигналов.
			0x02	 // 128x32 - 0x02.
	#else
		0xA8,	// Установка коэффициента мультиплексирования.
			0x3F,	 // 128x32 - 0x1F.
		0xDA,	// Конфигурацию выводов COM-сигналов.
			0x12	 // 128x32 - 0x02.
	#endif
};

// Настройка дисплея, необходимо передать адрес и тип.
void oled_init(uint8_t Addr, uint8_t Contrast, uint8_t Flip) {
	// Используем буфер данных для отправки команд инициализации.
	uint8_t BufPos = 0;
	#ifdef SPI_CS_PIN	
		// Для SPI тип команда или данные отпределяется выводом DC.
		for (uint8_t i = 0; i < 15; i++) {
			SendBuffer[BufPos++] = pgm_read_byte(OledBaseConfig + i);
		}
		if (Flip) {
			SendBuffer[BufPos++] = 0xA0; 	// Отображение по горизонтали, A1 - нормально, A0 - зеркально.
			SendBuffer[BufPos++] = 0xC0;	// Отображение по вертикали, С8 - нормально, С0 - зеркально.		
		}
		SendBuffer[BufPos++] = 0x81;	// Установка контрастности. 
		SendBuffer[BufPos++] = 	Contrast;	// Значение контрастности.
		SendBuffer[BufPos++] = 0xAF;	// 0xAF - экран включен в нормальном режиме, 0xAE - экран выключен.
		spi_send_array(SendBuffer, BufPos, 0);
	#else
		// Для I2C тип команда или данные отпределяется байтом-флагом 0x80.
		SendBuffer[BufPos++] = Addr;		// Первый элемент это адрес устройства.
		// Проставляем про запас в каждый чётный флаг команды - 0x80.
		for (uint8_t i = 0; i < 32; i++ ) {SendBuffer[i * 2 + 1] = 0x80;}
		for (uint8_t i = 0; i < 15; i++) {
			BufPos++;
			SendBuffer[BufPos++] = pgm_read_byte(OledBaseConfig + i);
		}
		if (Flip) {
			BufPos++;
			SendBuffer[BufPos++] = 0xA0; 	// Отображение по горизонтали, A1 - нормально, A0 - зеркально.
			BufPos++;
			SendBuffer[BufPos++] = 0xC0;	// Отображение по вертикали, С8 - нормально, С0 - зеркально.		
		}
		BufPos++;
		SendBuffer[BufPos++] = 0x81;	// Установка контрастности.
		BufPos++;
		SendBuffer[BufPos++] = 	Contrast;	// Значение контрастности.
		BufPos++;
		SendBuffer[BufPos++] = 0xAF;	// 0xAF - экран включен в нормальном режиме, 0xAE - экран выключен.

		while (!(oled_ready()));				// Ожидание готовности интерфейса.
		i2c_send_array(SendBuffer, BufPos);		// Отправка.
		while (!(oled_ready()));

		SendBuffer[1] = 0x40;	// Второй элемент комманда отправки массива данных.
		// Заготовка для буфера команд.
		CommandBuffer[0] = Addr;	// Первый элемент это адрес устройства.
		// Все нечетныне элементы 0x80 означает, что следующий байт - это команда.
		for (uint8_t i = 0; i < COMMAND_BUFFER_SIZE / 2; i++ ) {
			CommandBuffer[i * 2 + 1] = 0x80;	
		}
	#endif

	// Очистка экрана.
	oled_clear_buffer();
	while (!(oled_ready()));
	oled_send_buffer();
	while (!(oled_ready()));
}

void oled_set_bright(uint16_t Bright) {
	#ifdef SPI_CS_PIN
		CommandBuffer[0] = 0x81;	// Установка контрастности. 
		CommandBuffer[1] = Bright;	// Значение контрастности.
		oled_send_command(2);		// Отправка.
	#else
		CommandBuffer[2] = 0x81;		// Установка контрастности.
		CommandBuffer[4] = Bright;		// Значение контрастности.

		while (!(oled_ready()));		// Ожидание готовности интерфейса.
		oled_send_command(5);			// Отправка.
		while (!(oled_ready()));
	#endif
}

void oled_clear_buffer() {
	for (uint16_t i = 0; i < DATA_BUFFER_SIZE; i++) {DataBuffer[i] = 0;}
}

void oled_shift_graph_block() {
	uint16_t ByteNumber = 0;
	for (uint8_t Row = 1; Row < 8; Row++) {
		for (uint8_t x = 0; x <= 100; x++) {
			ByteNumber = Row * 128 + x;
			DataBuffer[ByteNumber] = DataBuffer[ByteNumber + 1];
		}
	}
}

uint8_t oled_ready() {
	#ifdef SPI_CS_PIN
		return 1;
	#else
		return i2c_ready();
	#endif
}

void oled_send_buffer() {
	#ifdef SPI_CS_PIN
		CommandBuffer[0] = 0x40;	// Второй элемент команда отправки массива данных.
		spi_send_array(CommandBuffer, 1, 0);
		spi_send_array(DataBuffer, DATA_BUFFER_SIZE, 1);
	#else
		i2c_send_array(SendBuffer, SEND_BUFFER_SIZE);
	#endif
}

void oled_send_command(uint16_t BufferSize) {
	#ifdef SPI_CS_PIN
		spi_send_array(CommandBuffer, BufferSize, 0);
	#else
		while (!(oled_ready()));
		i2c_send_array(CommandBuffer, BufferSize);
		while (!(oled_ready()));
	#endif
}

void oled_draw_mode(uint8_t Mode) {DrawMode = Mode;}

void oled_set_clip_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
	Xmin = x0;
	Ymin = y0;
	Xmax = x1;
	Ymax = y1;
}

void oled_disable_clip_window() {
	Xmin = 0;
	Ymin = 0;
	Xmax = OLED_WIDTH - 1;
	Ymax = OLED_HEIGHT - 1;
}

void oled_draw_pixel(uint8_t x, uint8_t y) {
	if (x < Xmin || x > Xmax) {return;}
	if (y < Ymin || y > Ymax) {return;}

	uint16_t ByteNumber = ((y >> 3) << 7) + x;
	if (ByteNumber < DATA_BUFFER_SIZE) {
		switch (DrawMode) {			// Режим отрисовки пикселя.
			case 0:					// 0 - номальный.
				DataBuffer[ByteNumber] |= 1 << (y % 8);
				break;
			case 1:					// 1 - инверсия (XOR).
				DataBuffer[ByteNumber] ^= 1 << (y % 8);
				break;
			case 2:					// 2 - стирание.
				DataBuffer[ByteNumber] &= ~(1 << (y % 8));
				break;
		}
	}
}

void oled_draw_h_line(uint8_t x, uint8_t y, uint8_t l) {
	for (uint8_t i = 0; i < l; i++) {
		oled_draw_pixel(x + i, y);
	}
}

void oled_draw_v_line(uint8_t x, uint8_t y, uint8_t l) {
	for (uint8_t i = 0; i < l; i++) {
		oled_draw_pixel(x, y + i);
	}
}

void oled_draw_box(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t Mode) {
	uint8_t PrevDrawMode = DrawMode;
	DrawMode = Mode;
	for (uint8_t i = 0; i < h; i++) {
		oled_draw_h_line(x, y + i, w);
	}
	DrawMode = PrevDrawMode;
}

void oled_draw_frame(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
	if (w > 1) {
		oled_draw_h_line(x, y, w);
		if (h > 1) {oled_draw_h_line(x, y + h - 1, w);}
	}
	if (h > 2) {
		oled_draw_v_line(x, y + 1, h - 2);
		if (w > 1) {oled_draw_v_line(x + w - 1, y + 1, h - 2);}
	}
}

void oled_draw_xbmp(uint8_t x, uint8_t y, const uint8_t* xbm, uint8_t w, uint8_t h) {
	uint8_t blen = (w + 7) >> 3;	// Ширина в байтах.
	uint8_t mask;					// Маска получения бита.

	for (uint8_t i = 0; i < h; i++) {
		for (uint8_t k = 0; k < blen; k++) {
			mask = 1;
			for (uint8_t j = 0; j < 8; j++) {
				uint8_t px = k * 8 + j;
				if (px < w) {	// Ширина изображения.
					px += x;
					//if (px >= OLED_WIDTH) {break;}	// Достигнут предел экрана по X.
					if (px >= OLED_WIDTH) {px -= OLED_WIDTH;}	// Бегущая строка.
					if (pgm_read_byte(xbm + k) & mask) {oled_draw_pixel(px, y);}
				}
				mask <<= 1;
			}
		}
		xbm += blen;					// Смещаем позицию в массиве на 1 строку.
		y++;							// Переход на следующуй строку.
		if (y >= OLED_HEIGHT) {return;}	// Достигнут предел экрана по Y.
	}
}

void oled_set_font(const uint8_t* FontName) {
	FontFirstSymbol = pgm_read_byte(FontName);
	FontSymbolsCount = pgm_read_byte(FontName + 1);
	FontHeight = pgm_read_byte(FontName + 2);
	FontSpace = pgm_read_byte(FontName + 3);
	Font = FontName + 4;
}

const uint8_t* oled_get_char_array(char Symbol) {
	const uint8_t* CharArray = Font;
	// Символ есть в шрифте.
	if (Symbol >= FontFirstSymbol && Symbol < FontFirstSymbol + FontSymbolsCount) {
		for (uint8_t i = 0; i < Symbol - FontFirstSymbol; i++) {
			CharArray += ((pgm_read_byte(CharArray) + 7) >> 3) * FontHeight + 1;
		}
	}
	else {return 0;}
	return CharArray + 1;
}

uint8_t oled_print_char(uint8_t x, uint8_t y, char Symbol) {
	Symbol = char_shift(Symbol);

	const uint8_t* CharArray = Font;
	// Символ есть в шрифте.
	if (Symbol >= FontFirstSymbol && Symbol < FontFirstSymbol + FontSymbolsCount) {
		for (uint8_t i = 0; i < Symbol - FontFirstSymbol; i++) {
			CharArray += ((pgm_read_byte(CharArray) + 7) >> 3) * FontHeight + 1;
		}
	}
	else {return 0;}

	uint8_t CharWidth = pgm_read_byte(CharArray);
	oled_draw_xbmp(x, y, CharArray + 1, CharWidth, FontHeight);
	return CharWidth;	// Возвращаем ширину напечатанного символа.
}

void oled_print_string(uint8_t x, uint8_t y, char* String, uint8_t MsgSize) {
	uint8_t DeltaX = 0;
	for (uint8_t i = 0; i < MsgSize; i++) {
		uint8_t CharWidth = oled_print_char(x + DeltaX, y, String[i]);
		if (CharWidth) {DeltaX += (CharWidth + FontSpace);}
	}
}

void oled_print_string_f(uint8_t x, uint8_t y, const char* String, uint8_t MsgSize) {
	uint8_t DeltaX = 0;
	for (uint8_t i = 0; i < MsgSize; i++) {
		char Symbol = pgm_read_byte(String + i);
		uint8_t CharWidth = oled_print_char(x + DeltaX, y, Symbol);
		if (CharWidth) {DeltaX += (CharWidth + FontSpace);}
	}
}

// Смещение символов для таблицы шрифта.
char char_shift(char Symbol) {
	switch (Symbol) {
		case '+':
			Symbol = 0x3a;
			break;
		case '-':
			Symbol = 0x3b;
			break;
		case '.':
			Symbol = 0x3c;
			break;
		case ' ':
			Symbol = 0x3d;
			break;
		case '_':
			Symbol = 0x3e;
			break;
	}
	if (Symbol > 64) {Symbol = Symbol - 2;}
	if (Symbol < 0) {Symbol = Symbol + 191;}
	return Symbol;
}


/*
		0		1
	-----------------
	01234567 01234567

	0 1
	---
	0 0
	1 1
	2 2
	3 3
	4 4
	5 5
	6 6
	7 7

*/


/*
	0x80 — означает, что следом идёт один байт команды,
	0xC0 — означает, что следом идёт один байт данных,
	0x40 — следом идёт много байт данных.

	https://microsin.net/adminstuff/hardware/ssd1306-oled-controller.html
*/