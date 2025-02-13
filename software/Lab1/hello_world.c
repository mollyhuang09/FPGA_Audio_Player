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

int background()
{
	int j;
	int x = 0;
	int grainsize = 4;
	int g_taskProcessed = 0;

	for(j = 0; j < grainsize; j++)
	{
	g_taskProcessed++;
	}
	return x;
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
void writeEgm(int period){
	IOWR(EGM_BASE, 2, period);
	IOWR(EGM_BASE, 3, period/2);
}
void tightPolling(int period){
	while(period <= 5000) {
		int count = 0;
		int first_period = 0;
		while(isBusy()){
			if(first_period == 0) {
				int first = 0;
				while((IORD(STIMULUS_IN_BASE, 0) & 0x1) == 1 ){
					if (first == 0){
						IOWR(RESPONSE_OUT_BASE, 0, 1);
						IOWR(RESPONSE_OUT_BASE, 0, 0);
						first_period++;
					}

						background();
						count++;
						}
				while((IORD(STIMULUS_IN_BASE, 0) & 0x1) == 0) {
						background();
						count++;
					}
			} else {
				if ((IORD(STIMULUS_IN_BASE, 0) & 0x1) == 1 ){
					IOWR(RESPONSE_OUT_BASE, 0, 1);
					IOWR(RESPONSE_OUT_BASE, 0, 0);
				}
				for(int a = 1; a <= count-1; a++){
					background();
				}
			}

		}
		period += 2;
		printf("%d, %d, %d, %d, %d, %d", period, period/2, count, latency(), missed(), multi());
	}
}


int main()
{
  int period;
  printf("Hello from Nios II!\n");
  printf("Period, Pulse_Width, BG_Tasks Run, Latency, Missed, Multiple");
  IOWR(EGM_BASE, 0, 1);
  period = 2;
  tightPolling(period);

  return 0;
}
