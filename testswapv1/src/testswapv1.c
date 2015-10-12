/*
 ============================================================================
 Name        : testswapv1.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include <util.h>

int s ;
t_msg* msg;

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
void swiniciar(int pid, int cant_pag){
	msg = string_message(SWAP_INICIAR, "", 2, pid, cant_pag);
	enviar_y_destroy_mensaje(s, msg);
	msg = recibir_mensaje(s);
	print_msg(msg);
	destroy_message(msg);
}
void swleer(int pid, int pag){
	msg = string_message(SWAP_LEER, "", 2, pid, pag);
	enviar_y_destroy_mensaje(s, msg);
	msg = recibir_mensaje(s);
	print_msg(msg);
	destroy_message(msg);
}
void swescribir(int pid, int pag, char* cont){
	msg = string_message(SWAP_ESCRIBIR, cont, 2, pid, pag);
	enviar_y_destroy_mensaje(s, msg);
	msg = recibir_mensaje(s);
	print_msg(msg);
	destroy_message(msg);
}
void swfinalizar(int pid){
	msg = string_message(SWAP_FINALIZAR, "", 1, pid);
	enviar_y_destroy_mensaje(s, msg);
	msg = recibir_mensaje(s);
	print_msg(msg);
	destroy_message(msg);

}
int main(void) {
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */


	char** input_user;
	int pid, pagina, cant_paginas;
	char* contenido;
	char comando_usuario[1000];

	s = client_socket("127.0.0.1", 6000);

	while(true){
		memset(comando_usuario, '\0', 1000);
		fgets(comando_usuario, 1000, stdin);
		input_user = separar_por_espacios(comando_usuario);

		if (string_starts_with(input_user[0], "i")) {
			printf("Iniciar\n");
			pid = atoi(input_user[1]);
			cant_paginas = atoi(input_user[2]);
			swiniciar(pid, cant_paginas);
		} else if (string_starts_with(input_user[0], "l")) {
			printf("LEER\n");
			pid = atoi(input_user[1]);
			pagina = atoi(input_user[2]);
			swleer(pid, pagina);
		} else if (string_starts_with(input_user[0], "e")) {
			printf("ESCRIBIR\n");
			pid = atoi(input_user[1]);
			pagina = atoi(input_user[2]);
			contenido = strdup(input_user[3]);
			swescribir(pid, pagina, contenido);
			free(contenido);
		} else if (string_starts_with(input_user[0], "f")) {
			printf("FINALIZAR\n");
			pid = atoi(input_user[1]);
			swfinalizar(pid);
		} else {
			printf("error\n");
		}

		free_split(input_user);
	}///////////////////////////////////////////////////

	close(s);

	return EXIT_SUCCESS;
}
