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
#include <util.h>

pthread_t th_server_cpu;
int socket_swap;

int main(void) {
	inicializar();
	socket_swap = conectar_con_swap();
	server_socket_select(PUERTO_ESCUCHA(), procesar_mensaje_cpu);
	finalizar();
	return EXIT_SUCCESS;
}

int inicializar(){

	// ARCHIVO DE CONFIGURACION
	cfg = config_create(CONFIG_PATH);

	// ARCHIVO DE LOG
	clean_file(LOGGER_PATH);
	logger = log_create(LOGGER_PATH, "procesoAdminMem", true, LOG_LEVEL_TRACE);

	// ESTRUCTURA MEMORIA
	memoria = list_create();

	// ESTRUCTURA TLB
	TLB 	= list_create();

	// ESTRUCTURA PAGINAS
	paginas = list_create();

	return 0;
}

int finalizar(){

	// ARCHIVO DE CONFIGURACION
	config_destroy(cfg);

	// ARCHIVO DE LOG
	log_destroy(logger);

	// ESTRUCTURA TLB y MEMORIA
	list_destroy_and_destroy_elements(TLB,(void*)destruir_pagina);
	list_destroy_and_destroy_elements(paginas,(void*)destruir_pagina);
	list_destroy_and_destroy_elements(memoria,(void*)destruir_memoria);

	return 0;
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
	//print_msg(msg);
	char* buff_pag  = NULL;
	char* buff_pag_esc  = NULL;
	int st ;
	int pid;
	int nro_pagina;
	int cant_paginas;
	int entrada;
	t_msg* resp = NULL;
	t_memoria* pagina_en_memoria;
	int flag_TLB;


//	t_pagina* pagina;

	switch (msg->header.id) {
		case MEM_INICIAR:
			//param 0 cant_paginas
			//param 1 PID

			pid 	= msg->argv[0];
			cant_paginas = msg->argv[1];

			log_trace(logger, "Iniciar Proceso %d con %d paginas",pid,cant_paginas);
			destroy_message(msg);

			st = iniciar_proceso_CPU(pid,cant_paginas);

			switch(st){
				case 0:
					resp = argv_message(MEM_OK, 1 ,0);
					enviar_y_destroy_mensaje(socket, resp);
					log_info(logger, "El proceso %d fue inicializado correctamente",pid);
					break;
				case 1:
					resp = argv_message(MEM_NO_OK, 1 ,0);
					enviar_y_destroy_mensaje(socket, resp);
					log_warning(logger, "No hay espacio suficiente para alocar la memoria del proceso %d",pid);
					break;
			}

			break;

		case MEM_LEER:
			//param 0 PID / 1 Pagina
			pid 		= msg->argv[0];
			nro_pagina  = msg->argv[1];
			destroy_message(msg);
			log_trace(logger, "Leer pagina %d del proceso", nro_pagina,pid);

			// SE FIJA SI ESTA LA PAGINA EN LA TLB
			/*if(TLB_HABILITADA()){
				entrada = buscar_pagina_en_TLB(pid,nro_pagina);
				flag_TLB = 1;
			} else {
				entrada = -1;
				flag_TLB = 0;
			}
			gl_TLB_total++;

			// SE FIJA SI ESTA LA PAGINA EN MEMORIA
			if(entrada == -1){
				sleep(RETARDO_MEMORIA());
				entrada = buscar_pagina_en_paginas(pid,nro_pagina);
			}else{
				gl_TLB_acierto++;
			}

			// POSIBLES VALORES = >=0 (posicion en memoria) -1 (no esta en memoria) -2 (no existe la pagina)
			if(entrada == -2){
				st = 0;
			}else{
				if(entrada == -1){
					if(memoria->elements_count<CANTIDAD_MARCOS()){*/
						buff_pag = swap_leer_pagina(pid, nro_pagina);
						if(buff_pag != NULL){
							// ME FIJO CUANTAS PAGINAS TIENE EN MEMORIA
							/*gl_PID=pid;
							gl_nro_pagina=nro_pagina;
							cant_paginas = list_count_satisfying(paginas,(void*)es_la_pagina_segun_PID_y_nro_pagina);

							if(cant_paginas<MAXIMO_MARCOS_POR_PROCESO()){
								agregar_pagina_en_memoria(pid,nro_pagina,buff_pag);
								*/st = 2;
							}else{/*
								st = reemplazar_pagina_en_memoria_segun_algoritmo(pid,nro_pagina,buff_pag);
							}
							sleep(RETARDO_MEMORIA());
						}else{*/
							st = 1;
						}/*
					}else{
						st=3;
					}
				}
				else {
					sleep(RETARDO_MEMORIA());
					buff_pag = string_duplicate(usar_pagina_en_memoria_segun_algoritmo(pid,nro_pagina,entrada,flag_TLB));
					st = 2;
				}
			}

*/
			switch(st){
				case 0:
					resp = argv_message(MEM_NO_OK, 1 ,0);
					log_warning(logger, "No existe la pagina %d solicitada por el proceso %d",nro_pagina,pid);
					break;
				case 1:
					resp = argv_message(MEM_NO_OK, 1 ,0);
					log_error(logger, "La pagina %d del proceso %d no pudo ser cargada en memoria",nro_pagina,pid);
					break;
				case 2:
					//sleep(RETARDO_MEMORIA());
					resp = string_message(MEM_OK, buff_pag, 0);
					log_info(logger, "La pagina %d del proceso %d fue leida correctamente",nro_pagina,pid);
					break;
				case 3:
					resp = argv_message(MEM_NO_OK, 1 ,0);
					log_error(logger, "No hay lugar en memoria para guardar la pagina %d del proceso %d",nro_pagina,pid);
					break;
				case 4:
					resp = argv_message(MEM_NO_OK, 1 ,0);
					log_error(logger, "No se pudo guardar la informacion de la pagina desalojada del proceso %d en SWAP",pid);
					break;
			}

			enviar_y_destroy_mensaje(socket, resp);
			FREE_NULL(buff_pag);

			break;

		case MEM_ESCRIBIR:
			//param 0 PID / 1 Pagina
			buff_pag_esc 	= string_duplicate(msg->stream);
			pid 		= msg->argv[0];
			nro_pagina 	= msg->argv[1];
			destroy_message(msg);
			log_trace(logger, "Escribir en Memoria la pagina %d del PID %d y texto: \"%s\"", nro_pagina, pid,buff_pag);

			// SE FIJA SI ESTA LA PAGINA EN LA TLB
			if(TLB_HABILITADA()){
				entrada = buscar_pagina_en_TLB(pid,nro_pagina);
				flag_TLB = 1;
			} else {
				entrada = -1;
				flag_TLB = 0;
			}
			gl_TLB_total++;

			// SE FIJA SI ESTA LA PAGINA EN MEMORIA
			if(entrada==-1){
				sleep(RETARDO_MEMORIA());
				entrada = buscar_pagina_en_paginas(pid,nro_pagina);
			}else{
				gl_TLB_acierto++;
			}

			if(entrada == -2){
				st = 0;
				log_error(logger, "No existe la pagina %d del proceso %d",nro_pagina,pid);
			}else{
				if(entrada == -1){
					if(memoria->elements_count<CANTIDAD_MARCOS()){
						// OPCIONAL
							buff_pag_esc = swap_leer_pagina(pid, nro_pagina);
							FREE_NULL(buff_pag_esc);
						// OPCIONAL
						gl_PID=pid;
						gl_nro_pagina=nro_pagina;
						cant_paginas = list_count_satisfying(paginas,(void*)es_la_pagina_segun_PID_y_nro_pagina);

						if(cant_paginas<MAXIMO_MARCOS_POR_PROCESO()){
							agregar_pagina_en_memoria(pid,nro_pagina,buff_pag);
							st = 1;
						}else{
							st = reemplazar_pagina_en_memoria_segun_algoritmo(pid,nro_pagina,buff_pag);
						}

					}else{
						st=0;
						log_error(logger, "No hay lugar en memoria para guardar la pagina %d del proceso %d",nro_pagina,pid);
					}
				}
				else {
					sleep(RETARDO_MEMORIA());
					usar_pagina_en_memoria_segun_algoritmo(pid,nro_pagina,entrada,flag_TLB);
					pagina_en_memoria = list_get(memoria,entrada);
					free(pagina_en_memoria->contenido);
					pagina_en_memoria->contenido=buff_pag;
					st = 1;
					log_info(logger, "La pagina %d del proceso %d fue modificada exitosamente",nro_pagina,pid);
				}
				setear_flag_modificado(pid,nro_pagina);
			}

			if(st==1||st==2){
				resp = argv_message(MEM_OK, 1, 0);
			}else{
				resp = argv_message(MEM_NO_OK, 1, 0);
			}

			enviar_y_destroy_mensaje(socket, resp);
			break;

		case MEM_FINALIZAR:
			//param 0 PID
			log_info(logger, "MEM_FINALIZAR");

			gl_PID = msg->argv[0];
			destroy_message(msg);

			// LE AVISO AL SWAP QUE LIBERE EL ESPACIO
			st = swap_finalizar(gl_PID);
			if(st ==0){
				resp = argv_message(MEM_OK, 1, 0);
				enviar_y_destroy_mensaje(socket, resp);
				break;
			}

			// ELIMINO LAS ESTRUCTURAS

/*			list_remove_and_destroy_by_condition(memoria,(void*)es_la_memoria_segun_PID,(void*)destruir_memoria);
			list_remove_and_destroy_by_condition(TLB,(void*)es_la_pagina_segun_PID,(void*)destruir_pagina);
			list_remove_and_destroy_by_condition(paginas,(void*)es_la_pagina_segun_PID,(void*)destruir_pagina);
			list_iterate(TLB,(void*)recalcular_entrada);
			list_iterate(paginas,(void*)recalcular_entrada);*/

			// LE AVISOA LA CPU COMO TERMINO
			resp = argv_message(MEM_OK, 1, 0);
			enviar_y_destroy_mensaje(socket, resp);
			break;

		default:
			log_warning(logger, "LA OPCION SELECCIONADA NO ESTA REGISTRADA");
			break;
	}



	//destroy_message(msg);
}

