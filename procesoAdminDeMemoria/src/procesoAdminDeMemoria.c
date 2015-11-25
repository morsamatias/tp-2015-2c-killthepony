/*
 ============================================================================
 Name        : procesoAdminDeMemoria.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "procesoAdminDeMemoria.h"


int main(void) {

	system("clear");

	inicializar();

	iniciar_signals();

	// INICIALIZA ESTRUCTURAS
	socket_swap = conectar_con_swap(); 												// SE CONECTA CON EL SWAP
	pthread_create(&t_tasas_globales, NULL, (void*) tasa_aciertos_TLB_total,NULL);	// CREO EL HILO QUE MANEJA LAS ESTADISTICAS GLOBALES DE TASA
	server_socket_select(PUERTO_ESCUCHA(), procesar_mensaje_cpu); 					// PROCESA LOS MENSAJES
	finalizar();																	// ELIMINA ESTRUCTURAS
	return EXIT_SUCCESS;
}

int inicializar(){ ///////////////////
	int i;

	// ARCHIVO DE CONFIGURACION
	CONFIG_PATH = "/home/utnso/Escritorio/git/tp-2015-2c-killthepony/procesoAdminDeMemoria/Debug/config.txt";
	cfg = config_create(CONFIG_PATH);

	// ARCHIVO DE LOG
	//LOGGER_PATH = "/home/utnso/Escritorio/git/tp-2015-2c-killthepony/procesoAdminDeMemoria/Debug/log.txt";
	//clean_file(LOGGER_PATH);
	clean_file("log_general.txt");
	clean_file("log_errores.txt");
	clean_file("log_memoria.txt");
	clean_file("log_estadisticas.txt");
	clean_file("log_status_memoria.txt");
	log_general_p = log_create("log_general.txt", "Adm_Mem", true, LOG_LEVEL_TRACE);
	log_general_f = log_create("log_general.txt", "Adm_Mem", LOG_TODO_POR_PANTALLA(), LOG_LEVEL_TRACE);
	log_errores = log_create("log_errores.txt", "Adm_Mem", LOG_TODO_POR_PANTALLA(), LOG_LEVEL_TRACE);
	log_memoria = log_create("log_memoria.txt", "Adm_Mem", LOG_TODO_POR_PANTALLA(), LOG_LEVEL_TRACE);
	log_estadisticas = log_create("log_estadisticas.txt", "Adm_Mem", LOG_TODO_POR_PANTALLA(), LOG_LEVEL_TRACE);
	log_print_mem = log_create("log_status_memoria.txt", "Adm_Mem", false, LOG_LEVEL_TRACE);

	// ESTRUCTURA MEMORIA
	memoria = (t_marco**)malloc(CANTIDAD_MARCOS()*sizeof(t_marco*));

	for(i=0;i<CANTIDAD_MARCOS();i++){
		memoria[i] = (t_marco*)malloc(sizeof(t_marco));
		memoria[i]->contenido = malloc((TAMANIO_MARCO()+1)*sizeof(char));
		memoria[i]->libre = 1;
	}

	// ESTRUCTURA TLB y TABLA DE PAGINAS
	TLB 	= list_create();
	paginas = list_create();

	// INICIALIZAR SEMAFOROS
	sem_init(&mutex_TLB, 0, 1);
	sem_init(&mutex_PAGINAS, 0, 1);

	iniciar_signals();

	return 0;
}

int finalizar(){  ///////////////////
	int i;

	// ARCHIVO DE CONFIGURACION
	config_destroy(cfg);

	// ARCHIVO DE LOG
	log_destroy(log_general_p);
	log_destroy(log_general_f);

	// DESTRUIR LA MEMORIA
	for(i=0;i<CANTIDAD_MARCOS();i++){
		free(memoria[i]->contenido);
		free(memoria[i]);
	}
	free(memoria);

	// ESTRUCTURA TLB y TABLA DE PAGINAS
	list_destroy_and_destroy_elements(paginas,(void*)destruir_proceso);
	list_destroy(TLB);

	return 0;
}




void procesar_mensaje_cpu(int socket, t_msg* msg){
	//print_msg(msg);
	char* buff_pag  = NULL;
	char* buff_pag_esc  = NULL;
	int st,pid,nro_pagina,cant_paginas, flag_reemplazo;
	t_msg* resp = NULL;
	t_proceso* proceso;
	t_pagina* pagina;
	t_busq_marco b_marco;


//	t_pagina* pagina;

	switch (msg->header.id) {
		case MEM_INICIAR: ///////
			//param 0 cant_paginas
			//param 1 PID

			pid 			= msg->argv[0];
			cant_paginas 	= msg->argv[1];

			log_info(log_general_p, "Iniciar Proceso %d con %d paginas",pid,cant_paginas);
			destroy_message(msg);

			st = iniciar_proceso_CPU(pid,cant_paginas);

			switch(st){
				case 0:
					resp = argv_message(MEM_OK, 1 ,0);
					enviar_y_destroy_mensaje(socket, resp);
					log_info(log_general_p, "El proceso %d fue inicializado correctamente",pid);
					break;
				case 1:
					resp = argv_message(MEM_NO_OK, 1 ,0);
					enviar_y_destroy_mensaje(socket, resp);
					log_info(log_general_p, "No hay espacio suficiente para alocar la memoria del proceso %d",pid);
					log_error(log_errores, "No hay espacio suficiente para alocar la memoria del proceso %d",pid);
					break;
			}

			break;

		case MEM_LEER: /////////////
			//param 0 PID / 1 Pagina
			pid 		= msg->argv[0];
			nro_pagina  = msg->argv[1];
			destroy_message(msg);
			log_info(log_memoria, "Leer pagina %d del proceso %d", nro_pagina,pid);
			flag_reemplazo=0;

			//loguear_memoria();
			// BUSCO EL PROCESO
			gl_PID=pid;
			gl_nro_pagina=nro_pagina;
			proceso = list_find(paginas,(void*)es_el_proceso_segun_PID);

			sem_wait(&mutex_PAGINAS);
			// BUSCO EL MARCO EN LA TLB Y EN LA TABLA DE PAGINAS
			b_marco = buscar_marco_de_pagina_en_TLB_y_tabla_paginas(pid,nro_pagina);

			// POSIBLES VALORES = >=0 (posicion en memoria) -1 (no esta en memoria) -2 (no existe la pagina)
			if(b_marco.marco == -2){
				st = 0;
			}else{
				if(b_marco.marco == -1){
					cant_paginas = list_count_satisfying(proceso->paginas,(void*)la_pagina_esta_cargada_en_memoria);
					b_marco.marco = encontrar_marco_libre();
					if(b_marco.marco == -1 /* NO HAY MAS LUGAR*/ && cant_paginas==0 /* NO HAY PAGINAS PARA SACAR*/){
						st=3;
					}else{
						buff_pag = swap_leer_pagina(pid, nro_pagina);
						if(buff_pag != NULL){
							if(cant_paginas<MAXIMO_MARCOS_POR_PROCESO()&& b_marco.marco != -1){
								agregar_pagina_en_memoria(proceso,nro_pagina,buff_pag);
								st = 2;
							}else{
								st = reemplazar_pagina_en_memoria_segun_algoritmo(proceso,nro_pagina,buff_pag);
								flag_reemplazo=1;
							}
							dormir_memoria();
						}else{
							st = 1;
						}
					}
				}
				else {
					dormir_memoria();
					buff_pag = string_duplicate(memoria[b_marco.marco]->contenido);
					// SI ES LRU MUEVO LA PAGINA AL FINAL
					if(string_equals_ignore_case(ALGORITMO_REEMPLAZO(),"LRU")){
						pagina  = list_remove_by_condition(proceso->paginas,(void*)es_la_pagina_segun_PID_y_nro_pagina);
						list_add(proceso->paginas,pagina);
					}
					st = 2;
				}
			}

			sem_post(&mutex_PAGINAS);
			loguear_memoria();

			switch(st){
				case 0:
					resp = argv_message(MEM_NO_OK, 1 ,0);
					log_info(log_memoria, "No existe la pagina %d solicitada por el proceso %d",nro_pagina,pid);
					log_error(log_errores, "No existe la pagina %d solicitada por el proceso %d",nro_pagina,pid);
					break;
				case 1:
					resp = argv_message(MEM_NO_OK, 1 ,0);
					log_info(log_memoria, "La pagina %d del proceso %d no fue recibida bien del SWAP",nro_pagina,pid);
					log_error(log_errores, "La pagina %d del proceso %d no fue recibida bien del SWAP",nro_pagina,pid);
					break;
				case 2:
					resp = string_message(MEM_OK, buff_pag, 0);
					if(flag_reemplazo)
						log_info(log_memoria, "Lectura Correcta --> Pagina: %d - PID: %d - TBL_HIT: %d - Marco: %d",
								nro_pagina,pid, b_marco.TLB_HIT, b_marco.marco);
					else
						log_info(log_memoria, "Lectura Correcta --> Pagina: %d - PID: %d - TBL_HIT: %d - Marco: %d - Algoritmo: %s",
								nro_pagina,pid, b_marco.TLB_HIT, b_marco.marco, ALGORITMO_REEMPLAZO());
					break;
				case 3:
					resp = argv_message(MEM_NO_OK, 1 ,0);
					log_info(log_memoria, "No hay lugar en memoria para guardar la pagina %d del proceso %d",nro_pagina,pid);
					log_error(log_errores, "No hay lugar en memoria para guardar la pagina %d del proceso %d",nro_pagina,pid);
					break;
				case 4:
					resp = argv_message(MEM_NO_OK, 1 ,0);
					log_info(log_memoria, "No se pudo guardar la informacion de la pagina desalojada del proceso %d en SWAP",pid);
					log_error(log_errores, "No se pudo guardar la informacion de la pagina desalojada del proceso %d en SWAP",pid);
					break;
			}

			enviar_y_destroy_mensaje(socket, resp);
			FREE_NULL(buff_pag);

			break;

		case MEM_ESCRIBIR:
			//param 0 PID / 1 Pagina
			buff_pag 	= string_duplicate(msg->stream);
			pid 		= msg->argv[0];
			nro_pagina 	= msg->argv[1];
			destroy_message(msg);
			log_info(log_memoria, "Escribir en Memoria la pagina %d del PID %d y texto: \"%s\"", nro_pagina, pid,buff_pag);
			flag_reemplazo=0;

			//loguear_memoria();
			// BUSCO EL PROCESO y la PAGINA
			sem_wait(&mutex_PAGINAS);
			gl_PID=pid;
			gl_nro_pagina=nro_pagina;
			proceso = list_find(paginas,(void*)es_el_proceso_segun_PID);
			pagina  = list_find(proceso->paginas,(void*)es_la_pagina_segun_PID_y_nro_pagina);

			// BUSCO EL MARCO EN LA TLB Y EN LA TABLA DE PAGINAS
			b_marco = buscar_marco_de_pagina_en_TLB_y_tabla_paginas(pid,nro_pagina);

			if(b_marco.marco == -2){
				st = 0;
			}else{
				if(b_marco.marco == -1){
					cant_paginas = list_count_satisfying(proceso->paginas,(void*)la_pagina_esta_cargada_en_memoria);
					b_marco.marco = encontrar_marco_libre();
					if(b_marco.marco == -1 /* NO HAY MAS LUGAR*/ && cant_paginas==0 /* NO HAY PAGINAS PARA SACAR*/){
						st=3;
					}else{
						// OPCIONAL
						buff_pag_esc = swap_leer_pagina(pid, nro_pagina);
						FREE_NULL(buff_pag_esc);
						// OPCIONAL

						if(buff_pag != NULL){
							if(cant_paginas<MAXIMO_MARCOS_POR_PROCESO()&& b_marco.marco != -1){
								agregar_pagina_en_memoria(proceso,nro_pagina,buff_pag);
								st = 2;
							}else{
								st = reemplazar_pagina_en_memoria_segun_algoritmo(proceso,nro_pagina,buff_pag);
								flag_reemplazo=1;
							}
							pagina->modificado=1;
							dormir_memoria();
						}else{
							st = 1;
						}
					}
				}
				else {
					strncpy(memoria[b_marco.marco]->contenido,buff_pag,TAMANIO_MARCO());
					memoria[b_marco.marco]->contenido[TAMANIO_MARCO()]='\0';
					pagina->modificado=1;
					// SI ES LRU MUEVO LA PAGINA AL FINAL
					if(string_equals_ignore_case(ALGORITMO_REEMPLAZO(),"LRU")){
						list_remove_by_condition(proceso->paginas,(void*)es_la_pagina_segun_PID_y_nro_pagina);
						list_add(proceso->paginas,pagina);
					}
					dormir_memoria();
					st = 2;
				}
			}
			sem_post(&mutex_PAGINAS);
			loguear_memoria();

			// BORRADO DE CODIGO EN BKP

			switch(st){
				case 0:
					resp = argv_message(MEM_NO_OK, 1 ,0);
					log_info(log_memoria, "Escritura Erronea --> No existe la pagina %d solicitada por el proceso %d",nro_pagina,pid);
					log_error(log_errores, "Escritura Erronea --> No existe la pagina %d solicitada por el proceso %d",nro_pagina,pid);
					break;
				case 1:
					resp = argv_message(MEM_NO_OK, 1 ,0);
					log_info(log_memoria, "Escritura Erronea --> La pagina %d del proceso %d no fue recibida bien del SWAP",nro_pagina,pid);
					log_error(log_errores, "Escritura Erronea --> La pagina %d del proceso %d no fue recibida bien del SWAP",nro_pagina,pid);
					break;
				case 2:
					//dormir_memoria();
					resp = argv_message(MEM_OK, 1 ,0);
					if(!flag_reemplazo)
						log_info(log_memoria, "Escritura Correcta --> Pagina: %d - PID: %d - TBL_HIT: %d - Marco: %d",
								nro_pagina,pid, b_marco.TLB_HIT, b_marco.marco);
					else
						log_info(log_memoria, "Escritura Correcta --> Pagina: %d - PID: %d - TBL_HIT: %d - Marco: %d - Algoritmo: %s",
								nro_pagina,pid, b_marco.TLB_HIT, b_marco.marco, ALGORITMO_REEMPLAZO());
					break;
				case 3:
					resp = argv_message(MEM_NO_OK, 1 ,0);
					log_info(log_memoria, "Escritura Erronea --> No hay lugar en memoria para guardar la pagina %d del proceso %d",nro_pagina,pid);
					log_error(log_errores, "Escritura Erronea --> No hay lugar en memoria para guardar la pagina %d del proceso %d",nro_pagina,pid);
					break;
				case 4:
					resp = argv_message(MEM_NO_OK, 1 ,0);
					log_info(log_memoria, "Escritura Erronea --> No se pudo guardar la informacion de la pagina desalojada del proceso %d en SWAP",pid);
					log_error(log_errores, "Escritura Erronea --> No se pudo guardar la informacion de la pagina desalojada del proceso %d en SWAP",pid);
					break;
			}

			enviar_y_destroy_mensaje(socket, resp);
			break;

		case MEM_FINALIZAR:
			//param 0 PID

			gl_PID = msg->argv[0];
			destroy_message(msg);

			log_info(log_general_p, "Finalizar Proceso %d",gl_PID);

			// LE AVISO AL SWAP QUE LIBERE EL ESPACIO
			st = swap_finalizar(gl_PID);
			if(st !=0){
				log_info(log_general_p, "No se pudo finalizar el proceso %d por un problema en el SWAP",gl_PID);
				log_error(log_errores, "No se pudo finalizar el proceso %d por un problema en el SWAP",gl_PID);
				resp = argv_message(MEM_NO_OK, 1, 0);
				enviar_y_destroy_mensaje(socket, resp);
				break;
			}

			loguear_memoria();
			// ELIMINO LAS ESTRUCTURAS
			sem_wait(&mutex_PAGINAS);
			list_remove_and_destroy_by_condition(paginas,(void*)es_el_proceso_segun_PID,(void*)destruir_proceso);
			sem_post(&mutex_PAGINAS);
			loguear_memoria();

			// LE AVISOA LA CPU COMO TERMINO
			log_info(log_general_p, "Se finalizo correctamente el proceso %d",gl_PID);
			resp = argv_message(MEM_OK, 1, 0);
			enviar_y_destroy_mensaje(socket, resp);
			break;


		case CPU_NUEVO:
			break;

		case CAIDA_PLANIFICADOR:
			log_info(log_general_p, "Se cerro el Planificador por lo cual se termina el proceso");
			resp = argv_message(CAIDA_PLANIFICADOR, 1, 0);
			enviar_y_destroy_mensaje(socket_swap, resp);
			exit(0);
			break;

		default:
			log_error(log_errores, "El mensaje %d no es correcto",msg->header.id);
			break;
	}



	//destroy_message(msg);
}

