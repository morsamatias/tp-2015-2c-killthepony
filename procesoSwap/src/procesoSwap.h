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
char* LOGGER_PATH =						    "log.txt";
char* LOGGER_PATH_MPROC_ASIGNADO =	 		"log_mproc_asignado.txt";
char* LOGGER_PATH_MPROC_LIBERADO =			"log_mproc_liberado.txt";
char* LOGGER_PATH_MPROC_RECHAZADO =			"log_mproc_rechazado.txt";
char* LOGGER_PATH_COMPACTACION = 	    	"log_compactacion.txt";
char* LOGGER_PATH_LECTURAS_ESCRITURAS =     "log_lecturas_escrituras.txt";
char* LOGGER_PATH_ERRORES = 				"log_errores.txt";
char* LOGGER_PATH_PAGINAS_CANT_LECT_ESC =   "log_paginas_cant_lecturas_escrituras.txt";

char* swap;

int TAMANIO_SWAP;

t_log* logger;
t_log* logger_mproc_asignado;
t_log* logger_mproc_liberado;
t_log* logger_mproc_rechazado;
t_log* logger_compactacion;
t_log* logger_lecturas_escrituras;
t_log* logger_errores;
t_log* logger_paginas_cant_lect_esc;

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

t_ocupado* 			swap_ocupar								(int pid, int pagina, int paginas);
t_proceso*		 	proc_buscar								(int pid);

int 				compactar								();
int 				hueco_inicio_bytes						();
int 				hueco_tamanio_bytes						(t_ocupado* hueco);
int 				iniciar_server							();
int 				inicializar								();
int 				finalizar								();
int 				ordenar									();
int 				swap_cant_huecos_libres					();
int 				swap_buscar_hueco_libre					(int paginas);
int 				swap_ocupar_hueco						(t_ocupado* ocupado);

char* 				swap_inicializar						() ;

void 				esp_libre_inicializar					();
void 				est_leer								(int pid);
void 				est_escribir							(int pid);
void 				est_print								(int pid);
void 				est_eliminar							(int pid);
void 				_copiar									(t_ocupado* ocup);
void 				dormir_compactacion						();
void 				dormir_swap								();
void 				hueco_print_info						(const char* texto_inicial, t_ocupado* hueco, t_log* log);
void 				procesar_mensaje_mem					(int socket, t_msg* msg);
void 				pagina_print_info						(const char* texto_inicio, int pid, int pagina, char* contenido);
void 				print_ocupado							();
void 				print_libre								();
void 				swap_destroy							();
void 				libre_destroy							();
void 				ocupado_destroy							();
//void _ocupar_hueco(t_ocupado* o);
void 				proceso_destroy							();
void				esp_libre_inicializar					();

void mostrar_listas();
///////////////////////////////////////////////////////////////////////////////////

#endif /* PROCESOSWAP_H_ */
