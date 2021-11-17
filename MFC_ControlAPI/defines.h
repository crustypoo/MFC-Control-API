/*
 * defines.c
 *
 *  Created on: Oct 15, 2015
 *      Author: aliu
 */

#ifndef DEFINE_H
#define DEFINES_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<errno.h>
#include<stdbool.h>
#include<math.h>
#include<ctype.h>
#include<unistd.h>
#include<fcntl.h>
#include<termios.h>
#include<dirent.h>
#include<time.h>

#define MACHINE_ID "LONGCAT_V01"

#define ERR_NOERR   "NO ERRORS"
#define ERR_OPENDIR "LC_alicat error @ automated_mfc_config: Could not open port directory!"
#define ERR_NO_MFC  "LC_alicat error @ automated_mfc_config: No MFCs attached!"
#define ERR_WR  	"LC_alicat error: Problem writing to MFC!"
#define ERR_RD  	"LC_alicat error: Problem reading from MFC!"
#define ERR_RD_INC  "LC_alicat error: Incomplete read!"
#define ERR_MFC_NUM "LC_alicat error: MFC addressed is out of bounds!"
#define ERR_FTOS 	"LC_alicat error: Failed conversion of Float to type String!"
#define ERR_FORM    "LC_alicat error: CMD format error!"
#define ERR_MOE     "LC_alicat error: SP is beyond Margin of Error!"
#define ERR_UNI     "LC-alicat error: Could not find unit!"

#define DIR_ADDR "/dev/"
#define PORT_SUBSTR "ttyUSB"

#define SP_CMD_A   "AS"
#define SP_CMD_G   "GS"
#define SP_CMD_D   "DS"
#define POLL_CMD_A "*@=A\r"
#define POLL_CMD_G "*@=G\r"
#define POLL_CMD_D "*@=D\r"
#define STREAM_CMD "*@=@\r"
#define READ_CMD_A "A\r"
#define READ_CMD_G "G\r"
#define READ_CMD_D "D\r"

#define TIMEOUT 150000
#define RES_LEN 30
#define SIZE_BUFF 256

#define NUM_MFC 17

#define ANNEAL 0
#define GROWTH 1
#define DELAM  2

#define A "A"
#define G "G"
#define D "D"

#define NUM_ZONES 3

#define ZONE_A "Anneal"
#define ZONE_G "Growth"
#define ZONE_D "Delam"

#define NUM_GAS_TYPES 6

#define AIR  0
#define HE   1
#define H2   2
#define AR   3
#define CO2  4
#define C2H2 5
#define NON  6

#define GAS_AIR  " Air"
#define GAS_HE   " He"
#define GAS_H2   " H2"
#define GAS_AR   " Ar"
#define GAS_CO2  " CO2"
#define GAS_C2H2 " C2H2"
#define GAS_NON  " UNRECOGNIZED"

#define ID		   0
#define PRESSURE   1
#define TEMP       2
#define VOL_FLOW   3
#define MASS_FLOW  4
#define SETPOINT   5
#define GAS        6

#define COL_0 "Pressure"
#define COL_1 "Temperature"
#define COL_2 "Volumetric Flow"
#define COL_3 "Mass Flow"
#define COL_4 "Set Point"
#define COL_5 "Gas"

#define MOE 5 // 5% Margin of Error for setpoint

typedef struct Port_descriptors{
	int mfc_id[NUM_MFC];
	bool ps[NUM_MFC]; // connected?
	int pd[NUM_MFC];  // FD
	int gas_type[NUM_MFC]; // He? H2?
	int zone[NUM_MFC]; // Anneal? Growth? Delam?
	int num_mfc; // Total number of MFCs
	bool stream_mode[NUM_MFC]; // Stream Mode on?
	bool write_failure[NUM_MFC];
	int op_range[NUM_MFC];
} Port_descriptor;

typedef struct Port_values{
	double pressure[NUM_MFC];
	double temp[NUM_MFC];
	double vflow[NUM_MFC];
	double mflow[NUM_MFC];
	double s_point[NUM_MFC];
}Port_value;

#endif