t_busq_marco buscar_marco_de_pagina_en_TLB_y_tabla_paginas(int pid, int nro_pagina){
	t_proceso* proceso;
	t_busq_marco busc_marco;

	gl_PID=pid;
	proceso = list_find(paginas,(void*)es_el_proceso_segun_PID);

	// SE FIJA SI ESTA LA PAGINA EN LA TLB
	if(TLB_HABILITADA()){
		busc_marco.marco = buscar_marco_en_TLB(pid,nro_pagina);
	} else {
		busc_marco.marco = -1;
	}
	proceso->TLB_total++;
	gl_TLB_total++;

	// SE FIJA SI ESTA LA PAGINA EN MEMORIA
	if(busc_marco.marco == -1){
		dormir_memoria();
		busc_marco.marco = buscar_marco_en_paginas(proceso,nro_pagina);
		busc_marco.TLB_HIT=0;
	}else{
		proceso->TLB_hit++;
		gl_TLB_hit++;
		busc_marco.TLB_HIT=1;
	}

	return busc_marco;
}

int encontrar_marco_libre(){ ////////////////
	int i;
	for (i=0;i<CANTIDAD_MARCOS();i++){
		if(memoria[i]->libre)
					return i;
	}

	// EN CASO QUE ESTE LLENA LA MEMORIA
	return -1;
}

