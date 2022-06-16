#include "rc5/rc5.h"

struct gpio_callback rc5_rx_irq;
struct gpio_dt_spec rc5_decoder = GPIO_DT_SPEC_GET(DT_ALIAS(tsop), gpios);

static volatile int64_t isr_end_timestamp = 0;
static volatile int64_t isr_begin_timestamp = 0;

static const int short_min_us = 444;
static const int short_max_us = 1333;
static const int long_min_us = 1334;
static const int long_max_us = 2222;

static const int trans[4] = {0x01, 0x91, 0x9b, 0xfb};
volatile int command;
int counter;
volatile bool has_new;

typedef enum {
    STATE_START1, 
    STATE_MID1, 
    STATE_MID0, 
    STATE_START0, 
    STATE_ERROR, 
    STATE_BEGIN, 
    STATE_END
} State;


State state = STATE_BEGIN;

void rc5_signal_received(void) {
	int64_t isr_begin_timestamp = k_uptime_ticks();
	int64_t elapsed_since_last_isr = isr_begin_timestamp - isr_end_timestamp;

	int64_t elapsed_us = (elapsed_since_last_isr * 1000 * 1000) / CONFIG_SYS_CLOCK_TICKS_PER_SEC;

	int event = 0;
	if(gpio_pin_get_dt(&rc5_decoder)){
		event = STATE_START1;
	}else{
		event = STATE_MID0;
	}
	if(elapsed_us > long_min_us && elapsed_us < long_max_us){
		event += 4;
	}else if(elapsed_us < short_min_us || elapsed_us > short_max_us){
		rc5_reset();
	}

	if(state == STATE_BEGIN){
		counter--;
		command |= 1 << counter;
		state = STATE_MID1;
		isr_end_timestamp = k_uptime_ticks();
		return;
	}

	// todo: what the fuck
	State newstate = (trans[state] >> event) & 0x03;

	if(newstate == state || state > STATE_START0){
		rc5_reset();
		return;
	}
	
	state = newstate;
	if(state == STATE_MID0){
		counter--;
	}else if(state == STATE_MID1){
		counter--;
		command |= 1 << counter;
	}
	if(counter == 0 && (state == STATE_START1 || state == STATE_MID0)){
		state = STATE_END;
		has_new = true;
		gpio_pin_interrupt_configure_dt(&rc5_decoder, GPIO_INT_DISABLE);
	}

	isr_end_timestamp = k_uptime_ticks();
}

bool rc5_new_command_received(int* new_command) 
{ 
    if(has_new) 
    { 
        *new_command = command; 
    } 
    
    return has_new; 
}


void rc5_reset()
{
	has_new = 0;
	counter = 14;
	command = 0;
	state = STATE_BEGIN;

	gpio_pin_interrupt_configure_dt(&rc5_decoder, GPIO_INT_EDGE_BOTH);
}

void rc5_init(void) {
	gpio_pin_configure_dt(&rc5_decoder, GPIO_INPUT);
	
	gpio_init_callback(&rc5_rx_irq, rc5_signal_received, BIT(rc5_decoder.pin));
	gpio_add_callback(rc5_decoder.port, &rc5_rx_irq);

	rc5_reset();
}
