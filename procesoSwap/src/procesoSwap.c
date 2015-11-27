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


/////////////////////////////////////////////VARIABLES/////////////////////////////////////////
bool FIN = false;

//////////////////////////////////////////////MAIN() //////////////////////////////////////////

int main(void) {

	system("clear");

	inicializar();

	server_socket_select(PUERTO_ESCUCHA(), procesar_mensaje_mem);

	finalizar();

	return EXIT_SUCCESS;
}

//////////////////////////////////////////////RETARDO///////////////////////////////////////////////////////

void dormir_swap(){

	/*
	int retardo = RETARDO_SWAP();
	
	if (retardo == 0) {
		sleep(retardo);
	}else{
		usleep(RETARDO_SWAP_MINIMO()*1000);
	}*/
	
	if (RETARDO_SWAP() == 0) {
		usleep(RETARDO_SWAP_MINIMO()*1000);
	}else{
		sleep(RETARDO_SWAP());
	}


}


void dormir_compactacion(){

	if (RETARDO_COMPACTACION() == 0) {
		usleep(RETARDO_COMPACTACION_MINIMO()*1000);
	}else{
		sleep(RETARDO_COMPACTACION());
	}

}
//////////////////////////////////////////////INICIALIZAR//////////////////////////////////////////////////////////


char* swap_inicializar() {

	clean_file(NOMBRE_SWAP());

	TAMANIO_SWAP = CANTIDAD_PAGINAS() * TAMANIO_PAGINA();
	char CREAR_DATA[1024];

	char* TRUNC_DATA = string_from_format("truncate -s %d %s", TAMANIO_SWAP, NOMBRE_SWAP());
	//printf("%s\n", TRUNC_DATA);

	system(TRUNC_DATA);
	free(TRUNC_DATA);

	sprintf(CREAR_DATA, "dd if=/dev/zero of=%s bs=%d count=%d", NOMBRE_SWAP(), TAMANIO_PAGINA(), CANTIDAD_PAGINAS());
	//printf("%s\n", CREAR_DATA);
	system(CREAR_DATA);

	log_trace(logger, "Creado archivo swap %s de %d paginas de tamaño %d.", NOMBRE_SWAP(), CANTIDAD_PAGINAS(), TAMANIO_PAGINA());



	return file_get_mapped(NOMBRE_SWAP());
}


void esp_libre_inicializar(){
	esp_libre = list_create();
	t_libre* libre = malloc(sizeof(t_libre));
	libre->posicion = 0;
	libre->cantidad = CANTIDAD_PAGINAS();

	list_add(esp_libre, libre);
}

int inicializar(){

	cfg = config_create(CONFIG_PATH);

	clean_file(LOGGER_PATH);
	clean_file(LOGGER_PATH_MPROC_ASIGNADO);
	clean_file(LOGGER_PATH_MPROC_LIBERADO);
	clean_file(LOGGER_PATH_MPROC_RECHAZADO);
	clean_file(LOGGER_PATH_COMPACTACION);
	clean_file(LOGGER_PATH_LECTURAS_ESCRITURAS);
	clean_file(LOGGER_PATH_PAGINAS_CANT_LECT_ESC);


	logger = log_create(LOGGER_PATH, "swap", true, LOG_LEVEL_TRACE);
	logger_mproc_asignado = log_create(LOGGER_PATH_MPROC_ASIGNADO, "swap", true, LOG_LEVEL_TRACE);
	logger_mproc_liberado = log_create(LOGGER_PATH_MPROC_LIBERADO, "swap", true, LOG_LEVEL_TRACE);
	logger_mproc_rechazado = log_create(LOGGER_PATH_MPROC_RECHAZADO, "swap", true, LOG_LEVEL_TRACE);
	logger_compactacion = log_create(LOGGER_PATH_COMPACTACION, "swap", true, LOG_LEVEL_TRACE);
	logger_lecturas_escrituras = log_create(LOGGER_PATH_LECTURAS_ESCRITURAS, "swap", true, LOG_LEVEL_TRACE);
	logger_errores = log_create(LOGGER_PATH_ERRORES, "swap", true, LOG_LEVEL_TRACE);
	logger_paginas_cant_lect_esc = log_create(LOGGER_PATH_PAGINAS_CANT_LECT_ESC, "swap", true, LOG_LEVEL_TRACE);



	swap = swap_inicializar();
	/////////////////////////////////////
	//inicializo las paginas con nros del 1 al X
	/*int i;
	for (i = 0; i < CANTIDAD_PAGINAS(); ++i) {
		memset(swap + i, i, TAMANIO_PAGINA());
	}*/
	/////////////////////////


	////////////////////////////////////////
	//fix temporal
	//cargo el archivo de letras A porque sino me da error el envio de cadena vacia la lib de socket!!!
	//int TAMANIO_SWAP = CANTIDAD_PAGINAS() * TAMANIO_PAGINA();
	//memset(swap, 'A', TAMANIO_SWAP);

	////////////////////////////////////

	esp_ocupado =  list_create();

	esp_libre_inicializar();

	procesos = list_create();


	return 0;
}