void tasa_aciertos_TLB_total(){
	float valor;
	//log_info(log_general_p, "arranco ¬¬");
	while(1){
		//log_info(log_general_p, "me duermo ¬¬");
		gl_TLB_hit=0.0;
		gl_TLB_total = 0;
		sleep(60);
		//log_info(log_general_p, "despierto ¬¬");
		//list_iterate(paginas,(void*)sumar_tasas_TLB);
		//log_info(log_general_p, "a ver que onda: %f - %d",gl_TLB_hit,gl_TLB_total);
		if(gl_TLB_total!=0){
			valor= (gl_TLB_hit/gl_TLB_total)*100;
			log_info(log_general_p, "TLB - Tasa de aciertos: %f",valor);
			log_info(log_estadisticas, "TLB - Tasa de aciertos: %f",valor);
		} else {
			log_info(log_general_p, "TLB - Sin pedidos ultimo minuto");
			log_info(log_estadisticas, "TLB - Sin pedidos ultimo minuto");
		}
	}
}

void sumar_tasas_TLB(t_proceso* proceso){
	gl_TLB_hit =+ proceso->TLB_hit;
	gl_TLB_total =+ proceso->TLB_total;
}

void tasa_aciertos_TLB(t_proceso* proceso){
	float valor;
	valor= (proceso->TLB_hit/proceso->TLB_total)*100;
	log_info(log_general_p, "Tasa de aciertos para el proceso %d: %f",proceso->PID,valor);
	log_info(log_estadisticas, "Tasa de aciertos para el proceso %d: %f",proceso->PID,valor);
}


