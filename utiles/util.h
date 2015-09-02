/*
 * util.h
 *
 *  Created on: 26/4/2015
 *      Author: utnso
 */

#ifndef UTIL_H_
#define UTIL_H_

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
#define LEN_KEYVALUE 1024*10 //longitud de la key de map o reduce

#define PATH_MAX_LEN 1024 //size maximo de un path

#define REG_SIZE 10
#define BACK_LOG 100

/* Funciones Macro */
#define FREE_NULL(p) \
    {free(p); \
    p = NULL;}

#define NUM_PIPES          2

#define PARENT_WRITE_PIPE  0
#define PARENT_READ_PIPE   1

#define READ_FD  0
#define WRITE_FD 1




//#define free_null(p) ({free(p);(p)=(NULL);})
/*
 * Compara dos numeros y retorna el mínimo.
 */
#define min(n, m) (n < m ? n : m)

/*
 * Compara dos numeros y retorna el máximo.
 */
#define max(n, m) (n > m ? n : m)

/* FIN Funciones Macro */

#define MAP 0
#define REDUCE 1
#define MAX_PATH 1024

/****************** IDS DE MENSAJES. ******************/

typedef enum {

	NODO_CONECTAR_CON_FS, //verifica que el nodo se conecte con el fs
	FS_NODO_OK, //el fs le contesta que ya esta conectado
	NODO_SALIR,
	CPU_NUEVO,
	CPU_BASE,
	PCB,
	PCB_A_EJECUTAR,
	MEM_INICIAR,
	MEM_OK,
	MEM_LEER,
	MEM_ESCRIBIR,
	MEM_FINALIZAR
} t_msg_id;

/****************** ESTRUCTURAS DE DATOS. ******************/

typedef struct{
	char ip[15];
	int puerto;
}t_red;

typedef struct {
	int id;
	t_red red;
}t_cpu_base;

typedef struct{
	//t_cpu_base *base;
	int id;
	int socket;
}t_cpu;

typedef struct {
	char path[MAX_PATH];
	int pc;
	int cant_a_ejectuar;
	int cant_sentencias;
}t_pcb;



typedef struct{
	char origen[1024];
	char destino[1024];
	pthread_mutex_t* mutex;
	int contador_ftok;
}t_ordenar;

typedef struct {
	int fd;
	char destino[1024];
}t_reader;

typedef struct{
	char ip[15];
	int puerto;
	char archivo[255];//nombre del archivo guardado en tmp
}t_files_reduce;

typedef struct {
	int id;
	t_red red;
}t_nodo_base;

typedef struct{
	int job_id;
	int id;
	//t_nodo_base* nodo_base;
	char* resultado;//el nombre del archivo ya mapeado(solo el nombre porque siempre lo va buscar en el tmp del nodo)
	bool empezo;
	//bool error;
	bool termino;//para saber si termino
}t_mapreduce;

typedef struct{
	t_nodo_base* nodo_base;
	char archivo[255];//list of string
}t_nodo_archivo;
typedef struct{
	t_mapreduce* info;
	t_nodo_base* nodo_base_destino;
	t_list* nodos_archivo;//list of t_nodo_archivo
	bool final;
}t_reduce;

typedef struct{
	t_nodo_base* base;
	int numero_bloque;//del nodo
}t_archivo_nodo_bloque; //

typedef struct{
	t_mapreduce* info;
	t_archivo_nodo_bloque* archivo_nodo_bloque;
	int archivo_id;
	int parte_numero;
	//int numero_bloque;//para saber a que bloque del archivo tengo que aplicarle el map
}t_map;


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


int reader_and_save_as(t_reader* reader);
int escribir_todo(int writer, char* data, int len);

int ordenar(t_ordenar* param_ordenar);
int ejecutar_script(char* path_script, char* name_script, int(*reader_writer)(int fdreader, int fdwriter), pthread_mutex_t* mutex, int c_ftok);

bool file_exists(const char* filename);
//void free_null(void** data);
char* file_combine(char* f1, char* f2);
size_t file_get_size(char* filename);
void* file_get_mapped(char* filename);
void file_mmap_free(char* mapped, char* filename);
float  bytes_to_kilobytes(size_t bytes);
int enviar_mensaje_nodo_base(int fd, t_nodo_base* nb);
t_nodo_base* recibir_mensaje_nodo_base(int fd);
int enviar_mensaje_reduce(int fd, t_reduce* reduce);
t_nodo_archivo* nodo_archivo_create();
int recibir_mensaje_script(int socket, char* save_as);

t_reduce* recibir_mensaje_reduce(int fd);
int read_line(char* linea, int fd);
t_cpu_base* cpu_base_new(int id, char* ip, int puerto);
sem_t* sem_crear(int* shmid, key_t* shmkey, int contador_ftok);
float bytes_to_megabytes(size_t bytes);
int enviar_nodo_base(int fd, t_nodo_base* nb);

int enviar_mensaje_map(int fd, t_map* map);
t_archivo_nodo_bloque* archivo_nodo_bloque_create(t_nodo_base* nb, int numero_bloque);
int cant_registros(char** registros) ;
int enviar_mensaje_flujo(int unSocket, int8_t tipo, int tamanio, void *buffer);
int recibir_mensaje_flujo(int unSocket, void** buffer);
t_map* recibir_mensaje_map(int fd);
int recibir_mensaje_script_y_guardar(int fd, char* path_destino);
int enviar_mensaje_script(int fd, char* path_script);
void print_map(t_map* map);
t_map* map_create(int id, int job_id, char* resultado);
t_mapreduce* mapreduce_create(int id, int job_id, char* resultado);
int enviar_mensaje_sin_header(int sock_fd, int tamanio, void* buffer);
void map_free(t_map* map);



/****************** FUNCIONES SOCKET. ******************/
int server_socket_select(uint16_t port, void (*procesar_mensaje)(int, t_msg*));
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

int enviar_mensaje_nodo_close(int fd);
int recibir_mensaje_nodo_ok(int fd);
int enviar_mensaje_nodo_ok(int fd);

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
 * Genera una nueva secuencia de enteros pseudo-random a retornar por rand().
 */
void seedgen(void);

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
t_reduce* reduce_create(int id, int job_id, char* resultado, t_nodo_base* nodo_destino);
char* nodo_base_to_string(t_nodo_base* nb);
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
#endif /* UTIL_H_ */
