

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


pthread_mutex_t mutex;
t_log* logger;
t_config* cfg;

//int socket_planif;
//int socket_mem;

int* socket_planificador;
int socket_planificador_especial;
int* socket_memoria;

int* porcentaje;
int* porcentaje_a_planificador;

/////////////////////////////////////////////////ESTRUCTURAS/////////////////////////////////////////

typedef struct{
	e_sentencia sentencia;
	int pagina;
	char* texto;
	int cant_paginas;
	unsigned int tiempo;
	int pid;
}t_sentencia;

typedef struct{
	e_sentencia sentencia;
	t_pcb* pcb;
	int cantidad_sentencias;
	unsigned int tiempo;
}t_resultado_pcb;
///////////////////////////////////////////////PROTOTIPOS//////////////////////////////////////////////

int inicializar();
int finalizar();
int conectar_con_planificador(int numero);
int conectar_con_planificador_especial();
int enviar_porcentaje_a_planificador();
void* hilo_responder_porcentaje();
int procesar_mensaje_planif(t_msg* msg,int numero);
t_resultado_pcb ejecutar(t_pcb* pcb,int socket_mem);
int procesar_mensaje_planif(t_msg* msg,int numero);
t_resultado_pcb ejecutar(t_pcb* pcb,int numero);
int pcb_tiene_que_seguir_ejecutando(t_pcb* pcb);
int sent_ejecutar_iniciar(t_sentencia* sent,int socket_mem);
char* sent_ejecutar_leer(t_sentencia* sent,int socket_mem);
int sent_ejecutar_finalizar(t_sentencia* sent,int socket_mem);
int conectar_con_memoria(int numero);
void* hilo_cpu(int *numero_hilo);
char* IP_PLANIFICADOR();
int PUERTO_PLANIFICADOR();
char* IP_MEMORIA();
int PUERTO_MEMORIA();
int CANTIDAD_HILOS();
int RETARDO();
int RETARDO_MINIMO();

int avisar_a_planificador(t_resultado_pcb respuesta,int socket_planif);



#endif /* PROCESOCPU_H_ */