void tasa_aciertos_TLB(){
	float valor;
	valor= (gl_TLB_acierto/gl_TLB_total)*100;
	log_info(logger, "TLB - Tasa de aciertos: %f",valor);
}


void agregar_pagina_en_memoria(int PID, int pagina, char* contenido){
	gl_PID=PID;
	gl_nro_pagina=pagina;

	// AGREGO LA PAGINA EN MEMORIA
	t_memoria* pagina_en_memoria;
	pagina_en_memoria = crear_memoria(PID,pagina,1,contenido);
	list_add(memoria,pagina_en_memoria);

	// ACTUALIZO EL INDICE DE LA PAGINA EN LA TABLA DE PAGINAS
	t_pagina* pagina_tabla;
	int entrada = memoria->elements_count-1;
	pagina_tabla = list_find(paginas,(void*)es_la_pagina_segun_PID_y_nro_pagina);
	pagina_tabla->entrada = entrada;

	// ACTUALIZO LA TLB
	agregar_pagina_en_TLB(PID,pagina,entrada);
}


int reemplazar_pagina_en_memoria_segun_algoritmo(int PID, int pagina, char* contenido){
	gl_PID=PID;
	t_memoria* pagina_en_memoria;
	int st;
	t_pagina* pag;

	if(string_equals_ignore_case(ALGORITMO_REEMPLAZO(),"FIFO")){
		// BUSCO LA PAGINA PARA ELIMINAR DE MEMORIA
		pagina_en_memoria = list_find(memoria,(void*)es_la_memoria_segun_PID);

		// ME FIJO SI ESTA MODIFICADA Y LA GUARDO EN SWAP
		if(pagina_en_memoria->modificado)
			st = swap_escribir_pagina(PID, pagina_en_memoria->pagina, pagina_en_memoria->contenido);

		if(st==-1) return 4;

		// ME FIJO SI ESTA EN LA TLB y LA SACO
		gl_nro_pagina = pagina_en_memoria->pagina;
		list_remove_and_destroy_by_condition(TLB,(void*)es_la_pagina_segun_PID_y_nro_pagina,(void*)destruir_pagina);

		// LA BUSCO EN la TABLA DE PAGINAS y ACTUALIZO LAS ENTRADAS
		pag = list_find(paginas,(void*)es_la_pagina_segun_PID_y_nro_pagina);
		gl_entrada=pag->entrada;
		pag->entrada=-1;
		list_iterate(paginas,(void*)actualizar_entradas_en_tabla_de_paginas);

		// LA ELIMINO DE MEMORIA
		list_remove_and_destroy_element(memoria,gl_entrada,(void*)destruir_memoria);

	}

	agregar_pagina_en_memoria(PID, pagina, contenido);
	return 2;
}