void agregar_pagina_en_memoria(t_proceso* proceso, int nro_pagina, char* contenido){ ///////////////////

	// BUSCO UN MARCO LIBRE
	int marco = encontrar_marco_libre();

	// AGREGO EL CONTENIDO A MEMORIA
	strncpy(memoria[marco]->contenido,contenido,TAMANIO_MARCO());
	memoria[marco]->contenido[TAMANIO_MARCO()]='\0';
	memoria[marco]->libre = 0;

	// BUSCO LA PAGINA EN LA TABLA DE PROCESOS
	gl_nro_pagina = nro_pagina;
	gl_PID 		  = proceso->PID;

	// PONGO LA PAGINA AL FINAL DE LA LISTA DE PAGINAS DEL PROCESO
	t_pagina* pagina = list_remove_by_condition(proceso->paginas,(void*)es_la_pagina_segun_PID_y_nro_pagina);
	list_add(proceso->paginas,pagina);

	// ACTUALIZO ATRIBUTOS
	pagina->marco=marco;
	pagina->presencia=1;
	pagina->usado=1;


	// AGREGO LA PAGINA EN LA TLB, SIEMPRE Y CUANDO ESTE HABILITADA Y YA NO EXISTA LA PAGINA
	sem_wait(&mutex_TLB);
	if(TLB_HABILITADA() && !list_any_satisfy(TLB,(void*)es_la_pagina_segun_PID_y_nro_pagina)){
		// SI ESTA LLENA SACO EL ULTIMO AGREGADO
		if(TLB->elements_count==ENTRADAS_TLB()){
			list_remove(TLB,0);
		}
		list_add(TLB,pagina);
	}
	sem_post(&mutex_TLB);
}

