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
pthread_mutex_t mutex;

typedef struct{
	int pid;
	int posicion;
	int cantidad;
}t_ocupado;

typedef struct{
	int posicion;
	int cantidad;
}t_libre;

t_list* esp_ocupado;
t_list* esp_libre;

int iniciar_server();

int inicializar();
int finalizar();

void procesar_mensaje_mem(int socket, t_msg* msg);


#endif /* PROCESOSWAP_H_ */
