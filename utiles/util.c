/*
 * util.c
 *
 *  Created on: 26/4/2015
 *      Author: utnso
 */

#include "util.h"


/////////////////////////////////////////////////////////////////RETARDO/////////////////////////////////////////

void dormir(int segundos,int milisegundos){

	if (segundos == 0) {
		usleep(milisegundos*100);
	}else{
		sleep(segundos);
	}

}


///////////////////////////////////////////////////// ARCHIVOS Y MAPPEO/////////////////////////////////////////////
size_t file_get_size(char* filename) {
	struct stat st;
	stat(filename, &st);
	return st.st_size;
}


/*
 * devuelve el arhivo mappeado modo lectura y escritura
 */

void* file_get_mapped(char* filename) {

	char *addr;
	int fd;
	struct stat sb;
	size_t length;

	fd = open(filename, O_RDWR);
	if (fd == -1)
		handle_error("open");

	if (fstat(fd, &sb) == -1) /* To obtain file size */
		handle_error("fstat");



	length = sb.st_size;
	addr = mmap(NULL, length, PROT_READ | PROT_WRITE,MAP_SHARED | MAP_NORESERVE, fd, 0);
	if (addr == MAP_FAILED)
		handle_error("mmap");
	return addr;

}

///////////////////////////////////////////////////////LIBERAR MAPEO//////////////////////////////////////////////////

void file_mmap_free(char* mapped, char* filename) {
	munmap(mapped, file_get_size(filename));
}


int len_hasta_enter(char* strings){
		int i=0;
		while(strings[i]!='\n' && strings[i]!='\0')
			i++;

		return i+1;
}


int cant_registros(char** registros) {
	int i = 0;
	while (registros[i] != NULL) {
		i++;
	}
	return i;
}


bool file_exists(const char* filename) {
	bool rs = true;

	FILE* f = NULL;
	f = fopen(filename, "r");
	if (f != NULL) {
		fclose(f);
		rs = true;
	} else
		rs = false;

	return rs;
}

/*
 ***************************************************
 */
int server_socket_select(uint16_t port, void (*procesar_mensaje)(int, t_msg*)) {
	fd_set master, read_fds;
	int fdNuevoNodo, fdmax, newfd;
	int i;

	if ((fdNuevoNodo = server_socket(port)) < 0) {
		handle_error("No se pudo iniciar el server");
	}
	printf("Server iniciado: %d.\n", port);

	FD_ZERO(&master); // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);
	FD_SET(fdNuevoNodo, &master);

	fdmax = fdNuevoNodo; // por ahora el maximo

	//log_info(logger, "inicio thread eschca de nuevos nodos");
	// bucle principal
	for (;;) {
		read_fds = master; // cópialo
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			handle_error("Error en select");
		}

		// explorar conexiones existentes en busca de datos que leer
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!
				if (i == fdNuevoNodo) {	// gestionar nuevas conexiones

					newfd = accept_connection(fdNuevoNodo);
					if (newfd < 0) {
						printf("no acepta mas conexiones\n");
					} else {
						FD_SET(newfd, &master); // añadir al conjunto maestro
						if (newfd > fdmax) { // actualizar el máximo
							fdmax = newfd;
						}
					}

				} else { // gestionar datos de un cliente ya conectado
					t_msg *msg = recibir_mensaje(i);
					if (msg == NULL) {
						/* Socket closed connection. */
						printf("Conexion cerrada %d\n", i);
						close(i);
						FD_CLR(i, &master);
					} else {
						procesar_mensaje(i, msg);

					}

				} //fin else procesar mensaje nodo ya conectado
			}
		}
	}
	return 0;
}
int server_socket(uint16_t port) {
	int sock_fd, optval = 1;
	struct sockaddr_in servername;

	/* Create the socket. */
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		perror("socket");
		return -1;
	}

	/* Set socket options. */
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval)
			== -1) {
		perror("setsockopt");
		return -2;
	}

	/* Fill ip / port info. */
	servername.sin_family = AF_INET;
	servername.sin_addr.s_addr = htonl(INADDR_ANY);
	servername.sin_port = htons(port);

	/* Give the socket a name. */
	if (bind(sock_fd, (struct sockaddr *) &servername, sizeof servername) < 0) {
		perror("bind");
		return -3;
	}

	/* Listen to incoming connections. */
	if (listen(sock_fd, 127) < 0) {
		perror("listen");
		return -4;
	}

	return sock_fd;
}

