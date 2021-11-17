/*
 * LC_alicat.c
 * Created by Andy Liu
 * October 19, 2015
 */

#include "defines.h"

int automated_mfc_config(Port_descriptor * pd);
void print_pd(Port_descriptor * pd);
//void print_unit(Port_descriptor * pd, int addr);

char * alicat_error_str(int error);
bool checkif_port(char * port);
int checkif_mfc(int fd);
int mfc_gastype(int fd, int zone);
int mfc_attached(char * port, Port_descriptor * pd, int num_mfc);
int get_mfc_identity(int fd, Port_descriptor * pd, int num_mfc);
double get_pressure(Port_descriptor * pd, int num_mfc);
double get_temperature(Port_descriptor * pd, int num_mfc);
double get_vflow(Port_descriptor * pd, int num_mfc);
double get_mflow(Port_descriptor * pd, int num_mfc);
double get_sp(Port_descriptor * pd, int num_mfc);
int get_all(Port_descriptor * pd, int num_mfc, Port_value * pv);
int set_sp(Port_descriptor * pd, int num_mfc, double sp);
int select_mfc(Port_descriptor * pd, int zone, int gas);

int automated_mfc_config(Port_descriptor * pd){
	printf("Start port configuration\n");
	bzero(pd, sizeof(Port_descriptor));

	if(pd != NULL){
		DIR *d;
		struct dirent *dir;

		d = opendir(DIR_ADDR);
		if(d == NULL){
			return -1;
		} else {
			printf("%s opened!\n", DIR_ADDR);

			int num_mfc = 0;
			while((dir = readdir(d)) != NULL){
				if(checkif_port(dir->d_name) == true){
					printf("%s\n", dir->d_name);
					int attached = mfc_attached(dir->d_name, pd, num_mfc);
					if (attached != -1){
						num_mfc++;
					}
				}
			}
			if (num_mfc > 0)
				pd -> num_mfc = num_mfc;
			else
				return -2;
		}

	}
	return 0;
}

void print_pd(Port_descriptor * pd){

    time_t current_time;
    char* time_string;

    current_time = time(NULL);
    time_string = ctime(&current_time);

	printf("********************************************************\n");
	printf("* GAS PANEL STATUS\n");
	printf("* Machine: %s\n", MACHINE_ID);
	printf("* Current system time: %s", time_string);
	printf("* Number of mfcs connected: %d\n", pd->num_mfc);
	printf("********************************************************\r\n");

	int i;
	for (i = 0; i < pd -> num_mfc; i++){
		printf("********************************************************\n");
		printf("* DEVICE %d STATUS\n", pd->mfc_id[i]);
		printf("********************************************************\n");
		if(pd -> ps[i] > 0)
			printf("	Connection: Connected!\n");
		else
			printf("	Connection: Disconnected\n");

		if(pd -> stream_mode[i] != false)
			printf("	Output Mode: Streaming\n");
		else
			printf("	Output Mode: Polling\n");

		switch(pd->zone[i]){
		case ANNEAL:
			printf("	Growth Zone: %s\n", ZONE_A);
			break;
		case GROWTH:
			printf("	Growth Zone: %s\n", ZONE_G);
			break;
		case DELAM:
			printf("	Growth Zone: %s\n", ZONE_D);
			break;
		default:
			break;
		}

		switch(pd->gas_type[i]){
		case AIR:
			printf("	Gas Type: %s\n", GAS_AIR);
			break;
		case H2:
			printf("	Gas Type: %s\n", GAS_H2);
			break;
		case HE:
			printf("	Gas Type: %s\n", GAS_HE);
			break;
		case CO2:
			printf("	Gas Type: %s\n", GAS_CO2);
			break;
		case AR:
			printf("	Gas Type: %s\n", GAS_AR);
			break;
		case C2H2:
			printf("	Gas Type: %s\n", GAS_C2H2);
			break;
		case NON:
			printf("	Gas Type: %s\n", GAS_NON);
		}
	}
}

