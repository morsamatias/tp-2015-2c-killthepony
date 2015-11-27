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
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>

////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// ESTRUCTURAS //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

typedef struct {
	int PID;
	int pagina;
	int marco;
	int modificado;
	int presencia;
	int usado;
} t_pagina;

typedef struct {
	int PID;
	t_list* paginas;
	float TLB_hit;
	int TLB_total;
	int fallos_pagina;
	int accesos_swap;
} t_proceso;

typedef struct {
	char* contenido;
	int	  libre;
} t_marco;

typedef struct {
	int marco;
	int TLB_HIT;
} t_busq_marco;


////////////////////////////////////////////////////////////////////////////////////
////////////////////////// VARIABLES GLOBALES //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


pthread_t 	th_server_cpu;
int 		socket_swap;

char* 		CONFIG_PATH ;
char* 		LOGGER_PATH ;
t_list* 	TLB;
t_list*		paginas;
t_marco**	memoria;
t_config* 	cfg;
int 		gl_PID;
int			gl_nro_pagina;
float		gl_TLB_hit;
int			gl_TLB_total;
pthread_t 	t_tasas_globales;

// Semaforos
sem_t mutex_TLB;
sem_t mutex_PAGINAS;

// Loggers
//t_log* 		logger;
t_log* 	log_general_p;
//t_log* 	log_general_f;
t_log* 	log_errores;
t_log* 	log_memoria;
t_log* 	log_estadisticas;
t_log*	log_print_mem;


////////////////////////////////////////////////////////////////////////////////////
////////////////////// DECLARACIONES DE FUNCIONES //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

int 	iniciar_server();
int 	inicializar();
int 	finalizar();
int 	conectar_con_swap();
char* 	swap_leer_pagina(int pid, int pagina);
int 	swap_nuevo_proceso(int pid, int paginas);
int 	swap_escribir_pagina(int pid, int pagina, char* contenido);
int 	swap_finalizar(int pid);
int 	PUERTO_ESCUCHA();
char*	IP_SWAP();
int 	PUERTO_SWAP();
int 	MAXIMO_MARCOS_POR_PROCESO();
int 	CANTIDAD_MARCOS();
int 	TAMANIO_MARCO();
int 	ENTRADAS_TLB();
int 	TLB_HABILITADA();
int 	RETARDO_MEMORIA();
char* 	ALGORITMO_REEMPLAZO();
int 	RETARDO_MEMORIA_MINIMO();

//// NUEVAS

void 			destruir_proceso								(t_proceso* proceso);
int 			es_el_proceso_segun_PID							(t_proceso* proceso);
void 			procesar_mensaje_cpu							(int socket, t_msg* msg);
void 			crear_estructuras_de_un_proceso					(int PID, int paginas);
int  			iniciar_proceso_CPU								(int pid, int paginas);
int  			escribir_pagina									(int pid, int pagina, char* contenido);
int 			buscar_marco_en_TLB								(int PID,int nro_pagina);
t_pagina* 		crear_pagina									(int PID, int pagina, int entrada);
void 			destruir_pagina									(t_pagina* pagina);
int 			es_la_pagina_segun_PID_y_nro_pagina				(t_pagina* pagina);
int 			es_la_pagina_segun_PID							(t_pagina* pagina);
int 			la_pagina_esta_cargada_en_memoria				(t_pagina* pagina);
int 			buscar_marco_en_paginas							(t_proceso* proceso,int nro_pagina);
void			agregar_pagina_en_memoria						(t_proceso* proceso,int nro_pagina,char*buff_pag);
int 			quitar_pagina_de_la_memoria						(t_pagina* pagina);
int 			reemplazar_pagina_en_memoria_segun_algoritmo	(t_proceso* proceso, int pagina, char* contenido);
void 			actualizar_entradas_en_tabla_de_paginas			(t_pagina* pagina);
void 			agregar_pagina_en_TLB							(int PID, int pagina, int entrada);
void 			tasa_aciertos_TLB								();
void 			tasa_aciertos_TLB_total							();
void 			sumar_tasas_TLB									(t_proceso* proceso);
int 			encontrar_marco_libre							();
void			quitar_todas_las_paginas_de_memoria_del_proceso	(t_proceso* proceso);
void 			quitar_pagina_de_la_memoria_signal				(t_pagina* pagina);
t_busq_marco 	buscar_marco_de_pagina_en_TLB_y_tabla_paginas	(int pid, int nro_pagina);
t_pagina* 		mover_y_devolver_primer_pagina_al_final_de_la_lista(t_list* lista_paginas);
t_pagina* 		buscar_pagina_victima_CLOCK						(t_list* lista_paginas);
void 			dormir_memoria									();
void 			loguear_memoria									();

// MANEJO DE SEÑALES

void iniciar_signals();
void handler		(int signal);
void handler_SIGUSR1();
void handler_SIGUSR2();
void handler_SIGPOLL();

#endif /* PROCESOADMINDEMEMORIA_H_ */
