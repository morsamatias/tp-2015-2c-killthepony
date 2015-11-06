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

t_log* logger;
char* LOG_PATH = "log.txt";

int inicializar();
int finalizar();
//int iniciar_consola();

t_cpu* cpu;
pthread_t th_server_cpu;
pthread_t contador_IO_PCB;
time_t time1;

int main(void) {
	inicializar();

	char comando_usuario[COMMAND_MAX_SIZE];
	printf("INICIANDO CONSOLA\n");
	time1 = time(NULL);
	//pthread_create(&th_server_cpu, NULL, (void*)iniciar_server_select, NULL);

	pthread_create(&contador_IO_PCB, NULL, (void*) Hilo_IO, (void*) PID_GLOBAL);
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

	int consola = 0;

	FD_ZERO(&master); // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);
	FD_SET(fdNuevoNodo, &master);
	FD_SET(consola, &master); // agrego consola stdin
	fdmax = fdNuevoNodo; // por ahora el maximo
	printf("Consola iniciada \n");
	printf("Opciones Disponibles: \n ");
	printf("CORRER Path \n FINALIZAR Pid \n PS \n CPU \n");
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
					if (socket == fdNuevoNodo) { // gestionar nuevas conexiones
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
							log_trace(logger,
									"El cpu %d, socket: %d se desconecto",
									cpu->id, cpu->socket);

							pcb = pcb_buscar_por_cpu(cpu->id);

							if (pcb != NULL) {
								log_trace(logger,
										" La CPU tenia a cargo los siguentes procesos  %d",
										pcb->pid);
							} else {
								log_trace(logger, "Ningun pcb a replanificar");
							}

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
		if (file_exists(path_completo)) {
			correr_proceso(path_completo);
		} else {
			log_trace(logger, "Mcod no existente\n");
		}
		break;
	case FINALIZAR:
		pid = atoi(input_user[1]);
		log_trace(logger, "finalizar pid: %d\n", pid);
		t_pcb* pcb;
		PID_GLOBAL = pid;
		pcb = list_get(pcbs, pos_del_pcb(pid));
		pcb->pc = pcb->cant_sentencias;
		break;
	case PS:
		log_trace(logger, "PS listar procesos\n");
		int i = 0;
		t_pcb* pcb2;
		while ((i + 1) <= list_size(pcbs)) {

			pcb2 = list_get(pcbs, i);

			//log_info(log_pantalla,"mProc	%d PID:	%s nombre	->	%d estado /n",pcb->pid,pcb->path,pcb->estado);

			switch (pcb2->estado) {

			case 0:

				log_trace(logger, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
						pcb2->pid, pcb2->path, "NEW");

				break;

			case 1:

				log_trace(logger, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
						pcb2->pid, pcb2->path, "READY");

				break;

			case 2:

				log_trace(logger, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
						pcb2->pid, pcb2->path, "BLOCK");

				break;

			case 3:

				log_trace(logger, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
						pcb2->pid, pcb2->path, "EXEC");

				break;

			case 4:

				log_trace(logger, "mProc	 PID: %d	 nombre %s	->	 estado %s\n",
						pcb2->pid, pcb2->path, "FINISH");

				break;

			default:

				log_trace(logger,
						"No se puede determinar el estado del proceso %d",
						pcb->pid);

				break;

			}

			i++;
		}
		break;
	case CPU:
		i = 0;
		log_trace(logger, "Uso CPU en el ultimo min \n");

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

		 */

		t_msg* pedido_uso = argv_message(CPU_PORCENTAJE_UTILIZACION, 0);

		enviar_mensaje(cpu_especial, pedido_uso);

		destroy_message(pedido_uso);

		break;

		/*	case SALIR:			//exit
		 fin = true;
		 break;*/
	default:
		log_trace(logger, "Comando desconocido\n");
		break;
	}
	free_split(input_user);
}

