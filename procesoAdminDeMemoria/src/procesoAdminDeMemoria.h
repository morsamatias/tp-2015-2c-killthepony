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
#include <commons/log.h>

#include "config_mem.h"

char* LOGGER_PATH = "log.txt";

t_log* logger;

int iniciar_server();

int inicializar();
int finalizar();
void procesar_mensaje_cpu(int socket, t_msg* msg);
int conectar_con_swap();

char* swap_leer_pagina(int pid, int pagina);
int swap_nuevo_proceso(int pid, int paginas);

//// NUEVAS

void eliminar_estructuras_de_un_proceso(t_proceso* proceso);
void crear_estructuras_de_un_proceso(int PID, int paginas);
int iniciar_proceso_CPU(int pid, int paginas);


#endif /* PROCESOADMINDEMEMORIA_H_ */
