/*
 ============================================================================
 Name        : procesoCPU.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "procesoCPU.h"

int socket_planif;
int socket_mem;
int main(void) {

	inicializar();

	socket_planif = conectar_con_planificador();
	if (socket_planif < 0) {
		return -1;
	}
	socket_mem = conectar_con_memoria();
	if (socket_mem < 0) {
		return -1;
	}


	t_msg* msg = NULL;
	//while(socket_planif>0){
	while(true){
		log_trace(logger, "Esperando peticiones del planificador");
		msg = recibir_mensaje(socket_planif);
		log_trace(logger, "Nuevo mensaje del planificador");

		procesar_mensaje_planif(msg);
	}

	finalizar();
	return EXIT_SUCCESS;
}

int procesar_mensaje_planif(t_msg* msg){
	//print_msg(msg);
	t_pcb* pcb = NULL;
	switch(msg->header.id){
	case PCB_A_EJECUTAR:
		destroy_message(msg);

		pcb = recibir_mensaje_pcb(socket_planif);
		//pcb_print(pcb);

		log_trace(logger, "Ejecutando %s, PC: %d, cant: %d, cant_sent: %d", pcb->path, pcb->pc, pcb->cant_a_ejectuar, pcb->cant_sentencias);
		ejecutar(pcb);
		log_trace(logger, "Fin ejecucion %s, PC: %d, cant: %d, cant_sent: %d", pcb->path, pcb->pc, pcb->cant_a_ejectuar, pcb->cant_sentencias);

		break;


	}

	destroy_message(msg);
	return 0;
}

int pcb_tiene_que_seguir_ejecutando(t_pcb* pcb){
	return pcb->pc < pcb->cant_sentencias ;
}

/*
 * formatear para que quede solo texto
 */
char* get_texto_solo(char* texto){
	char* txt;
	//borro la commilla de principio y final
	int len = strlen(texto)-2;
	txt = malloc(len+1);
	memset(txt, 0, len);//inicializo en 0
	memcpy(txt, texto+1, len);
	txt[len] = '\0';
	return txt;
}
/*
 * el param seria p.e: escribir 3 "hola"
 */
t_sentencia* sentencia_crear(char* sentencia, int pid){
	t_sentencia* sent = malloc(sizeof(*sent));

	sent->pid = pid;

	char** split =  string_split(sentencia, " ");

	if(string_starts_with(sentencia, "iniciar ")){
		sent->sentencia = iniciar;
		sent->cant_paginas = atoi(split[1]);
	}
	else if(string_starts_with(sentencia, "leer ")){
		sent->sentencia = leer;
		sent->pagina = atoi(split[1]);
	}
	else if(string_starts_with(sentencia, "escribir ")){
		sent->sentencia = escribir;
		sent->pagina = atoi(split[1]);

		sent->texto = get_texto_solo(split[2]);
	}
	else if(string_starts_with(sentencia, "entrada-salida ")){
		sent->sentencia = io;
		sent->tiempo = atoi(split[1]);
	}
	else if(string_starts_with(sentencia, "finalizar")){
		sent->sentencia = final;
	}else{
		printf("Error para desconocidoooooooooooooooooooooooooooooooooo\n");
		sent->sentencia = error;
	}
	return sent;
}


int sent_ejecutar_iniciar(t_sentencia* sent){
	int rs = 0;
	t_msg* msg = NULL;
	msg = argv_message(MEM_INICIAR, 2, sent->pid, sent->cant_paginas);
	log_trace(logger, "Enviando mensaje MemIniciar %d",sent->cant_paginas);
	enviar_y_destroy_mensaje(socket_mem, msg);

	log_trace(logger, "Esperando Rta MemIniciar %d",sent->cant_paginas);
	msg = recibir_mensaje(socket_mem);
	if (msg != NULL) {

		if (msg->header.id == MEM_OK) {
			//el argv[0] es el estado
			log_trace(logger, "Rta mensaje MemIniciar %d", msg->argv[0]);
			rs = 0; // OK
		}else{
			rs = -1;// NO OK
		}
		destroy_message(msg);
		return rs;
	} else {
		log_trace(logger, "RTA MemIniciar NULL");
		return -2;
	}
}


int sent_ejecutar_finalizar(t_sentencia* sent){
	int rs = 0;
	t_msg* msg = NULL;
	msg = argv_message(MEM_FINALIZAR, 1, sent->pid);
	log_trace(logger, "Enviando mensaje MEM_FINALIZAR ");
	enviar_y_destroy_mensaje(socket_mem, msg);

	log_trace(logger, "Esperando Rta MEM_FINALIZAR");
	msg = recibir_mensaje(socket_mem);
	if (msg != NULL) {

		if (msg->header.id == MEM_OK) {
			//el argv[0] es el estado
			log_trace(logger, "Rta mensaje MEM_FINALIZAR");
			rs = 0; // OK
		}else{
			rs = -1;// NO OK
		}
		destroy_message(msg);
		return rs;
	} else {
		log_trace(logger, "RTA MEM_FINALIZAR NULL");
		return -2;
	}
}


