// Блоки данных для экрана.

#ifndef _BLOCKS_H_
	#define _BLOCKS_H_

	void draw_error_box();
	void draw_no_signal();
	void draw_config_box(uint8_t x, uint8_t y);
	void draw_acceleration();
	
	void draw_current_errors();
	void draw_saved_errors();
	void draw_adc_value();
	void draw_graph(uint8_t GraphNumber);

	void draw_water_temp_f(uint8_t x, uint8_t y);
	void draw_O2_sensor_f(uint8_t x, uint8_t y);
	void draw_O2_sensor_h(uint8_t x, uint8_t y);
	void draw_trottle_f(uint8_t x, uint8_t y);
	void draw_rpm_f(uint8_t x, uint8_t y);
	void draw_map_f(uint8_t x, uint8_t y);
	void draw_inj_corr_h(uint8_t x, uint8_t y);
	void draw_angle_h(uint8_t x, uint8_t y);
	void draw_speed_f(uint8_t x, uint8_t y, uint8_t Type);
	void draw_battery_f(uint8_t x, uint8_t y);
	void draw_battery_h(uint8_t x, uint8_t y);
	void draw_ff_fc_f(uint8_t x, uint8_t y);
	void draw_airtemp_h(uint8_t x, uint8_t y);
	void draw_oil_pressure_f(uint8_t x, uint8_t y);
	void draw_pulse_widgh_f(uint8_t x, uint8_t y);
	void draw_gbc_f(uint8_t x, uint8_t y);
	void draw_air_flow_f(uint8_t x, uint8_t y);

	void draw_distance_f(uint8_t x, uint8_t y);
	void draw_afc_f(uint8_t x, uint8_t y);
	void draw_fuel_burned_f(uint8_t x, uint8_t y);

	void draw_statistics();

#endif