int correr_proceso(char* path) {
	t_pcb* pcb = NULL;
	pcb = pcb_nuevo(path);
	pcb->pid = get_nuevo_pid();
	log_trace(logger, "Pid asignado: %d", pcb->pid);

	pcb->cant_sentencias = get_cant_sent(path);

	int algoritmo = ALGORITMO_PLANIFICACION();

	if (algoritmo == 0) {

		log_trace(logger, "El algoritmo de planificación será FIFO");

		pcb->cant_a_ejectuar = get_cant_sent(path);
	} else {

		log_trace(logger, "El algoritmo de planificación será Round Robin");

		pcb->cant_a_ejectuar = QUANTUM();
		// en caso de que se RR es el Q
	}

	log_trace(logger, "Proceso mProc a ejecutar: %s", pcb->path);

	pcb->estado = NEW
	;

	pcb->tiempo_inicio_proceso = (double) clock() / CLOCKS_PER_SEC;

	log_trace(logger, "Tiempo de inicio del proceso %d es %ld", pcb->pid,
			pcb->tiempo_inicio_proceso);

	pcb->cantidad_IO = 0;

	pcb->tiempo_espera = 0;

	log_trace(logger,
			"El proceso %d se encuentra en la cola de procesos Nuevos para ejecutar el programa Mcod %s",
			pcb->pid, pcb->path);

	pcb_agregar(pcb);

	t_ready* new = malloc(sizeof(t_ready));

	new->pid = pcb->pid;

	list_add(list_ready, new);

	pcb->estado = READY
	;

	pcb->tiempo_inicio_ready = (double) clock() / CLOCKS_PER_SEC;

	log_trace(logger,
			"El proceso %d se encuentra en la cola de procesos Listos",
			pcb->pid);

	t_cpu* cpu = NULL;
	if (cpu_disponible()) {
		cpu = cpu_seleccionar();
		if (cpu != NULL) {
			pcb->cpu_asignado = cpu->id;
			cpu_ejecutar(cpu, pcb);
			cpu->estado = 0;
		} else {
			printf(
					"No existe CPU activa para asignar al proceso %d. El proceso queda en READY",
					pcb->pid);
		}
	} else {
		printf(
				"No existe CPU activa para asignar al proceso %d. El proceso queda en READY",
				pcb->pid);
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

	switch (msg->header.id) {
	case CPU_NUEVO:
		//el ID esta en la pos 0
		id_cpu = msg->argv[0];
		log_trace(logger, "Nuevo CPU id: %d", id_cpu);
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
					printf(
							"No existe CPU activa para asignar al proceso %d. El proceso queda en READY",
							pcb->pid);
				}
			} else {
				printf("No existe CPU activa para asignarle un nuevo proceso");
			}

		}

		break;

	case PCB_IO:

		pcb = es_el_pcb_buscado_por_id(msg->argv[0]);

		pcb->cantidad_IO = pcb->cantidad_IO + 1;

		if (pcb->cantidad_IO == 1) {

			pcb->tiempo_entrada_salida = (double) clock() / CLOCKS_PER_SEC;

			log_trace(logger,
					"Tiempo en el que el proceso %d inicia la Entrada-Salida es %ld",
					pcb->pid, pcb->tiempo_entrada_salida);

			pcb->tiempo_respuesta = (pcb->tiempo_entrada_salida
					- pcb->tiempo_inicio_proceso);

		}

		PID_GLOBAL_BLOCK = pcb->pid;
		IO_GLOBAL = msg->argv[1];

		int cantIO = msg->argv[1];

		PID_GLOBAL_EXEC = pcb->pid;

		if (list_any_satisfy(list_exec, (void*) es_el_pcb_buscado_por_id)) {
			list_remove_by_condition(list_exec,
					(void*) es_el_pcb_buscado_en_exec);
		}
		t_block* blocked = malloc(sizeof(t_block));

		blocked->pid = msg->argv[0];

		blocked->tiempoIO = cantIO;
		blocked->estado = 0;

		list_add(list_block, blocked);

		pcb->estado = BLOCK;

		log_trace(logger,
				"El proceso %d se encuentra en la cola de procesos en Bloqueados",
				pcb->pid);

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

		destroy_message(msg);

		break;

	case PCB_FINALIZAR:
		log_trace(logger, "EMPEZO A FINALIZAR");
		pcb = es_el_pcb_buscado_por_id(msg->argv[0]);

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

		pcb->tiempo_fin_proceso = (double) clock() / CLOCKS_PER_SEC;

		log_trace(logger,
				"Tiempo en el que el proceso %d finaliza operatoria es %ld",
				pcb->pid, pcb->tiempo_fin_proceso);

		pcb->tiempo_retorno = (pcb->tiempo_fin_proceso
				- pcb->tiempo_inicio_proceso);

		if (pcb->cantidad_IO == 0) {

			pcb->tiempo_entrada_salida = (double) clock() / CLOCKS_PER_SEC;

			pcb->tiempo_respuesta = (pcb->tiempo_fin_proceso
					- pcb->tiempo_inicio_proceso);

		}

		cpu = cpu_buscar_por_socket(socket);

		cpu->estado = 1;

		log_trace(logger,
				"El proceso %d se encuentra en la cola de procesos Finalizados",
				pcb->pid);

		log_trace(logger,
				"Métricas del Proceso %d \n El Tiempo de Retorno fue de %d \n El Tiempo de Respuesta fue de %d \n El Tiempo de Espera fue de \n",
				pcb->pid, pcb->tiempo_retorno, pcb->tiempo_respuesta);

		//printf("Hay que finalizar el proceso");

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
					printf(
							"No existe CPU activa para asignar al proceso %d. El proceso queda en READY",
							pcb->pid);
				}
			} else {
				printf("No existe CPU activa para asignarle un nuevo proceso");
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

		pcb->pid = msg->argv[0];

		log_trace(logger,
				"Operaciones realizadas por el proceso %d hasta el momento son:",
				pcb->pid);

		m = 1;

		switch (msg->argv[m]) {

		case iniciar:

			log_trace(logger, "	mProc	%d	-	Iniciado.", pcb->pid);

			break;

		case leer:

			pagina = msg->argv[2];

			log_trace(logger, "mProc	%d	-	Pagina	%d	leida: %s \n", pcb->pid,
					pagina, msg->stream);

			break;

		case escribir:

			pagina = msg->argv[2];

			log_trace(logger, "mProc	%d	-	Pagina	%d	escrita: %s \n", pcb->pid,
					msg->argv[m + 1], msg->stream);

			break;

		case io:

			segundos = msg->argv[2];

			log_trace(logger, "mProc	%d	en	entrada-salida	de	tiempo	%d. \n",
					pcb->pid, segundos);

			break;

		case final:

			log_trace(logger, "mProc	%d	Finalizado.", pcb->pid);

			m++;

			break;

		case error:

			log_trace(logger, "mProc	%d	-	Fallo.", pcb->pid);

			break;

		default:

			log_trace(logger, "No se comprende el mensaje enviado");

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

		log_trace(logger,
				"El proceso de Pid %d finalizó su Quantum y pasa del estado en Ejecución al estado Listo",
				msg->argv[0]);

		t_ready* ready = malloc(sizeof(ready));

		ready->pid = msg->argv[0];

		list_add(list_ready, ready);

		t_pcb* pcb = es_el_pcb_buscado_por_id(msg->argv[0]);

		pcb->estado = READY
		;

		pcb->tiempo_inicio_ready = (double) clock() / CLOCKS_PER_SEC;

		log_trace(logger, "El proceso %d se encuentra en la cola de Listos",
				pcb->pid);

		if (pcb->pc != pcb->cant_sentencias) {

			pcb->pc = pcb->pc + QUANTUM();

		}

		pcb->cpu_asignado = 100; //Hay que poner un número alto

		cpu = cpu_buscar_por_socket(socket);

		cpu->estado = 1;

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
					printf(
							"No existe CPU activa para asignar al proceso %d. El proceso queda en READY",
							pcb->pid);
				}
			} else {
				printf("No existe CPU activa para asignarle un proceso");
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

	case CPU_PORCENTAJE_UTILIZACION:

		cpu = cpu_buscar_por_socket(msg->argv[0]);
		uso_cpu = msg->argv[1];
		cpu->usoUltimoMinuto = uso_cpu;
		int i;

		if ((list_count_satisfying(cpus, (void*) cpus_sin_dato_uso)) < 1) {
			i = 0;

			while ((i + 1) <= list_size(cpus)) {

				cpu = cpu_buscar(i);

				if (cpu != NULL) {

					log_trace(logger, "Porcentaje de Uso de la Cpu %d: %d",
							cpu->id, cpu->usoUltimoMinuto);

				}

				i++;
			}

		} else {

			log_trace(logger,
					"Aún no está la información del Porcentaje de Uso de todas las CPUs");
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

		log_trace(logger,
				"Se conecta con el Planificador el Hilo al que se pedirán las solicitudes de Uso de CPUs");

		break;

	default:

		printf("El código de mensaje enviado es incorrecto");

		break;
	}

	return 0;
}

