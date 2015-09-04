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
int conectar_con_swap();

#endif /* PROCESOADMINDEMEMORIA_H_ */
