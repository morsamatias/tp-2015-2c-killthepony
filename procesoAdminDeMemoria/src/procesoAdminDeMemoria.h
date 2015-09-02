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

char* CONFIG_PATH = "config.txt";
char* LOGGER_PATH = "log.txt";

t_log* logger;
t_config* config;

int PUERTO_ESCUCHA();
int iniciar_server();

int inicializar();
int finalizar();
int conectar_con_swap();

#endif /* PROCESOADMINDEMEMORIA_H_ */
