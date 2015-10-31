/*
 * signals.h
 *
 *  Created on: 11/9/2015
 *      Author: utnso
 */
#ifndef SIGNALS_H_
#define SIGNALS_H_

#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include "procesoAdminDeMemoria.h"
/*
 * en una consola del so
 * kill -USR1 pid
 * kill -USR2 pid
 * kill -SIGIO pid  //SIGPOLL
 */
int iniciar_signals();
void manejar_SIGUSR1();
void manejar_SIGUSR2();
void manejar_SIGPOLL();


void manejar_SIGUSR1(){
	printf("Recibido SIGUSR1\n");
	fflush(stdout);

	//SIGUSR1: deberá limpiar completamente la TLB (TLB flush),
	//utilizando un hilo correctamente sincronizado para esto, evitando problemas de concurrencia.

	//LINEA 123 Y DONDE SE USE LA FUNCION (buscar_marco_de_pagina_en_TLB_y_tabla_paginas)
	//SEMAFORO PARA QUE NO PUEDA USAR LA TLB HASTA QUE LA VACIE.
	//y TAMBIEN PARA QUE
	//NO SE PUEDE VACIAR SI ALGUIEN ESTA USANDO
int pid= fork();
if(pid==-1){

	printf("error al crear el fork");
	exit(EXIT_FAILURE);
}


if (pid==0){


	//procesoHijo
	 printf("Limpio TLB");
	 list_clean(TLB);
	   _exit(EXIT_SUCCESS);

}else{

	int status;

	 (void)waitpid(pid, &status, 0);

}

}

void manejar_SIGUSR2(){
	printf("Recibido SIGUSR2\n");
	fflush(stdout);
	int i;
	/*SIGUSR2: deberá limpiar completamente la memoria principal, actualizandolos
	bits que sean necesarios en las tablas de páginas de los diferentes procesos.
	Para evitar problemas de concurrencia, aquí también se deberá utilizar un hilo
	correctamente sincronizado.*/
	int pid= fork();
	if(pid==-1){

		printf("error al crear el fork");
		exit(EXIT_FAILURE);
	}


	if (pid==0){


		//procesoHijo
		printf("Limpio Memoria Principal");

		for(i=0;i<CANTIDAD_MARCOS();i++)
				memoria[i] = NULL;
		printf("Actualizo TLB y Tabla de Paginas");

		list_clean(TLB);

		list_clean_and_destroy_elements(paginas,(void*)destruir_proceso);

		   _exit(EXIT_SUCCESS);

	}else{

		int status;

		 (void)waitpid(pid, &status, 0);

	}

}
void manejar_SIGPOLL(){
	printf("Recibido SIGPOLL\n");
	fflush(stdout);
int i;
	/*SIGPOLL: deberá realizar un volcado (dump) del contenido de la memoria principal, en
	el archivo log de Administrador de Memoria, creando para tal fin un proceso
	nuevo13. El volcado deberá indicar el número de marco y su contenido,
	utilizando una fila por cada marco.*/
int pid= fork();
	if(pid==-1){

		printf("error al crear el fork");
		exit(EXIT_FAILURE);
	}


	if (pid==0){


		//procesoHijo
		printf("DUMP\n");
		for(i=0;i<CANTIDAD_MARCOS();i++){
				log_info(logger, "Numero Marco %i, Contenido:%s \n",i, memoria[1]->contenido);

										}

		   _exit(EXIT_SUCCESS);

	}else{

		int status;

		 (void)waitpid(pid, &status, 0);

	}


}

void sig_handler_SIGUSR1(int signo) {
	if (signo == SIGUSR1){
		manejar_SIGUSR1();
	}
}

void sig_handler_SIGUSR2(int signo) {
	if (signo == SIGUSR2){
		manejar_SIGUSR2();
	}
}



void sig_handler_SIGPOLL(int signo) {
	if (signo == SIGPOLL){
		manejar_SIGPOLL();

	}
}


int iniciar_signals(){

	if (signal(SIGUSR1, sig_handler_SIGUSR1) == SIG_ERR){
		perror("No se puede capturar SIGUSR1\n");
		return -1;
	}
	if (signal(SIGUSR2, sig_handler_SIGUSR2) == SIG_ERR){
		perror("No se puede capturar SIGUSR2\n");
		return -1;
	}
	if (signal(SIGPOLL, sig_handler_SIGPOLL) == SIG_ERR){
		perror("No se puede capturar SIGPOLL\n");

		return -1;
	}

	return 0;
}



#endif /* SIGNALS_H_ */

