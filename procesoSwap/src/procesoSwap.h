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


////////////////////////////////////////////////////////////////////VARIABLES/////////////////////////////////
char* LOGGER_PATH = "log.txt";
char* swap;

int TAMANIO_SWAP;

t_log* logger;
t_list* esp_ocupado;
t_list* esp_libre;
t_list* procesos ;//para las estadisticas

///////////////////////////////////////////////////////////////////ESTRUCTURAS//////////////////////////////

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

//////////////////////////////////////////////////////////////PROTOTIPOS/////////////////////

t_ocupado* swap_ocupar(int pid, int pagina, int paginas);

int compactar();
int iniciar_server();
int inicializar();
int finalizar();
int ordenar();
int swap_cant_huecos_libres();
int swap_buscar_hueco_libre(int paginas);
int swap_ocupar_hueco(t_ocupado* ocupado);

char* swap_inicializar() ;

void esp_libre_inicializar();
void _copiar(t_ocupado* ocup);
void dormir_compactacion();
void dormir_swap();
void procesar_mensaje_mem(int socket, t_msg* msg);
void swap_destroy();
void libre_destroy();
void ocupado_destroy();
void proceso_destroy();
void esp_libre_inicializar();

///////////////////////////////////////////////////////////////////////////////////

#endif /* PROCESOSWAP_H_ */
