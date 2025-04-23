// Экраны БК.

#ifndef _DISPLAY_H_
	#define _DISPLAY_H_

	void display_draw_init();
	void display_draw_data(uint8_t Timer);
	void display_draw_no_signal();
	void display_draw_finish();

	#define DISPLAY_MAIN			0
	#define DISPLAY_SECOND			1
	#define DISPLAY_ACCELERATION	2
	#define DISPLAY_GRAPH			3
	#define DISPLAY_CURRENT_ERRORS	4
	#define DISPLAY_SAVED_ERRORS	5
	#define DISPLAY_ADC				6
	#define DISPLAY_STATISTICS		10
	
#endif