/*
 * config_planif.h
 *
 *  Created on: 2/9/2015
 *      Author: utnso
 */

#ifndef CONFIG_PLANIF_H_
#define CONFIG_PLANIF_H_

#include <commons/config.h>

t_config* cfg = NULL;
char* CONFIG_PATH = "/home/utnso/Escritorio/git/tp-2015-2c-killthepony/procesoPlanificador/Debug/config.txt";

int PUERTO_ESCUCHA();

int ALGORITMO_PLANIFICACION();

int QUANTUM();

/*
 *
 */

int PUERTO_ESCUCHA(){
	return config_get_int_value(cfg, "PUERTO_ESCUCHA");
}

int ALGORITMO_PLANIFICACION(){
	return config_get_int_value(cfg, "ALGORITMO_PLANIFICACION");
}

int QUANTUM(){
	return config_get_int_value(cfg, "QUANTUM");
}



#endif /* CONFIG_PLANIF_H_ */
