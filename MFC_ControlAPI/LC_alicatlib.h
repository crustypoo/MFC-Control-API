/*
 * LC_alicatlib.h
 *
 *  Created on: Oct 9, 2015
 *      Author: aliu
 */

#ifndef LC_ALICATLIB_H_
#define LC_ALICATLIB_H_

#include "defines.h"

int automated_mfc_config(Port_descriptor * pd);
void print_pd(Port_descriptor * pd);
double get_pressure(Port_descriptor * pd, int num_mfc);
double get_temperature(Port_descriptor * pd, int num_mfc);
double get_vflow(Port_descriptor * pd, int num_mfc);
double get_mflow(Port_descriptor * pd, int num_mfc);
double get_sp(Port_descriptor * pd, int num_mfc);
double get_all(Port_descriptor * pd, int num_mfc, Port_value * pv);
int set_sp(Port_descriptor * pd, int num_mfc, double sp);
int select_mfc(Port_descriptor * pd, int zone, int gas);

#endif /* LC_ALICATLIB_H_ */