void agregar_pagina_en_TLB(int PID, int pagina, int entrada){
	t_pagina* new_pagina;
	new_pagina = crear_pagina(PID,pagina,entrada);
	if(TLB->elements_count==ENTRADAS_TLB())
		list_remove_and_destroy_element(TLB,0,(void*)destruir_pagina);
	list_add(TLB,new_pagina);
}

char* usar_pagina_en_memoria_segun_algoritmo(int PID, int nro_pagina, int entrada,int flag_TLB){
	// ACTUALIZO LA MEMORIA y LA TABLA DE PAGINAS
	t_memoria* pagina_en_memoria;
	pagina_en_memoria = list_remove(memoria,entrada);
	gl_entrada=entrada;
	list_iterate(paginas,(void*)actualizar_entradas_en_tabla_de_paginas);
	list_add(memoria,pagina_en_memoria);

	// ACTUALIZO LA TLB
	t_pagina* pagina;
	gl_PID = PID;
	gl_nro_pagina=nro_pagina;
	if(flag_TLB){
		pagina = list_remove_by_condition(TLB,(void*)es_la_pagina_segun_PID_y_nro_pagina);
		list_add(TLB,pagina);
	}
	return(pagina_en_memoria->contenido);
}

void actualizar_entradas_en_tabla_de_paginas(t_pagina* pagina){
	if(pagina->entrada>gl_entrada)
		pagina->entrada--;
}

