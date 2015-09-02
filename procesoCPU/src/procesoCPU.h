/*
 * procesoCPU.h
 *
 *  Created on: 29/8/2015
 *      Author: utnso
 */

#ifndef PROCESOCPU_H_
#define PROCESOCPU_H_

#include <util.h>
#include "config_cpu.h"
#include <pthread.h>

//config
#include <commons/log.h>

char* LOGGER_PATH = "log.txt";

t_log* logger;

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

int inicializar();
int finalizar();
int conectar_con_planificador();
int procesar_mensaje_planif(t_msg* msg);
int ejecutar(t_pcb* pcb);
int pcb_tiene_que_seguir_ejecutando(t_pcb* pcb);

int conectar_con_memoria();

pthread_mutex_t mutex;


#endif /* PROCESOCPU_H_ */
