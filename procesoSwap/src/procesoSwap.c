/*
 ============================================================================
 Name        : procesoSwap.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "procesoSwap.h"

pthread_mutex_t mutex;
int main(void) {
	inicializar();

	server_socket_select(PUERTO_ESCUCHA(), procesar_mensaje_mem);

	finalizar();
	return EXIT_SUCCESS;
}

int inicializar(){

	cfg = config_create(CONFIG_PATH);

	clean_file(LOGGER_PATH);
	logger = log_create(LOGGER_PATH, "procesoSwap", true, LOG_LEVEL_TRACE);
	pthread_mutex_init(&mutex, NULL);
	return 0;
}

int finalizar(){

	config_destroy(cfg);
	log_destroy(logger);
	return 0;
}

void procesar_mensaje_mem(int socket, t_msg* msg){
	printf("Inicio msj\n");
	//print_msg(msg);
	switch (msg->header.id) {
		case MEM_INICIAR:

			pthread_mutex_lock(&mutex);
			log_trace(logger, "Nuevo Proceso. pid: %d, Paginas: %d", msg->argv[0], msg->argv[1]);
			pthread_mutex_unlock(&mutex);

			destroy_message(msg);

			msg = argv_message(MEM_INICIAR, 0);
			enviar_y_destroy_mensaje(socket, msg);
			break;
		default:
			break;
	}

	puts("Fin msj");
}
