

#ifndef PROCESOCPU_H_
#define PROCESOCPU_H_

//////////////////////////////////////////////BIBLIOTECAS/////////////////////////////////////////
#include <util.h>
#include <commons/config.h>
#include <pthread.h>
#include <commons/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <commons/log.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <time.h>
#include <features.h>
#include <semaphore.h>


/////////////////////////////////////////////////////VARIABLES///////////////////////////////////////



char ipmem[15];
char ipplanif[15];

pthread_mutex_t mutex;
t_log* logger;
t_config* cfg;

int* socket_planificador;
int* socket_memoria;
int* sentencias_ejecutadas_ultimo_min;
int* porcentaje_a_planificador;

int socket_planificador_especial;
int puertomem;
int cant_hilos;
int puertoplanif;
int flag_error_memoria;
int flag_error_planificador;

/////////////////////////////////////////////////ESTRUCTURAS/////////////////////////////////////////

typedef struct{
	e_sentencia sentencia;
	int pagina;
	char* texto;
	int cant_paginas;
	unsigned int tiempo;
	int pid;
	int hilo;
}t_sentencia;

typedef struct{
	e_sentencia sentencia;
	t_pcb* pcb;
	int cantidad_sentencias;
	unsigned int tiempo;
	bool ejecuto_ok;
	t_list* resultados_sentencias;
}t_resultado_pcb;

///////////////////////////////////////////////PROTOTIPOS//////////////////////////////////////////////


int 			avisar_a_planificador(t_resultado_pcb respuesta,int socket_planif, int hilo);
int 			CANTIDAD_HILOS();
int 			conectar_con_memoria(int numero);
int 			conectar_con_planificador(int numero);
int 			conectar_con_planificador_especial();
int 		    dividir(unsigned long int sent_ejecutadas,int sent_maximas);
int				desconexion_planificador();
int 			desconexion_memoria();
int 			enviar_porcentaje_a_planificador();
int 			enviar_logs(int socket, t_list* resultados_sentencias);
int 			finalizar();
int 			inicializar();
int 			PUERTO_MEMORIA();
int 			PUERTO_PLANIFICADOR();
int 			procesar_mensaje_planif(t_msg* msg,int numero);
int 			procesar_mensaje_planif(t_msg* msg,int numero);
int 			pcb_tiene_que_seguir_ejecutando(t_pcb* pcb);
int				RETARDO();
int 			RETARDO_MINIMO();
int 			sent_ejecutar_iniciar(t_sentencia* sent,int socket_mem);
int 			sent_ejecutar_finalizar(t_sentencia* sent,int socket_mem);

t_msg* 			sent_to_msg(t_sentencia* sent);
t_resultado_pcb ejecutar(t_pcb* pcb,int socket_mem, int hilo);

void 			dormir2();
void 		    inicializar_porcentajes();
void  			inicializar_resultados_sentencias();
void* 			hilo_responder_porcentaje();
void* 			hilo_cpu(int *numero_hilo);
void* 			hilo_porcentaje() ;

char* 			CONFIG_PAT();
char* 			IP_PLANIFICADOR();
char* 			IP_MEMORIA();
char* 			LOGGER_PAT();
char* 			sent_ejecutar_leer(t_sentencia* sent,int socket_mem);


#endif /* PROCESOCPU_H_ */
