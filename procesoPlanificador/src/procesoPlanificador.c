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
#include <math.h>
#include <time.h>

int PID = 1;
int PID_GLOBAL = 1;
int PID_GLOBAL_READY = 1;
int PID_GLOBAL_EXEC = 1;
int PID_GLOBAL_BLOCK = 1;
int PID_GLOBAL_FINISH = 1;
int IO_GLOBAL = 1;

#define RR 1;
#define FIFO 0;


t_log* logger;
t_log* log_consola;
t_log* log_listas;
t_log* log_porcentajes;


char* LOG_PATH = "log.txt";

char* LOG_PATH_CONSOLA = "log_consola.txt";

char* LOG_PATH_LISTAS = "log_listas.txt";


char* LOG_PATH_PORCENTAJE = "log_porcentaje.txt";

int inicializar();
int finalizar();
//int iniciar_consola();

t_cpu* cpu;
pthread_t th_server_cpu;
pthread_t contador_IO_PCB;
time_t time1;

int main(void) {

	system("clear");

	inicializar();

	char comando_usuario[COMMAND_MAX_SIZE];
	printf("INICIANDO CONSOLA\n");
	time1 = time(NULL);
	//pthread_create(&th_server_cpu, NULL, (void*)iniciar_server_select, NULL);

	sem_init(&sem_IO, 0, 0);

	pthread_create(&contador_IO_PCB, NULL, (void*) Hilo_IO, (void*) PID_GLOBAL);


	t_cpu* cpu = NULL;
	t_pcb* pcb = NULL;
	int port = PUERTO_ESCUCHA();
	////////////////
	fd_set master, read_fds;
	int fdNuevaCPU, fdmax, newfd;
	int socket;

	if ((fdNuevaCPU = server_socket(port)) < 0) {
		handle_error("No se pudo iniciar el server");
	}
	printf("server iniciado en %d\n", port);

	int consola = 0;

	FD_ZERO(&master); // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);
	FD_SET(fdNuevaCPU, &master);
	FD_SET(consola, &master); // agrego consola stdin
	fdmax = fdNuevaCPU; // por ahora el maximo
	printf("Consola iniciada \n");
	printf("Opciones Disponibles: \n ");
	printf("CORRER Path \n FINALIZAR Pid \n PS \n CPU \n LS \n");
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

				if (socket == consola) {
					leer_comando_consola(comando_usuario);
					procesar_msg_consola(comando_usuario);

				} else {
					if (socket == fdNuevaCPU) { // gestionar nuevas conexiones
						//char * ip;
						//newfd = accept_connection_and_get_ip(fdNuevoNodo, &ip);
						newfd = accept_connection(fdNuevaCPU);
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
							//printf("Conexion cerrada %d\n", socket);

									t_cpu* cpu2;
							//si llega aca se desconecto el cpu

							//busco el cpu por el socket
						cpu2 = cpu_buscar_por_socket(socket);
							//si el cpu se desconecto habria que sacarlo de la lista de cpus, o marcarlo como desconectado
							// hay que buscar los pcbs que esta procesando este socket, para replanificarlos en otra cpu

if (cpu2 != NULL)
{
							log_warning(logger,
									"El cpu con socket: %d ID:%d   se desconecto",
									 socket,cpu2->id);
							log_warning(log_consola,
										"El cpu con socket: %d se desconecto",
											 socket);


							t_pcb* pcb2;

							pcb2 = pcb_buscar_por_cpu(cpu2->id);

							if (pcb2 != NULL) {

								PID_GLOBAL=pcb2->pid;
								log_info(logger,
										" La CPU tenia a cargo los siguentes procesos  %d",
										pcb2->pid);


								//list_remove(pcbs,pcb2->pid);
								//no se replanifica nada
								//por eso no se tienen en cuenta esto
								//log_trace(logger, "Ningun pcb a replanificar");
								gl_pcb=pcb2->pid;
								/*if (list_any_satisfy(pcbs, (void*) _es_pcb_buscando_por_id)) {
									list_remove_by_condition(pcbs,
											(void*)_es_pcb_buscando_por_id);
								}*/
								switch(pcb2->estado){

								case 3:

									PID_GLOBAL_EXEC=pcb2->pid;


						list_remove_by_condition(list_exec, (void*) es_el_pcb_buscado_en_exec);
														pcb2->estado=5;

														break;

								}

								list_remove_by_condition(pcbs,(void*)es_el_pcb_buscado);
							}
							else{
								log_info(logger,
												" La CPU no tenia a cargo ningun proceso");
							}
}
								FD_CLR(socket, &master);
								close(socket);

								list_remove_by_condition(cpus, (void*) _cpu_buscar_por_socket);

						} else {
							//print_msg(msg);
							procesar_mensaje_cpu(socket, msg);

						}

					} //fin else procesar mensaje nodo ya conectado
				} //fin else esConsola

			} //fin if ISSET

		} //fin for

	}
	//iniciar_consola();

	//pthread_join(th_server_cpu, NULL);
	finalizar();
	return EXIT_SUCCESS;
}

int get_cant_sent(char* path) {
	char* mapped = file_get_mapped(path);
	char** sentencias = string_split(mapped, "\n");
	int cant_sent = split_count(sentencias);
	file_mmap_free(mapped, path);
	free_split(sentencias);
	return cant_sent;
}

