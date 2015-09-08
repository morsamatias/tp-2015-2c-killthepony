/*
 ============================================================================
 Name        : procesoSwap.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "procesoSwap.h"

int main(void) {
	inicializar();

	server_socket_select(PUERTO_ESCUCHA(), procesar_mensaje_mem);

	finalizar();
	return EXIT_SUCCESS;
}
char* swap_inicializar() {

	TAMANIO_SWAP = CANTIDAD_PAGINAS() * TAMANIO_PAGINA();
	char CREAR_DATA[1024];
	sprintf(CREAR_DATA, "dd if=/dev/zero of=%s bs=%d count=%d", NOMBRE_SWAP(), TAMANIO_PAGINA(), CANTIDAD_PAGINAS());
	printf("%s\n", CREAR_DATA);
	system(CREAR_DATA);

	log_trace(logger, "Creado archivo swap %s de %d paginas de tamaño %d", NOMBRE_SWAP(), CANTIDAD_PAGINAS(), TAMANIO_PAGINA());

	return file_get_mapped(NOMBRE_SWAP());
}

void swap_destroy() {
	munmap(swap, TAMANIO_SWAP);
//mapped = NULL;
}

int inicializar(){

	cfg = config_create(CONFIG_PATH);

	clean_file(LOGGER_PATH);
	logger = log_create(LOGGER_PATH, "procesoSwap", true, LOG_LEVEL_TRACE);
	pthread_mutex_init(&mutex, NULL);

	swap = swap_inicializar();


	esp_ocupado =  list_create();

	esp_libre =  list_create();
	t_libre* libre = malloc(sizeof(t_libre));
	libre->posicion = 0;
	libre->cantidad = CANTIDAD_PAGINAS();

	list_add(esp_libre, libre);




	return 0;
}

int finalizar(){

	config_destroy(cfg);
	log_destroy(logger);
	swap_destroy();

	list_destroy(esp_libre);
	list_destroy(esp_ocupado);
	return 0;
}

/*
 * si no encuentra nada devuelve -1;
 */
int swap_buscar_hueco_libre(paginas){

	bool _swap_buscar_hueco_libre(t_libre* libre){
		return libre->cantidad >= paginas;
	}

	t_libre* l;
	l = list_find(esp_libre, (void*)_swap_buscar_hueco_libre);
	if(l==NULL){
		return -1;
	}else{
		return l->posicion;
	}

}

t_libre* swap_buscar_hueco_que_empiece_en(int pagina){
	bool _swap_buscar_libre(t_libre* libre){
		return libre->posicion == pagina;
	}
	return list_find(esp_libre, (void*)_swap_buscar_libre);
}

int swap_ocupar(int pid, int pagina, int paginas){
	//primero agrego a la lista de espacio ocupado lo que pide el proceso
	t_ocupado* ocupado = malloc(sizeof(t_ocupado));
	ocupado->pid = pid;
	ocupado->posicion = pagina;
	ocupado->cantidad = paginas	;
	list_add(esp_ocupado, ocupado);

	//ahora agrego a la lista de espacio libre
	t_libre* libre = NULL;
	//siempre exisste el hueco que empieza en la pagina
	libre = swap_buscar_hueco_que_empiece_en(pagina);

	libre->posicion = libre->posicion + paginas;
	libre->cantidad = libre->cantidad - paginas;


	return 0;
}

int swap_nuevo_proceso(int pid, int paginas){
	int comienzo = 0;
	comienzo = swap_buscar_hueco_libre(paginas);
	if(comienzo>=0){
		swap_ocupar(pid, comienzo, paginas);
		return 0;
	}else{
		log_trace(logger, "pid: %d, paginas: %d. No hay hueco libre para el proceso", pid, paginas);
		return -1;
	}

}

int swap_escribir(int pid, int pagina, char* contenido){
	size_t size = strlen(contenido);
	memcpy(swap + (TAMANIO_PAGINA() * pagina), contenido, size);
	//memcpy

	memset(swap + (TAMANIO_PAGINA() * pagina)+size, '\0', TAMANIO_PAGINA() - size);

	return 0;
}

/*
 * devuelve una copia del contenido de la pagina;
 */
char* swap_leer(int pid, int pagina){
	char* contenido = malloc(TAMANIO_PAGINA());
	memset(contenido, '\0', TAMANIO_PAGINA());

	memcpy(contenido, swap + (TAMANIO_PAGINA() * pagina), TAMANIO_PAGINA());

	return contenido;
}

t_ocupado* swap_buscar_ocupado_por_pid(pid){
	bool _swap_buscar_ocupado_por_pid(t_ocupado* ocupado){
		return ocupado->pid == pid;
	}

	return list_find(esp_ocupado, (void*)_swap_buscar_ocupado_por_pid);
}


int esp_libre_eliminar(int posicion){

	bool _esp_libre_buscar(t_libre* libre){
		return libre->posicion == posicion;
	}

	list_remove_and_destroy_by_condition(esp_libre, (void*)_esp_libre_buscar, free);

	return 0;
}

