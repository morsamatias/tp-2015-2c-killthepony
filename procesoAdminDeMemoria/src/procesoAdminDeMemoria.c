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

	socket_swap = conectar_con_swap();

	server_socket_select(PUERTO_ESCUCHA(), procesar_mensaje_cpu);
	//pthread_create(&th_server_cpu, NULL, (void*) iniciar_server, NULL);

	//pthread_join(th_server_cpu, NULL);


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
	msg = argv_message(SWAP_INICIAR, 2, pid, paginas);
	enviar_y_destroy_mensaje(socket_swap, msg);

	msg = recibir_mensaje(socket_swap);
	if(msg!=NULL){
		if(msg->header.id == SWAP_OK){
			log_trace(logger, "SWAP_INICIAR OKOK OK");
			destroy_message(msg);
			return 0;
		}
		log_trace(logger, "SWAP_INICIAR NO OK");
		destroy_message(msg);

	}else{
		log_trace(logger, "SWAP_INICIAR > Erro al recibir msg");
	}


	return -1;
}

char* swap_leer_pagina(int pid, int pagina){
	t_msg* msg = NULL;

	msg = argv_message(SWAP_LEER, 2, pid, pagina);
	enviar_y_destroy_mensaje(socket_swap, msg);

	msg = recibir_mensaje(socket_swap);

	if(msg!=NULL){
		if(msg->header.id == SWAP_OK){
			char* contenido;
			contenido = string_duplicate(msg->stream);
			log_trace(logger, "SWAP_LEER pid: %d, pag: %d, Contenido: %s", pid, pagina, contenido);
			destroy_message(msg);
			return contenido;
		}else{
			log_trace(logger, "SWAP_LEER pid: %d, pag: %d, NOOOOO", pid, pagina);
		}
		destroy_message(msg);
	}else
		log_trace(logger, "SWAP_LEER Erro al recibir mensaje");



	return NULL;
}

int swap_escribir_pagina(int pid, int pagina, char* contenido){
	t_msg* msg = NULL;

	msg = string_message(SWAP_ESCRIBIR,contenido, 2, pid, pagina);
	enviar_y_destroy_mensaje(socket_swap, msg);

	msg = recibir_mensaje(socket_swap);

	if(msg!=NULL){
		if(msg->header.id == SWAP_OK){
			log_trace(logger, "SWAP_ESCRIBIR pid: %d, pag: %d, Contenido: %s", pid, pagina, contenido);
			destroy_message(msg);
			return 0;
		}else{
			log_trace(logger, "SWAP_ESCRIBIR pid: %d, pag: %d, NOOOOO", pid, pagina);
		}
		destroy_message(msg);
	}else
		log_trace(logger, "SWAP_ESCRIBIR Erro al recibir mensaje");

	return -1;
}


int swap_finalizar(int pid){
	t_msg* msg = NULL;

	msg = argv_message(SWAP_FINALIZAR, 1, pid);
	enviar_y_destroy_mensaje(socket_swap, msg);

	msg = recibir_mensaje(socket_swap);

	if(msg!=NULL){
		if(msg->header.id == SWAP_OK){
			log_trace(logger, "SWAP_FINALIZAR pid: %d", pid);
		}else{
			log_trace(logger, "SWAP_FINALIZAR pid: %d NOOOOO", pid);
		}
		destroy_message(msg);
	}else
		log_trace(logger, "SWAP_FINALIZAR Erro al recibir mensaje");

	return 0;
}