int get_nuevo_pid() {
	return PID++;
}


 int mostrar_contenido_listas(){

	int i;

	t_ready* pcb_ready;

	t_block* pcb_block;

	t_finish* pcb_finish;

	t_exec* pcb_exec;

	log_info(logger,"LISTA DE READY ");

	log_info(log_listas,"LISTA DE READY ");

	if(list_size(list_ready)==0){

		log_info(logger,"Lista Vacia\n");


			log_info(log_listas,"Lista Vacia \n ");

	}


	for (i = 0; i < list_size(list_ready); i++) {

		pcb_ready=list_get(list_ready,i);


		if (i==(list_size(list_ready)-1)){

	log_info(logger,"Proceso %d\n ",pcb_ready->pid);

	log_info(log_listas,"Proceso %d\n ",pcb_ready->pid);

	}else{


		log_info(logger,"Proceso %d ",pcb_ready->pid);

		log_info(log_listas,"Proceso %d",pcb_ready->pid);}
	}

	log_info(logger,"LISTA DE BLOQUEADOS ");

	log_info(log_listas,"LISTA DE BLOQUEADOS ");

	if(list_size(list_block)==0){

		log_info(logger,"Lista Vacia\n");


			log_info(log_listas,"Lista Vacia \n ");

	}


	for (i = 0; i < list_size(list_block); i++) {


				pcb_block=list_get(list_block,i);

				if (i==(list_size(list_block)-1)){

					log_info(logger,"Proceso %d\n ",pcb_block->pid);

					log_info(log_listas,"Proceso %d\n ",pcb_block->pid);

					}else{


						log_info(logger,"Proceso %d ",pcb_block->pid);

						log_info(log_listas,"Proceso %d",pcb_block->pid);}


	}

	log_info(logger,"LISTA DE EJECUCION  ");

	log_info(log_listas,"LISTA DE EJECUCION  ");

	if(list_size(list_exec)==0){

		log_info(logger,"Lista Vacia\n");


			log_info(log_listas,"Lista Vacia \n ");

	}


	for (i = 0; i < list_size(list_exec); i++) {


		pcb_exec=list_get(list_exec,i);

		if (i==(list_size(list_exec)-1)){

		log_info(logger,"Proceso %d\n",pcb_exec->pid);

		log_info(log_listas,"Proceso %d \n",pcb_exec->pid);

		}else{

			log_info(logger,"Proceso %d",pcb_exec->pid);

					log_info(log_listas,"Proceso %d ",pcb_exec->pid);


		}
		}

	log_info(logger,"LISTA DE FINALIZADOS ");


	log_info(log_listas,"LISTA DE FINALIZADOS ");

	if(list_size(list_finish)==0){

		log_info(logger,"Lista Vacia\n");


			log_info(log_listas,"Lista Vacia \n ");

	}

	for (i = 0; i < list_size(list_finish); i++) {



		pcb_finish=list_get(list_finish,i);


		if (i==(list_size(list_finish)-1)){

		log_info(logger,"Proceso %d \n ",pcb_finish->pid);

		log_info(log_listas,"Proceso %d \n",pcb_finish->pid);

		}else{
			log_info(logger,"Proceso %d ",pcb_finish->pid);

					log_info(log_listas,"Proceso %d ",pcb_finish->pid);

		}
		}
	return 0;

}
void procesar_msg_consola(char* msg) {

	char* path;
	char* path_completo = (char*) malloc(sizeof(char) * 1000);
	int pid;
	//char* buff  ;
	//char comando[COMMAND_MAX_SIZE];
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
	int espacio = encontrar_espacio(msg);
	path = string_substring_from(msg, (espacio + 1));
	switch (cmd) {
	case CORRER:

		path = input_user[1];
		/*if(file_exists(path)){
		 correr_proceso(path); }
		 else{
		 printf("mcod no existente, por favor intente ejecutar otro mcod\n");
		 }*/
		printf("Correr path: %s\n", path);
		//	memset(path_completo, 0, 1000);
		strcpy(path_completo,
				"/home/utnso/Escritorio/git/tp-2015-2c-killthepony/tests/");

		strcat(path_completo, path);

		log_trace(logger, "Correr path completo: %s\n", path_completo);

		log_info(log_consola, "Correr path completo: %s\n", path_completo);

		if (file_exists(path_completo)) {

			correr_proceso(path_completo);
		} else {

			log_trace(logger, "Mcod no existente\n");
			log_trace(log_consola, "Mcod no existente\n");
		}

		break;

	case FINALIZAR:

		pid = atoi(input_user[1]);

		log_info(logger,
				"Finalizar proceso cuyo pid es: %d\n", pid);

		log_info(log_consola,
						"Finalizar proceso cuyo pid es: %d\n", pid);

		t_pcb* pcb;

		PID_GLOBAL = pid;

		if (list_any_satisfy(pcbs, (void*) es_el_pcb_buscado)) {
			PID_GLOBAL = pid;
			pcb = list_get(pcbs,
					pos_del_pcb(pid));

			pcb->pc = pcb->cant_sentencias - 1;

			//log_info(logger,"pcb->pc: %d",pcb->pc);

			//log_info(log_consola,"pcb->pc: %d",pcb->pc);

		} else {
			log_error(logger,
					"No existe un proceso con el PID ingresado");
			log_error(log_consola,
								"No existe un proceso con el PID ingresado");
		}
		break;
	case PS:

		log_info(logger,
				"PS listar estados de los procesos\n");

		log_info(log_consola,
				"PS listar estados de los procesos\n");

		int i = 0;

		t_pcb* pcb2;

		while ((i + 1) <= list_size(pcbs)) {

			pcb2 = list_get(pcbs, i);

			//log_info(log_pantalla,"mProc	%d PID:	%s nombre	->	%d estado /n",pcb->pid,pcb->path,pcb->estado);

			switch (pcb2->estado) {

			case 0:
/*
				log_trace(logger, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
						pcb2->pid, pcb2->path, "NEW");
*/
				log_info(logger, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
						pcb2->pid, pcb2->path, "NEW");

				log_info(log_consola, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
						pcb2->pid, pcb2->path, "NEW");

				break;

			case 1:

				log_info(logger, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
						pcb2->pid, pcb2->path, "READY");


				log_info(log_consola, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
						pcb2->pid, pcb2->path, "READY");

				break;

			case 2:

				log_info(logger, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
						pcb2->pid, pcb2->path, "BLOCK");


				log_info(log_consola, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
						pcb2->pid, pcb2->path, "BLOCK");

				break;

			case 3:

				log_info(logger, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
						pcb2->pid, pcb2->path, "EXEC");


				log_info(log_consola, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
						pcb2->pid, pcb2->path, "EXEC");

				break;

			case 4:

				log_info(logger, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
						pcb2->pid, pcb2->path, "FINISH");


				log_info(log_consola, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
						pcb2->pid, pcb2->path, "FINISH");

				break;
			case 5:
				log_info(logger, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
									pcb2->pid, pcb2->path, "FINISH CON ERROR POR CAIDA DE CPU");


				log_info(log_consola, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
									pcb2->pid, pcb2->path, "FINISH CON ERROR POR CAIDA DE CPU");
				break;

			case 6:
				log_info(logger, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
									pcb2->pid, pcb2->path, "PROCESO ABORTADO POR FALLO AL INICIAR");


				log_info(log_consola, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
									pcb2->pid, pcb2->path, "PROCESO ABORTADO POR FALLO AL INICIAR");
				break;


			default:

				log_error(logger,
						"No se puede determinar el estado del proceso %d",
						pcb2->pid);

				log_error(log_consola,
						"No se puede determinar el estado del proceso %d",
						pcb2->pid);

				break;

			}

			i++;
		}
		break;

	case LS:


		log_info(logger,"Estado de las Listas: \n");

		mostrar_contenido_listas();

		break;

	case CPU:
		i = 0;
		log_info(logger,
				"Porcentaje de Usos de las CPUs en el último minuto \n");


		log_info(log_porcentajes,
				"Porcentaje de Usos de las CPUs en el último minuto \n");

		//	int uso;
		//	int uso_rodondeado;

		/*

		 if (list_size(cpus) > 0) {
		 while ((i + 1) <= list_size(cpus)) {

		 cpu = cpu_buscar(i);

		 if(cpu!=NULL){

		 t_msg* pedido_uso = argv_message(CPU_PORCENTAJE_UTILIZACION,0);

		 enviar_mensaje(cpu->socket,pedido_uso);

		 destroy_message(pedido_uso);}

		 //	cpu=(t_cpu)(list_get(cpus,i));

		 //int tiempoUsado = cpu->usoUltimoMinuto;
		 //uso = 60 / tiempoUsado;
		 //uso_rodondeado = round_2(uso, 0);
		 //printf("Cpu %d: %d", cpu->id, cpu->usoUltimoMinuto);
		 i++;
		 }

		 } else {
		 printf("No hay CPUs activas por el momento");
		 }


		//log_trace(logger,"Operaciones realizadas por el proceso %d hasta el momento son:", pcb->pid);



		 log_trace(logger,"Operaciones realizadas por el proceso %d hasta el momento son:", pcb->pid);

		 m=1;

		 while(msg->argv[m]!='\0'){

		 switch (msg->argv[m]) {

		 case iniciar:

		 log_trace(logger,"	mProc	%d	-	Iniciado.", pcb->pid);

		 m++;

		 break;


		 case leer:

		 log_trace(logger,"mProc	%d	-	Pagina	%d	leida:", pcb->pid,msg->argv[m+1]);

		 m++;

		 m++;

		 break;

		 case escribir:

		 log_trace(logger,"mProc	%d	-	Pagina	%d	escrita:", pcb->pid,msg->argv[m+1]);

		 m++;

		 m++;

		 break;

		 case io:

		 log_trace(logger,"mProc	%d	en	entrada-salida	de	tiempo	%d.", pcb->pid,msg->argv[1]);

		 m++;

		 break;

		 case final:

		 log_trace(logger,"mProc	%d	Finalizado.", pcb->pid);

		 m++;

		 break;

		 case error:

		 log_trace(logger,"mProc	%d	-	Fallo.", pcb->pid);

		 m++;

		 break;

		 default:

		 log_trace(logger,"No se comprende el mensaje enviado");

		 break;

		 }

		 }

		 */



		t_msg* pedido_uso = argv_message(CPU_PORCENTAJE_UTILIZACION, 0);

		enviar_mensaje(cpu_especial, pedido_uso);

		destroy_message(pedido_uso);
		t_msg* msg_porcent;
		int cpu;
		int porcentaje;
		//leo los X mensajes que me envia
		for (i = 0; i < list_size(cpus); i++) {
			msg_porcent = recibir_mensaje(cpu_especial);
			cpu = msg_porcent->argv[0];
			porcentaje = msg_porcent->argv[1];
			destroy_message(msg_porcent);
			mostrar_porcentaje(cpu, porcentaje);
		}

		break;

		/*	case SALIR:			//exit
		 fin = true;
		 break;*/
	default:
		log_error(logger, "Comando desconocido\n");
		break;
	}
	free_split(input_user);
}

