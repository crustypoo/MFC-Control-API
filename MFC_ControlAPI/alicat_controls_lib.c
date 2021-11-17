/*
 * alicat_controls_lib.c
 *
 *  Created on: Oct 15, 2015
 *      Author: aliu
 */

/*
#include "defines.c"

Port_descriptor * automated_mfc_config();
void print_id(Port_descriptor * pd, int device_num);
void print_pd(Port_descriptor * pd);
void config_mfc_bank(Port_descriptor * pd);
Port_stat * init_port_mon();
Address * init_address_book();
Port_descriptor * establish_descriptors(Port_stat *ps, Address * addr);
int close_ports(Port_descriptor * pd);
void free_all(Port_stat *ps, Port_descriptor *pd, Address *addr);
bool checkif_port(char * addr);
int open_port(char * addr, bool stat);
int config_port(int pd);
int id_port(Port_descriptor * pd, int device_num);
int port_gastype(char * gt);
int mfc_stream_mode(Port_descriptor * pd);
int mfc_setpoint_percentage(Port_descriptor * pd, int device_num, double percentage);
double parse_mfc_returns(Port_descriptor * pd, int device_num, int measure);

Port_descriptor * automated_mfc_config(){
	printf("start config\n");
	Port_descriptor * pd = malloc(sizeof(Port_descriptor));
	if (pd != NULL){
		DIR *d;
		struct dirent *dir;

		d = opendir("/dev/");
		if (d){
			printf("opened dir\n");
			int device_num = 0;
			while ((dir = readdir(d)) != NULL){
				bool chk = checkif_port(dir->d_name);
				if (chk == true){
					printf("%s is valid COM port\n", dir->d_name);
					if(strlen(dir->d_name) > 6 && strlen(dir->d_name) < 9){
						char dev[] = "/dev/";
						if (strcat(dev, dir->d_name) != NULL){
							pd -> pd[device_num] = open_port(dev, pd->ps[device_num]);
							if(pd->pd[device_num] != -1){
								if(config_port(pd->pd[device_num]) != -1){
									if (id_port(pd, device_num) != -1){
										print_id(pd, device_num);
										device_num++;
									}
								}
							}
						} else {
							fprintf(stderr, "Error Function 'automated_mfc_config': %s!\n", strerror(errno));
						}
					}
				}
			}
		closedir(d);
		pd->num_mfc = device_num + 1;
		}
		return pd;
	}
	return NULL;
}

void print_id(Port_descriptor * pd, int device_num){
	char * gas_name;
	char * zone_name;

	switch(pd->gas_type[device_num]){
	case HE:
		gas_name = GAS_HE;
		break;
	case H2:
		gas_name = GAS_H2;
		break;
	case AR:
		gas_name = GAS_AR;
		break;
	case AIR:
		gas_name = GAS_AIR;
		break;
	case CO2:
		gas_name = GAS_CO2;
		break;
	case C2H2:
		gas_name = GAS_C2H2;
		break;
	}

	switch(pd->zone[device_num]){
	case ANNEAL:
		zone_name = ZONE_A;
		break;
	case GROWTH:
		zone_name = ZONE_G;
		break;
	case DELAM:
		zone_name = ZONE_D;
		break;
	}

	printf("MFC #%d ID:\n", pd->mfc_id[device_num]);
	printf("\tZone: %s\n", zone_name);
	printf("\tGas type: %s\n", gas_name);
	if (pd->stream_mode[device_num] == false){
		printf("\tOutput mode: Polling\n");
	}else{
		printf("\tOutput mode: Streaming\n");
	}
}

void print_pd(Port_descriptor * pd){
	int i;
	char * gas_name;
	char * zone_name;
	printf("%d MFC connected!\n", pd->num_mfc);
	for(i = 0; i < pd->num_mfc; i++){
		switch(pd->gas_type[i]){
		case HE:
			gas_name = GAS_HE;
			break;
		case H2:
			gas_name = GAS_H2;
			break;
		case AR:
			gas_name = GAS_AR;
			break;
		case AIR:
			gas_name = GAS_AIR;
			break;
		case CO2:
			gas_name = GAS_CO2;
			break;
		case C2H2:
			gas_name = GAS_C2H2;
			break;
		}

		switch(pd->zone[i]){
		case ANNEAL:
			zone_name = ZONE_A;
			break;
		case GROWTH:
			zone_name = ZONE_G;
			break;
		case DELAM:
			zone_name = ZONE_D;
			break;
		}

		printf("MFC #%d ID:\n", pd->mfc_id[i]);
		printf("\tZone: %s\n", zone_name);
		printf("\tGas type: %s\n", gas_name);
		if (pd->stream_mode[i] == false){
			printf("\tOutput mode: Polling\n");
		}else{
			printf("\tOutput mode: Streaming\n");
		}
	}
}

bool checkif_port(char * addr){
	bool chk;
	if(strstr(addr, "ttyUSB") != NULL){
		chk = true;
		printf("%s is a valid port!\n", addr);
		return chk;
	} else {
		chk = false;
		printf("%s is not a valid port!\n", addr);
		return chk;
	}
}

int open_port(char * addr, bool status){
	int pd;
	pd = open(addr, O_RDWR | O_NOCTTY | O_NDELAY);
	if (pd == -1){
		fprintf(stderr, "Function Error 'open_port': %s!\n", strerror(errno));
		status = false;
		return -1;
	} else {
		printf("Address %s connected!\n", addr);
		status = true;
		return pd;
	}
}

int config_port(int pd){
	struct termios config;
	memset(&config, 0, sizeof(config));
	if (tcgetattr(pd, &config) != 0){
		fprintf(stderr, "Function Error 'config_port': %s!\n", strerror(errno));
		return -1;
	}

	config.c_cflag |= (CREAD | CLOCAL);
	config.c_cflag &= ~CRTSCTS;
	config.c_lflag |= ICANON;
	config.c_lflag &= ~(ECHO | ECHOE);
	config.c_lflag &= ~PARENB;
	config.c_lflag &= ~CSTOPB;
	config.c_lflag &= ~CSIZE;
	config.c_lflag |= CS8;
	cfsetspeed(&config,B19200);

	tcflush(pd, TCIOFLUSH);
	if (tcsetattr(pd, TCSANOW, &config) == -1){
		fprintf(stderr, "Function Error 'config_port': %s!\n",strerror(errno));
		return -1;
	}

	return 0;
}

int id_port(Port_descriptor * pd, int device_num){
	char *cmd_strA = READ_CMD_A;
	char *cmd_strG = READ_CMD_G;
	char *cmd_strD = READ_CMD_D;
	char *cmd_str;
	char buffer[256];

	bool success = false;
	int res;
	int i = 0;
	while (success == false &&  i < 3){
		printf("%d\n",i);
		switch(i){
		case 0:
			cmd_str = cmd_strA;
			break;
		case 1:
			cmd_str = cmd_strG;
			break;
		case 2:
			cmd_str = cmd_strD;
			break;
		}
		printf("%s", cmd_str);
		int bytes_w = write(pd->pd[device_num], cmd_str, strlen(cmd_str));
		if (bytes_w == -1){
			printf("Error Function 'id_port': write failure\n");
			pd->write_failure[device_num] = true;
			return -1;
		} else if(bytes_w > -1 && bytes_w < strlen(cmd_str)){
			printf("Error Function 'id_port': retrying write\n");
		} else if(bytes_w == strlen(cmd_str)){
			res = read(pd->pd[device_num], buffer, 256);
			int j = 0;
			while(res < RES_LEN && j < TIMEOUT){
				res = read(pd->pd[device_num], buffer, 256);
				if(res > 0){
					success = true;
					buffer[res] = '\0';
					pd->zone[device_num] = i;
					pd->stream_mode[device_num] = false;
					switch (port_gastype(buffer)){
					case AIR:
						pd->gas_type[device_num] = AIR;
						break;
					case HE:
						pd->gas_type[device_num] = HE;
						break;
					case H2:
						pd->gas_type[device_num] = H2;
						break;
					case AR:
						pd->gas_type[device_num] = AR;
						break;
					case CO2:
						pd->gas_type[device_num] = CO2;
						break;
					case C2H2:
						pd->gas_type[device_num] = C2H2;
						break;
					case -1:
						pd->gas_type[device_num] = -1;
						pd->ps[device_num] = false;
					}
				}else {
					success = false;
					pd->gas_type[device_num] = -1;
					pd->ps[device_num] = false;
				}
				i++;
				j++;
			}
		}
	}
	if (success == false){
		printf("Error Function 'id_port': Unidentified MFC!\n");
		return -1;
	} else {
		pd->mfc_id[device_num] = device_num + 1;
		pd->write_failure[device_num] = false;
		return 0;
	}
}

int port_gastype(char * gt){
	if (strstr(gt, GAS_AIR) != NULL){
		return AIR;
	}else if (strstr(gt, GAS_HE) != NULL){
		return HE;
	}else if (strstr(gt, GAS_H2) != NULL){
		return H2;
	}else if (strstr(gt, GAS_AR) != NULL){
		return AR;
	}else if (strstr(gt, GAS_CO2) != NULL){
		return CO2;
	}else if (strstr(gt, GAS_C2H2) != NULL){
		return C2H2;
	}else {
		printf("Error Function 'port_gastype': Unrecognized gas type!\n");
		printf("str input: %s\n", gt);
		return -1;
	}
}

int mfc_stream_mode(Port_descriptor *pd){
	char * cmd_str = STREAM_CMD;
	bool chk = true;
	int i = 0;
	while(i < pd->num_mfc){
		if(pd->stream_mode[i] == false){
			int bytes_w = write(pd->pd[i], cmd_str, strlen(cmd_str));
			if(bytes_w == -1){
				printf("Error Function 'mfc_stream_mode': write failed on device #%d\n", pd->mfc_id[i]);
				pd->write_failure[i] = true;
				chk = false;
				i++;
			} else if (bytes_w > -1 && bytes_w < strlen(cmd_str)){
				printf("Error Function 'mfc_stream_mode': incomplete write on device #%d\n", pd->mfc_id[i]);
			} else if (bytes_w == strlen(cmd_str)){
				pd->stream_mode[i] = true;
				i++;
			}
		} else {
			i++;
		}
	}

	if (chk == false){
		printf("Error Function 'mfc_stream_mode': write failures detected!\n");
		return -1;
	} else {
		return 0;
	}
}

int mfc_poll_mode(Port_descriptor *pd){
	char * cmd_str;
	bool chk = true;
	int i = 0;
	while(i < pd->num_mfc){
		if(pd->stream_mode[i] == true){
			switch(pd->zone[i]){
			case ANNEAL:
				cmd_str = POLL_CMD_A;
				break;
			case GROWTH:
				cmd_str = POLL_CMD_G;
				break;
			case DELAM:
				cmd_str = POLL_CMD_D;
				break;
			}
			int bytes_w = write(pd->pd[i], cmd_str, strlen(cmd_str));
			if(bytes_w == -1){
				printf("Error Function 'mfc_poll_mode': write failed on device #%d\n", pd->mfc_id[i]);
				pd->write_failure[i] = true;
				chk = false;
				i++;
			} else if (bytes_w > -1 && bytes_w < strlen(cmd_str)){
				printf("Error Function 'mfc_poll_mode': incomplete write on device #%d\n", pd->mfc_id[i]);
				printf("Retrying write...\n");
			} else if (bytes_w == strlen(cmd_str)){
				pd->stream_mode[i] = false;
				i++;
			}
		} else {
			i++;
		}
	}

	if (chk == false){
		printf("\n");
		printf("Error Function 'mfc_poll_mode': write failures detected!\n");
		return -1;
	} else {
		return 0;
	}
}

//incomplete until further work
int mfc_setpoint_percentage(Port_descriptor * pd, int device_num, double percentage){
	char *cmd_str;
	char cmd_num[6];
	int value = (int) (percentage * 64000) / 100;
	sprintf(cmd_num, "%d\n", value);
	switch (pd->zone[device_num]){
	case ANNEAL:
		cmd_str = A;
 		if (strcat(cmd_str, cmd_num) != NULL){
 			if(write(pd->pd[device_num], cmd_str, 7) != -1){
 				double sp = parse_mfc_returns(pd, device_num, SETPOINT);
 				if (sp != -1){
 					printf("Device #%d setpoint is now %f\n", device_num + 1, sp);
 					return 0;
 				} else {
 					printf("Error Function 'mfc_setpoint_percentage': error in return message\n");
 					return -1;
 				}
 			}
 		} else{
 			printf("Error Function 'mfc_setpoint_percentage': cmd format failed!\n");
 			return -1;
 		}
 		break;
	case GROWTH:
		cmd_str = G;
 		if (strcat(cmd_str, cmd_num) != NULL){
 			if(write(pd->pd[device_num], cmd_str, 7) != -1){
 				double sp = parse_mfc_returns(pd, device_num, SETPOINT);
 				if (sp != -1){
 					printf("Device #%d setpoint is now %f\n", device_num + 1, sp);
 					return 0;
 				} else {
 					printf("Error Function 'mfc_setpoint_percentage': error in return message\n");
 					return -1;
 				}
 			}
 		} else{
 			printf("Error Function 'mfc_setpoint_percentage': cmd format failed!\n");
 			return -1;
 		}
 		break;
	case DELAM:
		cmd_str = D;
 		if (strcat(cmd_str, cmd_num) != NULL){
 			if(write(pd->pd[device_num], cmd_str, 7) != -1){
 				double sp = parse_mfc_returns(pd, device_num, SETPOINT);
 				if (sp != -1){
 					printf("Device #%d setpoint is now %f\n", device_num + 1, sp);
 					return 0;
 				} else {
 					printf("Error Function 'mfc_setpoint_percentage': error in return message\n");
 					return -1;
 				}
 			}
 		} else{
 			printf("Error Function 'mfc_setpoint_percentage': cmd format failed!\n");
 			return -1;
 		}
 		break;
	}
	return -1;
}

double parse_mfc_returns(Port_descriptor * pd, int device_num, int measure){
	char buff[256];
	int res = 0;
	while(res == 0){
		res = read(pd->pd[device_num], buff, 256);
	}
	if (res == -1){
		printf("Error Function 'parse_mfc_returns': read failed!\n");
		return -1;
	}else {
		char * token;
		char * str_received = buff;
		char * separator = DELIMITER;
		buff[res] = '\0';

		int i;
		for (i = 0; i < measure; i++){
			if (i == 0){
				token = strtok(str_received, separator);
			} else {
				token = strtok(NULL, separator);
			}
		}
		if (token == NULL){
			printf("Error Function 'parse_mfc_returns': parse failed!\n");
			return -1;
		}
		return atof(token);
	}
}


// gen 1 function
Port_stat * init_port_mon(){
	Port_stat *ps = malloc(sizeof(Port_stat));
	if (ps != NULL){
		int i;
		for (i = 0; i<NUM_MFC; i++){
			ps -> port_num[i] = false;
		}
		return ps;
	} else {
		printf("Function Error 'init_port_mon': failed to allocate size 'Port_stat'!\n");
		return NULL;
	}
}

// gen 1 function
Address * init_address_book(){
	Address * addr = malloc(sizeof(Address));
	if (addr != NULL){
		addr -> fa[0] = PORT_1;
		addr -> fa[1] = PORT_2;
		addr -> fa[2] = PORT_3;
		addr -> fa[3] = PORT_4;
		addr -> fa[4] = PORT_5;
		addr -> fa[5] = PORT_6;
		addr -> fa[6] = PORT_7;
		addr -> fa[7] = PORT_8;
		addr -> fa[8] = PORT_9;
		addr -> fa[9] = PORT_10;
		addr -> fa[10] = PORT_11;
		addr -> fa[11] = PORT_12;
		addr -> fa[12] = PORT_13;
		addr -> fa[13] = PORT_14;
		addr -> fa[14] = PORT_15;
		addr -> fa[15] = PORT_16;
		return addr;
	} else {
		printf("Function Error 'init_address_book': failed to allocate size 'Address'!\n");
		return NULL;
	}
}

// gen 1 function
Port_descriptor * establish_descriptors(Port_stat *ps, Address * addr){
	Port_descriptor *md = malloc(sizeof(Port_descriptor));
	if (md != NULL){
		int i;
		for (i = 0; i<NUM_MFC; i++){
			if (ps -> port_num[i] == false){
				int errnum;
				md -> pd[i] = open(addr -> fa[i], O_RDWR | O_NOCTTY | O_NDELAY);
				if (md -> pd[i] == -1){
					errnum = errno;
					fprintf(stderr, "Function Error 'establish_descriptors': %s!\n", strerror(errnum));
				} else {
					printf("Port #%d %s connected!\n", i+1, addr -> fa[i]);
					ps -> port_num[i] = true;
				}
			}
		}
		return md;
	} else {
		printf("Function Error 'init_address_book': failed to allocate size 'Address'!\n");
		return NULL;
	}
}

// gen 1 function
void config_mfc_bank(Port_descriptor * pd){
	int i;
	for (i = 0; i<NUM_MFC; i++){
		if (pd->ps[i] == true){
			int filedesc = pd -> pd[i];
			struct termios config;
			memset(&config, 0, sizeof(config));
			if (tcgetattr(filedesc, &config) != 0){
				fprintf(stderr, "Function Error 'config_mfc_bank'failed @ PORT#0%d: %s!\n", i+1,strerror(errno));
				pd -> ps[i] = false;
			}

			config.c_cflag |= (CREAD | CLOCAL);
			config.c_lflag |= ICANON;
			config.c_cflag &= ~CRTSCTS;
			config.c_lflag &= ~(ECHO | ECHOE);
			config.c_lflag &= ~CSTOPB;
			config.c_lflag &= ~PARENB;
			config.c_lflag &= ~CSIZE;
			config.c_lflag |= CS8;
			cfsetspeed(&config,B19200);

			tcflush(filedesc, TCIOFLUSH);
			if (tcsetattr(filedesc, TCSANOW, &config) == -1){
				fprintf(stderr, "Function Error 'config_mfc_bank'failed @ PORT#0%d: %s!\n", i+1,strerror(errno));
			}
		}
	}
}

// gen 1 function
int close_ports(Port_descriptor * pd){
	int i,j, k;
	int err_chk = 0;
	int err[NUM_MFC];
	for (j = 0; j < NUM_MFC; j++){
		err[j] = 0;
	}
	for (i = 0; i < NUM_MFC; i++){
		if (close(pd -> pd[i]) == -1){
			err[i] = errno;
		}
	}
	for (k=0; k<NUM_MFC; k++){
		if(err[k] != 0){
			fprintf(stderr, "Function Error 'close_ports': %s!\n", strerror(err[k]));
			if (err_chk == 0){
				err_chk = -1;
			}
		}
	}
	return err_chk;
}

// gen 1 function
void free_all(Port_stat *ps, Port_descriptor *pd, Address *addr){
	free(ps);
	free(pd);
	free(addr);
}

// gen 1 function
/*
int mfc_poll_mode(Port_descriptor *pd, int addr){
	char * cmd_str = POLL_CMD_A;
	if (write(pd->pd[addr], cmd_str, 4) == -1){
		fprintf(stderr, "Error Function 'mfc_poll_mode': %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

int mfc_stream_mode(Port_descriptor *pd, int addr){
	char * cmd_str = STREAM_CMD;
	if (write(pd->fd[addr], cmd_str, ))
}

int mfc_gas_mode(){

}

int mfc_sp(char *mfc_addr, ){

}

int mfc_identity(Port_descriptor *pd, Address *addr){
	int i;
	for(i = 0; i < NUM_MFC; i++){

	}
}
*/
