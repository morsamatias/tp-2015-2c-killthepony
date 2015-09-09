/*
 * config_mem.h
 *
 *  Created on: 4/9/2015
 *      Author: utnso
 */

#ifndef CONFIG_MEM_H_
#define CONFIG_MEM_H_


#include <commons/config.h>


////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// ESTRUCTURAS //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

typedef struct {
	int PID;
	int pagina;
	int entrada;
} t_paginas;

typedef struct {
	int entrada;
	int libre;
	int modificado;
	char* bloque;
} t_memoria;

typedef struct {
	int PID;
	t_list* paginas;
} t_proceso;


////////////////////////////////////////////////////////////////////////////////////
////////////////////////// VARIABLES GLOBALES //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


t_paginas* 	TBL;
t_memoria* 	memoria;
t_list*		l_paginas_por_proceso ;




char* 		CONFIG_PATH = "config.txt";
t_config* 	cfg;
int 		PUERTO_ESCUCHA();
char* 		IP_SWAP();
int 		PUERTO_SWAP();
int 		MAXIMO_MARCOS_POR_PROCESO();
int 		CANTIDAD_MARCOS();
int 		TAMANIO_MARCO();
int 		ENTRADAS_TLB();
int 		TLB_HABILITADA();
int 		RETARDO_MEMORIA();

////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// DECLARACIONES //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


int PUERTO_ESCUCHA(){
	return config_get_int_value(cfg, "PUERTO_ESCUCHA");
}
char* IP_SWAP(){
	return config_get_string_value(cfg, "IP_SWAP");
}
int PUERTO_SWAP(){
	return config_get_int_value(cfg, "PUERTO_SWAP");
}
int MAXIMO_MARCOS_POR_PROCESO(){
	return config_get_int_value(cfg, "MAXIMO_MARCOS_POR_PROCESO");
}
int CANTIDAD_MARCOS(){
	return config_get_int_value(cfg, "CANTIDAD_MARCOS");
}
int TAMANIO_MARCO(){
	return config_get_int_value(cfg, "TAMANIO_MARCO");
}
int ENTRADAS_TLB(){
	return config_get_int_value(cfg, "ENTRADAS_TLB");
}
int TLB_HABILITADA(){
	return config_get_int_value(cfg, "TLB_HABILITADA");
}
int RETARDO_MEMORIA(){
	return config_get_int_value(cfg, "RETARDO_MEMORIA");
}




#endif /* CONFIG_MEM_H_ */
