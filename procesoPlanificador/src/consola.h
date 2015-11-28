/*
 * consola.h
 *
 *  Created on: 29/8/2015
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_


#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "commons/string.h"
//#include <util.h>
//#include "directorios.h"

typedef enum {
	CORRER,
	NADA,
	SALIR,
	FINALIZAR,
	PS,
	CPU,
	LS,
	NODO_AGREGAR
} e_comando;

#define COMMAND_MAX_SIZE 1000

e_comando parsear_comando(char* input_user);
void leer_comando_consola(char* comando);
//char* consola_leer_param_char(char* param);
char** separar_por_espacios(char* string) ;
/*
 * esta funcion magica saca el \n de una cadena, el fgets me devuele \n y tnego que borrarlo
 */
void strip(char *s);

int encontrar_espacio(char* msg);


#endif /* CONSOLA_H_ */
