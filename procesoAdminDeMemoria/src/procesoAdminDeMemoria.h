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
} t_proceso;

typedef struct {
	char* contenido;
	int	  libre;
} t_marco;


////////////////////////////////////////////////////////////////////////////////////
////////////////////////// VARIABLES GLOBALES //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


pthread_t 	th_server_cpu;
int 		socket_swap;
t_log* 		logger;
char* 		CONFIG_PATH = "config.txt";
char* 		LOGGER_PATH = "log.txt";
t_list* 	TLB;
t_list*		paginas;
t_marco**	memoria;
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
//char* 		ALGORITMO_REEMPLAZO();

int 		gl_PID;
int			gl_nro_pagina;

int			gl_TLB_hit;
int			gl_TLB_total;





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
int conectar_con_swap();
char* swap_leer_pagina(int pid, int pagina);
int swap_nuevo_proceso(int pid, int paginas);
int swap_escribir_pagina(int pid, int pagina, char* contenido);
int swap_finalizar(int pid);

//// NUEVAS

void 		destruir_proceso							(t_proceso* proceso);
int 		es_el_proceso_segun_PID						(t_proceso* proceso);
void 		procesar_mensaje_cpu						(int socket, t_msg* msg);
void 		crear_estructuras_de_un_proceso				(int PID, int paginas);
int  		iniciar_proceso_CPU							(int pid, int paginas);
int  		escribir_pagina								(int pid, int pagina, char* contenido);
int 		buscar_marco_en_TLB							(int PID,int nro_pagina);
t_pagina* 	crear_pagina								(int PID, int pagina, int entrada);
void 		destruir_pagina								(t_pagina* pagina);
int 		es_la_pagina_segun_PID_y_nro_pagina			(t_pagina* pagina);
int 		es_la_pagina_segun_PID						(t_pagina* pagina);
int 		la_pagina_esta_cargada_en_memoria			(t_pagina* pagina);
int 		buscar_marco_en_paginas						(int PID,int nro_pagina);
void		agregar_pagina_en_memoria					(t_proceso* proceso,int nro_pagina,char*buff_pag);
int 		reemplazar_pagina_en_memoria_segun_algoritmo(t_proceso* proceso, int pagina, char* contenido);
void 		actualizar_entradas_en_tabla_de_paginas		(t_pagina* pagina);
void 		agregar_pagina_en_TLB						(int PID, int pagina, int entrada);
void 		tasa_aciertos_TLB							();
void 		tasa_aciertos_TLB_global					();
void 		sumar_tasas_TLB								(t_proceso* proceso);
int 		encontrar_marco_libre						();
int 		buscar_marco_de_pagina_en_TLB_y_tabla_paginas(int pid, int nro_pagina);

#endif /* PROCESOADMINDEMEMORIA_H_ */