///////////////////////////////////////////////////////FINALIZAR//////////////////////////////////////////

void swap_destroy() {
	munmap(swap, TAMANIO_SWAP);
//mapped = NULL;
}

void libre_destroy(t_libre* libre){
	FREE_NULL(libre);
}

void ocupado_destroy(t_ocupado* ocupado){
	FREE_NULL(ocupado);
}

void proceso_destroy(t_proceso* proc){
	FREE_NULL(proc);
}

int finalizar(){

	config_destroy(cfg);
	log_destroy(logger);
	swap_destroy();

	list_destroy_and_destroy_elements(esp_libre, (void*)libre_destroy);
	list_destroy_and_destroy_elements(esp_ocupado, (void*)ocupado_destroy);
	list_destroy_and_destroy_elements(procesos, (void*)proceso_destroy);



	printf("Fin OK \n");
	return 0;
}

/////////////////////////////////////////////////////COMPACTACION///////////////////////////

int swap_cant_huecos_libres(){
	int huecos = 0;

	void _contar(t_libre* libre){
		huecos += libre->cantidad;
	}
	list_iterate(esp_libre, (void*)_contar);

	return huecos;
}

int compactar(){
	log_trace(logger_compactacion, "Iniciando compactacion");
	//creo una nueva lista copia de la lista de ocupados
	t_list* esp_ocupado_new = NULL;
	esp_ocupado_new = list_create();
	typedef struct{
		int pid;
		int tamanio;
		char* contenido;
	}t_pagina_tmp;
	t_list* paginas_tmp = list_create();
	t_pagina_tmp* page_tmp;
	//int tam_hueco;
	void _copiar(t_ocupado* ocup){
		page_tmp = malloc(sizeof(*page_tmp));
		page_tmp->pid = ocup->pid;

		//me guardo el contenido de tod0 el hueco
		page_tmp->tamanio = ocup->cantidad*TAMANIO_PAGINA();
		page_tmp->contenido = malloc(page_tmp->tamanio );
		memcpy(page_tmp->contenido, swap+ocup->posicion, page_tmp->tamanio );
		list_add(paginas_tmp, page_tmp);

		list_add(esp_ocupado_new, ocup);

	}
	list_iterate(esp_ocupado, (void*)_copiar);

	//borro los elementos de la lista,
	esp_ocupado->elements_count = 0;

	//limpio la lista de esp libre
	list_destroy(esp_libre);
	esp_libre_inicializar();

	//inicializo toda la particion
	memset(swap, 0, TAMANIO_SWAP);
	//int comienzo = 0;

	void _ocupar_hueco(t_ocupado* o){
		o->posicion= swap_buscar_hueco_libre(o->cantidad);
		swap_ocupar(o->pid, o->posicion, o->cantidad);

		bool _buscar_page_tmp(t_pagina_tmp* p){
			return p->pid ==o->pid;
		}
		page_tmp = list_find(paginas_tmp, (void*)_buscar_page_tmp);
		memcpy(swap + (o->posicion), page_tmp->contenido, page_tmp->tamanio);
	}
	//ocupo tod0 de nuevo
	list_iterate(esp_ocupado_new, (void*)_ocupar_hueco);


	//limpio tmp
	void _pagina_tmp_destroy(t_pagina_tmp* p){
		free(p->contenido);
		//free(p);
	}
	list_destroy_and_destroy_elements(paginas_tmp, (void*)_pagina_tmp_destroy);

	//ordenpor las dudas
	ordenar();

	//limpio la lista creada
	FREE_NULL(esp_ocupado_new);


	/*
	void _ocupar(t_ocupado* ocupado){
		swap_ocupar_hueco(ocupado);
	}
	list_iterate(esp_ocupado_new, (void*)_ocupar);
	*/

	dormir_compactacion();

	mostrar_listas();

	log_trace(logger_compactacion, "Compactacion finalizada");
	return 0;
}