int client_socket(char *ip, uint16_t port) {
	int sock_fd;
	struct sockaddr_in servername;

	/* Create the socket. */
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		perror("sockettt");
		return -1;
	}



	/* Fill server ip / port info. */
	servername.sin_family = AF_INET;
	servername.sin_addr.s_addr = inet_addr(ip);
	servername.sin_port = htons(port);
	memset(&(servername.sin_zero), 0, 8);

	/* Connect to the server. */
	if (connect(sock_fd, (struct sockaddr *) &servername, sizeof servername)
			< 0) {
		perror("connect");
		return -2;
	}

	return sock_fd;
}

int accept_connection(int sock_fd) {
	struct sockaddr_in clientname;
	size_t size = sizeof clientname;

	int new_fd = accept(sock_fd, (struct sockaddr *) &clientname,(socklen_t *) &size);
	if (new_fd < 0) {

		perror("accept");
		return -1;
	}

	return new_fd;
}

int accept_connection_and_get_ip(int sock_fd, char **ip) {
	struct sockaddr_in clientname;
	size_t size = sizeof clientname;

	int new_fd = accept(sock_fd, (struct sockaddr *) &clientname,
			(socklen_t *) &size);
	if (new_fd < 0) {
		perror("accept");
		return -1;
	}

	*ip = inet_ntoa(clientname.sin_addr);

	return new_fd;
}

t_msg *id_message(t_msg_id id) {

	t_msg *new = malloc(sizeof *new);

	new->header.id = id;
	new->argv = NULL;
	new->stream = NULL;
	new->header.argc = 0;
	new->header.length = 0;

	return new;
}




t_msg *argv_message(t_msg_id id, uint16_t count, ...) {
	va_list arguments;
	va_start(arguments, count);

	int32_t *val = malloc(count * sizeof *val);

	int i;
	for (i = 0; i < count; i++) {
		val[i] = va_arg(arguments, uint32_t);
	}

	t_msg *new = malloc(sizeof *new);
	new->header.id = id;
	new->header.argc = count;
	new->argv = val;
	new->header.length = 0;
	new->stream = NULL;

	va_end(arguments);

	return new;
}

int enviar_mensaje_sin_header(int sock_fd, int tamanio, void* buffer){
	int total=0, pending =tamanio;
	char *bufferAux = malloc(pending);
	memcpy(bufferAux, buffer, tamanio);

	/* Send message(s). */

	while (total < pending) {
		int sent = send(sock_fd, bufferAux, tamanio, MSG_NOSIGNAL);
		if (sent < 0) {
			FREE_NULL(bufferAux);
			return -1;
		}
		total += sent;
		pending -= sent;
	}
	FREE_NULL(bufferAux);
	return 0;
}

//Mande un mensaje a un socket determinado usando una estructura
int enviar_mensaje_flujo(int unSocket, int8_t tipo, int tamanio, void *buffer) {
	t_header_base header;
	int auxInt;
	//Que el tamanio lo mande
	void* bufferAux;

	header.type = tipo;
	header.payloadlength = tamanio;
	bufferAux = malloc(sizeof(t_header_base) + tamanio);
	memcpy(bufferAux, &header, sizeof(t_header_base));
	memcpy((bufferAux + (sizeof(t_header_base))), buffer, tamanio);
	auxInt = send(unSocket, bufferAux, (sizeof(t_header_base) + tamanio), 0);
	FREE_NULL(bufferAux);
	return auxInt;
}

int recibir_mensaje_flujo(int unSocket, void** buffer) {

	t_header_base header;
	int auxInt;
	if ((auxInt = recv(unSocket, &header, sizeof(t_header_base), 0)) >= 0) {
		*buffer = malloc(header.payloadlength);
		if ((auxInt = recv(unSocket, *buffer, header.payloadlength, 0)) >= 0) {
			return auxInt;
		}
	}
	return auxInt;

}

int sendAll(uint32_t fd, char *buf, uint32_t len,uint32_t opt){
	int32_t total = 0;
	int32_t left = len;
	int32_t sent = 0;

	while (total < len) {
		sent = send(fd, buf + total, left, opt);
		if (sent <= 0) {
			break;
		}
		total += sent;
		left -= sent;
	}

	if (sent == 0)
		//return SOCK_DISCONNECTED;
		return -1;
	else
		return -2;//no envio el paquete completo

	return total;

}