int quitar_pagina_de_la_memoria(t_pagina* pag){
	int st;
	// CHEQUEO SI ESTA MODIFICADA
	if(pag->modificado)
		st = swap_escribir_pagina(pag->PID, pag->pagina, memoria[pag->marco]->contenido);
	if(st==-1) return 0;

	// ME FIJO SI ESTA EN LA TLB y LA SACO
	gl_nro_pagina = pag->pagina;

	sem_wait(&mutex_TLB);
	if(list_any_satisfy(TLB,(void*)es_la_pagina_segun_PID_y_nro_pagina))
		list_remove_by_condition(TLB,(void*)es_la_pagina_segun_PID_y_nro_pagina);
	sem_post(&mutex_TLB);

	// ACTUALIZO LA MEMORIA Y LA TABLA DE PAGINAS
	if(pag->marco!=-1)
		memoria[pag->marco]->libre = 1;
	pag->marco = -1;
	pag->modificado = 0;
	pag->presencia = 0;
	pag->usado = 0;
	return 1;

}

int reemplazar_pagina_en_memoria_segun_algoritmo(t_proceso* proceso, int pagina, char* contenido){
	gl_PID		  = proceso->PID;
	int st;
	t_pagina* pag;

	// BUSCO LA PAGINA "VICTIMA" PARA QUITAR DE MEMORIA
	if(string_equals_ignore_case(ALGORITMO_REEMPLAZO(),"FIFO")||
			string_equals_ignore_case(ALGORITMO_REEMPLAZO(),"LRU")){
		pag = mover_y_devolver_primer_pagina_al_final_de_la_lista(proceso->paginas);
	} else {
		pag = buscar_pagina_victima_CLOCK(proceso->paginas);
	}

	// SACO LA PAGINA DE MEMORIA
	st = quitar_pagina_de_la_memoria(pag);

	// AGREGO LA NUEVA PAGINA
	agregar_pagina_en_memoria(proceso, pagina, contenido);

	if(st==0)
		st=4;
	else
		st=2;

	return st;
}

