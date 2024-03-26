/*
 * sha204_swi.c
 *
 * Created: 3/26/2024 14:52:55
 *  Author: Student
 */ 

uint8_t sha204p_wakeup(void)
{
	swi_set_signal_pin(0);
	delay_10us(SHA204_WAKEUP_PULSE_WIDTH);
	swi_set_signal_pin(1);
	delay_ms(SHA204_WAKEUP_DELAY);
	return SHA204_SUCCESS;
}