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

pthread_t th_server_cpu;


int main(void) {
	inicializar();

	pthread_create(&th_server_cpu, NULL, (void*)iniciar_server_select, NULL);


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
		pcb->cpu_asignado = cpu->id;
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

int iniciar_server_select(){
	t_cpu* cpu = NULL;
	t_pcb* pcb = NULL;
	int port = PUERTO_ESCUCHA();
	////////////////
	fd_set master, read_fds;
	int fdNuevoNodo, fdmax, newfd;
	int socket;

	if ((fdNuevoNodo = server_socket(port)) < 0) {
		handle_error("No se pudo iniciar el server");
	}
	printf("server iniciado en %d\n", port);

	FD_ZERO(&master); // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);
	FD_SET(fdNuevoNodo, &master);

	fdmax = fdNuevoNodo; // por ahora el maximo

	//log_info(logger, "inicio thread eschca de nuevos nodos");
	// bucle principal
	for (;;) {
		read_fds = master; // cópialo
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			handle_error("Error en select");
		}

		// explorar conexiones existentes en busca de datos que leer
		for (socket = 0; socket <= fdmax; socket++) {
			if (FD_ISSET(socket, &read_fds)) { // ¡¡tenemos datos!!
				if (socket == fdNuevoNodo) {	// gestionar nuevas conexiones
					//char * ip;
					//newfd = accept_connection_and_get_ip(fdNuevoNodo, &ip);
					newfd = accept_connection(fdNuevoNodo);
					if (newfd < 0) {
						printf("no acepta mas conexiones\n");
					} else {
						//printf("nueva conexion desde IP: %s\n", ip);
						FD_SET(newfd, &master); // añadir al conjunto maestro
						if (newfd > fdmax) { // actualizar el máximo
							fdmax = newfd;
						}
					}

				} else { // gestionar datos de un cliente ya conectado
					t_msg *msg = recibir_mensaje(socket);
					if (msg == NULL) {
						printf("Conexion cerrada %d\n", socket);
						close(socket);
						FD_CLR(socket, &master);

						//si llega aca se desconecto el cpu

						//busco el cpu por el socket
						cpu = cpu_buscar_por_socket(socket);
						//si el cpu se desconecto habria que sacarlo de la lista de cpus, o marcarlo como desconectado
						//todo: hay que buscar los pcbs que esta procesando este socket, para replanificarlos en otra cpu
						log_trace(logger, "El cpu %d, socket: %d se desconecto", cpu->id, cpu->socket);
						pcb = pcb_buscar_por_cpu(cpu->id);
						if(pcb== NULL){
							log_trace(logger, "Hay que replanificar el pcb %d", pcb->pid );
						}else{
							log_trace(logger, "Ningun pcb a replanificar?");
						}


					} else {
						//print_msg(msg);
						procesar_mensaje_cpu(socket, msg);

					}

				} //fin else procesar mensaje nodo ya conectado
			}
		}
	}
	return 0;
}


int procesar_mensaje_cpu(int socket, t_msg* msg){
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

