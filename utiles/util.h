/*
 * util.h
 *
 *  Created on: 26/4/2015
 *      Author: utnso
 */

#ifndef UTILES_UTIL_H_
#define UTILES_UTIL_H_
///////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////BIBLIOTECAS///////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
#include "commons/collections/list.h"
#include "commons/string.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <net/if.h>
#include <wait.h>
#include <pthread.h>
#include <errno.h>          /* errno, ECHILD            */
#include <semaphore.h>      /* sem_open(), sem_destroy(), sem_wait().. */

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////// CONSTANTES///////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
#define handle_error(msj) \
	do{perror(msj);exit(EXIT_FAILURE);} while(0)

#define MB_EN_B  1024*1024//1mb

#define PATH_MAX_LEN 1024 //size maximo de un path

#define REG_SIZE 10
#define BACK_LOG 100

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////* Funciones Macro *///////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

#define FREE_NULL(p) \
    {free(p); \
    p = NULL;}

 //Compara dos numeros y retorna el mínimo.

#define min(n, m) (n < m ? n : m)

/*
 * Compara dos numeros y retorna el máximo.
 */
#define max(n, m) (n > m ? n : m)

/* FIN Funciones Macro */


#define MAX_PATH 1024

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************** IDS DE MENSAJES. ************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum {

	///////////////////CPU////////////////
	CPU_NUEVO,
	CPU_BASE,
	CPU_ESPECIAL,
	CPU_PORCENTAJE_UTILIZACION,

	PCB,
	PCB_A_EJECUTAR,
	PCB_IO,
	PCB_ERROR,
	PCB_FIN_QUANTUM,
	PCB_LOGUEO,
	PCB_FINALIZAR,

	MEM_IO,
	MEM_INICIAR,
	MEM_OK,
	MEM_LEER,
	MEM_ESCRIBIR,
	MEM_FINALIZAR,
	MEM_NO_OK,

	SWAP_INICIAR,
	SWAP_OK,
	SWAP_LEER,
	SWAP_ESCRIBIR,
	SWAP_FINALIZAR,
	SWAP_NO_OK,

	/////////////ERROR / FINALIZAR/////////////////////////////////
	SENTENCIAS_EJECUTADAS,
	CAIDA_PLANIFICADOR,
	CAIDA_MEMORIA,

	FINALIZAR_CPU

} t_msg_id;

///////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////ESCTRUCTURAS/////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
	iniciar,
	leer,
	escribir,
	io,
	final,
	error
}e_sentencia;


typedef struct{
	char ip[15];
	int puerto;
}t_red;

typedef struct {
	int id;
	t_red red;
}t_cpu_base;


typedef struct {
	char path[MAX_PATH];
	int pc;
	int cant_a_ejectuar;
	int cant_sentencias;
	int pid;
	int estado;
	int cpu_asignado;//para saber a que cpu fue asignado, en el caso de que el cpu se desconecte, busco el pcb que hay que replanificar
	double tiempo_inicio_proceso;
	double tiempo_fin_proceso;
	int tiempo_retorno;
	double tiempo_entrada_salida;
	int tiempo_respuesta;
	int cantidad_IO;
	double tiempo_inicio_ready;
	double tiempo_fin_ready;
	int tiempo_espera;
}t_pcb;

typedef struct {
	int8_t type;
	int16_t payloadlength;
}__attribute__ ((__packed__)) t_header_base;

typedef struct {
	t_msg_id id;
	uint32_t length;
	uint16_t argc;
}__attribute__ ((__packed__)) t_header;

typedef struct {
	t_header header;
	char *stream;
	int32_t *argv;
}__attribute__ ((__packed__)) t_msg;


//test envio struct
typedef struct{
		int pid;
		int pagina;
		char texto[1024];
}__attribute__ ((__packed__)) t_foo;

void pcb_free(t_pcb* pcb);

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////PROTOTIPOS/////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

void*		 	file_get_mapped										(char* filename);
void		 	dormir												(int segundos,int milisegundos);
void 		    file_mmap_free										(char* mapped, char* filename);

bool 		    file_exists											(const char* filename);

size_t 		    file_get_size										(char* filename);
t_cpu_base*     cpu_base_new										(int id, char* ip, int puerto);

float	        bytes_to_kilobytes									(size_t bytes);
float		    bytes_to_megabytes									(size_t bytes);

int			 	cant_registros										(char** registros) ;

////////////////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************MENSAJES****************************************/
////////////////////////////////////////////////////////////////////////////////////////////////////////

int 			enviar_mensaje_flujo								(int unSocket, int8_t tipo, int tamanio, void *buffer);
int 			enviar_mensaje_script								(int fd, char* path_script);
int 			enviar_mensaje_sin_header							(int sock_fd, int tamanio, void* buffer);
int 			recibir_mensaje_flujo								(int unSocket, void** buffer);
int 			recibir_mensaje_script_y_guardar					(int fd, char* path_destino);
int 			recibir_mensaje_script								(int socket, char* save_as);

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/****************************************************** FUNCIONES SOCKET. ******************************/
/////////////////////////////////////////////////////////////////////////////////////////////////////////


int 			accept_connection									(int sock_fd);
int 			accept_connection_and_get_ip						(int sock_fd, char **ip);
int 			client_socket										(char* ip, uint16_t port);
int 			len_hasta_enter										(char* strings);
int 			mandarMensaje										(int unSocket, int8_t tipo, int tamanio, void *buffer) ;
int 			server_socket_select								(uint16_t port, void (*procesar_mensaje)(int, t_msg*));
int 			server_socket										(uint16_t port);
int 			recibirMensaje										(int unSocket, void** buffer) ;
int 			recibir_linea										(int sock_fd, char*linea);

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////MENSAJES///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

t_msg*			argv_message										(t_msg_id id, uint16_t count, ...);
t_msg*			id_message											(t_msg_id id);
t_msg*			string_message										(t_msg_id id, char *message, uint16_t count, ...);
t_msg*			recibir_mensaje										(int sock_fd);

int			 	enviar_mensaje										(int sock_fd, t_msg *msg);

void 			destroy_message										(t_msg *mgs);

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/****************************************************** FUNCIONES FILE SYSTEM. ****************************/
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void 			create_file											(char *path, size_t size);
void 			clean_file											(char *path);
void 			memcpy_from_file									(char *dest, char *path, size_t size);
void			write_file											(char *path, char* data, size_t size);

char* 			read_file											(char *path, size_t size);
char* 			read_file_and_clean									(char *path, size_t size);
char* 			read_whole_file										(char *path);
char* 			read_whole_file_and_clean							(char *path);

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************* FUNCIONES AUXILIARES. ****************************/
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void 			lock												(pthread_mutex_t* mutex);
void 			free_split											(char** splitted);
void 			print_msg											(t_msg *msg);
void 			unlock												(pthread_mutex_t* mutex);

char*			id_string											(t_msg_id id);
char* 			convertir_path_absoluto								(char* file);

int 			enviar_mensaje_pcb									(int socket, t_pcb* pcb);
int 			enviar_mensaje_cpu									(int sock, t_msg*);
int 			enviar_y_destroy_mensaje							(int sock, t_msg* msg);
int 			pcb_print											(t_pcb* pcb);
int 			recv_timeout										(int s , int timeout);
int 			split_count											(char** splitted);

t_msg* 			cpu_base_message									(t_cpu_base* cb);
t_cpu_base* 	recibir_mensaje_cpu_base							(int s);

t_pcb* 			pcb_nuevo											(char* path);
t_pcb* 			recibir_mensaje_pcb									(int socket);

#endif /* UTILES_UTIL_H_ */
