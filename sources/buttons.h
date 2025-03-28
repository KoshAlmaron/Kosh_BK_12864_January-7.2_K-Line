// Кнопки.

#ifndef _BUTTONS_H_
	#define _BUTTONS_H_

	#define BTN_UP 		0
	#define BTN_DOWN	1
	#define BTN_LEFT	2
	#define BTN_RIGHT	3
	
	void buttons_init();
	void button_action();
	void buttons_clear();
	void buttons_update();
	uint8_t buttons_get_state(uint8_t N);

#endif