void recalcular_entrada(t_pagina* pagina){
	if(pagina->entrada!=-1){
		t_memoria* pag_en_mem;
		int i;
		for(i=0;i<memoria->elements_count;i++);{
			pag_en_mem = list_get(memoria,i);
			if(pag_en_mem->PID == pagina->PID && pag_en_mem->pagina==pagina->pagina)
				pagina->entrada = i;
		}
	}
}


void setear_flag_modificado(int pid, int nro_pagina){
	t_memoria* pagina;
	gl_PID=pid;
	gl_nro_pagina=nro_pagina;
	if(list_any_satisfy(memoria,(void*)es_la_memoria_segun_PID_y_pagina)){
		pagina = list_find(memoria,(void*)es_la_memoria_segun_PID_y_pagina);
		pagina->modificado=1;
	}
}

int es_la_memoria_segun_PID_y_pagina(t_memoria* pagina){
	return(pagina->PID==gl_PID && pagina->pagina == gl_nro_pagina);
}

int buscar_pagina_en_TLB(int PID,int nro_pagina){
	t_pagina* pagina;
	gl_PID=PID;
	gl_nro_pagina=nro_pagina;
	if(list_any_satisfy(TLB,(void*)es_la_pagina_segun_PID_y_nro_pagina)){
		pagina = list_find(TLB,(void*)es_la_pagina_segun_PID_y_nro_pagina);
		return pagina->entrada;
	} else {
		return -1;
	}
}

int buscar_pagina_en_paginas(int PID,int nro_pagina){
	t_pagina* pagina;
	gl_PID=PID;
	gl_nro_pagina=nro_pagina;
	if(list_any_satisfy(paginas,(void*)es_la_pagina_segun_PID_y_nro_pagina)){
		pagina = list_find(paginas,(void*)es_la_pagina_segun_PID_y_nro_pagina);
		return pagina->entrada;
	} else {
		return -2;
	}
}

