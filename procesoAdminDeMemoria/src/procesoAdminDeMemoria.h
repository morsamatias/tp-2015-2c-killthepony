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
} t_pagina;

typedef struct {
	int PID;
	int pagina;
	int modificado;
	char* contenido;
} t_memoria;

/*typedef struct {
	int posicion_memoria;
	int posicion_TLB;
} t_pagina_proceso;

typedef struct {
	int PID;
	int cant_paginas;
	t_list* paginas;
} t_proceso;*/



////////////////////////////////////////////////////////////////////////////////////
////////////////////////// VARIABLES GLOBALES //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

t_log* 		logger;
char* 		CONFIG_PATH = "config.txt";
char* 		LOGGER_PATH = "log.txt";
t_list* 	TLB;
t_list*		paginas;
t_list* 	memoria;
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

int			INDICE_TLB = 0;
int 		gl_PID;
int			gl_nro_pagina;
int			gl_entrada;
float		gl_TLB_acierto = 0.0;
int			gl_TLB_total = 0;



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

//// NUEVAS

void 		procesar_mensaje_cpu						(int socket, t_msg* msg);
void 		crear_estructuras_de_un_proceso				(int PID, int paginas);
int  		iniciar_proceso_CPU							(int pid, int paginas);
int  		escribir_pagina								(int pid, int pagina, char* contenido);
int  		es_la_memoria_segun_PID						(t_memoria* pagina);
int  		es_la_memoria_segun_PID_y_pagina			(t_memoria* pagina);
void 		setear_flag_modificado						(int pid, int nro_pagina);
int 		buscar_pagina_en_TLB						(int PID,int nro_pagina);
t_pagina* 	crear_pagina								(int PID, int pagina, int entrada);
t_memoria* 	crear_memoria								(int PID, int pagina, int modificado, char* contenido);
void 		destruir_pagina								(t_pagina* pagina);
void 		destruir_memoria							(t_memoria* memoria);
int 		es_la_pagina_segun_PID_y_nro_pagina			(t_pagina* pagina);
int 		es_la_pagina_segun_PID						(t_pagina* pagina);
int 		buscar_pagina_en_paginas					(int PID,int nro_pagina);
void		agregar_pagina_en_memoria					(int pid,int nro_pagina,char*buff_pag);
void		modificar_pagina_en_memoria_segun_algoritmo	(int pid,int nro_pagina,char*buff_pag);
void 		actualizar_entradas_en_tabla_de_paginas		(t_pagina* pagina);
int 		reemplazar_pagina_en_memoria_segun_algoritmo(int PID, int pagina, char* contenido);
char* 		usar_pagina_en_memoria_segun_algoritmo		(int PID, int nro_pagina, int entrada,int flag_TLB);
void 		actualizar_entradas_en_tabla_de_paginas		(t_pagina* pagina);
void 		agregar_pagina_en_TLB						(int PID, int pagina, int entrada);
void 		recalcular_entrada							(t_pagina* pagina);
void 		tasa_aciertos_TLB							();

#endif /* PROCESOADMINDEMEMORIA_H_ */
