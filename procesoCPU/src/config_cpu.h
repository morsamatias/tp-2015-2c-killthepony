/*
 * config_cpu.h
 *
 *  Created on: 2/9/2015
 *      Author: utnso
 */

#ifndef CONFIG_CPU_H_
#define CONFIG_CPU_H_


#include <commons/config.h>

char* CONFIG_PATH = "/home/utnso/Escritorio/git/tp-2015-2c-killthepony/procesoCPU/Debug/config.txt";

t_config* cfg;

char* IP_PLANIFICADOR();
int PUERTO_PLANIFICADOR();
char* IP_MEMORIA();
int PUERTO_MEMORIA();
int ID();
/*
 *
 */
int ID(){
	return config_get_int_value(cfg, "ID");
}

char* IP_MEMORIA(){
	return config_get_string_value(cfg, "IP_MEMORIA");
}
int PUERTO_MEMORIA(){
	return config_get_int_value(cfg, "PUERTO_MEMORIA");
}

char* IP_PLANIFICADOR(){
	return config_get_string_value(cfg, "IP_PLANIFICADOR");
}
int PUERTO_PLANIFICADOR(){
	return config_get_int_value(cfg, "PUERTO_PLANIFICADOR");
}


#endif /* CONFIG_CPU_H_ */