int mandarMensaje(int unSocket, int8_t tipo, int tamanio, void *buffer) {

	t_header_base header;
	int auxInt;
	//Que el tamanio lo mande
	void* bufferAux;
	header.type = tipo;
	header.payloadlength = tamanio;
	bufferAux = malloc(sizeof(t_header_base) + tamanio);
	memcpy(bufferAux, &header, sizeof(t_header_base));
	memcpy((bufferAux + (sizeof(t_header_base))), buffer, tamanio);

	auxInt = sendAll(unSocket, bufferAux, (sizeof(t_header_base) + tamanio), 0);
	free(bufferAux);
	return auxInt;
}

//Mande un mensaje a un socket determinado
//Recibe un mensaje del servidor - Version Lucas
int recibirMensajeConHeader(int unSocket, t_header_base* header, void** buffer) {

	int auxInt;
	if ((auxInt = recv(unSocket, header, sizeof(t_header_base), 0)) >= 0) {
		*buffer = malloc(header->payloadlength);
		if ((auxInt = recv(unSocket, *buffer, header->payloadlength, 0)) >= 0) {
			return auxInt;
		}
	}
	return auxInt;

}
int recibirMensaje(int unSocket, void** buffer) {

	t_header_base header;
	int auxInt;
	if ((auxInt = recv(unSocket, &header, sizeof(t_header_base), 0)) >= 0) {
		*buffer = malloc(header.payloadlength);
		if ((auxInt = recv(unSocket, *buffer, header.payloadlength, MSG_WAITALL))
				>= 0) {
			return auxInt;
		}
	}
	return auxInt;

}
//Recibe un mensaje del servidor - Version Lucas
int recibirHeader(int unSocket, t_header_base* header) {
	int auxInt;
	if ((auxInt = recv(unSocket, header, sizeof(t_header_base), 0)) >= 0) {
		return auxInt;
	}
	return auxInt;
}

int recibirData(int unSocket, t_header_base header, void** buffer) {
	int auxInt;
	*buffer = malloc(header.payloadlength);
	if ((auxInt = recv(unSocket, buffer, header.payloadlength, 0)) >= 0) {
		return auxInt;
	}
	return auxInt;
}



t_msg *string_message(t_msg_id id, char *message, uint16_t count, ...) {
	va_list arguments;
	va_start(arguments, count);

	int32_t *val = NULL;
	if (count > 0)
		val = malloc(count * sizeof *val);

	int i;
	for (i = 0; i < count; i++) {
		val[i] = va_arg(arguments, uint32_t);
	}

	t_msg *new = malloc(sizeof *new);
	new->header.id = id;
	new->header.argc = count;
	new->argv = val;
	new->header.length = strlen(message);
	new->stream = string_duplicate(message);

	va_end(arguments);

	return new;
}

int recibir_linea(int sock_fd, char*linea){
	//char* linea = malloc(LEN_KEYVALUE);
	char caracter=NULL;
	int bytes_leidos = 0;
	int status;
	do{
		status = recv(sock_fd, &caracter, 1, 0);
		linea[bytes_leidos] = caracter;
		bytes_leidos++;
	}while(status>0 && caracter!='\n' && caracter!='\0');
	if (caracter == '\n') {
		//status = -2;		//fin de linea
		linea[bytes_leidos] = '\0';
		//return linea;
		return 0;//fin OK
	}
	////////////////////////////////////////-
	if (caracter == '\0'){
		status = -3;
		return -1;//algo paso
	}
	/////////////////////////////////////
	perror("El nodo perdio conexion\n");
	//si llego hasta aca el nodo perdio conexion
	return -2;
}

t_cpu_base* cpu_base_new(int id, char* ip, int puerto){
	t_cpu_base* new = malloc(sizeof*new);
	new->id = id;
	strcpy(new->red.ip, ip);
	new->red.puerto = puerto;
	return new;
}



