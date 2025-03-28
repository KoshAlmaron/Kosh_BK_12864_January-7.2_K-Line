// Датчик аварийного давления масла.

#ifndef _DADM_H_
	#define _DADM_H_

	void dadm_init();
	void dadm_test();
	uint8_t dadm_get_state(uint16_t RPM);

#endif