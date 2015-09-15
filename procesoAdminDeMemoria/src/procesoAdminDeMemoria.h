/*
 * procesoAdminDeMemoria.h
 *
 *  Created on: 30/8/2015
 *      Author: utnso
 */

#ifndef PROCESOADMINDEMEMORIA_H_
#define PROCESOADMINDEMEMORIA_H_

#include <util.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/log.h>

////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// ESTRUCTURAS //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

typedef struct {
	int PID;
	int pagina;
	int entrada;
	int libre;
} t_paginas;

typedef struct {
	int entrada;
	int libre;
	int modificado;
	char* bloque;
} t_memoria;

typedef struct {
	int posicion_memoria;
	int posicion_TLB;
} t_posicion_pagina;

typedef struct {
	int PID;
	int cant_paginas;
	t_posicion_pagina* paginas; // EL INDICE ES LA PAGINA Y EL VALOR ES LA ENTRADA EN LA MEM PRINCIPAL
	int prox_reemplazo_FIFO;
} t_proceso;



////////////////////////////////////////////////////////////////////////////////////
////////////////////////// VARIABLES GLOBALES //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

t_log* 		logger;
char* 		CONFIG_PATH = "config.txt";
char* 		LOGGER_PATH = "log.txt";
t_paginas* 	TLB;
t_memoria* 	memoria;
t_list*		l_paginas_por_proceso ;
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
char* 		ALGORITMO_REEMPLAZO();

int			FIFO_TLB = 0;

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
char* ALGORITMO_REEMPLAZO(){
	return config_get_string_value(cfg, "ALGORITMO_REEMPLAZO");
}







int iniciar_server();
int inicializar();
int finalizar();
void procesar_mensaje_cpu(int socket, t_msg* msg);
int conectar_con_swap();
char* swap_leer_pagina(int pid, int pagina);
int swap_nuevo_proceso(int pid, int paginas);

//// NUEVAS

void eliminar_estructuras_de_un_proceso(t_proceso* proceso);
void eliminar_estructuras_de_todos_los_procesos(t_proceso* proceso);
void crear_estructuras_de_un_proceso(int PID, int paginas);
int  iniciar_proceso_CPU(int pid, int paginas);
int  escribir_pagina(int pid, int pagina, char* contenido);


#endif /* PROCESOADMINDEMEMORIA_H_ */
