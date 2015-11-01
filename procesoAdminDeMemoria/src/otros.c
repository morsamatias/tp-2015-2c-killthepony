#include "procesoAdminDeMemoria.h"

int PUERTO_ESCUCHA(){
	return config_get_int_value(cfg, "PUERTO_ESCUCHA");
}
char* IP_SWAP(){
	return config_get_string_value(cfg, "IP_SWAP");
}
int PUERTO_SWAP(){
	return config_get_int_value(cfg, "PUERTO_SWAP");
}
int MAXIMO_MARCOS_POR_PROCESO(){
	return config_get_int_value(cfg, "MAXIMO_MARCOS_POR_PROCESO");
}
int CANTIDAD_MARCOS(){
	return config_get_int_value(cfg, "CANTIDAD_MARCOS");
}
int TAMANIO_MARCO(){
	return config_get_int_value(cfg, "TAMANIO_MARCO");
}
int ENTRADAS_TLB(){
	return config_get_int_value(cfg, "ENTRADAS_TLB");
}
int TLB_HABILITADA(){
	return config_get_int_value(cfg, "TLB_HABILITADA");
}
int RETARDO_MEMORIA(){
	return config_get_int_value(cfg, "RETARDO_MEMORIA");
}
char* ALGORITMO_REEMPLAZO(){
	return config_get_string_value(cfg, "ALGORITMO_REEMPLAZO");
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