t_msg *recibir_mensaje(int sock_fd) {

	t_msg *msg = malloc(sizeof *msg);
	msg->argv = NULL;
	msg->stream = NULL;

	/* Get message info. */

	int status = recv(sock_fd, &(msg->header), sizeof(t_header), MSG_WAITALL);


	if (status < 0) {
		/* An error has ocurred or remote connection has been closed. */
		//printf("error al recibir header sock %d\n", sock_fd);
		perror("rec header");
		FREE_NULL(msg);
		return NULL;
	}
	if(status == 0 ){
		//printf("sock cerrado!%d\n", sock_fd);
		FREE_NULL(msg);
		return NULL;
	}


	/* Get message data. */
	if (msg->header.argc > 0) {
		msg->argv = malloc(msg->header.argc * sizeof(uint32_t));

		if (recv(sock_fd, msg->argv, msg->header.argc * sizeof(uint32_t), MSG_WAITALL) <= 0) {
			perror("rec args");
			FREE_NULL(msg->argv);
			FREE_NULL(msg);
			return NULL;
		}
	}

	if (msg->header.length > 0) {
		msg->stream = malloc(msg->header.length + 1);

		if (recv(sock_fd, msg->stream, msg->header.length, MSG_WAITALL) <= 0) {
			perror("rec stream");
			FREE_NULL(msg->stream);
			FREE_NULL(msg->argv);
			FREE_NULL(msg);
			return NULL;
		}

		msg->stream[msg->header.length] = '\0';
	}else{
		//si es null, hago que cargue una cadena vacia para que pueda hacer printf
		msg->stream = strdup("");
	}

	return msg;
}

t_msg* cpu_base_message(t_cpu_base* cb){
	t_msg* msg =NULL;
	msg = string_message(CPU_BASE, cb->red.ip,2, cb->red.puerto, cb->id);
	return msg;
}


int enviar_mensaje_cpu_base(int sock, t_cpu_base* cpu_base){
	t_msg* msg = NULL;
	msg = cpu_base_message(cpu_base);

	int rs = enviar_mensaje(sock, msg);
	destroy_message(msg);
	return rs;

	return 0;
}

t_cpu_base* recibir_mensaje_cpu_base(int s){
	t_msg* msg = recibir_mensaje(s);
	t_cpu_base* nb = NULL;
	if(msg->header.id == CPU_BASE){
		nb = cpu_base_new(msg->argv[1], msg->stream, msg->argv[0]);
	}
	destroy_message(msg);
	return nb;
}

int enviar_y_destroy_mensaje(int sock, t_msg* msg){
	int rs = enviar_mensaje(sock, msg);
	destroy_message(msg);
	return rs;
}

int enviar_mensaje(int sock_fd, t_msg *msg) {
	int total = 0;
	int pending = msg->header.length + sizeof(t_header)
			+ msg->header.argc * sizeof(uint32_t);
	char *buffer = malloc(pending);

	/* Fill buffer with the struct's data. */
	memcpy(buffer, &(msg->header), sizeof(t_header));

	int i;
	for (i = 0; i < msg->header.argc; i++)
		memcpy(buffer + sizeof(t_header) + i * sizeof(uint32_t), msg->argv + i,
				sizeof(uint32_t));

	memcpy(buffer + sizeof(t_header) + msg->header.argc * sizeof(uint32_t),
			msg->stream, msg->header.length);

	/* Send message(s). */
	while (total < pending) {
		//int sent = send(sock_fd, buffer, msg->header.length + sizeof msg->header + msg->header.argc * sizeof(uint32_t), MSG_NOSIGNAL);
		int sent = send(sock_fd, buffer, msg->header.length + sizeof msg->header + msg->header.argc * sizeof(uint32_t), MSG_NOSIGNAL);
		if (sent < 0) {
			free(buffer);
			return -1;

		}
		total += sent;
		pending -= sent;
	}

	FREE_NULL(buffer);

	return total;
}

void destroy_message(t_msg *msg) {
	if (msg->header.length  ){
		FREE_NULL(msg->stream);
	}
	else
		if(msg->stream != NULL && string_is_empty(msg->stream))
			FREE_NULL(msg->stream);
		FREE_NULL(msg->argv);
	FREE_NULL(msg);
}

void create_file(char *path, size_t size) {

	FILE *f = fopen(path, "wb");

	fseek(f, size - 1, SEEK_SET);

	fputc('\n', f);

	fclose(f);
}

void clean_file(char *path) {

	FILE *f = fopen(path, "wb");

	fclose(f);
}

