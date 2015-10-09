/*
 * util.h
 *
 *  Created on: 26/4/2015
 *      Author: utnso
 */

#ifndef UTILES_UTIL_H_
#define UTILES_UTIL_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <stdlib.h>
 #include <sys/time.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <sys/sendfile.h>

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

#include "commons/collections/list.h"
#include "commons/string.h"
#include <pthread.h>
#include <stdbool.h>

#include <wait.h>


#include <sys/types.h>      /* key_t, sem_t, pid_t      */
#include <sys/shm.h>        /* shmat(), IPC_RMID        */
#include <errno.h>          /* errno, ECHILD            */
#include <semaphore.h>      /* sem_open(), sem_destroy(), sem_wait().. */
#include <fcntl.h>          /* O_CREAT, O_EXEC          */


#define handle_error(msj) \
	do{perror(msj);exit(EXIT_FAILURE);} while(0)

#define MB_EN_B  1024*1024//1mb

#define PATH_MAX_LEN 1024 //size maximo de un path

#define REG_SIZE 10
#define BACK_LOG 100

/* Funciones Macro */
#define FREE_NULL(p) \
    {free(p); \
    p = NULL;}



/*
 * Compara dos numeros y retorna el mínimo.
 */
#define min(n, m) (n < m ? n : m)

/*
 * Compara dos numeros y retorna el máximo.
 */
#define max(n, m) (n > m ? n : m)

/* FIN Funciones Macro */


#define MAX_PATH 1024

/****************** IDS DE MENSAJES. ******************/
typedef enum {
	iniciar,
	leer,
	escribir,
	io,
	final,
	error
}e_sentencia;


typedef enum {
	CPU_NUEVO,
	CPU_BASE,
	PCB,
	PCB_A_EJECUTAR,
	PCB_IO,
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
	SENTENCIAS_EJECUTADAS,
	PCB_FINALIZAR,
	CPU_PORCENTAJE_UTILIZACION,
	PCB_QUANTUM

} t_msg_id;


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



bool file_exists(const char* filename);

size_t file_get_size(char* filename);
void* file_get_mapped(char* filename);
void file_mmap_free(char* mapped, char* filename);
float  bytes_to_kilobytes(size_t bytes);

int recibir_mensaje_script(int socket, char* save_as);

t_cpu_base* cpu_base_new(int id, char* ip, int puerto);
sem_t* sem_crear(int* shmid, key_t* shmkey, int contador_ftok);
float bytes_to_megabytes(size_t bytes);

int cant_registros(char** registros) ;
int enviar_mensaje_flujo(int unSocket, int8_t tipo, int tamanio, void *buffer);
int recibir_mensaje_flujo(int unSocket, void** buffer);
int recibir_mensaje_script_y_guardar(int fd, char* path_destino);
int enviar_mensaje_script(int fd, char* path_script);


int enviar_mensaje_sin_header(int sock_fd, int tamanio, void* buffer);

/****************** FUNCIONES SOCKET. ******************/
int server_socket_select(uint16_t port, void (*procesar_mensaje)(int, t_msg*));
int recibirMensaje(int unSocket, void** buffer) ;
int mandarMensaje(int unSocket, int8_t tipo, int tamanio, void *buffer) ;
/*
 * Crea, vincula y escucha un socket desde un puerto determinado.
 */
int server_socket(uint16_t port);
int recibir_linea(int sock_fd, char*linea);

//char* ip_get();

/*
 * Crea y conecta a una ip:puerto determinado.
 */
int client_socket(char* ip, uint16_t port);

/*
 * Acepta la conexion de un socket.
 */
int accept_connection(int sock_fd);
int accept_connection_and_get_ip(int sock_fd, char **ip);
int len_hasta_enter(char* strings);
/*
 * Recibe un t_msg a partir de un socket determinado.
 */
t_msg *recibir_mensaje(int sock_fd);

/*
 * Envia los contenidos de un t_msg a un socket determinado.
 */
int enviar_mensaje(int sock_fd, t_msg *msg);

/****************** FUNCIONES T_MSG. ******************/

/*
 * Crea un t_msg sin argumentos, a partir del id.
 */
t_msg *id_message(t_msg_id id);

/*
 * Crea un t_msg a partir de count argumentos.
 */
t_msg *argv_message(t_msg_id id, uint16_t count, ...);

/*
 * Crea un t_msg a partir de un string y count argumentos.
 */
t_msg *string_message(t_msg_id id, char *message, uint16_t count, ...);

/*
 * Libera los contenidos de un t_msg.
 */
void destroy_message(t_msg *mgs);

/****************** FUNCIONES FILE SYSTEM. ******************/

/*
 * Crea un archivo de size bytes de tamaño.
 */
void create_file(char *path, size_t size);

/*
 * Vacía el archivo indicado por path. Si no existe lo crea.
 */
void clean_file(char *path);

/*
 * Lee un archivo y retorna los primeros size bytes de su contenido.
 */
char* read_file(char *path, size_t size);

/*
 * Si existe, copia el contenido del archivo path en dest.
 */
void memcpy_from_file(char *dest, char *path, size_t size);

/*
 * Elimina los primeros size bytes del archivo path, y los retorna.
 */
char* read_file_and_clean(char *path, size_t size);

/*
 * Lee un archivo y retorna todo su contenido.
 */
char* read_whole_file(char *path);

/*
 * Lee un archivo y retorna todo su contenido, vaciándolo.
 */
char* read_whole_file_and_clean(char *path);

/*
 * Abre el archivo indicado por path (si no existe lo crea) y escribe size bytes de data.
 */
void write_file(char *path, char* data, size_t size);

/****************** FUNCIONES AUXILIARES. ******************/


/*
 * Muestra los contenidos y argumentos de un t_msg.
 */
void print_msg(t_msg *msg);

/*
 * Convierte t_msg_id a string.
 */
char *id_string(t_msg_id id);
//int convertir_path_absoluto(char** destino, char* file);
char* convertir_path_absoluto(char* file);
void free_split(char** splitted);
int split_count(char** splitted);
int recv_timeout(int s , int timeout);
int enviar_y_destroy_mensaje(int sock, t_msg* msg);

int enviar_mensaje_cpu(int sock, t_msg*);
t_msg* cpu_base_message(t_cpu_base* cb);
t_cpu_base* recibir_mensaje_cpu_base(int s);

int enviar_mensaje_pcb(int socket, t_pcb* pcb);

t_pcb* pcb_nuevo(char* path);
t_pcb* recibir_mensaje_pcb(int socket);
int pcb_print(t_pcb* pcb);

void lock(pthread_mutex_t* mutex);
void unlock(pthread_mutex_t* mutex);
#endif /* UTILES_UTIL_H_ */
