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
t_proceso* proc_buscar(int pid);

int compactar();
int hueco_inicio_bytes();
int hueco_tamanio_bytes(t_ocupado* hueco);
int iniciar_server();
int inicializar();
int finalizar();
int ordenar();
int swap_cant_huecos_libres();
int swap_buscar_hueco_libre(int paginas);
int swap_ocupar_hueco(t_ocupado* ocupado);

char* swap_inicializar() ;

void esp_libre_inicializar();
void est_leer(int pid);
void est_escribir(int pid);
void est_print(int pid);
void est_eliminar(int pid);
void _copiar(t_ocupado* ocup);
void dormir_compactacion();
void dormir_swap();
void hueco_print_info(const char* texto_inicial, t_ocupado* hueco);
void procesar_mensaje_mem(int socket, t_msg* msg);
void pagina_print_info(const char* texto_inicio, int pid, int pagina, char* contenido);
void print_ocupado();
void print_libre();
void swap_destroy();
void libre_destroy();
void ocupado_destroy();
void _ocupar_hueco(t_ocupado* o);
void proceso_destroy();
void esp_libre_inicializar();

///////////////////////////////////////////////////////////////////////////////////

#endif /* PROCESOSWAP_H_ */
