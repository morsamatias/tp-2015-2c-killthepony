

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

int socket_planif;
int socket_mem;

/////////////////////////////////////////////////ESTRUCTURAS/////////////////////////////////////////
typedef enum {
	iniciar,
	leer,
	escribir,
	io,
	final,
	error
}e_sentencia;

typedef struct{
	e_sentencia sentencia;
	int pagina;
	char* texto;
	int cant_paginas;
	unsigned int tiempo;
}t_sentencia;

///////////////////////////////////////////////PROTOTIPOS//////////////////////////////////////////////

int inicializar();
int finalizar();
int conectar_con_planificador();
int procesar_mensaje_planif(t_msg* msg);
t_resultado_pcb ejecutar(t_pcb* pcb);
int pcb_tiene_que_seguir_ejecutando(t_pcb* pcb);
int conectar_con_memoria();
void* hilo_cpu(int *numero_hilo);
char* IP_PLANIFICADOR();
int PUERTO_PLANIFICADOR();
char* IP_MEMORIA();
int PUERTO_MEMORIA();
int ID();
int CANTIDAD_HILOS();
int RETARDO();


#endif /* PROCESOCPU_H_ */