/*
 * si no encuentra nada devuelve -1;
 */

////////////////////////////////////////////////////BUSCAR ESPACIOS LIBRES///////////////////////////
int swap_buscar_hueco_libre(int paginas){

	bool _swap_buscar_hueco_libre(t_libre* libre){
		return libre->cantidad >= paginas;
	}

	t_libre* l;
	l = list_find(esp_libre, (void*)_swap_buscar_hueco_libre);
	if(l==NULL){
		//si no hay hueco libre, me fijo si es por fragmentacion o no
		if(swap_cant_huecos_libres()>=paginas){
			compactar();

			//una vez que compacto, siempre va encontrar el hueco
			l = list_find(esp_libre, (void*)_swap_buscar_hueco_libre);
			return l->posicion;
		}else{
			log_trace(logger_mproc_rechazado, "No se pudo crear el proceso porque no hay %d paginas libres");
			return -1;
		}

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


/////////////////////////////////////////////////////////////ORDENAR////////////////////////////////////////////////////////

void ordenar_ocupado(){
	bool _ordenar_ocupado(t_ocupado* o1, t_ocupado* o2) {
		return o1->posicion > o2->posicion;
	}
	list_sort(esp_ocupado, (void*) _ordenar_ocupado);
}

void ordenar_libre(){
	bool _ordenar_libre(t_libre* l1, t_libre* l2) {
		return l1->posicion > l2->posicion;
	}
	list_sort(esp_libre, (void*)_ordenar_libre);
}

int ordenar(){

	ordenar_ocupado();

	ordenar_libre();

	return 0;
}
////////////////////////////////////////////////////////////OCUṔAR HUECO/////////////////////////////////////////////////

int swap_ocupar_hueco(t_ocupado* ocupado){

	list_add(esp_ocupado, ocupado);

	//ahora agrego a la lista de espacio libre
	//swap_agregar_lista_esp_libre(pagina);

	t_libre* libre = NULL;
	//siempre exisste el hueco que empieza en la pagina
	libre = swap_buscar_hueco_que_empiece_en(ocupado->posicion);

	libre->posicion = libre->posicion + ocupado->cantidad;
	libre->cantidad = libre->cantidad - ocupado->cantidad;


	return 0;
}

t_ocupado* swap_ocupar(int pid, int pagina, int paginas){
	//primero agrego a la lista de espacio ocupado lo que pide el proceso
	t_ocupado* ocupado = malloc(sizeof(t_ocupado));
	ocupado->pid = pid;
	ocupado->posicion = pagina;
	ocupado->cantidad = paginas	;



	swap_ocupar_hueco(ocupado);

	ordenar();

	return ocupado;
}

int hueco_inicio_bytes(t_ocupado* hueco){
	return hueco->posicion * TAMANIO_PAGINA();
}

int hueco_tamanio_bytes(t_ocupado* hueco){
	return hueco->cantidad * TAMANIO_PAGINA();
}

void hueco_print_info(const char* texto_inicial, t_ocupado* hueco, t_log* log_ok){
	log_info(log_ok, "%s > PID: %d, Paginas: %d, Inicio: byte %d, Tamaño: %d bytes", texto_inicial, hueco->pid, hueco->cantidad, hueco_inicio_bytes(hueco), hueco_tamanio_bytes(hueco));
}

t_proceso* proc_nuevo(int pid){
	t_proceso* new = malloc(sizeof(*new));
	new->pid = pid;
	new->cant_lecturas = 0;
	new->cant_escrituras = 0;

	return new;
}
void est_nuevo_proceso(int pid){
	t_proceso* proc = proc_nuevo(pid);

	list_add(procesos, proc);
}

int swap_nuevo_proceso(int pid, int paginas){
	int comienzo = 0;
	comienzo = swap_buscar_hueco_libre(paginas);
	if(comienzo>=0){
		t_ocupado* hueco;
		hueco = swap_ocupar(pid, comienzo, paginas);

		hueco_print_info("mProc Asignado", hueco, logger_mproc_asignado);
		return 0;
	}else{
		log_warning(logger_mproc_rechazado, "OCUPADO pid: %d, paginas: %d. No hay hueco libre para el proceso", pid, paginas);
		return -1;
	}

}

int swap_escribir(int pid, int pagina, char* contenido){
	size_t size = strlen(contenido);

	bool _buscar(t_ocupado* o){
		return o->pid == pid;
	}

	t_ocupado* o;
	o = list_find(esp_ocupado, (void*)_buscar);

	int comienzo;
	comienzo = o->posicion*TAMANIO_PAGINA();

	memcpy(swap + (TAMANIO_PAGINA() * pagina) + comienzo, contenido, size);
	//memcpy

	memset(swap + (TAMANIO_PAGINA() * pagina)+size + comienzo, '\0', TAMANIO_PAGINA() - size);

	/*
	if (msync(swap + (TAMANIO_PAGINA() * pagina)+size, size, MS_SYNC) < 0) {
		perror("msync failed with error:");
		return -1;
	} else
		(void) printf("%s", "msync completed successfully.");
*/
	return 0;
}

/*
 * devuelve una copia del contenido de la pagina;
 */
char* swap_leer(int pid, int pagina){
	char* contenido = malloc(TAMANIO_PAGINA()+1);
	memset(contenido, '\0', TAMANIO_PAGINA()+1);
	//memset(contenido, 'A', TAMANIO_PAGINA());//inicializo con A porque la lib de socket no sporta envio de cadena vacia !!!

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

	//list_remove_and_destroy_by_condition(esp_libre, (void*)_esp_libre_buscar, free);
	list_remove_by_condition(esp_libre, (void*)_esp_libre_buscar);

	return 0;
}

int unir_huecos_contiguos(t_libre* libre){
	ordenar_libre();

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

	//ordenar_libre();

	return 0;
}

int swap_liberar(int pid){
	t_ocupado* ocupado = swap_buscar_ocupado_por_pid(pid);

	if(ocupado == NULL){
		log_error(logger_errores, "El PID %d no se encontro en la lista de procesos para liberar", pid);
		return -1;
	}

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

	//ordenar_libre();
	unir_huecos_contiguos(libre);


	//borrar el bloque ocupado
	bool _swap_buscar_ocupado_por_pid(t_ocupado* ocup) {
		return ocup->pid == pid;
	}
	list_remove_by_condition(esp_ocupado, (void*)_swap_buscar_ocupado_por_pid);

	hueco_print_info("mProc Liberado", ocupado, logger_mproc_liberado);

	FREE_NULL(ocupado);

	//ordenar();
	ordenar_ocupado();

	return 0;
}

void pagina_print_info(const char* texto_inicio, int pid, int pagina, char* contenido){
	int inicio = pagina * TAMANIO_PAGINA();
	int size = strlen(contenido);
	log_info(logger_lecturas_escrituras, "%s > PID: %d, Pagina: %d, Inicio: byte %d, Tamaño: %d bytes, Contenido: \"%s\"", texto_inicio, pid, pagina, inicio, size, contenido);
}

t_proceso* proc_buscar(int pid){
	bool _proc_buscar(t_proceso* proc){
		return proc->pid == pid;
	}
	return list_find(procesos, (void*)_proc_buscar);
}

void est_leer(int pid){
	t_proceso* proc = proc_buscar(pid);
	proc->cant_lecturas++;
}

void est_escribir(int pid){
	t_proceso* proc = proc_buscar(pid);
	proc->cant_escrituras++;
}


void est_print(int pid){
	t_proceso* proc = proc_buscar(pid);

	log_info(logger_paginas_cant_lect_esc, "PID %d, Lecturas: %d, Escrituras: %d", pid, proc->cant_lecturas, proc->cant_escrituras);
}

void est_eliminar(int pid){
	bool _proc_buscar(t_proceso* proc){
		return proc->pid == pid;
	}

	list_remove_and_destroy_by_condition(procesos, (void*)_proc_buscar,(void*) proceso_destroy);
}

///////////////////////////////////////////////////////////PRINTS////////////////////////////////
void print_ocupado() {
	printf("**********************************ESPACIO OCUPADO \n");
	void _print_ocupado(t_ocupado* o) {
		printf("-pid:%d. -Posicion:%d. Cantidad:%d.\n", o->pid, o->posicion, o->cantidad);
	}
	list_iterate(esp_ocupado, (void*) _print_ocupado);
}
void print_libre() {
	printf("*********************************ESPACIO LIBRE \n");
	void _print_libre(t_libre* l) {
		printf("-Posicion:%d. -Cantidad:%d.\n", l->posicion, l->cantidad);
	}

	list_iterate(esp_libre, (void*) _print_libre);
}

void mostrar_listas(){
	printf("*************************INICIO PRINTS**************************\n");
	print_ocupado();
	print_libre();
	printf("*************************FIN PRINTS**************\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void procesar_mensaje_mem(int socket_mem, t_msg* msg){
	char* contenido;
	int pid, pagina, paginas;
	int st;


	//print_msg(msg);
	switch (msg->header.id) {
		case SWAP_INICIAR:

			pid = msg->argv[0];
			paginas = msg->argv[1];

			//log_trace(logger, "SWAP_INICIAR . pid: %d, Paginas: %d", pid, paginas);
			destroy_message(msg);

			//sleep(RETARDO_SWAP());

			if(paginas>swap_cant_huecos_libres()){
				st=-1;
			} else {
				st=swap_nuevo_proceso(pid, paginas);

			}

			if(st==-1){
				msg = argv_message(SWAP_NO_OK, 0);
				log_error(logger_mproc_rechazado, "PID: %d, No hay espacio suficiente para guardar %d paginas", pid, paginas);
			}else{
				est_nuevo_proceso(pid);
				msg = argv_message(SWAP_OK, 0);
				log_trace(logger_mproc_asignado, "PID: %d.  Se reservaron %d paginas", pid, paginas);
			}

			enviar_y_destroy_mensaje(socket_mem, msg);


			break;

		case SWAP_LEER:
			pid = msg->argv[0];
			pagina = msg->argv[1];
			destroy_message(msg);

			contenido = swap_leer(pid, pagina);
			est_leer(pid);

			dormir_swap();
			pagina_print_info("Lectura", pid, pagina, contenido);


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
			log_trace(logger_lecturas_escrituras, "SWAP_ESCRIBIR. pid: %d, Pagina: %d, contenido: \"%s\"", pid, pagina, contenido);

			swap_escribir(pid, pagina, contenido);
			est_escribir(pid);

			dormir_swap();

			pagina_print_info("Escritura", pid, pagina, contenido);

			FREE_NULL(contenido);

			//envio 1 = true
			msg = argv_message(SWAP_OK, 0);
			enviar_y_destroy_mensaje(socket_mem, msg);

			break;

		case SWAP_FINALIZAR:
			pid = msg->argv[0];
			destroy_message(msg);
			//log_trace(logger, "SWAP_FINALIZAR. pid: %d", pid);
			//envio 1 = true

			//puede que finalice alguno que no haya comenzado a ejecutar
			if(swap_liberar(pid)!= -1)
			{
				est_print(pid);
				est_eliminar(pid);
			}


			//sleep(RETARDO_SWAP());

			msg = argv_message(SWAP_OK, 0);
			enviar_y_destroy_mensaje(socket_mem, msg);

			//FIN = true;
			//return ;
			break;

		case CAIDA_PLANIFICADOR:
					log_info(logger, "PROCESO TERMINADO PORQUE SE CERRO EL PLANIFICADOR.");
					exit(0);
		break;
		case -10:
			destroy_message(msg);

			finalizar();

			exit(0);
			break;
		case 0:
			destroy_message(msg);
			log_trace(logger, "Memoria conectada OK");
			break;
		default:
			log_error(logger_errores, "Mensaje desconocido");
			destroy_message(msg);
			break;
	}//FIN SWITCH

	mostrar_listas();



}
