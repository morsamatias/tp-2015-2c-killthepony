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

void procesar_mensaje_mem(int socket_mem, t_msg* msg){
	char* buffer;
	int pid, pagina;
	//print_msg(msg);
	switch (msg->header.id) {
		case SWAP_INICIAR:

			pthread_mutex_lock(&mutex);
			log_trace(logger, "SWAP_INICIAR . pid: %d, Paginas: %d", msg->argv[0], msg->argv[1]);
			pthread_mutex_unlock(&mutex);

			destroy_message(msg);

			//envio 1 = true
			msg = argv_message(SWAP_OK, 0);
			enviar_y_destroy_mensaje(socket_mem, msg);
			break;

		case SWAP_LEER:
			log_trace(logger, "SWAP_LEER. pid: %d, Pagina: %d", msg->argv[0], msg->argv[1]);
			pid = msg->argv[0];
			pagina = msg->argv[1];
			destroy_message(msg);
			//envio 1 = true
			buffer = string_from_format("Pid: %d, pag: %d, contenido: HOLAAA", pid, pagina);
			msg = string_message(SWAP_OK,buffer , 0);
			enviar_y_destroy_mensaje(socket_mem, msg);
			free(buffer);
			/*
			t_foo* foo;
			recibirMensaje(socket_mem, (void*)&foo);
			printf("pid: %d, pagina: %d, texto: %s\n", foo->pid, foo->pagina, foo->texto);
			free(foo);
			*/
			break;
		case SWAP_ESCRIBIR:
			log_trace(logger, "SWAP_ESCRIBIR. pid: %d, Pagina: %d, conteindo: %s", msg->argv[0],msg->argv[1], msg->stream);
			destroy_message(msg);
			//envio 1 = true
			msg = argv_message(SWAP_OK, 0);
			enviar_y_destroy_mensaje(socket_mem, msg);

			break;

		case SWAP_FINALIZAR:
			log_trace(logger, "SWAP_FINALIZAR. pid: %d", msg->argv[0]);
			destroy_message(msg);
			//envio 1 = true
			msg = argv_message(SWAP_OK, 0);
			enviar_y_destroy_mensaje(socket_mem, msg);

			break;

		default:
			break;
	}


}
