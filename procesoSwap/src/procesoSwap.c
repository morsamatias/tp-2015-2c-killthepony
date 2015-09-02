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

int main(void) {
	inicializar();

	server_socket_select(PUERTO_ESCUCHA(), procesar_mensaje);

	finalizar();
	return EXIT_SUCCESS;
}

int inicializar(){

	config = config_create(CONFIG_PATH);

	clean_file(LOGGER_PATH);
	logger = log_create(LOGGER_PATH, "procesoSwap", true, LOG_LEVEL_TRACE);

	return 0;
}

int finalizar(){

	config_destroy(config);
	log_destroy(logger);
	return 0;
}

void procesar_mensaje(int socket, t_msg* msg){
	printf("Inicio msj\n");
	print_msg(msg);

	destroy_message(msg);
	puts("Fin msj");
}

int PUERTO_ESCUCHA(){
	return config_get_int_value(config, "PUERTO_ESCUCHA");
}