int unir_huecos_contiguos(t_libre* libre){
	int i;
	t_libre* libre_ant = NULL;
	for (i = 0; i < list_size(esp_libre); i++) {
		libre_ant = list_get(esp_libre, i);
		//si es el anterior
		if (libre_ant->posicion + libre_ant->cantidad == libre->posicion) {
			break;
		}else{
			libre_ant = NULL;
		}
	}

	t_libre* libre_sig = NULL;
	for (i = 0; i < list_size(esp_libre); i++) {
		libre_sig = list_get(esp_libre, i);
		//si es el anterior
		if (libre->posicion + libre->cantidad == libre_sig->posicion) {
			break;
		}else{
			libre_sig = NULL;
		}
	}

	///////////////////////////////////
	if (libre_ant != NULL) {
		libre_ant->cantidad += libre->cantidad;

		esp_libre_eliminar(libre->posicion);

		libre = libre_ant;
	}
	if (libre_sig != NULL) {
		libre->cantidad += libre_sig->cantidad;

		esp_libre_eliminar(libre_sig->posicion);
	}

	//////////////////////////////////
	/////////////////////////////////
	/*
	if(libre_ant!=NULL && libre_sig!=NULL){
		libre->posicion -= libre_ant->cantidad;
		libre->cantidad+=libre_sig->cantidad;

		esp_libre_eliminar(libre_ant->posicion);
		esp_libre_eliminar(libre_sig->posicion);

	}else{
		if(libre_ant!=NULL){
			libre_ant->cantidad+=libre->cantidad;


			esp_libre_eliminar(libre->posicion);
		}else{
			if(libre_sig!=NULL){
				libre->cantidad += libre_sig->cantidad;


				esp_libre_eliminar(libre_sig->posicion);
			}
		}
	}*/



	/*
	t_list* nueva_lista_libres = list_create();

	t_libre* libre = NULL;
	int i, pagina;

	int cant_huecos = list_size(esp_libre);
	t_libre* libre_sig;
	for (i = 0; i < cant_huecos; i++) {
		libre = list_get(esp_libre, i);
		pagina = libre->posicion + libre->cantidad;

		//leo el siguiente para saber si es contiguo (ya esta ordenado)
		libre_sig = list_get(esp_libre, i+1);
		if(libre_sig->posicion == pagina){
			//si no existe, no es un hueco contiguo, lo agrego a la lista
			list_add(nueva_lista_libres, libre);
		}else{
			//si el siguiente es contiguo, le sumo a la cantidad siguiente la actual
			libre->cantidad = libre->cantidad + libre_sig->cantidad;

			list_add(nueva_lista_libres, (void*)libre);
			i++;
		}
	}

	//reemplazo la lista vieja por la nueva
	esp_libre = nueva_lista_libres;*/

	return 0;
}

int swap_liberar(int pid){
	t_ocupado* ocupado = swap_buscar_ocupado_por_pid(pid);

	//me posiciono en el hueco
	int posicion = ocupado->posicion + ocupado->cantidad;

	t_libre* libre = NULL;

	libre = swap_buscar_hueco_que_empiece_en(posicion);

	if(libre != NULL){
		//si lo encuentra, solo lo actualizo
		libre->posicion =libre->posicion - ocupado->cantidad;
		libre->cantidad =libre->cantidad + ocupado->cantidad;
	}else{
		//si no existe, lo agrego, va quedar fragmentado
		libre = malloc(sizeof(t_libre));
		libre->posicion = ocupado->posicion;
		libre->cantidad = ocupado->cantidad;

		list_add(esp_libre, (void*)libre);
	}
	//todo: falta verificar si se generan dos huecos juntos
	unir_huecos_contiguos(libre);


	//borrar el bloque ocupado
	bool _swap_buscar_ocupado_por_pid(t_ocupado* ocup) {
		return ocup->pid == pid;
	}
	list_remove_by_condition(esp_ocupado, (void*)_swap_buscar_ocupado_por_pid);
	FREE_NULL(ocupado);

	return 0;
}

void procesar_mensaje_mem(int socket_mem, t_msg* msg){
	char* contenido;
	int pid, pagina, paginas;
	//print_msg(msg);
	switch (msg->header.id) {
		case SWAP_INICIAR:

			paginas = msg->argv[1];
			pid = msg->argv[0];
			pthread_mutex_lock(&mutex);
			log_trace(logger, "SWAP_INICIAR . pid: %d, Paginas: %d", pid, paginas);
			pthread_mutex_unlock(&mutex);
			destroy_message(msg);

			swap_nuevo_proceso(pid, paginas);

			//envio 1 = true
			msg = argv_message(SWAP_OK, 0);
			enviar_y_destroy_mensaje(socket_mem, msg);
			break;

		case SWAP_LEER:
			pid = msg->argv[0];
			pagina = msg->argv[1];
			destroy_message(msg);

			log_trace(logger, "SWAP_LEER. pid: %d, Pagina: %d", pid, pagina);
			//envio 1 = true
			//contenido = string_from_format("Pid: %d, pag: %d, contenido: HOLAAA", pid, pagina);
			contenido = swap_leer(pid, pagina);

			msg = string_message(SWAP_OK,contenido , 0);
			enviar_y_destroy_mensaje(socket_mem, msg);
			free(contenido);
			/*
			t_foo* foo;
			recibirMensaje(socket_mem, (void*)&foo);
			printf("pid: %d, pagina: %d, texto: %s\n", foo->pid, foo->pagina, foo->texto);
			free(foo);
			*/
			break;
		case SWAP_ESCRIBIR:

			pid = msg->argv[0];
			pagina = msg->argv[1];
			contenido = string_duplicate(msg->stream);
			destroy_message(msg);
			log_trace(logger, "SWAP_ESCRIBIR. pid: %d, Pagina: %d, conteindo: %s", pid, pagina, contenido);

			swap_escribir(pid, pagina, contenido);

			FREE_NULL(contenido);
			//envio 1 = true
			msg = argv_message(SWAP_OK, 0);
			enviar_y_destroy_mensaje(socket_mem, msg);

			break;

		case SWAP_FINALIZAR:
			pid = msg->argv[0];
			destroy_message(msg);
			log_trace(logger, "SWAP_FINALIZAR. pid: %d", pid);
			//envio 1 = true

			swap_liberar(pid);

			msg = argv_message(SWAP_OK, 0);
			enviar_y_destroy_mensaje(socket_mem, msg);



			break;

		default:
			break;
	}


}
