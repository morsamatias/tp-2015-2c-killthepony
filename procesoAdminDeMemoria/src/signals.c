/*
 * signals.h
 *
 *  Created on: 11/9/2015
 *      Author: utnso
 */
#ifndef SIGNALS_C_
#define SIGNALS_C_

#include "procesoAdminDeMemoria.h"
/*
 * en una consola del so
 * kill -SIGUSR1 pid
 * kill -SIGUSR2 pid
 * kill -SIGPOLL pid
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// MANEJO GENERAL SEÑALES /////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void iniciar_signals(){
	if (signal(SIGUSR1, handler) == SIG_ERR){
		perror("No se puede capturar SIGUSR1\n");
	}
	if (signal(SIGUSR2, handler) == SIG_ERR){
		perror("No se puede capturar SIGUSR2\n");
	}
	if (signal(SIGPOLL, handler) == SIG_ERR){
		perror("No se puede capturar SIGPOLL\n");
	}
}

void handler(int signal){
	pthread_t hilo_signal;
	switch(signal){
		case SIGUSR1: // BORRADO DE LA TLB
			pthread_create(&hilo_signal, NULL, (void*) handler_SIGUSR1,NULL);
			break;
		case SIGUSR2: // BORRADO DE LAS PAGINAS EN MEMORIA
			pthread_create(&hilo_signal, NULL, (void*) handler_SIGUSR2,NULL);
			break;
		case SIGPOLL: // VOLCADO DE LA MEMORIA EN ARCHIVO
			handler_SIGPOLL();
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////// SEÑAL SIGUSR1 - BORRADO TLB ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void handler_SIGUSR1(){
	log_info(log_general_p, "Señal Capturada: SIGUSR1 - Borrar TLB");
	if(TLB_HABILITADA()){
		sem_wait(&mutex_TLB);
		list_clean(TLB);
		sem_post(&mutex_TLB);
	}
	log_info(log_general_p, "Borrado Completo TLB");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// SEÑAL SIGUSR2 - BORRADO MEMORIA //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void handler_SIGUSR2(){


	log_info(log_general_p, "Señal Capturada: SIGUSR2 - Borrar Memoria");
	sem_wait(&mutex_PAGINAS);
	// GUARDO LAS VARIABLES GLOBALES EN UNA AUXILIAR POR LAS DUDAS
	int gl_PID_aux = gl_PID;
	int	gl_nro_pagina_aux = gl_nro_pagina;

	// BORRO LAS PAGINAS DE MEMORIA
	list_iterate(paginas,(void*)quitar_todas_las_paginas_de_memoria_del_proceso);

	// RESTAURO LAS VARIABLES GLOABLES
	gl_PID = gl_PID_aux;
	gl_nro_pagina = gl_nro_pagina_aux;

	sem_post(&mutex_PAGINAS);
	log_info(log_general_p, "Borrado Completo Memoria");
}

void quitar_todas_las_paginas_de_memoria_del_proceso(t_proceso* proceso){
	list_iterate(proceso->paginas,(void*)quitar_pagina_de_la_memoria_signal);
}

void quitar_pagina_de_la_memoria_signal(t_pagina* pagina){
	quitar_pagina_de_la_memoria(pagina);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// SEÑAL SIGPOLL - DUMP MEMORIA ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void handler_SIGPOLL(){
	log_info(log_general_p, "Señal Capturada: SIGPOLL - Dump Memoria");

	int i, pid;
	FILE* archivo;

	pid=fork();
	if(pid<0){
		log_info(log_general_p, "Error al querer hacer Dump Memoria - No se pudo hacer FORK");
		log_error(log_errores, "Error al querer hacer Dump Memoria - No se pudo hacer FORK");
	} else
		if(pid==0) {
			// REALIZO EL DUMP DE LA MEMORIA
			archivo = fopen("/home/utnso/Escritorio/git/tp-2015-2c-killthepony/procesoAdminDeMemoria/Debug/Dump_Memoria.txt","w");
			for(i=0;i<CANTIDAD_MARCOS();i++){
				if(memoria[i]->libre)
					fprintf(archivo,"Marco: %d	Contenido: <vacio>\n",i);
				else
					fprintf(archivo,"Marco: %d	Contenido: %s\n",i,memoria[i]->contenido);
			}
			fclose(archivo);
			log_info(log_general_p, "Realizado correctamente Dump Memoria");
			exit(1);
	} else {
			// CONTINUO LA EJECUCION NORMAL
	}
}


#endif /* SIGNALS_C_ */

