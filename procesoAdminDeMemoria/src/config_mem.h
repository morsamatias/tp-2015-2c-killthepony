/*
 * config_mem.h
 *
 *  Created on: 4/9/2015
 *      Author: utnso
 */

#ifndef CONFIG_MEM_H_
#define CONFIG_MEM_H_


#include <commons/config.h>

char* CONFIG_PATH = "config.txt";


int PUERTO_ESCUCHA();
char* IP_SWAP();
int PUERTO_SWAP();

t_config* cfg;



int PUERTO_ESCUCHA(){
	return config_get_int_value(cfg, "PUERTO_ESCUCHA");
}

char* IP_SWAP(){
	return config_get_string_value(cfg, "IP_SWAP");
}
int PUERTO_SWAP(){
	return config_get_int_value(cfg, "PUERTO_SWAP");
}



#endif /* CONFIG_MEM_H_ */
