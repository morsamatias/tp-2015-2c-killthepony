/*
 * consola.c
 *
 *  Created on: 29/8/2015
 *      Author: utnso
 */

#include "consola.h"


e_comando parsear_comando(char* comando) {

	if (string_equals_ignore_case(comando, "CORRER"))
		return CORRER;
	if (string_equals_ignore_case(comando, "FINALIZAR"))
		return FINALIZAR;
	if (string_equals_ignore_case(comando, "PS"))
		return PS;
	if (string_equals_ignore_case(comando, "CPU"))
		return CPU;
	if (string_equals_ignore_case(comando, "SALIR"))
		return SALIR;

	return NADA;
}


void strip(char *s) {
	char *p2 = s;
	while (*s != '\0') {
		if (*s != '\t' && *s != '\n') {
			*p2++ = *s++;
		} else {
			++s;
		}
	}
	*p2 = '\0';
}



char** separar_por_espacios(char* string) {
	char** res = string_split(string, " ");
	int i = 0;
	while (res[i] != NULL) {
		strip(res[i]); //saco el \n si es que lo tiene
		i++;
	}
	return res;
}

void leer_comando_consola(char* comando) {
	memset(comando, '\0', COMMAND_MAX_SIZE);
	fgets(comando, COMMAND_MAX_SIZE, stdin);
	//comando[strlen(comando)] = ' '	;

	//memset(comando, ' ', COMMAND_MAX_SIZE-strlen(comando));

}


int encontrar_espacio(char* msg)
{
	int tam,i;
	tam=string_length(msg);
	for( i=0;i<tam;i++)
	{
		if (msg[i]==' ') return(i);
	}
	return(-1);
}