void mostrar_porcentaje(int cpu_id, int porcentaje) {
	//t_cpu* cpu = cpu_buscar(cpu_id);
	//cpu->usoUltimoMinuto = porcentaje;
	log_info(logger,
			"Porcentaje de Uso de la Cpu %d: %d", cpu_id, porcentaje);
	log_info(log_porcentajes,
			"Porcentaje de Uso de la Cpu %d: %d", cpu_id, porcentaje);

}

int correr_proceso(char* path) {
	t_pcb* pcb = NULL;
	int quantum;

	pcb = pcb_nuevo(path);

	pcb->pid = get_nuevo_pid();

	log_info(logger,
			"Pid asignado: %d", pcb->pid);

	log_info(log_consola,
			"Pid asignado: %d", pcb->pid);

	pcb->cant_sentencias = get_cant_sent(path);

	//char* algoritmo = ALGORITMO_PLANIFICACION();

	if (string_equals_ignore_case(ALGORITMO_PLANIFICACION(), "FIFO")) {

		log_info(logger,
				"El algoritmo de planificación será FIFO");

		log_info(log_consola,
				"El algoritmo de planificación será FIFO");

		pcb->cant_a_ejectuar = get_cant_sent(path);
	} else {

		log_info(logger,
				"El algoritmo de planificación será Round Robin");

		log_info(log_consola,
				"El algoritmo de planificación será Round Robin");

		if(string_equals_ignore_case(QUANTUM(), "N/A")){

			pcb->cant_a_ejectuar = 2;

			log_info(logger,
							"El Quantum por defecto será 2 instrucciones");

					log_info(log_consola,
							"El Quantum por defecto será 2 instrucciones");

		}else{

		quantum=atoi(QUANTUM());

		pcb->cant_a_ejectuar = quantum;

		log_info(logger,"El Quantum será de %d instrucciones", pcb->cant_a_ejectuar);

		log_info(log_consola,"El Quantum será de %d instrucciones", pcb->cant_a_ejectuar);

		// en caso de que se RR es el Q
	}}

	log_info(logger,
			"Proceso mProc a ejecutar: %s", pcb->path);

	log_info(log_consola,
			"Proceso mProc a ejecutar: %s", pcb->path);

	pcb->estado = NEW
	;

	pcb->tiempo_inicio_proceso = time(NULL);

	//log_trace(logger, "Tiempo de inicio del proceso %d es %ld", pcb->pid,pcb->tiempo_inicio_proceso);

	pcb->cantidad_IO = 0;

	pcb->tiempo_espera = 0;

	log_info(logger,
			"El proceso %d se encuentra en la cola de procesos Nuevos para ejecutar el programa Mcod %s",
			pcb->pid, pcb->path);

	log_info(log_listas,
			"El proceso %d se encuentra en la cola de procesos Nuevos para ejecutar el programa Mcod %s",
			pcb->pid, pcb->path);

	pcb->pc=0;

	pcb_agregar(pcb);

	t_ready* new = malloc(sizeof(t_ready));

	new->pid = pcb->pid;

	list_add(list_ready, new);

	pcb->estado = READY
	;

	pcb->tiempo_inicio_ready = time(NULL);
t_cpu* cpu = NULL;


	log_info(logger,"El proceso %d se encuentra en la cola de procesos Listos",pcb->pid);

	log_info(log_listas,"El proceso %d se encuentra en la cola de procesos Listos",pcb->pid);



	fflush(stdout);

	if (cpu_disponible()) {
		cpu = cpu_seleccionar();


		if (cpu != NULL) {

			pcb->cpu_asignado = cpu->id;

			cpu_ejecutar(cpu, pcb);


			cpu->estado = 0;
		} else {
		/*	printf(
					"No existe CPU activa para asignar al proceso %d. El proceso queda en READY",
					pcb->pid);*/

		log_info(logger,
				"No existe una CPU libre para asignar al proceso %d. El proceso queda en READY",pcb->pid);


		log_info(log_listas,
						"No existe una CPU libre para asignar al proceso %d. El proceso queda en READY",pcb->pid);
				}
	}
	else {


		log_info(logger,
						"No existe una CPU libre para asignar al proceso %d. El proceso queda en READY",pcb->pid);
		log_info(log_listas,
						"No existe una CPU libre para asignar al proceso %d. El proceso queda en READY",pcb->pid);


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

//MEnsajes de la cpu
/*iniciar,
 leer,
 escribir,
 io,
 final,
 error*/

int procesar_mensaje_cpu(int socket, t_msg* msg) {
	//print_msg(msg);
	int id_cpu;
	t_cpu* cpu = NULL;
	t_pcb* pcb = NULL;
	//char* pid_string;
	int uso_cpu;
	int pagina;
	int segundos;
	int n;
	int m;
	int i;
	t_msg* msge;
	int quantum;

	switch (msg->header.id) {
	case CPU_NUEVO:
		//el ID esta en la pos 0
		id_cpu = msg->argv[0];
		log_info(logger,
				"Nuevo CPU id: %d", id_cpu);

		log_info(log_consola,
				"Nuevo CPU id: %d", id_cpu);


		destroy_message(msg);

		//si no existe lo agrego
		if (!cpu_existe(id_cpu)) {

			cpu = cpu_nuevo(id_cpu);

			cpu->socket = socket;

			cpu->estado = 1;

			cpu_agregar(cpu);
		} else {
			//si existe, modifico el socket, verificar bien si esta bien porque puedo quedar un socket abierto
			cpu = cpu_buscar(id_cpu);
			cpu->socket = socket;
		}

		if (list_size(list_ready) > 0) {
			if (cpu_disponible()) {
				cpu = cpu_seleccionar();
				if (cpu != NULL) {
					t_ready* ready = list_get(list_ready, 0);
					t_pcb* pcb2;
					pcb2 = es_el_pcb_buscado_por_id(ready->pid);

					pcb2->cpu_asignado = cpu->id;
					cpu_ejecutar(cpu, pcb2);
					cpu->estado = 0;
				} else {
				/*	printf(
							"No existe CPU activa para asignar al proceso %d. El proceso queda en READY",
							pcb->pid);*/
					log_info(logger,
							"No existe una CPU libre para asignar al proceso %d. El proceso queda en READY",pcb->pid);

					log_info(log_consola,
						"No existe una CPU libre para asignar al proceso %d. El proceso queda en READY",pcb->pid);

				}
				}
			else {
				//printf("No existe CPU activa para asignarle un nuevo proceso");
				log_info(logger,
						"No existe una CPU libre para asignarle un nuevo proceso"	);

			log_info(log_consola,
					"No existe una CPU libre para asignarle un nuevo proceso"	);
									}

		}


		break;

	case PCB_IO:

		pcb = es_el_pcb_buscado_por_id(msg->argv[0]);

		log_info(logger, "El proceso %d realiza una Entrada/Salida", pcb->pid);

		log_info(log_consola, "El proceso %d realiza una Entrada/Salida", pcb->pid);

		pcb->cantidad_IO = pcb->cantidad_IO + 1;

		if (pcb->cantidad_IO == 1) {

			pcb->tiempo_entrada_salida = time(NULL);

			//log_trace(logger,
			//	"Tiempo en el que el proceso %d inicia la Entrada-Salida es %ld",
			//pcb->pid, pcb->tiempo_entrada_salida);

			pcb->tiempo_respuesta = difftime(pcb->tiempo_entrada_salida,
					pcb->tiempo_inicio_proceso);

		}


		IO_GLOBAL = msg->argv[2];

		int cantIO = msg->argv[2];

		PID_GLOBAL_EXEC = pcb->pid;

		if (list_any_satisfy(list_exec, (void*) es_el_pcb_buscado_en_exec)) {
			list_remove_by_condition(list_exec,
					(void*) es_el_pcb_buscado_en_exec);
		}
		t_block* blocked = malloc(sizeof(t_block));

		blocked->pid = msg->argv[0];

		blocked->tiempoIO = cantIO;
		blocked->estado = 0;

		list_add(list_block, blocked);

		pcb->estado = BLOCK
		;

		log_info(logger,
				"El proceso %d se encuentra en la cola de procesos en Bloqueados",
				pcb->pid);

		log_info(log_listas,
				"El proceso %d se encuentra en la cola de procesos en Bloqueados",
				pcb->pid);



		sem_post(&sem_IO);


		if((pcb->pc + 1)!=pcb->cant_sentencias){

		pcb->pc = pcb->pc + msg->argv[3];

		}



		cpu = cpu_buscar_por_socket(socket);

		cpu->estado = 1;

		//log_trace(logger,"Operaciones realizadas por el proceso %d hasta el momento son:", pcb->pid);

		/*

		 log_trace(logger,"Operaciones realizadas por el proceso %d hasta el momento son:", pcb->pid);

		 int m=2;

		 while(msg->argv[m]!='\0'){

		 switch (msg->argv[m]) {

		 case iniciar:

		 log_trace(logger,"	mProc	%d	-	Iniciado.", pcb->pid);

		 m++;

		 break;


		 case leer:

		 log_trace(logger,"mProc	%d	-	Pagina	%d	leida:", pcb->pid,msg->argv[m+1]);

		 m++;

		 m++;

		 break;

		 case escribir:

		 log_trace(logger,"mProc	%d	-	Pagina	%d	escrita:", pcb->pid,msg->argv[m+1]);

		 m++;

		 m++;

		 break;

		 case io:

		 log_trace(logger,"mProc	%d	en	entrada-salida	de	tiempo	%d.", pcb->pid,msg->argv[1]);

		 m++;

		 break;

		 case final:

		 log_trace(logger,"mProc	%d	Finalizado.", pcb->pid);

		 m++;

		 break;

		 case error:

		 log_trace(logger,"mProc	%d	-	Fallo.", pcb->pid);

		 m++;

		 break;

		 default:

		 log_trace(logger,"No se comprende el mensaje enviado");

		 break;

		 }

		 }

		 */

		t_msg* msge;

		int i;

		log_info(logger,
					"Operaciones realizadas por el proceso %d hasta el momento son:\n",
					pcb->pid);

		log_info(log_consola,
					"Operaciones realizadas por el proceso %d hasta el momento son:\n",
					pcb->pid);

		for (i=0;i<msg->argv[3];i++){

		msge=recibir_mensaje(socket);
		logueo (msge);
		destroy_message(msge);

		}

		if (list_size(list_ready) > 0) {
			if (cpu_disponible()) {
				cpu = cpu_seleccionar();
				if (cpu != NULL) {
					t_ready* ready = list_get(list_ready, 0);
					t_pcb* pcb2;
					pcb2 = es_el_pcb_buscado_por_id(ready->pid);

					pcb2->cpu_asignado = cpu->id;
					cpu_ejecutar(cpu, pcb2);
					cpu->estado = 0;
				} else {

					/*printf(
							"No existe CPU activa para asignar al proceso %d. El proceso queda en READY",
							pcb->pid);*/
					log_info(logger,
							"No existe una CPU libre para asignar al proceso %d. El proceso queda en READY",
														pcb->pid);
					log_info(log_consola,
								"No existe una CPU libre para asignar al proceso %d. El proceso queda en READY",
																			pcb->pid);
				}
			} else {

				//printf("No existe CPU activa para asignarle un nuevo proceso");
			log_info(logger,"No existe una CPU libre para asignarle un nuevo proceso");

			log_info(log_consola,"No existe una CPU libre para asignarle un nuevo proceso");

			}

		}

		destroy_message(msg);

		break;

	case PCB_FINALIZAR:

		pcb = es_el_pcb_buscado_por_id(msg->argv[0]);

		log_info(logger, "Se comienza a Finalizar el proceso %d \n", pcb->pid);


		log_info(log_consola, "Se comienza a Finalizar el proceso %d \n", pcb->pid);

		printf("pcb->pid %d\n", msg->argv[0]);

		PID_GLOBAL_FINISH = pcb->pid;

		PID_GLOBAL_EXEC = pcb->pid;
		/*
		 Tiempo de retorno: tiempo transcurrido entre la llegada de
		 un proceso y su finalización.
		 Tiempo de espera: tiempo que un proceso permanece en la
		 cola de preparados.
		 Tiempo de respuesta: tiempo que un proceso bloqueado
		 tarda en entrar en la CPU desde que ocurre el suceso que lo
		 bloquea.
		 */

		if (list_any_satisfy(list_exec, (void*) es_el_pcb_buscado_en_exec)) {

			list_remove_by_condition(list_exec,
					(void*) es_el_pcb_buscado_en_exec);

		}
		t_finish* finish = malloc(sizeof(t_finish));

		//t_pcb_finalizado* pcb2;

		finish->pid = PID_GLOBAL_FINISH;
		list_add(list_finish, finish);

		//pcb2->tiempo_total = difftime(time(NULL), time1);
		pcb->estado = FINISH
		;

		pcb->tiempo_fin_proceso = time(NULL);

		//log_trace(logger,
		//	"Tiempo en el que el proceso %d finaliza operatoria es %ld",
		//pcb->pid, pcb->tiempo_fin_proceso);

		pcb->tiempo_retorno = difftime(pcb->tiempo_fin_proceso,
				pcb->tiempo_inicio_proceso);

		if (pcb->cantidad_IO == 0) {

			pcb->tiempo_entrada_salida = time(NULL);

			pcb->tiempo_respuesta = difftime(pcb->tiempo_fin_proceso,
					pcb->tiempo_inicio_proceso);

		}

		cpu = cpu_buscar_por_socket(socket);

		cpu->estado = 1;

		log_info(logger,
				"El proceso %d se encuentra en la cola de procesos Finalizados",
				pcb->pid);


		log_info(log_listas,
				"El proceso %d se encuentra en la cola de procesos Finalizados",
				pcb->pid);

		log_info(logger,
				"Métricas del Proceso %d \n El Tiempo de Retorno fue de %d segundos \n El Tiempo de Respuesta fue de %d segundos \n El Tiempo de Espera fue de %d segundos\n",
				pcb->pid, pcb->tiempo_retorno, pcb->tiempo_respuesta,
				pcb->tiempo_espera);


		log_info(log_porcentajes,
				"Métricas del Proceso %d \n El Tiempo de Retorno fue de %d segundos \n El Tiempo de Respuesta fue de %d segundos \n El Tiempo de Espera fue de %d segundos\n",
				pcb->pid, pcb->tiempo_retorno, pcb->tiempo_respuesta,
				pcb->tiempo_espera);

		//printf("Hay que finalizar el proceso");

		log_info(logger,
					"Operaciones realizadas por el proceso %d hasta el momento son:\n",
					pcb->pid);

		log_info(log_consola,
					"Operaciones realizadas por el proceso %d hasta el momento son:\n",
					pcb->pid);


		for (i=0;i<msg->argv[3];i++){

		msge=recibir_mensaje(socket);
		logueo (msge);
		destroy_message(msge);

		}

		// Hay que ver si hay algún proceso en READY para ejecutar

		if (list_size(list_ready) > 0) {
			if (cpu_disponible()) {
				cpu = cpu_seleccionar();
				if (cpu != NULL) {
					t_ready* ready = malloc(sizeof(ready));

					ready = list_get(list_ready, 0);
					t_pcb* pcb2;
					pcb2 = es_el_pcb_buscado_por_id(ready->pid);

					pcb2->cpu_asignado = cpu->id;
					cpu_ejecutar(cpu, pcb2);
					cpu->estado = 0;
				} else {

					/*printf(
							"No existe CPU activa para asignar al proceso %d. El proceso queda en READY\n",
							pcb->pid);*/
					log_info(logger,"No existe una CPU libre para asignar al proceso %d. El proceso queda en READY\n",
							pcb->pid);
					log_info(log_consola,"No existe una CPU libre para asignar al proceso %d. El proceso queda en READY\n",
												pcb->pid);
				}
			} else {

			//	printf(
				//		"No existe CPU activa para asignarle un nuevo proceso\n");

			log_info(logger,"No existe una CPU libre para asignarle un nuevo proceso\n");

			log_info(log_consola,"No existe una CPU libre para asignarle un nuevo proceso\n");

			}

		}

		//log_trace(logger,"Operaciones realizadas por el proceso %d hasta el momento son:", pcb->pid);



		destroy_message(msg);



		break;

		/*

		 log_trace(logger,"Operaciones realizadas por el proceso %d hasta el momento son:", pcb->pid);

		 m=1;

		 while(msg->argv[m]!='\0'){

		 switch (msg->argv[m]) {

		 case iniciar:

		 log_trace(logger,"	mProc	%d	-	Iniciado.", pcb->pid);

		 m++;

		 break;


		 case leer:

		 log_trace(logger,"mProc	%d	-	Pagina	%d	leida:", pcb->pid,msg->argv[m+1]);

		 m++;

		 m++;

		 break;

		 case escribir:

		 log_trace(logger,"mProc	%d	-	Pagina	%d	escrita:", pcb->pid,msg->argv[m+1]);

		 m++;

		 m++;

		 break;

		 case io:

		 log_trace(logger,"mProc	%d	en	entrada-salida	de	tiempo	%d.", pcb->pid,msg->argv[1]);

		 m++;

		 break;

		 case final:

		 log_trace(logger,"mProc	%d	Finalizado.", pcb->pid);

		 m++;

		 break;

		 case error:

		 log_trace(logger,"mProc	%d	-	Fallo.", pcb->pid);

		 m++;

		 break;

		 default:

		 log_trace(logger,"No se comprende el mensaje enviado");

		 break;

		 }

		 */

		break;

	case PCB_LOGUEO:

		pcb = es_el_pcb_buscado_por_id(msg->argv[0]);

		m = 1;

		switch (msg->argv[m]) {

		case iniciar:

			log_info(logger, "	mProc	%d	-	Iniciado.", pcb->pid);


			log_info(log_consola, "	mProc	%d	-	Iniciado.", pcb->pid);

			break;

		case leer:

			pagina = msg->argv[2];

			log_info(logger, "mProc	%d	-	Pagina	%d	leida: %s \n", pcb->pid,
					pagina, msg->stream);


			log_info(log_consola, "mProc	%d	-	Pagina	%d	leida: %s \n", pcb->pid,
					pagina, msg->stream);

			break;

		case escribir:

			pagina = msg->argv[2];

			log_info(logger, "mProc	%d	-	Pagina	%d	escrita: %s \n", pcb->pid,
					msg->argv[m + 1], msg->stream);


			log_info(log_consola, "mProc	%d	-	Pagina	%d	escrita: %s \n", pcb->pid,
					msg->argv[m + 1], msg->stream);

			break;

		case io:

			segundos = msg->argv[2];

			log_info(logger, "mProc	%d	en	entrada-salida	de	tiempo	%d. \n",
					pcb->pid, segundos);


			log_info(log_consola, "mProc	%d	en	entrada-salida	de	tiempo	%d. \n",
					pcb->pid, segundos);

			break;

		case final:

			log_info(logger, "mProc	%d	Finalizado.", pcb->pid);

			log_info(log_consola, "mProc	%d	Finalizado.", pcb->pid);

			m++;

			break;

		case error:

			log_info(log_consola, "mProc	%d	-	Fallo.", pcb->pid);

			break;

		default:

			log_error(logger, "No se comprende el mensaje enviado \n");

			log_error(log_consola, "No se puede loguear operacion No se comprende el mensaje enviado \n");

			break;

		}

		destroy_message(msg);

		break;

		/////////////////////////////////
		//agrego esto para que pasen los tests
		//creo que habria que agregar un param mas si la ultima sentencia se ejecuto bien o mal
		/*switch (msg->argv[1]) {
		 case io:
		 log_trace(logger, "pid: %d, Fin por IO, tiempo: %d", msg->argv[0], msg->argv[2]);
		 break;
		 case final:
		 log_trace(logger, "pid: %d, Finalizacion exitosa", msg->argv[0]);
		 break;
		 case error:
		 log_trace(logger, "pid: %d, Fin por error", msg->argv[0]);
		 break;
		 default:
		 log_trace(logger, "pid: %d, cant_sent_ejec: %d, Fin por otra cosa, seguramente porque termino su quantum", msg->argv[0], msg->argv[3]);
		 }
		 break;*/
		////////////////////////////////////////
		///////////////////////////////////////
		//RR
	case PCB_FIN_QUANTUM:

		PID_GLOBAL_EXEC = (msg->argv[0]);

		list_remove_by_condition(list_exec, (void*) es_el_pcb_buscado_en_exec);

		log_info(logger,
				"El proceso de Pid %d finalizó su Quantum y pasa del estado en Ejecución al estado Listo \n",
				msg->argv[0]);

		log_info(log_listas,
				"El proceso de Pid %d finalizó su Quantum y pasa del estado en Ejecución al estado Listo \n",
				msg->argv[0]);

		t_ready* ready=malloc(sizeof(t_ready));

		ready->pid = msg->argv[0];

		list_add(list_ready, ready);

		t_pcb* pcb = es_el_pcb_buscado_por_id(msg->argv[0]);

		pcb->estado = READY
		;

		pcb->tiempo_inicio_ready = time(NULL);

		log_info(logger, "El proceso %d se encuentra en la cola de Listos \n",
				pcb->pid);

		log_info(log_listas, "El proceso %d se encuentra en la cola de Listos \n",
				pcb->pid);

		if ((pcb->pc + 1) != pcb->cant_sentencias) {

			quantum=atoi(QUANTUM());

			pcb->pc = pcb->pc + quantum;

		}

		pcb->cpu_asignado = 100; //Hay que poner un número alto

		cpu = cpu_buscar_por_socket(socket);

		cpu->estado = 1;

		log_info(logger,
			"Operaciones realizadas por el proceso %d hasta el momento son:\n",
			pcb->pid);


		log_info(log_consola,
			"Operaciones realizadas por el proceso %d hasta el momento son:\n",
			pcb->pid);

		for (i=0;i<msg->argv[3];i++){

		msge=recibir_mensaje(socket);
		logueo (msge);
		destroy_message(msge);

		}

		if (list_size(list_ready) > 0) {
			if (cpu_disponible()) {
				cpu = cpu_seleccionar();
				if (cpu != NULL) {
					t_ready* ready = malloc(sizeof(t_ready));
					ready = list_get(list_ready, 0);
					t_pcb* pcb2;
					pcb2 = es_el_pcb_buscado_por_id(ready->pid);

					pcb2->cpu_asignado = cpu->id;
					cpu_ejecutar(cpu, pcb2);
					cpu->estado = 0;
				} else {
				/*	printf(
							"No existe CPU activa para asignar al proceso %d. El proceso queda en READY \n",
							pcb->pid);*/
					log_info(logger,
							"No existe una CPU libre para asignar al proceso %d. El proceso queda en READY \n",
							pcb->pid);
					log_info(log_consola,
					         "No existe una CPU libre para asignar al proceso %d. El proceso queda en READY \n",
												pcb->pid);


				}
			} else {

				log_info(logger,"No existe una CPU libre para asignarle un nuevo proceso\n");
				//printf("No existe CPU activa para asignarle un proceso \n");

				log_info(log_consola,"No existe una CPU libre para asignarle un nuevo proceso\n");


			}

		}

		//log_trace(logger,"Operaciones realizadas por el proceso %d hasta el momento son:", pcb->pid);

		/*

		 log_trace(logger,"Operaciones realizadas por el proceso %d hasta el momento son:", pcb->pid);

		 m=1;

		 while(msg->argv[m]!='\0'){

		 switch (msg->argv[m]) {

		 case iniciar:

		 log_trace(logger,"	mProc	%d	-	Iniciado.", pcb->pid);

		 m++;

		 break;


		 case leer:

		 log_trace(logger,"mProc	%d	-	Pagina	%d	leida:", pcb->pid,msg->argv[m+1]);

		 m++;

		 m++;

		 break;

		 case escribir:

		 log_trace(logger,"mProc	%d	-	Pagina	%d	escrita:", pcb->pid,msg->argv[m+1]);

		 m++;

		 m++;

		 break;

		 case io:

		 log_trace(logger,"mProc	%d	en	entrada-salida	de	tiempo	%d.", pcb->pid,msg->argv[1]);

		 m++;

		 break;

		 case final:

		 log_trace(logger,"mProc	%d	Finalizado.", pcb->pid);

		 m++;

		 break;

		 case error:

		 log_trace(logger,"mProc	%d	-	Fallo.", pcb->pid);

		 m++;

		 break;

		 default:

		 log_trace(logger,"No se comprende el mensaje enviado");

		 break;

		 }

		 }

		 */



		destroy_message(msg);

		break;

		//////////////////////////////////////////////////////TERMINA CASE RR

		//termina case IO y final

		/*
		 switch (msg->argv[0]) {

		 case PCB_IO:

		 pcb = es_el_pcb_buscado_por_id(msg->argv[1]);

		 PID_GLOBAL_BLOCK = pcb->pid;
		 PID_GLOBAL_EXEC=pcb->pid;
		 IO_GLOBAL = msg->argv[2];

		 int cantIO = msg->argv[3];

		 if (list_any_satisfy(list_exec, (void*) es_el_pcb_buscado_en_exec)) {

		 list_remove_by_condition(list_exec,
		 (void*) es_el_pcb_buscado_en_exec);

		 t_block* block=malloc(sizeof(block));

		 block->pid = PID_GLOBAL_BLOCK;
		 block->tiempoIO=cantIO;
		 block->estado=0;


		 list_add(list_block, block);

		 pcb->estado=BLOCK;

		 log_trace(logger,"El proceso %d se encuentra en la cola de procesos en Bloqueados", pcb->pid);

		 pid_string = string_itoa(PID_GLOBAL_BLOCK);
		 }

		 if ((list_size(list_ready)) != 0) {
		 if (cpu_disponible()) cpu = cpu_seleccionar();
		 if(cpu!=NULL){
		 pcb->cpu_asignado = cpu->id;
		 cpu_ejecutar(cpu, pcb);
		 cpu->estado=0;}
		 else
		 {printf("No existe CPU activa para asignar al proceso %d. El proceso queda en READY", pcb->pid);
		 }
		 }else
		 {
		 printf("No existe CPU activa para asignarle un nuevo proceso");
		 }


		 break;

		 /*case PCB_FINQ:
		 //VUELVE EN EL FIN DEL QUANTUM

		 pcb = es_el_pcb_buscado_por_id(msg->argv[0]);

		 PID_GLOBAL_EXEC=pcb->pid;

		 if(list_any_satisfy(list_exec, (void*) es_el_pcb_buscado_en_exec)){

		 list_remove_by_condition(list_exec, (void*) es_el_pcb_buscado_en_exec);

		 }

		 log_trace(logger,"El proceso de Pid %d finalizó su Quantum y pasa del estado en Ejecución al estado Listo", msg->argv[0]);

		 t_ready* ready;

		 ready->pid=PID_GLOBAL_EXEC;

		 list_add(list_ready,ready);

		 if ((list_size(list_ready)) != 0) {
		 if (cpu_disponible()) cpu = cpu_seleccionar();
		 if(cpu!=NULL){
		 pcb->cpu_asignado = cpu->id;
		 cpu_ejecutar(cpu, pcb);
		 cpu->estado=0;}
		 else
		 {printf("No existe CPU activa para asignar al proceso %d. El proceso queda en READY", pcb->pid);
		 }
		 }else
		 {
		 printf("No existe CPU activa para asignarle un nuevo proceso");
		 }

		 break;


		 if ((list_size(list_ready)) != 0) {
		 if (cpu_disponible()) cpu = cpu_seleccionar();
		 if(cpu!=NULL){
		 pcb->cpu_asignado = cpu->id;
		 cpu_ejecutar(cpu, pcb);
		 cpu->estado=0;}
		 else
		 {printf("No existe CPU activa para asignar al proceso %d. El proceso queda en READY", pcb->pid);
		 }
		 }else
		 {
		 printf("No existe CPU activa para asignarle un nuevo proceso");
		 }

		 break;

		 if ((list_size(list_ready)) != 0) {
		 if (cpu_disponible()) cpu = cpu_seleccionar();
		 if(cpu!=NULL){
		 pcb->cpu_asignado = cpu->id;
		 cpu_ejecutar(cpu, pcb);
		 cpu->estado=0;}
		 else
		 {printf("No existe CPU activa para asignar al proceso %d. El proceso queda en READY", pcb->pid);
		 }
		 }else
		 {
		 printf("No existe CPU activa para asignarle un nuevo proceso");
		 }

		 break;

		 case PCB_FINALIZAR:

		 pcb = es_el_pcb_buscado_por_id(msg->argv[1]);

		 PID_GLOBAL_FINISH = pcb->pid;
		 /*
		 Tiempo de retorno: tiempo transcurrido entre la llegada de
		 un proceso y su finalización.
		 Tiempo de espera: tiempo que un proceso permanece en la
		 cola de preparados.
		 Tiempo de respuesta: tiempo que un proceso bloqueado
		 tarda en entrar en la CPU desde que ocurre el suceso que lo
		 bloquea.*/

		/*
		 if (list_any_satisfy(list_exec, (void*) es_el_pcb_buscado_en_exec)) {

		 list_remove_by_condition(list_exec,
		 (void*) es_el_pcb_buscado_en_exec);

		 }
		 t_finish* finish=malloc(sizeof(t_finish));
		 t_pcb_finalizado* pcb2;

		 finish->pid = PID_GLOBAL_FINISH;
		 list_add(list_finish, finish);

		 pcb2->tiempo_total = difftime(time(NULL), time1);
		 pcb->estado=FINISH;

		 cpu=cpu_buscar_por_socket(socket);

		 cpu->estado=1;

		 log_trace(logger,"El proceso %d del programa Mcod %s se encuentra en la cola de procesos Finalizados", pcb->pid, pcb->path);

		 printf("Hay que finalizar el proceso");

		 // Hay que ver si hay algún proceso en READY para ejecutar

		 if(list_size(list_ready)>0){
		 if (cpu_disponible()) {
		 cpu = cpu_seleccionar();
		 if(cpu!=NULL){
		 t_ready* ready=malloc(sizeof(t_ready));

		 ready=list_get(list_ready,0);
		 t_pcb* pcb2;
		 pcb2=es_el_pcb_buscado_por_id(ready->pid);

		 pcb2->cpu_asignado = cpu->id;
		 cpu_ejecutar(cpu, pcb2);
		 cpu->estado=0;}
		 else
		 {printf("No existe CPU activa para asignar al proceso %d. El proceso queda en READY", pcb->pid);
		 }
		 }else
		 {
		 printf("No existe CPU activa para asignarle un nuevo proceso");
		 }


		 }

		 break;*/


	case PCB_ERROR:

				pcb = es_el_pcb_buscado_por_id(msg->argv[0]);

				if(pcb!=NULL){

					if(msg->argv[1]==0){

						log_info(logger,"El proceso cuyo pid es: %d  no pudo iniciarse y se eliminará del sistema", pcb->pid);

						log_info(log_consola,
										"El proceso cuyo pid es: %d  no pudo iniciarse y se eliminará del sistema", pcb->pid);

						PID_GLOBAL_EXEC = msg->argv[0];

						list_remove_by_condition(list_exec, (void*) es_el_pcb_buscado_en_exec);

						pcb->estado=ABORTADO;

						cpu = cpu_buscar_por_socket(socket);

						cpu->estado = 1;

						break;

					}


				log_info(logger,
						"El proceso cuyo pid es: %d\n ha sufrido un Error y se Finalizará", pcb->pid);

				log_info(log_consola,
								"El proceso cuyo pid es: %d\n ha sufrido un Error y se Finalizará", pcb->pid);

				PID_GLOBAL = pcb->pid;

				pcb->pc = pcb->cant_sentencias - 1;

				log_info(logger,"pcb->pc: %d",pcb->pc);

				log_info(log_consola,"pcb->pc: %d",pcb->pc);

				}else{

					log_info(logger,
							"No existe el proceso que se indicó como error");

					log_info(log_consola,
							"No existe el proceso que se indicó como error");

				}


						PID_GLOBAL_EXEC = (msg->argv[0]);

						list_remove_by_condition(list_exec, (void*) es_el_pcb_buscado_en_exec);


						t_ready* ready2=malloc(sizeof(t_ready));

						ready2->pid = msg->argv[0];

						list_add(list_ready, ready2);

						pcb->estado = READY
						;

						pcb->tiempo_inicio_ready = time(NULL);

						log_info(logger, "El proceso %d se encuentra en la cola de Listos \n",
								pcb->pid);

						log_info(log_listas, "El proceso %d se encuentra en la cola de Listos \n",
								pcb->pid);

						pcb->cpu_asignado = 100; //Hay que poner un número alto

						cpu = cpu_buscar_por_socket(socket);

						cpu->estado = 1;

						/*log_info(logger,
							"Operaciones realizadas por el proceso %d hasta el momento son:\n",
							pcb->pid);


						log_info(log_consola,
							"Operaciones realizadas por el proceso %d hasta el momento son:\n",
							pcb->pid);*/

						for (i=0;i<msg->argv[3];i++){

						msge=recibir_mensaje(socket);
						logueo (msge);
						destroy_message(msge);

						}

						if (list_size(list_ready) > 0) {
							if (cpu_disponible()) {
								cpu = cpu_seleccionar();
								if (cpu != NULL) {
									t_ready* ready = malloc(sizeof(t_ready));
									ready = list_get(list_ready, 0);
									t_pcb* pcb2;
									pcb2 = es_el_pcb_buscado_por_id(ready->pid);

									pcb2->cpu_asignado = cpu->id;
									cpu_ejecutar(cpu, pcb2);
									cpu->estado = 0;
								} else {
								/*	printf(
											"No existe CPU activa para asignar al proceso %d. El proceso queda en READY \n",
											pcb->pid);*/
									log_info(logger,
											"No existe una CPU libre para asignar al proceso %d. El proceso queda en READY \n",
											pcb->pid);
									log_info(log_consola,
									         "No existe una CPU libre para asignar al proceso %d. El proceso queda en READY \n",
																pcb->pid);


								}
							} else {

								log_info(logger,"No existe una CPU libre para asignarle un nuevo proceso\n");
								//printf("No existe CPU activa para asignarle un proceso \n");

								log_info(log_consola,"No existe una CPU libre para asignarle un nuevo proceso\n");


							}

						}

						destroy_message(msg);

		break;

	case CPU_PORCENTAJE_UTILIZACION:

		cpu = cpu_buscar_por_socket(msg->argv[0]);
		uso_cpu = msg->argv[1];
		cpu->usoUltimoMinuto = uso_cpu;


		if ((list_count_satisfying(cpus, (void*) cpus_sin_dato_uso)) < 1) {
			i = 0;

			while ((i + 1) <= list_size(cpus)) {

				cpu = cpu_buscar(i);

				if (cpu != NULL) {

					log_info(logger, "Porcentaje de Uso de la Cpu %d: %d % \n",
							cpu->id, cpu->usoUltimoMinuto);

					log_info(log_porcentajes, "Porcentaje de Uso de la Cpu %d: %d % \n",
							cpu->id, cpu->usoUltimoMinuto);

				}

				i++;
			}

		} else {

			log_warning(logger,
					"Aún no está la información del Porcentaje de Uso de todas las CPUs\n");


			log_warning(log_porcentajes,
					"Aún no está la información del Porcentaje de Uso de todas las CPUs\n");

		}

		destroy_message(msg);

		break;

		// NUEVA VERSION

		// ULTIMA VERSION
		/*
		 //cpu = cpu_buscar_por_socket(socket);
		 n=0;
		 m=1;

		 int total_cpus=list_size(cpus);

		 while((n+1)<=total_cpus){

		 uso_cpu=msg->argv[m];

		 cpu=list_get(cpus,0);

		 cpu->usoUltimoMinuto=uso_cpu;

		 n++;

		 m++;

		 }

		 n=0;

		 while((n+1)<=list_size(cpus)){

		 cpu = cpu_buscar(n);

		 if(cpu!=NULL){

		 log_trace(logger, "Cpu %d: %d", cpu->id, cpu->usoUltimoMinuto);

		 }

		 n++;
		 }

		 break;
		 ULTIMA VERSION*/
		/*

		 if(cpu==NULL){
		 printf("Se produce un error por no existir la CPU");
		 }else{
		 cpu->usoUltimoMinuto=uso_cpu;

		 if((list_count_satisfying(cpus,(void*) cpus_sin_dato_uso))<1){
		 i=0;

		 while((i+1)<=list_size(cpus)){


		 cpu = cpu_buscar(i);

		 if(cpu!=NULL){

		 log_trace(logger, "Cpu %d: %d", cpu->id, cpu->usoUltimoMinuto);

		 }

		 i++;
		 }

		 }else{

		 log_trace(logger,"Aún no está la información del uso de todas las CPUs");
		 }

		 }*/

	case CPU_ESPECIAL:

		cpu_especial = socket;

		log_info(logger,
				"Se conecta con el Planificador el Hilo al que se pedirán las solicitudes de Uso de CPUs \n");


		log_info(log_consola,
				"Se conecta con el Planificador el Hilo al que se pedirán las solicitudes de Uso de CPUs \n");

		break;

	default:

		log_error(logger,"El código de mensaje enviado es incorrecto \n");

		log_error(log_consola,"El código de mensaje enviado es incorrecto \n");

		break;
	}

	return 0;
}

int finalizar() {
	config_destroy(cfg);
	list_destroy_and_destroy_elements(pcbs,(void*)eliminar_pcb);
	list_destroy_and_destroy_elements(cpus,(void*)eliminar_cpu);
	list_destroy_and_destroy_elements(list_ready,(void*)eliminar_ready);
	list_destroy_and_destroy_elements(list_exec,(void*)eliminar_exec);
	list_destroy_and_destroy_elements(list_block,(void*)eliminar_block);
	list_destroy_and_destroy_elements(list_finish,(void*)eliminar_finish);
	printf("Fin OK\n");
	return 0;
}

int inicializar() {

	cfg = config_create(CONFIG_PATH);

	int tipo_logueo = TIPO_LOG();
	clean_file(LOG_PATH);
	logger = log_create(LOG_PATH, "planificador", true, tipo_logueo);


	log_consola=log_create(LOG_PATH_CONSOLA,"PLANIFICADOR",false,LOG_LEVEL_INFO);

	log_listas=log_create(LOG_PATH_LISTAS,"PLANIFICADOR",false,LOG_LEVEL_INFO);

	log_porcentajes =log_create(LOG_PATH_PORCENTAJE,"PLANIFICADOR",false,LOG_LEVEL_INFO);

	clean_file(LOG_PATH_CONSOLA);
	clean_file(LOG_PATH_LISTAS);
	clean_file(LOG_PATH_PORCENTAJE);

	cpus = list_create();

	pcbs = list_create();

	list_ready = list_create();

	list_block = list_create();

	list_finish = list_create();

	list_exec = list_create();

	//t_cpu_especial* cpu_especial=malloc(sizeof(t_cpu_especial));

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
 F=pcb->cant_sentencias;
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
void Hilo_IO(int pid) {

	while (1) {

sem_wait(&sem_IO);

	while(list_size(list_block) != 0) {
			//hay bloqueados
			//if ((list_any_satisfy(list_block, (void*) _estado_bloqueado))) {
				if(list_all_satisfy(list_block, (void*) _estado_libre)){

						//Entrada salida libre

				t_block* block;

				block = list_get(list_block, 0);

				block->estado = 1; //i/o ocupado

				log_info(logger,
						" El proceso %i empieza su I/O de %i  \n",
						block->pid, block->tiempoIO);


				log_info(log_consola,
						" El proceso %i empieza su I/O de %i  \n",
						block->pid, block->tiempoIO);



				sleep(block->tiempoIO);

				log_info(logger,
						" El proceso %i termina su I/O de %i y vuelve a la cola de Listos \n",
						block->pid, block->tiempoIO);


				log_info(log_listas,
						" El proceso %i termina su I/O de %i y vuelve a la cola de Listos \n",
						block->pid, block->tiempoIO);


				t_ready* ready = malloc(sizeof(t_ready));

				ready->pid = block->pid;


				PID_GLOBAL_BLOCK = block->pid;

				block->estado=0;


				if (list_any_satisfy(list_block,
						(void*) es_el_pcb_buscado_en_block)) {

					list_remove_by_condition(list_block,
							(void*) es_el_pcb_buscado_en_block);

				}

				list_add(list_ready, ready);

				t_pcb* pcb;

				pcb = es_el_pcb_buscado_por_id(block->pid);

				pcb->estado = READY
				;

				pcb->tiempo_inicio_ready = time(NULL);


				if (list_size(list_ready) > 0) {
					if (cpu_disponible()) {
						cpu = cpu_seleccionar();
						if (cpu != NULL) {
							t_ready* ready = malloc(sizeof(t_ready));
							ready = list_get(list_ready, 0);
							t_pcb* pcb2;
							pcb2 = es_el_pcb_buscado_por_id(ready->pid);

							pcb2->cpu_asignado = cpu->id;
							cpu_ejecutar(cpu, pcb2);
							cpu->estado = 0;
						} else {
					/*		printf(
									"No existe CPU activa para asignar al proceso %d. El proceso queda en READY \n",
									pcb->pid);
						*/
						log_info(logger,
								"No existe una CPU libre para asignar al proceso %d. El proceso queda en READY \n",pcb->pid);

						log_info(log_consola,
								"No existe una CPU libre para asignar al proceso %d. El proceso queda en READY \n",pcb->pid);


						}
					} else {
						//printf(
							log_info(logger,
									"No existe una CPU libre para asignarle un proceso \n");

							log_info(log_consola,
									"No existe una CPU libre para asignarle un proceso \n");


					}

				}//fin pasar a ready

			} // fin if ocupado/libre

		}

	}

}

/*

void controlar_IO(char* pid_string) {

	t_block* block;

	block = list_get(list_block,
			es_el_pid_en_block(atoi(pid_string), list_block));

	sleep(IO_GLOBAL);

	PID_GLOBAL_BLOCK = atoi(pid_string);

	t_pcb* pcb = list_find(pcbs, (void*) es_el_pcb_buscado_por_id);

	list_remove_by_condition(list_block, (void*) es_el_pcb_buscado_en_block);

	t_ready* ready = malloc(sizeof(t_ready));

	ready->pid = block->pid;

	list_add(list_ready, ready);

	pcb->estado = READY
	;

	log_trace(logger,
			"El proceso %d se encuentra en la cola de procesos Listos \n",
			pcb->pid);

	pthread_exit(NULL);

}

*/

t_pcb* es_el_pcb_buscado_por_id(int pid) {

	int i = 0;
	t_pcb* pcb;

	//pcb = list_get(pcbs, i);

	while ((i + 1) <= list_size(pcbs)) {

		pcb = list_get(pcbs, i);

		if (pcb->pid == pid) {
			break;
		} else {
		}
		i++;
	}

	return pcb;
}

int es_el_pcb_buscado(t_pcb* pcb) {
	return (PID_GLOBAL == pcb->pid);
}

t_pcb* es_el_pcb_buscado_struct(t_pcb * pcb) {

	int i = 0;
	//t_pcb* pcb;

	pcb = list_get(pcbs, i);

	while ((i + 1) <= list_size(pcbs)) {

		pcb = list_get(pcbs, i);

		if (pcb->pid == PID_GLOBAL) {

			break;
		}
		else {

			i++;
		}
	}

	return pcb;
}

int pos_del_pcb(int pid) {

	int i = 0;

	t_pcb* pcb;

	while ((i + 1) <= list_size(pcbs)) {

		pcb = list_get(pcbs, i);

		if (pcb->pid == PID_GLOBAL) {
			break;
		} else {
			i++;
		}
	}

	return i;
}

int es_el_pid_en_block(int pid, t_list* list_block) {

	int i = 0;

	t_block* block;

	block = list_get(list_block, i);

	while ((i + 1) <= list_size(list_block)) {

		if (block->pid == pid) {
			break;
		} else {
			i++;
		}

	}
	return i;
}

int es_el_pcb_buscado_en_exec(t_exec* exec) {
	return (exec->pid == PID_GLOBAL_EXEC);
}


int es_el_pcb_buscado_en_block(t_block* block) {
	return (block->pid == PID_GLOBAL_BLOCK);
}

void cambiar_a_exec(int pid) {
	PID_GLOBAL_READY = pid;
	PID_GLOBAL = pid;
	list_remove_by_condition(list_ready, (void*) es_el_pcb_buscado_en_ready);
	t_pcb* pcb;

	pcb = list_find(pcbs, (void*) es_el_pcb_buscado);

	t_exec* exec = malloc(sizeof(t_exec));
	exec->pid = pid;
	list_add(list_exec, exec);
	pcb->tiempo_fin_ready = time(NULL);

	//log_trace(logger, "Tiempo de fin del proceso %d en la cola de Ready es %d",
	//	pcb->pid, pcb->tiempo_fin_ready);

	pcb->tiempo_espera = pcb->tiempo_espera
			+ difftime(pcb->tiempo_fin_ready, pcb->tiempo_inicio_ready);

	pcb->estado = EXEC
	;

	log_info(logger,
			"El proceso %d se encuentra en la cola de procesos en Ejecución y se está ejecutando en la CPU %d \n",
			pcb->pid, pcb->cpu_asignado);


	log_info(log_listas,
			"El proceso %d se encuentra en la cola de procesos en Ejecución y se está ejecutando en la CPU %d \n",
			pcb->pid, pcb->cpu_asignado);

	return;

}


int es_el_pcb_buscado_en_ready(t_ready* ready) {
	return (ready->pid == PID_GLOBAL_READY);
}

double round_2(double X, int k) {

	return floor(pow(10, k) * X + 0.5) / pow(10, k);

}

int cpus_sin_dato_uso(t_cpu* cpu) {

	return (cpu->usoUltimoMinuto == 0);

}

void logueo (t_msg* msg){

			t_pcb* pcb;

			int segundos;

			int pagina;

			int m;

			pcb = es_el_pcb_buscado_por_id(msg->argv[0]);

			m = 1;

			switch (msg->argv[m]) {

			case iniciar:

				log_info(logger, "	mProc	%d	-	Iniciado.", pcb->pid);

				log_info(log_consola, "	mProc	%d	-	Iniciado.", pcb->pid);

				break;

			case leer:

				pagina = msg->argv[2];

				log_info(logger, "mProc	%d	-	Pagina	%d	leida: %s \n", pcb->pid,
						pagina, msg->stream);

				log_info(log_consola, "mProc	%d	-	Pagina	%d	leida: %s \n", pcb->pid,
						pagina, msg->stream);

				break;

			case escribir:

				pagina = msg->argv[2];

				log_info(logger, "mProc	%d	-	Pagina	%d	escrita: %s \n", pcb->pid,
						msg->argv[m + 1], msg->stream);


				log_info(log_consola, "mProc	%d	-	Pagina	%d	escrita: %s \n", pcb->pid,
						msg->argv[m + 1], msg->stream);

				break;

			case io:

				segundos = msg->argv[2];

				log_info(logger, "mProc	%d	en	entrada-salida	de	tiempo	%d. \n",
						pcb->pid, segundos);

				log_info(log_consola, "mProc	%d	en	entrada-salida	de	tiempo	%d. \n",
						pcb->pid, segundos);

				break;

			case final:

				log_info(logger, "mProc	%d	Finalizado.", pcb->pid);


				log_info(log_consola, "mProc	%d	Finalizado.", pcb->pid);

				m++;

				break;

			case error:

				log_info(logger, "mProc	%d	-	Fallo.", pcb->pid);

				log_info(log_consola, "mProc	%d	-	Fallo.", pcb->pid);

				break;

			default:

				log_error(logger, "No se comprende el mensaje enviado \n");

				log_error(log_consola, "No se puedo loguear, no se comprende el mensaje enviado \n");

				break;

			}

}

void eliminar_pcb (t_pcb* pcb){
		free(pcb->path);
		free(pcb);
}

void eliminar_cpu (t_cpu* cpu){
	free(cpu);
}

void eliminar_ready (t_ready* ready){
	free(ready);
}
void eliminar_exec (t_exec* exec){
	free(exec);
}
void eliminar_block (t_block* block){
	free(block);
}
void eliminar_finish (t_finish* finish){
	free(finish);
}
