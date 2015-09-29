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
#include "util.h"


typedef enum {
	PCB_FIN,
		PCB_FINQ,
		PCB_FINALIZAR
} t_msg_id2;

int PID = 1;
int PID_GLOBAL=1;
int IO_GLOBAL=1;
t_log* logger;
char* LOG_PATH = "log.txt";

int inicializar();
int finalizar();
int iniciar_consola();


t_cpu* cpu;
pthread_t th_server_cpu;
pthread_t contador_IO_PCB;
time_t time1;

int main(void) {
	inicializar();
	printf("INICIANDO CONSOLA\n");
	time1=time(NULL);
	//pthread_create(&th_server_cpu, NULL, (void*)iniciar_server_select, NULL);

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

		int consola=0 ;

		FD_ZERO(&master); // borra los conjuntos maestro y temporal
		FD_ZERO(&read_fds);
		FD_SET(fdNuevoNodo, &master);
        FD_SET(consola,&master); // agrego consola stdin
		fdmax = fdNuevoNodo; // por ahora el maximo
		printf("Consola iniciada \n");
		fflush(stdout);
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

						if(socket==consola){

							procesar_msg_consola(msg);

						}
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
	//iniciar_consola();

	//pthread_join(th_server_cpu, NULL);
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


void procesar_msg_consola(t_msg* msg){

	char* path;
	int pid;
	//char* buff  ;
	char comando[COMMAND_MAX_SIZE];
	char** input_user;
//	printf("INICIO CONSOLA\n");
/*
	bool fin = false;
	while (!fin) {
		printf("\nINGRESAR COMANDO: ");
	*/
	//leer_comando_consola(comando);
		//separo todoo en espacios ej: [copiar, archivo1, directorio0]
		input_user = separar_por_espacios(msg);
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
			t_pcb* pcb;
			PID_GLOBAL=pid;
			pcb=list_get(pcbs,pos_del_pcb(pid));
			pcb->pc=pcb->cant_sentencias;
			break;
		case PS:
			printf("PS listar procesos\n");
			int i=0;
			t_pcb* pcb2;
			while ((i+1)<= list_size(pcbs)){

				pcb2=list_get(pcbs,i);

				//log_info(log_pantalla,"mProc	%d PID:	%s nombre	->	%d estado /n",pcb->pid,pcb->path,pcb->estado);

				printf("mProc	 PID: %d	 nombre %s	->	 estado %d\n",pcb2->pid,pcb2->path,pcb2->estado);

				i++;
			}
			break;
		case CPU:
			i=0;
			printf("Uso CPU en el ultimo min \n");

			int uso;
			int uso_rodondeado;
			if(list_size(cpus)>0){
				while((i+1)<=list_size(cpus)){

					cpu=cpu_buscar(i);
			//	cpu=(t_cpu)(list_get(cpus,i));

				int tiempoUsado= cpu->usoUltimoMinuto;
					uso=60/tiempoUsado;
					uso_rodondeado=round_2(uso,0);
					printf("Cpu %d: %d",cpu->id,uso_rodondeado);
					i++;
				}
				}else
				{
					printf("No hay CPUs activas por el momento");
			}
			break;
	/*	case SALIR:			//exit
			fin = true;
			break;*/
		default:
			printf("Comando desconocido\n");
			break;
		}
		free_split(input_user);
	}