char * alicat_error_str(int error){
	char * err;
	if (error == EXIT_SUCCESS){
		err = ERR_NOERR;
		return err;
	}

	switch(error){
	case -1:
		err = ERR_OPENDIR;
		break;
	case -2:
		err = ERR_NO_MFC;
		break;
	case -3:
		err = ERR_WR;
		break;
	case -4:
		err = ERR_RD;
		break;
	case -5:
		err = ERR_RD_INC;
		break;
	case -6:
		err = ERR_MFC_NUM;
		break;
	case -7:
		err = ERR_FTOS;
		break;
	case -8:
		err = ERR_FORM;
		break;
	case -9:
		err = ERR_MOE;
		break;
	case -10:
		err = ERR_UNI;
		break;
	default:
		err = ERR_NOERR;
		break;
	}

	return err;
}

bool checkif_port(char * port){
	if(strstr(port, PORT_SUBSTR) == NULL){
		return false;
	} else {
		return true;
	}
}

int checkif_mfc(int fd){
	char buff[SIZE_BUFF];

	int i;
	for (i = 0; i < NUM_ZONES; i++){
		char * cmd;
		switch (i){
		case 0:
			cmd = READ_CMD_A;
			break;
		case 1:
			cmd = READ_CMD_G;
			break;
		case 2:
			cmd = READ_CMD_D;
			break;
		}

		int size = strlen(cmd);

		fd_set rfds;
		struct timeval tv;
		int retval = 0;

		// setup for select()
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		tv.tv_sec = 0; // timeout = 1 sec
		tv.tv_usec = TIMEOUT; // no microsecond timeout

		int bytes_w = write(fd, cmd, size);
		if (bytes_w == size){
			retval = select(fd+1, &rfds, NULL, NULL, &tv);
			if (retval > 0){
				int res = read(fd, buff, SIZE_BUFF);
				printf("%d\n", res);
				buff[res] = '\0';
				if (res > RES_LEN && (strstr(buff, GAS_AIR) > 0 || strstr(buff, GAS_HE) > 0 || strstr(buff, GAS_H2) > 0
					|| strstr(buff, GAS_CO2) > 0 || strstr(buff, GAS_C2H2) > 0 || strstr(buff, GAS_AR) > 0)){
					return i;
				}
			}
		}
		retval = 0;
	}

	return -1;
}

int mfc_gastype(int fd, int zone){
	char buff[SIZE_BUFF];
	char * cmd;

	fd_set rfds;
	struct timeval tv;
	int retval;

	// setup for select()
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	tv.tv_sec = 0; // timeout = 1 sec
	tv.tv_usec = TIMEOUT; // no microsecond timeout

	switch (zone){
	case 0:
		cmd = READ_CMD_A;
		break;
	case 1:
		cmd = READ_CMD_G;
		break;
	case 2:
		cmd = READ_CMD_D;
		break;
	}

	int size = strlen(cmd);

	if(write(fd, cmd, size) == size){
		retval = select(fd + 1, &rfds, NULL, NULL, &tv);
		if (retval > 0){
			int res = read(fd, buff, SIZE_BUFF);
			if (res > RES_LEN){
				buff[res] = '\0';
				if (strstr(buff, GAS_AIR) != NULL){
					return AIR;
				}else if (strstr(buff, GAS_HE) != NULL){
					return HE;
				}else if (strstr(buff, GAS_H2) != NULL && strstr(buff, GAS_C2H2) == NULL){
					return H2;
				}else if (strstr(buff, GAS_AR) != NULL){
					return AR;
				}else if (strstr(buff, GAS_CO2) != NULL){
					return CO2;
				}else if (strstr(buff, GAS_C2H2) != NULL){
					return C2H2;
				}else {
					return NON;
				}
			}
			return -3;
		}
		return -4;
	}
	return -3;
}

