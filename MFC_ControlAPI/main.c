/*
 * main.c
 *
 *  Created on: Oct 14, 2015
 *      Author: aliu
 */

#include "LC_alicatlib.h"

int main(){
	printf("Hello\n");

	Port_descriptor * pd = malloc(sizeof(Port_descriptor));
	if (automated_mfc_config(pd) < 0){
		printf("hello\n");
		free(pd);
		return -1;
	} else {
		print_pd(pd);
		int i;
		for(i = 0; i<1000000; i++){
			double pressure = get_pressure(pd,0);
			double temp = get_temperature(pd, 0);
			double vflow = get_vflow(pd, 0);
			printf("%f, %f\n", pressure, vflow);
		}
		free(pd);
		return 0;
	}
}