int correr_proceso(char* path){
	t_pcb* pcb = NULL;
	pcb = pcb_nuevo(path);
	pcb->pid = get_nuevo_pid();

	pcb->cant_sentencias = get_cant_sent(path);
	pcb->cant_a_ejectuar = get_cant_sent(path); // en caso de que se RR es el Q
	pcb->estado=NEW;

	pcb_agregar(pcb);

	t_ready* new=malloc(sizeof(t_ready));

	new->pid=pcb->pid;

	list_add(list_ready,new);

	t_cpu* cpu = NULL;
	if(cpu_disponible()){
		cpu = cpu_seleccionar();
		pcb->cpu_asignado = cpu->id;
		cpu_ejecutar(cpu, pcb);
	}

	return 0;
}
/*
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
			t_pcb* pcb;
			PID_GLOBAL=pid;
			pcb=list_get(pcbs,pos_del_pcb(pid));
			pcb->pc=pcb->cant_sentencias;
			break;
		case PS:
			printf("PS listar procesos\n");
			int i=0;
			t_pcb* pcb;
			while ((i+1)<= list_size(pcbs)){

				pcb=list_get(pcbs,i);

				//log_info(log_pantalla,"mProc	%d PID:	%s nombre	->	%d estado /n",pcb->pid,pcb->path,pcb->estado);

				printf("mProc	 PID: %d	 nombre %s	->	 estado %d\n",pcb->pid,pcb->path,pcb->estado);

				i++;
			}
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
}*/

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
	t_pcb* pcb = NULL;
	char* pid_string;
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

		case PCB_IO:

			pcb = es_el_pcb_buscado_por_id(msg->argv[0]);

			PID_GLOBAL=pcb->pid;
			IO_GLOBAL=msg->argv[1];

			if(list_any_satisfy(list_exec, (void*) es_el_pcb_buscado)){

			list_remove_by_condition(list_exec, (void*) es_el_pcb_buscado_en_exec);

			t_block* block;

			block->pid=PID_GLOBAL;

			list_add(list_block,block);

			pid_string=string_itoa(PID_GLOBAL);

			pthread_create(&contador_IO_PCB, NULL, (void*)controlar_IO, (void*) pid_string);
			}

			if ((list_size(list_ready))!=0){
				if(cpu_disponible()){
						cpu = cpu_seleccionar();
						pcb->cpu_asignado = cpu->id;
						cpu_ejecutar(cpu, pcb);
					}


			}

			break;

		case PCB_FINQ:
 //VUELVE EN EL FIN DEL QUANTUM

			pcb = es_el_pcb_buscado_por_id(msg->argv[0]);

			PID_GLOBAL=pcb->pid;


			if(list_any_satisfy(list_exec, (void*) es_el_pcb_buscado)){

			list_remove_by_condition(list_exec, (void*) es_el_pcb_buscado_en_exec);

			}
			t_ready* ready;

			ready->pid=PID_GLOBAL;

			list_add(list_ready,ready);

			if(cpu_disponible()){
									cpu = cpu_seleccionar();
									pcb->cpu_asignado = cpu->id;
									cpu_ejecutar(cpu, pcb);
								}

			break;

		case PCB_FINALIZAR:

			pcb = es_el_pcb_buscado_por_id(msg->argv[0]);

			PID_GLOBAL=pcb->pid;
			/*
			Tiempo de retorno: tiempo transcurrido entre la llegada de
			un proceso y su finalización.
		     Tiempo de espera: tiempo que un proceso permanece en la
			cola de preparados.
		    Tiempo de respuesta: tiempo que un proceso bloqueado
			tarda en entrar en la CPU desde que ocurre el suceso que lo
			bloquea.
                    */

			if(list_any_satisfy(list_exec, (void*) es_el_pcb_buscado)){

			list_remove_by_condition(list_exec, (void*) es_el_pcb_buscado_en_exec);

			}
			t_finish finish;
			t_pcb_finalizado pcb;

			finish.pid=PID_GLOBAL;
			list_add(list_finish,finish);

          pcb.tiempo_total=difftime(time(NULL),time1);

			printf("Hay que finalizar el proceso");

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

	list_block=list_create();

	list_finish=list_create();

	list_exec=list_create();

	return 0;
}

void controlar_IO (char* pid_string){

	t_block* block;

	block=list_get(list_block, es_el_pid_en_block(atoi(pid_string),list_block));

	sleep(IO_GLOBAL);

	PID_GLOBAL=atoi(pid_string);

	list_remove_by_condition(list_block, (void*) es_el_pcb_buscado_en_block);

	t_ready* ready;

	ready->pid=block->pid;

	list_add(list_ready,ready);

	pthread_exit(NULL);

}

t_pcb* es_el_pcb_buscado_por_id(int pid){

	int i=0;
	t_pcb* pcb;

	pcb=list_get(pcbs,i);

	while((i+1)<=list_size(pcbs)){
	if(pcb->pid==pid){
		break;}
	else{	}
		i++;
}

	return pcb;
}

t_pcb* es_el_pcb_buscado(){

	int i=0;
	t_pcb* pcb;

	pcb=list_get(pcbs,i);

	while((i+1)<=list_size(pcbs)){

	if(pcb->pid==PID_GLOBAL){
		break;}
		else{
		i++;
		}
}

	return pcb;
}

int pos_del_pcb(int pid){

	int i=0;
	t_pcb* pcb;

	pcb=list_get(pcbs,i);

	while((i+1)<=list_size(pcbs)){

	if(pcb->pid==PID_GLOBAL){
		break;}
		else{
		i++;
		}
}

	return i;
}

int es_el_pid_en_block(int pid, t_list* list_block){

	int i=0;

	t_block* block;

	block=list_get(list_block,i);

	while((i+1)<=list_size(list_block)){

	if (block->pid==pid) {
	break;
	}else{
		i++;
	}

}
	return i;
}

int es_el_pcb_buscado_en_exec(t_exec* exec){
	return(exec->pid==PID_GLOBAL);
}

int es_el_pcb_buscado_en_block(t_block* block){
	return(block->pid==PID_GLOBAL);
}

void cambiar_a_exec (int pid){

	PID_GLOBAL=pid;

	list_remove_by_condition(list_ready, (void*) es_el_pcb_buscado_en_ready);

	t_exec* exec;

	exec->pid=pid;

	list_add(list_exec,exec);

	return;

}

int es_el_pcb_buscado_en_ready(t_ready* ready){
	return(ready->pid==PID_GLOBAL);
}

double round_2(double X, int k)
{

	return floor( pow(10,k)*X + 0.5) / pow(10,k) ;

}

