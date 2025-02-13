/*
 * "Hello World" example.
 *
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */

#include <stdio.h>
#include "system.h"
#include "sys/alt_irq.h"
#include "altera_avalon_pio_regs.h"
#include <string.h>

int background(){
	int j;
	int x = 0;
	int grainsize = 4;
	int g_taskProcessed = 0;

	IOWR(LED_PIO_BASE, 0, 0x1);
	IOWR(LED_PIO_BASE, 0, 0x0);

	for(j = 0; j < grainsize; j++)
	{
	g_taskProcessed++;
	}
	return x;
}

void enableEGM(){
	IOWR(EGM_BASE, 0, 1);
}

void disableEGM(){
	IOWR(EGM_BASE, 0, 0);
}

int latency(){
	return IORD(EGM_BASE, 4);
}

int missed(){
	return IORD(EGM_BASE, 5);
}

int multi(){
	return IORD(EGM_BASE, 6);
}

int isBusy(){
	return IORD(EGM_BASE, 1) & 0x1;
}
void writeResponse(){
	IOWR(RESPONSE_OUT_BASE, 0, 1);
	IOWR(RESPONSE_OUT_BASE, 0, 0);
}
static void interrupt_function (void* contxt, alt_u32 id){
	writeResponse();
	IOWR(STIMULUS_IN_BASE, 3, 0);
}
void writeEgm(int period){
	IOWR(EGM_BASE, 2, period);
	IOWR(EGM_BASE, 3, period/2);
}



int main(){
	alt_irq_register(STIMULUS_IN_IRQ, (void *)0, interrupt_function);

	//read switch
	int mode = IORD(SWITCH_PIO_BASE, 0) & 0x1;
	if(mode == 0) {
		printf("Interrupt method selected. \n");
	} else {
		printf("Tight polling method selected. \n");
	}

	int period;
	period = 2;
	printf("Period, Pulse_Width, BG_Tasks Run, Latency, Missed, Multiple \n\nPlease, press PB0 to continue.\n");

	//waiting for PB0 button
	while(IORD(BUTTON_PIO_BASE, 0) & 0x1){}

	if(mode == 0){
		//INTERRUPT METHOD
		disableEGM();
		IOWR(STIMULUS_IN_BASE, 2, 0x1);
		while(period <= 5000){
			int background_count = 0;
			writeEgm(period);

			enableEGM();

			while (isBusy() == 1){
				background();
				background_count++;
			}

			disableEGM();

			printf("%d, %d, %d, %d, %d, %d\n", period, period/2, background_count, latency(), missed(), multi());
			period += 2;
		}
	} else {
		//TIGHT POLLING
		disableEGM();
		while(period <= 5000) {
			writeEgm(period);
			int total_background_count = 0;
			int current_background_count = 0;
			int first_period = 0;
			int count = 0;
			int last_pulse = 0;
			int current_pulse = 0;

			enableEGM();

			//while on the positive edge of stimulus
			while( isBusy() == 1){
				current_pulse = IORD(STIMULUS_IN_BASE, 0) & 0x1;

				//first period for response
				if(first_period < 2) {
					if(current_pulse == 1 && last_pulse == 0){
						writeResponse();
					}else{
						background();
						total_background_count++;
						current_background_count++; //collecting the initial background count
					}
				} else { //for the rest of the periods

					//first response pulse
					if(last_pulse == 0 && current_pulse == 1){
						writeResponse();
						count = 0;
					}else if (count < (current_background_count-1)){
						background();
						total_background_count++;
						count++;
					}
				}

				if ((first_period == 0 && !current_pulse) || (first_period == 1 && current_pulse)){
					first_period++;
				}

				last_pulse = current_pulse;
			}

			disableEGM();

			printf("%d, %d, %d, %d, %d, %d\n", period, period/2, total_background_count, latency(), missed(), multi());
			period += 2;
		}
	}
}
