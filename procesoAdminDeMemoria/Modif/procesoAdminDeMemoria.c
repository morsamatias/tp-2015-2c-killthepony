/*
 ============================================================================
 Name        : procesoAdminDeMemoria.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "procesoAdminDeMemoria.h"

pthread_t th_server_cpu;
int socket_swap;

int main(void) {
	inicializar();

	pthread_create(&th_server_cpu, NULL, (void*) iniciar_server, NULL);

	socket_swap = conectar_con_swap();

	pthread_join(th_server_cpu, NULL);


	finalizar();

	return EXIT_SUCCESS;

}

int conectar_con_swap(){
	int sock ;
	sock = client_socket(IP_SWAP(), PUERTO_SWAP());

	if(sock<0){
		log_trace(logger, "Error al conectar con el swap. %s:%d", IP_SWAP(), PUERTO_SWAP());
	}else{
		log_trace(logger, "Conectado con swap. %s:%d", IP_SWAP(), PUERTO_SWAP());
	}

	//envio handshake

	t_msg* msg = string_message(0, "Hola soy el proceso admin de memoria", 0);
	if (enviar_mensaje(sock, msg)>0){
		log_trace(logger, "Mensaje enviado OK");
	}

	destroy_message(msg);

	return sock;
}

int swap_nuevo_proceso(int pid, int paginas){
	t_msg* msg = NULL;
	msg = argv_message(MEM_INICIAR, 2, pid, paginas);
	enviar_y_destroy_mensaje(socket_swap, msg);

	msg = recibir_mensaje(socket_swap);
	destroy_message(msg);

	return 0;
}

void procesar_mensaje_cpu(int socket, t_msg* msg){
	puts("Inicio msj**************************************");
	char* buff_pag  = NULL;
	int pid = 123;
	int paginas;
	switch (msg->header.id) {
		case MEM_INICIAR:
			//param 0 cant_paginas
			log_trace(logger, "Mem Iniciar Paginas: %d", msg->argv[0]);

			pid = 123;
			paginas = msg->argv[0];

			destroy_message(msg);

			swap_nuevo_proceso(pid, paginas);


			msg = argv_message(MEM_OK, 1 ,0);
			enviar_y_destroy_mensaje(socket, msg);

			break;
		case MEM_LEER:
			//param 0 nro pagina
			log_trace(logger, "Mem Leer pagina %d", msg->argv[0]);
			buff_pag = string_from_format("Contenido de la pagina %d", msg->argv[0]);
			destroy_message(msg);

			msg = string_message(MEM_OK, buff_pag, 0);
			enviar_y_destroy_mensaje(socket, msg);
			FREE_NULL(buff_pag);
			break;
		case MEM_ESCRIBIR:
			//param 0 nro pagina
			log_trace(logger, "MEM_ESCRIBIR pagina %d, texto: \"%s\"", msg->argv[0], msg->stream);
			destroy_message(msg);

			msg = argv_message(MEM_OK, 1, 0);
			enviar_y_destroy_mensaje(socket, msg);
			break;
		case MEM_FINALIZAR:
			//param 0 nro pagina
			log_trace(logger, "MEM_FINALIZAR");
			destroy_message(msg);

			msg = argv_message(MEM_OK, 1, 0);
			enviar_y_destroy_mensaje(socket, msg);
			break;
		default:
			break;
	}



	//destroy_message(msg);
	puts("Fin msj**************************************");
}

int iniciar_server(){

	//printf("Iniciando server\n");
	server_socket_select(PUERTO_ESCUCHA(), procesar_mensaje_cpu);

	return 0;
}


int inicializar(){

	// ARCHIVO DE CONFIGURACION
	cfg = config_create(CONFIG_PATH);

	// ARCHIVO DE LOG
	clean_file(LOGGER_PATH);
	logger = log_create(LOGGER_PATH, "procesoAdminMem", false, LOG_LEVEL_TRACE);

	// ESTRUCTURA TLB y MEMORIA
	TBL 	= (t_paginas*)malloc(ENTRADAS_TLB()*sizeof(t_paginas));
	memoria = (t_memoria*)malloc(CANTIDAD_MARCOS()*sizeof(t_memoria));
	l_paginas_por_proceso = list_create();

	return 0;
}

int finalizar(){

	// ARCHIVO DE CONFIGURACION
	config_destroy(cfg);

	// ARCHIVO DE LOG
	log_destroy(logger);

	// ESTRUCTURA TLB y MEMORIA
	free(TBL);
	free(memoria);
	list_destroy_and_destroy_elements(l_paginas_por_proceso,(void*)eliminar_estructuras_de_un_proceso);

	return 0;
}

void eliminar_estructuras_de_un_proceso(t_proceso* proceso){
	list_destroy_and_destroy_elements(proceso->paginas,(void*)eliminar_paginas_de_un_proceso);
	free(proceso);
}

void eliminar_paginas_de_un_proceso(t_paginas* pagina){
	memoria[pagina->entrada].libre=1;
	memoria[pagina->entrada].modificado=0;
	free(pagina);
}
