/*
 * procesoSwap.h
 *
 *  Created on: 30/8/2015
 *      Author: utnso
 */

#ifndef PROCESOSWAP_H_
#define PROCESOSWAP_H_

#include <util.h>
#include "config_swap.h"
#include <commons/log.h>

char* LOGGER_PATH = "log.txt";

t_log* logger;


char* swap;
int TAMANIO_SWAP;

typedef struct{
	int pid;
	int posicion;
	int cantidad;
}t_ocupado;

typedef struct{
	int posicion;
	int cantidad;
}t_libre;

typedef struct{
	int pid;
	int cant_lecturas;
	int cant_escrituras;
}t_proceso;


t_list* esp_ocupado;
t_list* esp_libre;

t_list* procesos;//para las estadisticas

int iniciar_server();

int inicializar();
int finalizar();
t_ocupado* swap_ocupar(int pid, int pagina, int paginas);
int swap_buscar_hueco_libre(int paginas);

int swap_ocupar_hueco(t_ocupado* ocupado);
int ordenar();
void procesar_mensaje_mem(int socket, t_msg* msg);


#endif /* PROCESOSWAP_H_ */
