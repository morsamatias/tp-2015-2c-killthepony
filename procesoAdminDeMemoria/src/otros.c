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
int RETARDO_MEMORIA_MINIMO(){
	return config_get_int_value(cfg, "RETARDO_MEMORIA_MILISEGUNDOS");
}
char* ALGORITMO_REEMPLAZO(){
	return config_get_string_value(cfg, "ALGORITMO_REEMPLAZO");
}



int conectar_con_swap(){
	int sock ;
	sock = client_socket(IP_SWAP(), PUERTO_SWAP());

	if(sock<0){
		log_info(log_general_p, "Error al conectar con el swap. %s:%d", IP_SWAP(), PUERTO_SWAP());
		log_error(log_errores, "Error al conectar con el swap. %s:%d", IP_SWAP(), PUERTO_SWAP());
	}else{
		log_info(log_general_p, "Conectado con swap. %s:%d", IP_SWAP(), PUERTO_SWAP());
	}

	//envio handshake

	t_msg* msg = string_message(0, "Hola soy el proceso admin de memoria", 0);
	if (enviar_mensaje(sock, msg)>0){
		log_info(log_general_f, "SWAP - Mensaje de confirmacion enviado correctamente");
	}

	destroy_message(msg);

	return sock;
}


int swap_nuevo_proceso(int pid, int nro_paginas){
	t_msg* msg = NULL;
	msg = argv_message(SWAP_INICIAR, 2, pid, nro_paginas);
	enviar_y_destroy_mensaje(socket_swap, msg);
	log_info(log_general_f, "SWAP - Pedido reserva: Proceso %d - Paginas %d",pid,nro_paginas);

	msg = recibir_mensaje(socket_swap);
	if(msg!=NULL){
		if(msg->header.id == SWAP_OK){
			log_info(log_general_f, "SWAP - Reserva correcta");
			destroy_message(msg);
			return 0;
		}
		log_info(log_general_f, "SWAP - No se pudo reservar %d paginas para el proceso %d",nro_paginas,pid);
		log_error(log_errores, "SWAP - No se pudo reservar %d paginas para el proceso %d",nro_paginas,pid);
		destroy_message(msg);

	}else{
		log_info(log_general_f, "SWAP - No se pudo reservar por problemas de comunicacion");
		log_error(log_errores, "SWAP - No se pudo reservar por problemas de comunicacion");
	}
	return -1;
}



char* swap_leer_pagina(int pid, int pagina){
	t_msg* msg = NULL;

	msg = argv_message(SWAP_LEER, 2, pid, pagina);
	enviar_y_destroy_mensaje(socket_swap, msg);
	log_info(log_general_f, "SWAP - Pedido lectura: Proceso %d - Pagina %d",pid,pagina);
	msg = recibir_mensaje(socket_swap);

	if(msg!=NULL){
		if(msg->header.id == SWAP_OK){
			char* contenido;
			contenido = string_duplicate(msg->stream);
			log_info(log_general_f, "SWAP - Lectura correcta: %s",contenido);
			destroy_message(msg);
			return contenido;
		}else{
			log_info(log_general_f, "SWAP - Lectura incorrecta");
			log_error(log_errores, "SWAP - Lectura incorrecta");
		}
		destroy_message(msg);
	}else{
		log_info(log_general_f, "SWAP - Lectura incorrecta por problemas de comunicacion");
		log_error(log_errores, "SWAP - Lectura incorrecta por problemas de comunicacion");
	}
	return NULL;
}

int swap_escribir_pagina(int pid, int pagina, char* contenido){
	t_msg* msg = NULL;

	msg = string_message(SWAP_ESCRIBIR,contenido, 2, pid, pagina);
	enviar_y_destroy_mensaje(socket_swap, msg);
	log_info(log_general_f, "SWAP - Pedido escritura: Proceso %d - Pagina %d - Contenido %s",pid,pagina,contenido);

	msg = recibir_mensaje(socket_swap);

	if(msg!=NULL){
		if(msg->header.id == SWAP_OK){
			log_info(log_general_f, "SWAP - Escritura correcta");
			destroy_message(msg);
			return 0;
		}else{
			log_info(log_general_f, "SWAP - Escritura incorrecta");
			log_error(log_errores, "SWAP - Escritura incorrecta");
		}
		destroy_message(msg);
	}else{
		log_info(log_general_f, "SWAP - Escritura incorrecta por problemas de comunicacion");
		log_error(log_errores, "SWAP - Escritura incorrecta por problemas de comunicacion");
	}

	return -1;
}

int swap_finalizar(int pid){
	t_msg* msg = NULL;

	msg = argv_message(SWAP_FINALIZAR, 1, pid);
	enviar_y_destroy_mensaje(socket_swap, msg);
	log_info(log_general_f, "SWAP - Pedido finalizacion: Proceso %d",pid);

	msg = recibir_mensaje(socket_swap);

	if(msg!=NULL){
		if(msg->header.id == SWAP_OK){
			log_info(log_general_f, "SWAP - Finalizacion correcta");
			return 0;
		}else{
			log_info(log_general_f, "SWAP - Finalizacion incorrecta");
			log_error(log_errores, "SWAP - Finalizacion incorrecta");
		}
		destroy_message(msg);
	}else{
		log_info(log_general_f, "SWAP - Finalizacion incorrecta por problemas de comunicacion");
		log_error(log_errores, "SWAP - Finalizacion incorrecta por problemas de comunicacion");
	}

	return -1;
}