int finalizar() {
	config_destroy(cfg);
	printf("Fin OK\n");
	return 0;
}

int inicializar() {

	cfg = config_create(CONFIG_PATH);

	int tipo_logueo = TIPO_LOG();

	logger = log_create(LOG_PATH, "planificador", true, tipo_logueo);

	cpus = list_create();

	pcbs = list_create();

	list_ready = list_create();

	list_block = list_create();

	list_finish = list_create();

	list_exec = list_create();

	//t_cpu_especial* cpu_especial=malloc(sizeof(t_cpu_especial));

	return 0;
}

void Hilo_IO(int pid) {

	while (1) {

		if ((list_size(list_block)) != 0) {
			//hay bloqueados
			if ((list_any_satisfy(list_block, (void*) _estado_bloqueado))) {

//entrada salida ocupada
				log_info(logger, "Entrada salida ocupada");

			} else {

				//Entrada salida libre
				t_block* block;

				block = list_get(list_block, 0);

				block->estado = 1; //i/o ocupado

				sleep(block->tiempoIO);

				log_info(logger,
						" El proceso %i termina su I/O de %i y vuelve a la cola de listos",
						block->pid, block->tiempoIO);

				t_ready* ready = malloc(sizeof(t_ready));

				ready->pid = block->pid;

				PID_GLOBAL = block->pid;

				list_add(list_ready, ready);

				t_pcb* pcb;

				pcb = es_el_pcb_buscado_por_id(block->pid);

				pcb->estado = READY
				;

				pcb->tiempo_inicio_ready = (double) clock() / CLOCKS_PER_SEC;

				if (list_any_satisfy(list_block,
						(void*) es_el_pcb_buscado_por_id)) {

					list_remove_by_condition(list_block,
							(void*) es_el_pcb_buscado_en_block);

				}

			}

		}
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
				}

			} else {

				printf("No existe CPU activa para asignarle un nuevo proceso");

			}

		}

	}

}

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
			"El proceso %d se encuentra en la cola de procesos Listos",
			pcb->pid);

	pthread_exit(NULL);

}

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
		} else {
			i++;
		}
	}

	return pcb;
}

int pos_del_pcb(int pid) {

	int i = 0;
	t_pcb* pcb;

	pcb = list_get(pcbs, i);

	while ((i + 1) <= list_size(pcbs)) {

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
	pcb->tiempo_fin_ready = (double) clock() / CLOCKS_PER_SEC;

	log_trace(logger, "Tiempo de fin del proceso %d en la cola de Ready es %ld",
			pcb->pid, pcb->tiempo_fin_ready);

	pcb->tiempo_espera = pcb->tiempo_espera
			+ (pcb->tiempo_fin_ready - pcb->tiempo_inicio_ready);

	pcb->estado = EXEC
	;

	log_trace(logger,
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

