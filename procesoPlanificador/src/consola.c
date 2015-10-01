/*
 * consola.c
 *
 *  Created on: 29/8/2015
 *      Author: utnso
 */

#include "consola.h"


e_comando parsear_comando(char* comando) {

	if (string_starts_with(comando, "r") || string_starts_with(comando, "c"))
		return CORRER;
	if (string_equals_ignore_case(comando, "f"))
		return FINALIZAR;
	if (string_equals_ignore_case(comando, "p"))
		return PS;
	if (string_equals_ignore_case(comando, "c"))
		return CPU;
	if (string_equals_ignore_case(comando, "exit"))
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