//returns -1 for failure, else value of FD
int mfc_attached(char * port, Port_descriptor * pd, int num_mfc){
	char addr[20] = "/dev/";
	int fd = open(strcat(addr, port), O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
	if( fd < 0 ){
		return -1;
	}
	struct termios oldconfig;
	memset(&oldconfig, 0, sizeof(oldconfig));
	struct termios newconfig;
	memset(&newconfig, 0, sizeof(newconfig));

	if (tcgetattr(fd, &newconfig) < 0){
		close(fd);
		return -1;
	} else
		memcpy(&oldconfig, &newconfig, sizeof(newconfig));

	// set serial port (8n1)
	fcntl(fd, F_SETFL, FNDELAY);
	cfsetspeed(&newconfig,B19200);
	newconfig.c_cflag |= (CREAD | CLOCAL);
	newconfig.c_cflag &= ~CRTSCTS;
	newconfig.c_lflag |= ICANON;
	newconfig.c_lflag &= ~(ECHO | ECHOE);
	newconfig.c_lflag &= ~PARENB;
	newconfig.c_lflag &= ~CSTOPB;
	newconfig.c_lflag &= ~CSIZE;
	newconfig.c_lflag |= CS8;

	if (tcsetattr(fd, TCSANOW, &newconfig) < 0){
		close(fd);
		return -1;
	} else {
		tcflush(fd, TCIOFLUSH);
	}

	if (get_mfc_identity(fd, pd, num_mfc) < 0){
		tcsetattr(fd, TCSANOW, &oldconfig);
		close(fd);
		return -1;
	}

	return 0;
}

int get_mfc_identity(int fd, Port_descriptor * pd, int num_mfc){
	int zone_type = checkif_mfc(fd);
	if (zone_type < 0){
		return -1;
	} else {
		pd -> zone[num_mfc] = zone_type;
		pd -> pd[num_mfc] = fd;
		pd -> ps[num_mfc] = true;
		pd -> stream_mode[num_mfc] = false;
		pd -> write_failure[num_mfc] = false;
		pd -> mfc_id[num_mfc] = num_mfc + 1;
	}

	int gas = mfc_gastype(fd, zone_type);
	if (gas < 0)
		return -1;

	pd->gas_type[num_mfc] = gas;

	return 0;
}

double get_pressure(Port_descriptor * pd, int num_mfc){
	if (num_mfc > pd->num_mfc-1){
		return 0;
	}

	char buff[SIZE_BUFF];
	double p;
	char * cmd;
	int fd = pd->pd[num_mfc];

	fd_set rfds;
	struct timeval tv;

	// setup for select()
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	tv.tv_sec = 0; // timeout = 1 sec
	tv.tv_usec = TIMEOUT; // no microsecond timeout

	switch(pd->zone[num_mfc]){
	case ANNEAL:
		cmd = READ_CMD_A;
		break;
	case GROWTH:
		cmd = READ_CMD_G;
		break;
	case DELAM:
		cmd = READ_CMD_D;
		break;
	}

	int size = strlen(cmd);

	if (write(fd, cmd, size) == size){
		if(select(fd + 1, &rfds, NULL, NULL, &tv) > 0){
			int res = read(fd, buff, SIZE_BUFF);
			if(res > RES_LEN){
				buff[res] = '\0';
				char * token;
				token = strtok(buff, " +-");
				token = strtok(NULL, " +-");

				p = atof(token);
				if(*(token - 1) == '-'){
					return -p;
				} else {
					return p;
				}
			}
			return 0;
		}
		return 0;
	}
	pd->write_failure[num_mfc] = true;
	return 0;
}

double get_temperature(Port_descriptor * pd, int num_mfc){
	if (num_mfc > pd->num_mfc-1){
		return 0;
	}

	char buff[SIZE_BUFF];
	double p;
	char * cmd;
	int fd = pd->pd[num_mfc];

	fd_set rfds;
	struct timeval tv;

	// setup for select()
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	tv.tv_sec = 0; // timeout = 1 sec
	tv.tv_usec = TIMEOUT; // no microsecond timeout

	switch(pd->zone[num_mfc]){
	case ANNEAL:
		cmd = READ_CMD_A;
		break;
	case GROWTH:
		cmd = READ_CMD_G;
		break;
	case DELAM:
		cmd = READ_CMD_D;
		break;
	}

	int size = strlen(cmd);

	if (write(fd, cmd, size) == size){
		if(select(fd + 1, &rfds, NULL, NULL, &tv) > 0){
			int res = read(fd, buff, SIZE_BUFF);
			if(res > RES_LEN){
				buff[res] = '\0';
				char * token;
				token = strtok(buff, " +-");
				token = strtok(NULL, " +-");
				token = strtok(NULL, " +-");

				p = atof(token);
				if(*(token - 1) == '-'){
					return -p;
				} else {
					return p;
				}
			}
			return 0;
		}
		return 0;
	}
	pd->write_failure[num_mfc] = true;
	return 0;
}

double get_vflow(Port_descriptor * pd, int num_mfc){
	if (num_mfc > pd->num_mfc-1){
		return 0;
	}

	char buff[SIZE_BUFF];
	double p = 0.00;
	char * cmd;
	int fd = pd->pd[num_mfc];

	fd_set rfds;
	struct timeval tv;

	// setup for select()
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	tv.tv_sec = 0; // timeout = 1 sec
	tv.tv_usec = TIMEOUT; // no microsecond timeout

	switch(pd->zone[num_mfc]){
	case ANNEAL:
		cmd = READ_CMD_A;
		break;
	case GROWTH:
		cmd = READ_CMD_G;
		break;
	case DELAM:
		cmd = READ_CMD_D;
		break;
	}

	int size = strlen(cmd);

	if (write(fd, cmd, size) == size){
		if(select(fd + 1, &rfds, NULL, NULL, &tv) > 0){
			int res = read(fd, buff, SIZE_BUFF);
			if(res > RES_LEN){
				buff[res] = '\0';
				char * token;
				token = strtok(buff, " +-");
				token = strtok(NULL, " +-");
				token = strtok(NULL, " +-");
				token = strtok(NULL, " +-");

				p = atof(token);
				if(*(token - 1) == '-'){
					return -p;
				} else {
					return p;
				}
			}
			return 0;
		}
		return 0;
	}
	pd->write_failure[num_mfc] = true;
	return 0;
}

double get_mflow(Port_descriptor * pd, int num_mfc){
	if (num_mfc > pd->num_mfc-1){
		return 0;
	}

	char buff[SIZE_BUFF];
	double p = 0.00;
	char * cmd;
	int fd = pd->pd[num_mfc];

	fd_set rfds;
	struct timeval tv;

	// setup for select()
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	tv.tv_sec = 0; // timeout = 1 sec
	tv.tv_usec = TIMEOUT; // no microsecond timeout

	switch(pd->zone[num_mfc]){
	case ANNEAL:
		cmd = READ_CMD_A;
		break;
	case GROWTH:
		cmd = READ_CMD_G;
		break;
	case DELAM:
		cmd = READ_CMD_D;
		break;
	}

	int size = strlen(cmd);

	if (write(fd, cmd, size) == size){
		if(select(fd + 1, &rfds, NULL, NULL, &tv) > 0){
			int res = read(fd, buff, SIZE_BUFF);
			if(res > RES_LEN){
				buff[res] = '\0';
				char * token;
				token = strtok(buff, " +-");
				token = strtok(NULL, " +-");
				token = strtok(NULL, " +-");
				token = strtok(NULL, " +-");
				token = strtok(NULL, " +-");

				p = atof(token);
				if(*(token - 1) == '-'){
					return -p;
				} else {
					return p;
				}
			}
			return 0;
		}
		return 0;
	}
	pd->write_failure[num_mfc] = true;
	return 0;
}

double get_sp(Port_descriptor * pd, int num_mfc){
	if (num_mfc > pd->num_mfc-1){
		return 0;
	}

	char buff[SIZE_BUFF];
	double p = 0.00;
	char * cmd;
	int fd = pd->pd[num_mfc];

	fd_set rfds;
	struct timeval tv;

	// setup for select()
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	tv.tv_sec = 0; // timeout = 1 sec
	tv.tv_usec = TIMEOUT; // no microsecond timeout

	switch(pd->zone[num_mfc]){
	case ANNEAL:
		cmd = READ_CMD_A;
		break;
	case GROWTH:
		cmd = READ_CMD_G;
		break;
	case DELAM:
		cmd = READ_CMD_D;
		break;
	}

	int size = strlen(cmd);

	if (write(fd, cmd, size) == size){
		if(select(fd + 1, &rfds, NULL, NULL, &tv) > 0){
			int res = read(fd, buff, SIZE_BUFF);
			if(res > RES_LEN){
				buff[res] = '\0';
				char * token;
				token = strtok(buff, " +-");
				token = strtok(NULL, " +-");
				token = strtok(NULL, " +-");
				token = strtok(NULL, " +-");
				token = strtok(NULL, " +-");
				token = strtok(NULL, " +-");

				p = atof(token);
				return p;
			}
			return 0;
		}
		return 0;
	}
	pd->write_failure[num_mfc] = true;
	return 0;
}

int get_all(Port_descriptor * pd, int num_mfc, Port_value * pv){
	if (num_mfc > pd->num_mfc-1){
		return -6;
	}

	char * pr;
	char * t;
	char * vf;
	char * mf;
	char * sp;

	char buff[SIZE_BUFF];
	char * cmd;
	int fd = pd->pd[num_mfc];

	fd_set rfds;
	struct timeval tv;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	tv.tv_sec = 0;
	tv.tv_usec = TIMEOUT;

	switch(pd->zone[num_mfc]){
	case ANNEAL:
		cmd = READ_CMD_A;
		break;
	case GROWTH:
		cmd = READ_CMD_G;
		break;
	case DELAM:
		cmd = READ_CMD_D;
		break;
	}

	int size = strlen(cmd);

	if (write(fd, cmd, size) == size){
		if (select(fd + 1, &rfds, NULL, NULL, &tv) > 0){
			int res = read(fd,buff,SIZE_BUFF);
			if(res > RES_LEN){
				buff[res] = '\0';
				printf("%s\n", buff);

				pr = strtok(NULL, " +-");
				t = strtok(NULL, " +-");
				vf = strtok(NULL, " +-");
				mf = strtok(NULL, " +-");
				sp = strtok(NULL, " +-");

				if(*(pr - 1) == '-'){
					pv->pressure[num_mfc] = -(atof(pr));
				} else {
					pv->pressure[num_mfc] = atof(pr);
				}

				if(*(t - 1) == '-'){
					pv->temp[num_mfc] = -(atof(t));
				} else {
					pv->temp[num_mfc] = atof(t);
				}

				if(*(vf - 1) == '-'){
					pv->vflow[num_mfc] = -(atof(vf));
				} else {
					pv->vflow[num_mfc] = atof(vf);
				}

				if(*(mf - 1) == '-'){
					pv->mflow[num_mfc] = -(atof(mf));
				} else {
					pv->mflow[num_mfc] = atof(mf);
				}

				pv->pressure[num_mfc] = atof(sp);

				return 0;
			}
			return -5;
		}
		return -4;
	}
	pd->write_failure[num_mfc] = true;
	return -3;
}

int set_sp(Port_descriptor * pd, int num_mfc, double sp){
	if (num_mfc > pd->num_mfc-1){
		return -6;
	}

	char buff[SIZE_BUFF];
	char cmd[50];
	char val[20];
	int fd = pd->pd[num_mfc];

	fd_set rfds;
	struct timeval tv;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	tv.tv_sec = 0;
	tv.tv_usec = TIMEOUT;

	if(sprintf(val, "%f\r", sp) < 0){
		return -7;
	}

	switch(pd->zone[num_mfc]){
	case ANNEAL:
		if (strcpy(cmd,SP_CMD_A))
			strcat(cmd, val);
		break;
	case GROWTH:
		if (strcpy(cmd,SP_CMD_G))
			strcat(cmd, val);
		break;
	case DELAM:
		if (strcpy(cmd,SP_CMD_D))
			strcat(cmd, val);
		break;
	}

	int size = strlen(cmd);
	if (size < 3)
		return -8;

	int bytes_w = write(fd, cmd, size);
	if(bytes_w == size){
		if(select(fd + 1, &rfds, NULL, NULL, &tv)){
			int res = read(fd, buff, SIZE_BUFF);
			if (res > RES_LEN){
				buff[res] = '\0';
				char * token;
				token = strtok(buff, " +-");
				token = strtok(NULL, " +-");
				token = strtok(NULL, " +-");
				token = strtok(NULL, " +-");
				token = strtok(NULL, " +-");
				token = strtok(NULL, " +-");

				double p = atof(token);
				double err = p - sp;
				if(err < 0.00){
					err = -err;
					double p_err = (err / sp) * 100.00;
					if (p_err > MOE){
						return -9;
					}
				} else {
					double p_err = (err / sp) * 100.00;
					if (p_err > MOE){
						return -9;
					}
				}
				return 0;
			}
			return -5;
		}
		return -4;
	}
	pd->write_failure[num_mfc] = true;
	return -3;
}

int select_mfc(Port_descriptor * pd, int zone, int gas){
	int tot_mfc = pd->num_mfc;

	int i;
	for(i = 0; i < tot_mfc; i++){
		if (pd->ps[i] == true){
			if (pd->zone[i] == zone){
				if (pd->gas_type[i] == gas){
					return i;
				}
			}
		}
	}
	return -10;
}
