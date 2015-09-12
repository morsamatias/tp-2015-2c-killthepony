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

int iniciar_signals();
void manejar_SIGUSR1();
void manejar_SIGUSR2();
void manejar_SIGPOLL();


void manejar_SIGUSR1(){
	printf("Recibido SIGUSR1\n");
}

void manejar_SIGUSR2(){
	printf("Recibido SIGUSR2\n");
}
void manejar_SIGPOLL(){
	printf("Recibido SIGPOLL\n");
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