void procesar_mensaje_cpu(int socket, t_msg* msg){
	puts("Inicio msj**************************************");
	//print_msg(msg);
	char* buff_pag  = NULL;
	int st ;
	int pid = 123;
	int pagina;
	int paginas;
	switch (msg->header.id) {
		case MEM_INICIAR:
			//param 0 cant_paginas
			//param 1 PID

			paginas = msg->argv[0];
			pid 	= msg->argv[1];

			log_trace(logger, "Iniciar Proceso %d con %d paginas",pid,paginas);
			destroy_message(msg);

			st = iniciar_proceso_CPU(pid,paginas);

			switch(st){
			case 0:
				msg = argv_message(MEM_OK, 1 ,0);
				enviar_y_destroy_mensaje(socket, msg);
				log_info(logger, "El proceso %d fue inicializado correctamente",pid);
				break;
			case 1:
				msg = argv_message(MEM_NO_OK, 1 ,0);
				enviar_y_destroy_mensaje(socket, msg);
				log_warning(logger, "La cantidad de paginas del proceso %d es mayor a la posible",pid);
				break;
			case 2:
				msg = argv_message(MEM_NO_OK, 1 ,0);
				enviar_y_destroy_mensaje(socket, msg);
				log_warning(logger, "No hay espacio suficiente para alocar la memoria del proceso %d",pid);
				break;
			}


			break;
		case MEM_LEER:
			//param 0 nro pagina
			log_trace(logger, "Mem Leer pagina %d", msg->argv[0]);

			//buff_pag = string_from_format("Contenido de la pagina %d", msg->argv[0]);
			pid = 123;
			pagina = msg->argv[0];
			destroy_message(msg);

			buff_pag = swap_leer_pagina(pid, pagina);

			if(buff_pag != NULL){
				msg = string_message(MEM_OK, buff_pag, 0);
				enviar_y_destroy_mensaje(socket, msg);
				FREE_NULL(buff_pag);
			}else{
				msg = string_message(MEM_NO_OK, "", 0);
				enviar_y_destroy_mensaje(socket, msg);
			}



			break;
		case MEM_ESCRIBIR:
			//param 0 nro pagina
			log_trace(logger, "MEM_ESCRIBIR pagina %d, texto: \"%s\"", msg->argv[0], msg->stream);

			buff_pag = string_duplicate(msg->stream);
			pid = 123;
			pagina = msg->argv[0];

			destroy_message(msg);

			st = swap_escribir_pagina(pid, pagina, buff_pag);
			if(st >=0){
				msg = argv_message(MEM_OK, 1, 0);
			}else{
				msg = argv_message(MEM_NO_OK, 1, 0);
			}


			enviar_y_destroy_mensaje(socket, msg);
			FREE_NULL(buff_pag);
			break;
		case MEM_FINALIZAR:
			//param 0 nro pagina
			log_trace(logger, "MEM_FINALIZAR");

			destroy_message(msg);
			pid = 123;
			swap_finalizar(pid);

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
	logger = log_create(LOGGER_PATH, "procesoAdminMem", true, LOG_LEVEL_TRACE);

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

int iniciar_proceso_CPU(int pid, int paginas){
	int st;

	// CHEQUEO QUE LA CANTIDAD DE PAGINAS POR PROCESO REAL SEA MENOR A LA MAXIMA
	if(paginas > MAXIMO_MARCOS_POR_PROCESO()){
		return 1;
	}

	// LE ENVIO AL PROCESO SWAP PARA QUE RESERVE EL ESPACIO
	st = swap_nuevo_proceso(pid, paginas);

	if(st==0){

		return 0;
	}
	else{
		return 2;
	}

}


void crear_estructuras_de_un_proceso(int PID,int paginas){
	int i;
	t_proceso* proceso= (t_proceso*)malloc((paginas+2)*sizeof(int));
	proceso->PID = PID;
	proceso->cant_paginas = paginas;
	for(i=0;i<paginas;i++)
		proceso->entradas[i]=-1;
}

void eliminar_estructuras_de_un_proceso(t_proceso* proceso){
	int i;
	for(i=0;i<proceso->cant_paginas;i++){
		if(proceso->entradas[i]!=-1){
				memoria[proceso->entradas[i]].libre=1;
				memoria[proceso->entradas[i]].modificado=0;
			}
	}
	free(proceso);
}