t_pagina* buscar_pagina_victima_CLOCK(t_list* lista_paginas){
	t_pagina* pag;				// PAGINA VICTIMA
	int flag_match = 0; 		// FLAG QUE INDICA CUANDO ENCONTRO A LA VICTIMA
	int flag_primer_vuelta = 1; // FLAG QUE CUAL ES LA PRIMER PAGINA
	int flag_find_00 = 1;		// FLAG QUE INDICA SI ESTAMOS BUSCANDO "00" (SIN USAR NI MODIFICAR)
	int flag_find_01 = 0;		// FLAG QUE INDICA SI ESTAMOS BUSCANDO "01" (SIN USAR MODIFICADO)
	int nro_pagina_inicial;		// PRIMER PAGINA PARA SABER CUANDO BUSCAR "01"

	while(!flag_match){
		pag = mover_y_devolver_primer_pagina_al_final_de_la_lista(lista_paginas);

		if(flag_primer_vuelta){
			nro_pagina_inicial = pag->pagina;
			flag_primer_vuelta = 0;
		} else {
			if(nro_pagina_inicial == pag->pagina){
				if(flag_find_00){
					flag_find_00 = 0;
					flag_find_01 = 1;
				} else {
					flag_find_00 = 1;
					flag_find_01 = 0;
				}
			}
		}

		if(flag_find_00 && !pag->usado && !pag->modificado && pag->presencia)
			flag_match = 1;

		if(flag_find_01){
			if(!pag->usado && pag->modificado && pag->presencia)
				flag_match = 1;
			else
				pag->usado = 0;
		}

	}
	return(pag);
}

t_pagina* mover_y_devolver_primer_pagina_al_final_de_la_lista(t_list* lista_paginas){
	t_pagina* pag;
	pag = list_remove_by_condition(lista_paginas,(void*)la_pagina_esta_cargada_en_memoria);
	list_add(lista_paginas,pag);
	return(pag);
}


int buscar_marco_en_TLB(int PID,int nro_pagina){
	t_pagina* pagina;
	gl_PID=PID;
	gl_nro_pagina=nro_pagina;
	sem_wait(&mutex_TLB);
	if(list_any_satisfy(TLB,(void*)es_la_pagina_segun_PID_y_nro_pagina)){
		pagina = list_find(TLB,(void*)es_la_pagina_segun_PID_y_nro_pagina);
		sem_post(&mutex_TLB);
		return pagina->marco;
	} else {
		sem_post(&mutex_TLB);
		return -1;
	}
}

