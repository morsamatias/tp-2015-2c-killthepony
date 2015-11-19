/*
 * config_swap.h
 *
 *  Created on: 4/9/2015
 *      Author: utnso
 */

#ifndef CONFIG_SWAP_H_
#define CONFIG_SWAP_H_
#include <commons/config.h>

char* CONFIG_PATH = "/home/utnso/Escritorio/git/tp-2015-2c-killthepony/procesoSwap/Debug/config.txt";

t_config* cfg;

int PUERTO_ESCUCHA();
int CANTIDAD_PAGINAS();
int TAMANIO_PAGINA();
int RETARDO_COMPACTACION();
int SWAP_RETARDO();

char* NOMBRE_SWAP();

//////////////////////////////////////////////////////////////CONFIGURACION////////////////////////////////////

int RETARDO_SWAP(){
	return config_get_int_value(cfg, "RETARDO_SWAP");
}

int RETARDO_SWAP_MINIMO(){
	 return config_get_int_value(cfg, "RETARDO_SWAP_MILISEGUNDOS");;
}

int PUERTO_ESCUCHA(){
	return config_get_int_value(cfg, "PUERTO_ESCUCHA");
}
int CANTIDAD_PAGINAS(){
	return config_get_int_value(cfg, "CANTIDAD_PAGINAS");
}
int TAMANIO_PAGINA(){
	return config_get_int_value(cfg, "TAMANIO_PAGINA");
}

int RETARDO_COMPACTACION(){
	return config_get_int_value(cfg, "RETARDO_COMPACTACION");
}

int RETARDO_COMPACTACION_MINIMO(){
	return config_get_int_value(cfg, "RETARDO_COMPACTACION_MILISEGUNDOS");
}

char* NOMBRE_SWAP(){
	return config_get_string_value(cfg, "NOMBRE_SWAP");
}

/////////////////////////////////////////////////////////////////////////////////

#endif /* CONFIG_SWAP_H_ */