char* read_file(char *path, size_t size) {

	FILE *f = fopen(path, "rb");
	if (f == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	char *buffer = malloc(size + 1);
	if (buffer == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	fread(buffer, size, 1, f);

	fclose(f);

	buffer[size] = '\0';

	return buffer;
}

void memcpy_from_file(char *dest, char *path, size_t size) {

	FILE *f = fopen(path, "rb");

	if (f != NULL) {
		fread(dest, size, 1, f);
		fclose(f);
	}
}

char *read_file_and_clean(char *path, size_t size) {

	FILE *f = fopen(path, "rb");
	if (f == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	char *buffer = malloc(size + 1);
	if (buffer == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	fread(buffer, size, 1, f);

	fclose(f);

	f = fopen(path, "wb");

	fclose(f);

	buffer[size] = '\0';

	return buffer;
}

char *read_whole_file(char *path) {

	FILE *f = fopen(path, "rb");
	if (f == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buffer = malloc(fsize + 1);
	if (buffer == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	fread(buffer, fsize, 1, f);

	fclose(f);

	buffer[fsize] = '\0';

	return buffer;
}

char *read_whole_file_and_clean(char *path) {

	FILE *f = fopen(path, "rb");
	if (f == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buffer = malloc(fsize + 1);
	if (buffer == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	fread(buffer, fsize, 1, f);

	fclose(f);

	buffer[fsize] = '\0';

	return buffer;
}

void write_file(char *path, char *data, size_t size) {

	FILE *f = fopen(path, "wb");

	fwrite(data, 1, size, f);

	fclose(f);
}

void print_msg(t_msg *msg) {
	int i;
	puts("\n==================================================");
	printf("CONTENIDOS DEL MENSAJE:\n");
	char* id = id_string(msg->header.id);
	printf("- ID: %s\n", id);
	FREE_NULL(id);

	for (i = 0; i < msg->header.argc; i++) {
		;
		printf("- ARGUMENTO %d: %d\n", i + 1, msg->argv[i]);
	}

	printf("- TAMAÑO: %d\n", msg->header.length);
	printf("- CUERPO: ");

	for (i = 0; i < msg->header.length; i++)
		putchar(*(msg->stream + i));
	puts("\n==================================================\n");
}
char *id_string(t_msg_id id) {
	char *buf;
	switch (id) {
	case CPU_NUEVO:
		buf = strdup("CPU_NUEVO");
		break;
	case MEM_OK:
		buf = strdup("MEM_OK");
		break;
	case MEM_NO_OK:
		buf = strdup("MEM_NO_OK");
		break;
	case SWAP_NO_OK:
		buf = strdup("SWAP_NO_OK");
		break;
	case SWAP_OK:
		buf = strdup("SWAP_OK");
		break;

	default:
		buf = string_from_format("%d, <AGREGAR A LA LISTA>", id);
		break;
	}
	return buf;
}




/*
 *
 */

float bytes_to_kilobytes(size_t bytes){
	return bytes / (1024 + 0.0);
}
float bytes_to_megabytes(size_t bytes){
	return bytes / ((1024*1024) + 0.0);
}


char* convertir_path_absoluto(char* file){
	char* destino = malloc(PATH_MAX_LEN);
	if (getcwd(destino, PATH_MAX_LEN) == NULL)
		handle_error("getcwd() error");
	strcat(destino, file);
	return destino;
}

int split_count(char** splitted){
	int count;
	for(count=0;splitted[count]!=NULL;count++);
	return count;
}


void free_split(char** splitted){
	int i = 0;
	while (splitted[i] != NULL) {
		FREE_NULL(splitted[i]);
		i++;
	}
	FREE_NULL(splitted);
}



int pcb_print(t_pcb* pcb){

	printf("Mostrar PCB::\n");
	printf("path: %s, pc: %d, cant_a_exec: %d\n", pcb->path, pcb->pc, pcb->cant_a_ejectuar);
	printf("**********************\n");
	return 0;
}

t_pcb* recibir_mensaje_pcb(int socket){
	t_msg* msg = NULL;
	t_pcb* new = NULL;

	msg = recibir_mensaje(socket);
	new = pcb_nuevo(msg->stream);
	//primero el pc, despues cant, a ejectuar//
	new->cant_a_ejectuar = msg->argv[1];
	new->pc = msg->argv[0];
	new->cant_sentencias = msg->argv[2];

	new->pid = msg->argv[3];//el pid

	return new;

}

int enviar_mensaje_pcb(int socket, t_pcb* pcb){

	t_msg* msg = NULL;
	int rs ;

	msg = string_message(PCB, pcb->path, 4, pcb->pc, pcb->cant_a_ejectuar, pcb->cant_sentencias, pcb->pid);

	rs = enviar_y_destroy_mensaje(socket, msg);

	return rs;
}



t_pcb* pcb_nuevo(char* path){
	t_pcb* new = malloc(sizeof(*new));
	strcpy(new->path, path);

	new->pc = 0;
	new->cant_a_ejectuar = 0;

	return new;
}


void lock(pthread_mutex_t* mutex){
	pthread_mutex_lock(mutex);
}
void unlock(pthread_mutex_t* mutex){
	pthread_mutex_unlock(mutex);
}