int buscar_marco_en_paginas(t_proceso* proceso,int nro_pagina){
	t_pagina* pagina;
	gl_PID=proceso->PID;
	gl_nro_pagina=nro_pagina;
	if(list_any_satisfy(proceso->paginas,(void*)es_la_pagina_segun_PID_y_nro_pagina)){
		pagina = list_find(proceso->paginas,(void*)es_la_pagina_segun_PID_y_nro_pagina);
		return pagina->marco;
	} else {
		return -2;
	}
}

int iniciar_proceso_CPU(int pid, int paginas){ ///////////////////
	int st;

	// LE ENVIO AL PROCESO SWAP PARA QUE RESERVE EL ESPACIO
	st = swap_nuevo_proceso(pid, paginas);

	if(st==0){
		crear_estructuras_de_un_proceso(pid,paginas);
		return 0;
	}
	else{
		return 1;
	}

}

int es_la_pagina_segun_PID(t_pagina* pagina){
	return(pagina->PID==gl_PID);
}

int es_el_proceso_segun_PID(t_proceso* proceso){
	return(proceso->PID==gl_PID);
}

int es_la_pagina_segun_PID_y_nro_pagina(t_pagina* pagina){
	return(pagina->PID==gl_PID && pagina->pagina==gl_nro_pagina);
}

int la_pagina_esta_cargada_en_memoria(t_pagina* pagina){
	return(pagina->presencia);
}


void crear_estructuras_de_un_proceso(int PID,int cant_paginas){ ///////////////////
	int i;
	t_proceso* 	new_proceso 	 = (t_proceso*)malloc(sizeof(t_proceso));
	t_pagina*  	new_pagina;

	// ASOCIO EL PID Y CREO LA LISTA DE PAGINAS
	new_proceso->PID = PID;
	new_proceso->paginas = list_create();
	new_proceso->TLB_hit = 0.0;
	new_proceso->TLB_total = 0;

	// AGREGO A LA LISTA CADA PAGINA
	for(i=0;i<cant_paginas;i++){
		new_pagina = crear_pagina(PID,i,-1);
		list_add(new_proceso->paginas,new_pagina);
	}

	// AGREGO EL PROCESO A LA TABLA DE PAGINAS
	list_add(paginas,new_proceso);
}



t_pagina* crear_pagina(int PID, int pagina, int marco){
	t_pagina* new_pagina = (t_pagina*)malloc(sizeof(t_pagina));
	new_pagina->PID = PID;
	new_pagina->marco = marco;
	new_pagina->pagina = pagina;
	new_pagina->modificado = 0;
	new_pagina->presencia=0;
	new_pagina->usado=0;
	return new_pagina;
}


void destruir_pagina(t_pagina* pagina){
	if(pagina->marco!=-1)
		memoria[pagina->marco]->libre=1;
	free(pagina);
}

void destruir_proceso(t_proceso* proceso){
	gl_PID = proceso->PID;

	// SACO LAS POSIBLES PAGINAS DE LA TLB
	sem_wait(&mutex_TLB);
	list_remove_by_condition(TLB,(void*)es_la_pagina_segun_PID);
	sem_post(&mutex_TLB);

	// IMPRIMO LAS ESTADISTICAS DE ACIERTO DE TLB DEL PROCESO
	tasa_aciertos_TLB(proceso);

	// ELIMINO LAS PAGINAS EN LA TABLA DE PAGINAS
	list_destroy_and_destroy_elements(proceso->paginas,(void*)destruir_pagina);
	free(proceso);
}


void dormir_memoria(){

	int retardo = RETARDO_MEMORIA();

	if (retardo == 0) {
		usleep(RETARDO_MEMORIA_MINIMO()*1000);
	}else{
		sleep(RETARDO_MEMORIA());
	}

}


void loguear_memoria(){
	int i;
	for(i=0;i<CANTIDAD_MARCOS();i++){
		if(memoria[i]->libre)
			log_info(log_print_mem,"Marco: %d	Contenido: <vacio> Flag: %d",i,memoria[i]->libre);
		else
			log_info(log_print_mem,"Marco: %d	Contenido: %s",i,memoria[i]->contenido,memoria[i]->libre);
	}
	log_info(log_print_mem,"---------------------------------------------------");
	log_info(log_print_mem,"---------------------------------------------------");
}