int sent_ejecutar_escribir(t_sentencia* sent){
	int rs = 0;

	t_msg* msg = NULL;
	msg = string_message(MEM_ESCRIBIR, sent->texto, 2, sent->pid, sent->pagina);
	log_trace(logger, "Enviando mensaje MEM_ESCRIBIR %d %s",	sent->pagina, sent->texto);
	enviar_y_destroy_mensaje(socket_mem, msg);

	log_trace(logger, "Esperando rta mensaje MEM_ESCRIBIR %d", sent->pagina, sent->texto);
	msg = recibir_mensaje(socket_mem);
	if (msg != NULL) {
		if (msg->header.id == MEM_OK) {
			//en el stream esta el contenido de la pagina
			log_trace(logger, "Rta mensaje MEM_ESCRIBIR %d", msg->argv[0]);
			rs = 0; // OK
		}else{
			rs = -1;
		}
		destroy_message(msg);

		return rs;
	} else {
		log_trace(logger, "RTA MEM_ESCRIBIR NULL");
		return -2;
	}
}


char* sent_ejecutar_leer(t_sentencia* sent){
	char* pagina = NULL;

	t_msg* msg = NULL;
	msg = argv_message(MEM_LEER, 2, sent->pid, sent->pagina);
	log_trace(logger, "Enviando mensaje MEM_LEER %d",	sent->pagina);
	enviar_y_destroy_mensaje(socket_mem, msg);

	log_trace(logger, "Esperando rta mensaje MEM_LEER %d", sent->pagina);
	msg = recibir_mensaje(socket_mem);
	if (msg != NULL) {
		if (msg->header.id == MEM_OK) {
			//en el stream esta el contenido de la pagina
			log_trace(logger, "Rta mensaje: MEM_LEER %s", msg->stream);
			pagina = string_duplicate(msg->stream);

		}else{
			pagina = NULL;
		}
		destroy_message(msg);

		return pagina;
	} else {
		log_trace(logger, "RTA MEM_LEER NULL");
		return NULL;
	}
}

int sent_ejecutar(t_sentencia* sent){
	char* pagina = NULL;
	switch (sent->sentencia) {
	case iniciar:
		sent_ejecutar_iniciar(sent);
		break;
	case leer:
		pagina  = sent_ejecutar_leer(sent);
		FREE_NULL(pagina);
		break;
	case escribir:
		sent_ejecutar_escribir(sent);
		break;
	case io:

		break;
	case error:
		log_trace(logger, "Error de sentencia en archivo");
		break;
	case final:
		sent_ejecutar_finalizar(sent);
		break;
	default:
		log_trace(logger, "case default");
		break;
	}

	return 0;
}

void sent_free(t_sentencia* sent){
	if(sent->sentencia == escribir)
		free(sent->texto);
	free(sent);
}

int ejecutar(t_pcb* pcb){

	char* mcod = file_get_mapped(pcb->path);
	char** sents = string_split(mcod, "\n");
	t_sentencia* sent = NULL;

	while(pcb_tiene_que_seguir_ejecutando(pcb)){

		sent = sentencia_crear(sents[pcb->pc], pcb->pid);

		sent_ejecutar(sent);

		sent_free(sent);
		pcb->pc++;
	}

	free_split(sents);

	file_mmap_free(mcod, pcb->path);
	return 0;
}

int conectar_con_memoria(){
	int sock;
	sock = client_socket(IP_MEMORIA(), PUERTO_MEMORIA());

	if (sock < 0) {
		log_trace(logger, "Error al conectar con  admin Mem. %s:%d",
				IP_MEMORIA(), PUERTO_MEMORIA());
	} else {
		log_trace(logger, "Conectado con admin Mem. %s:%d",
				IP_MEMORIA(), PUERTO_MEMORIA());
	}

	//envio handshake
	//envio un msj con el id del proceso
	t_msg* msg = string_message(CPU_NUEVO, "Hola soy un CPU", 1, ID());
	if (enviar_mensaje(sock, msg) > 0) {
		log_trace(logger, "Mensaje enviado OK");
	}
	destroy_message(msg);

	//enviar_mensaje_cpu(sock);

	return sock;
}

int conectar_con_planificador(){
	int sock ;
	sock = client_socket(IP_PLANIFICADOR(), PUERTO_PLANIFICADOR());

	if(sock<0){
		log_trace(logger, "Error al conectar con el planificador. %s:%d", IP_PLANIFICADOR(), PUERTO_PLANIFICADOR());
	}else{
		log_trace(logger, "Conectado con planificador. %s:%d", IP_PLANIFICADOR(), PUERTO_PLANIFICADOR());
	}

	//envio handshake
	//envio un msj con el id del proceso
	t_msg* msg = string_message(CPU_NUEVO, "Hola soy un CPU", 1, ID() );
	if (enviar_mensaje(sock, msg)>0){
		log_trace(logger, "Mensaje enviado OK");
	}
	destroy_message(msg);

	//enviar_mensaje_cpu(sock);

	return sock;
}


int inicializar(){

	cfg = config_create(CONFIG_PATH);
	printf("IP planif: %s:%d\n", IP_PLANIFICADOR(), PUERTO_PLANIFICADOR());


	clean_file(LOGGER_PATH);
	logger = log_create(LOGGER_PATH, "procesoCPU", true, LOG_LEVEL_TRACE);

	pthread_mutex_init(&mutex, NULL);

	return 0;
}

int finalizar(){

	config_destroy(cfg);
	log_destroy(logger);
	pthread_mutex_destroy(&mutex);
	return 0;
}