int iniciar_proceso_CPU(int pid, int paginas){
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


int es_la_memoria_segun_PID(t_memoria* memoria){
	return(memoria->PID==gl_PID);
}

int es_la_pagina_segun_PID(t_pagina* pagina){
	return(pagina->PID==gl_PID);
}

int es_la_pagina_segun_PID_y_nro_pagina(t_pagina* pagina){
	return(pagina->PID==gl_PID && pagina->pagina==gl_nro_pagina);
}


void crear_estructuras_de_un_proceso(int PID,int cant_paginas){
	int i;
	t_pagina* new_pagina;

	for(i=0;i<cant_paginas;i++){
		new_pagina = crear_pagina(PID,i,-1);
		list_add(paginas,new_pagina);
	}

}



t_pagina* crear_pagina(int PID, int pagina, int entrada){
	t_pagina* new_pagina = (t_pagina*)malloc(sizeof(t_pagina));
	new_pagina->PID = PID;
	new_pagina->entrada = entrada;
	new_pagina->pagina = pagina;
	return new_pagina;
}

t_memoria* crear_memoria(int PID, int pagina, int modificado, char* contenido){
	t_memoria* new_memoria = (t_memoria*)malloc(sizeof(t_memoria));
	new_memoria->PID = PID;
	new_memoria->modificado = modificado;
	new_memoria->pagina = pagina;
	new_memoria->contenido=strdup(contenido);
	return new_memoria;
}

void destruir_pagina(t_pagina* pagina){
	free(pagina);
}

void destruir_memoria(t_memoria* memoria){
	free(memoria->contenido);
	free(memoria);
}

/*
void borrar_paginas_de_un_proceso_en_la_memoria(t_pagina* pagina){
	if(pagina->PID==gl_PID && pagina->entrada){
		list_remove_and_destroy_element(memoria,pagina->entrada,(void*)destruir_memoria);
	}
}

void compactar_lista_memoria(){
	t_list* aux = list_create(); //CREO LISTA COMO BKP
	list_add_all(aux,memoria);   //MUEVO TOD O LO QUE ESTA EN MEMORIA A LA AUX, ORDENANDOLO
	list_clean(memoria);		 //LIMPIO LA MEMORIA
	list_add_all(memoria,aux);	 //VUELVO A METER TOD O EN LA MEMORIA ORDENADO
}
*/

/*
void borrar_paginas_de_un_proceso_en_la_memoria(t_pagina* pagina){
	if(pagina->PID==gl_PID && pagina->entrada){
		list_remove_and_destroy_element(memoria,pagina->entrada,(void*)destruir_memoria);
	}
}

void compactar_lista_memoria(){
	t_list* aux = list_create(); //CREO LISTA COMO BKP
	list_add_all(aux,memoria);   //MUEVO TOD O LO QUE ESTA EN MEMORIA A LA AUX, ORDENANDOLO
	list_clean(memoria);		 //LIMPIO LA MEMORIA
	list_add_all(memoria,aux);	 //VUELVO A METER TOD O EN LA MEMORIA ORDENADO
}
*/
/*void eliminar_estructuras_de_un_proceso(t_proceso* proceso){
	int i;
	t_pagina_proceso pagina;

	// DEJO DISPONIBLE TODAS LAS ENTRADAS USADAS POR EL PROCESO QUE TIENE EN LA MEMORIA
	for(i=0;i<proceso->cant_paginas;i++){
		pagina = list_get(proceso->paginas,i);
		if(pagina.posicion_memoria!=-1){
				memoria[pagina.posicion_memoria].libre=1;
				memoria[pagina.posicion_memoria].modificado=0;
			}
		if(pagina.posicion_TLB!=-1){
				quitar_de_la_lista_de_prioridad_TLB(proceso->PID,i);
			}
	}

	free(proceso);
}

void quitar_de_la_lista_de_prioridad_TLB(int PID, int pagina){
	int i;
	for(i=0;i<ENTRADAS_TLB();i++){
		if(TLB[i].PID == PID && TLB[i].pagina == pagina){
			TLB[i].PID = -1;
			TLB[i].entrada = -1;
			TLB[i].orden_seleccion = 0;
			TLB[i].pagina = -1;
		} else
			TLB[i].orden_seleccion++;
	}
}

void eliminar_estructuras_de_todos_los_procesos(t_proceso* proceso){
	int i;
	t_pagina_proceso pagina;

	// DEJO DISPONIBLE TODAS LAS ENTRADAS USADAS POR EL PROCESO QUE TIENE EN LA MEMORIA
	for(i=0;i<proceso->cant_paginas;i++){
			pagina = list_get(proceso->paginas,i);
			if(pagina.posicion_memoria!=-1){
					memoria[pagina.posicion_memoria].libre=1;
					memoria[pagina.posicion_memoria].modificado=0;
				}
			if(pagina.posicion_TLB!=-1){
					quitar_de_la_lista_de_prioridad_TLB(proceso->PID,i);
				}
		}

	// LIBERO EL ESPACIO EN EL SWAP
	swap_finalizar(proceso->PID);

	free(proceso);
}

int escribir_pagina(int pid, int pagina, char* contenido){
	int estado;
	t_proceso proceso;
	int entrada;
	int pos_tlb;

	// LE MANDO AL SWAP PARA QUE GUARDE LA INFO
	estado = swap_escribir_pagina(pid, pagina, contenido);
	if(estado==-1) return estado;

	if(string_equals_ignore_case("FIFO",ALGORITMO_REEMPLAZO())){
		entrada = encontrar_pagina_libre_FIFO(pid);
	} else {
		//COMPLETAR CON EL RESTO DE LOS ALGORITMOS
		entrada = 0;
	}

	memoria[entrada].libre = 0;
	memoria[entrada].modificado = 0;
	memoria[entrada].bloque = contenido;

	entrada = encontrar_pagina_en_TLB();
	TLB[pos_tlb].PID = pid;
	TLB[pos_tlb].entrada = entrada;
	TLB[pos_tlb].pagina = pagina;
	TLB[pos_tlb].


	return 1;
}*/
