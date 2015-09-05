/*
 * config_swap.h
 *
 *  Created on: 4/9/2015
 *      Author: utnso
 */

#ifndef CONFIG_SWAP_H_
#define CONFIG_SWAP_H_


#include <commons/config.h>
char* CONFIG_PATH = "config.txt";

t_config* cfg;

int PUERTO_ESCUCHA();


int PUERTO_ESCUCHA(){
	return config_get_int_value(cfg, "PUERTO_ESCUCHA");
}

#endif /* CONFIG_SWAP_H_ */
