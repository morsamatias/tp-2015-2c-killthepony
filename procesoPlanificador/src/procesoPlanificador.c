/*
 ============================================================================
 Name        : procesoPlanificador.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "procesoPlanificador.h"
#include "string.h"
#include <util.h>


int PID = 1;
t_log* logger;
char* LOG_PATH = "log.txt";

int inicializar();
int finalizar();
int iniciar_consola();
int iniciar_server();

pthread_t th_server_cpu;


int main(void) {
	inicializar();

	pthread_create(&th_server_cpu, NULL, (void*)iniciar_server, NULL);


	iniciar_consola();

	pthread_join(th_server_cpu, NULL);
	finalizar();
	return EXIT_SUCCESS;
}

int get_cant_sent(char* path){
	char* mapped = file_get_mapped(path);
	char** sentencias = string_split(mapped, "\n");
	int cant_sent = split_count(sentencias);
	file_mmap_free(mapped, path);
	free_split(sentencias);
	return cant_sent;
}

int get_nuevo_pid(){
	return PID++;
}

int correr_proceso(char* path){
	t_pcb* pcb = NULL;
	pcb = pcb_nuevo(path);
	pcb->pid = get_nuevo_pid();

	pcb->cant_sentencias = get_cant_sent(path);
	pcb->cant_a_ejectuar = get_cant_sent(path); // en caso de que se RR es el Q
	pcb->estado=NEW;

	pcb_agregar(pcb);

	t_new* new=malloc(sizeof(t_new));

	new->pid=pcb->pid;

	list_add(list_new,new);


	t_cpu* cpu = NULL;
	if(cpu_disponible()){
		cpu = cpu_seleccionar();

		cpu_ejecutar(cpu, pcb);
	}

	return 0;
}

int iniciar_consola() {
	char* path;
	int pid;
	//char* buff  ;
	char comando[COMMAND_MAX_SIZE];
	char** input_user;
	printf("INICIO CONSOLA\n");

	bool fin = false;
	while (!fin) {
		printf("\nINGRESAR COMANDO: ");
		leer_comando_consola(comando);
		//separo todoo en espacios ej: [copiar, archivo1, directorio0]
		input_user = separar_por_espacios(comando);
		e_comando cmd = parsear_comando(input_user[0]);

		switch (cmd) {
		case CORRER:
			path = input_user[1];
			printf("Correr path: %s\n", path);
			correr_proceso(path);
			break;
		case FINALIZAR:
			pid = atoi(input_user[1]);
			printf("finalizar pid: %d\n", pid);
			break;
		case PS:
			printf("PS listar procesos\n");
			break;
		case CPU:
			printf("Uso CPU en el ultimo min \n");
			break;
		case SALIR:			//exit
			fin = true;
			break;
		default:
			printf("Comando desconocido\n");
			break;
		}
		free_split(input_user);
	}
	return 0;
}

/*
 * CPUs
 */

int iniciar_server(){

	//printf("Iniciando server\n");
	//server_socket_select(PUERTO_ESCUCHA(), procesar_mensaje);
	int listener = server_socket(PUERTO_ESCUCHA());
	if (listener < 0) {
		printf("ERRROr listener %d\n", PUERTO_ESCUCHA());
		return -1;
	}
	int nuevaConexion;
	while (true) {

		nuevaConexion = accept_connection(listener);
		if (nuevaConexion < 0)
			perror("accept");

		log_info(logger, "Nueva Conexion socket: %d", nuevaConexion);

		procesar_mensaje_cpu(nuevaConexion);
	}

	return 0;
}

int procesar_mensaje_cpu(int socket){
	t_msg* msg = NULL;
	msg = recibir_mensaje(socket);
	//print_msg(msg);
	int id_cpu;
	t_cpu* cpu = NULL;
	switch(msg->header.id){
		case CPU_NUEVO:

			//el ID esta en la pos 0
			id_cpu = msg->argv[0];
			log_trace(logger, "Nuevo CPU id: %d", id_cpu);
			destroy_message(msg);

			//si no existe lo agrego
			if(!cpu_existe(id_cpu)){
				cpu = cpu_nuevo(id_cpu);
				cpu->socket = socket;
				cpu_agregar(cpu);
			}else{
				//si existe, modifico el socket, verificar bien si esta bien porque puedo quedar un socket abierto
				cpu = cpu_buscar(id_cpu);
				cpu->socket = socket;
			}

		break;
	default:
		printf("No msgjjj\n");
		break;
	}

	return 0;
}

int finalizar(){
	config_destroy(cfg);
	printf("Fin OK\n");
	return 0;
}

int inicializar(){

	cfg = config_create(CONFIG_PATH);
	logger = log_create(LOG_PATH, "planificador", true, LOG_LEVEL_TRACE);

	cpus = list_create();

	pcbs = list_create();

	list_ready=list_create();

	list_new=list_create();

	list_block=list_create();

	list_finish=list_create();

	list_exec=list_create();

	return 0;
}

