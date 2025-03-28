// Управление OLED на базе SSD1306.

#ifndef _OLED_H_
	#define _OLED_H_

	void oled_init(uint8_t Addr, uint8_t Contrast, uint8_t Flip);
	void oled_set_bright(uint16_t Bright);
	void oled_clear_buffer();
	void oled_shift_graph_block();
	uint8_t oled_ready();
	void oled_send_buffer();
	void oled_send_command(uint16_t BufferSize);

	void oled_draw_mode(uint8_t Mode);
	void oled_set_clip_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
	void oled_disable_clip_window();
	void oled_draw_pixel(uint8_t x, uint8_t y);
	void oled_draw_h_line(uint8_t x, uint8_t y, uint8_t l);
	void oled_draw_v_line(uint8_t x, uint8_t y, uint8_t l);
	void oled_draw_box(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t Mode);
	void oled_draw_frame(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
	void oled_draw_xbmp(uint8_t x, uint8_t y, const uint8_t* xbm, uint8_t w, uint8_t h);

	void oled_set_font(const uint8_t* FontName);
	const uint8_t* oled_get_char_array(char Symbol);
	uint8_t oled_print_char(uint8_t x, uint8_t y, char Symbol);
	void oled_print_string(uint8_t x, uint8_t y, char* String, uint8_t MsgSize);
	void oled_print_string_f(uint8_t x, uint8_t y, const char* String, uint8_t MsgSize);
	char char_shift(char Symbol);

#endif

/*
17506
1430

17394
1430
*/