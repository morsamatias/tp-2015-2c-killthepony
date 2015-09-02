/*
 * util.c
 *
 *  Created on: 26/4/2015
 *      Author: utnso
 */

#include "util.h"



sem_t* sem_crear(int* shmid, key_t* shmkey, int contador_ftok){
	sem_t* sem= NULL;
	//pid_t pid; /*      fork pid                */
	//unsigned int n; /*      fork count              */
	unsigned int value; /*      semaphore value         */

	/* initialize a shared variable in shared memory */

	char* sem_name = string_from_format("pSem_%d", contador_ftok);

	//shmkey = ftok("/dev/null", 5); /* valid directory name and a number */
	*shmkey = ftok("/dev/null", contador_ftok); /* valid directory name and a number */

	//printf("shmkey for p = %d\n", *shmkey);
	*shmid = shmget(*shmkey, sizeof(int), 0644 | IPC_CREAT);
	if (*shmid < 0) { /* shared memory error check */
		perror("shmget\n");
		exit(1);
	}

	///////////////////////////////////////////
	value = 0;
	/* initialize semaphores for shared processes */
	//sem = sem_open("pSem", O_CREAT | O_EXCL, 0644, value);


	//sem = sem_open("pSem", O_CREAT , 0644, value);
	sem = sem_open(sem_name, O_CREAT , 0644, value);
	if(sem==SEM_FAILED){
		perror("sem_open___");
		printf("***************************************************sdfadfasd\n");
		_exit(1);
	}

	/* name of semaphore is "pSem", semaphore is reached using this name */
	//sem_unlink("pSem");
	sem_unlink(sem_name);
	/* unlink prevents the semaphore existing forever */
	/* if a crash occurs during the execution         */
	//printf ("semaphores initialized.\n\n");

	FREE_NULL(sem_name);

	return sem;
}
int ejecutar_script(char* path_script, char* name_script, int(*reader_writer)(int fdreader, int fdwriter), pthread_mutex_t* mutex, int c_ftok){
/*
	key_t shmkey;
	int shmid;
	sem_t *sem;
	sem = sem_crear(&shmid, &shmkey, c_ftok);
*/
	int pipes[NUM_PIPES][2];
	pthread_mutex_lock(mutex);

	// pipes for parent to write and read

	if (pipe(pipes[PARENT_READ_PIPE]) == -1)
		handle_error("pipe");

	if ((pipe(pipes[PARENT_WRITE_PIPE])) == -1)
		handle_error("pipe");

	///////////////////////////////////////////
	pid_t  pid;
	pid = fork();
	pthread_mutex_unlock(mutex);
	if (pid  < 0){
		//handle_error("fork pipe stdin stdout");
		perror("fork!!!!!!!!!!!");
		return -1;
	}

	if (pid == 0) {
		if (dup2(pipes[PARENT_WRITE_PIPE][READ_FD], STDIN_FILENO) < 0) {
			perror("dup2 STDIN_FILENO");
			_exit(1);
		}
		if (dup2(pipes[PARENT_READ_PIPE][WRITE_FD], STDOUT_FILENO) < 0) {
			perror("dup2 STDIN_FILENO");
			_exit(1);
		}

		close(pipes[PARENT_WRITE_PIPE][READ_FD]);
		close(pipes[PARENT_READ_PIPE][WRITE_FD]);
		close(pipes[PARENT_READ_PIPE][READ_FD]);
		close(pipes[PARENT_WRITE_PIPE][WRITE_FD]);


		//execl("/usr/bin/sort", "sort", (char*) NULL);
		//sem_post(sem);

		int rs = 1;
		do {
			rs = execl(path_script, name_script, (char*) NULL);
			perror("Errro execv");
			fprintf(stderr, "hola path:%s, name: %s, Res: %d\n", path_script, name_script, rs);
			usleep(100000);

			_exit(127);
		} while (rs < 0);

		close(pipes[PARENT_READ_PIPE][READ_FD]);
		close(pipes[PARENT_WRITE_PIPE][WRITE_FD]);


		_exit(127);

	} else {
		/*
		printf("AAAAAAAAAAntes sem_wait(sem);\n");
		sem_wait(sem);
		printf("DDDDDDDDDESPUES sem_wait(sem);\n");

		shmctl(shmid, IPC_RMID, 0);
		sem_destroy (sem);
		*/

		int rs;

		rs = close(pipes[PARENT_WRITE_PIPE][READ_FD]);
		if(rs!=0){
			perror("close1");
		}
		rs = close(pipes[PARENT_READ_PIPE][WRITE_FD]);
		if(rs!=0){
			perror("close2");
		}

		rs = reader_writer(pipes[PARENT_READ_PIPE][READ_FD], pipes[PARENT_WRITE_PIPE][WRITE_FD]);
		int status;
		waitpid(pid, &status, 0);


		//puts("listo");
		return rs;
	}
}


int ordenar(t_ordenar* param_ordenar){

	int _reader_writer(int fdreader, int fdwriter){

		int _writer(int *fdwriter) {
			int fd = *fdwriter;

			FILE* file = fopen(param_ordenar->origen , "r");
			if(file==NULL){
				perror("fopen ordenar");
				close(fd);
				return -1;
			}
			char* linea = NULL;
			linea = malloc(LEN_KEYVALUE);
			if(linea == NULL){
				perror("linea malloc order");
				close(fd);
				return -1;
			}
			size_t len_linea = LEN_KEYVALUE;
			//linea = NULL;
			//size_t len_linea = 0;
			int rs=0, cant_writes=0;
			while ((getline(&linea, &len_linea, file)) > 0) {
				cant_writes = escribir_todo(fd, linea, strlen(linea));
				if(cant_writes<0){
					rs=-1;
					break;
				}
			}
			free(linea);
			fclose(file);
			close(fd);
			return rs;
		}
		int rs;
		pthread_t th_writer, th_reader;

		if((rs = pthread_create(&th_writer, NULL, (void*) _writer, (void*)&fdwriter))!=0){
			perror("pthread_create writerrrrrrrrrrrrrrrrrrrrrrr");
			return -1;
		}
		//usleep(100);

		t_reader treader;
		treader.fd = fdreader;
		strcpy(treader.destino, param_ordenar->destino);
		//treader.destino = param_ordenar->destino;

		if( (rs = pthread_create(&th_reader, NULL, (void*) reader_and_save_as, (void*)&treader))!=0 ){
			perror("pthread_create readerrrr");
			return -1;
		}
		//usleep(100);

		//joineo
		int rswriter=-1, rsreader=-1;

		if( (pthread_join(th_writer, (void*)&rswriter))!=0 ){
			perror("pthread_join writerrrrrrrrrrrrrrrrrrrrrrr");
			return -1;
		}

		if( (pthread_join(th_reader, (void*)&rsreader))!=0 ){
			perror("pthread_join reader");
			return -1;
		}

		//pthread_mutex_lock(param_ordenar->mutex);
		//printf("Fin orden resultado: %s\n", treader.destino);
		//pthread_mutex_unlock(param_ordenar->mutex);
		//free(param_ordenar->destino);
		//free(param_ordenar->origen);
		//free(param_ordenar);
		if(rswriter<0)
			return -1;//fallo writer
		if(rsreader<0)
			return -2;//fallo reader;
		return 0;
	}
	int rs=0;

	rs = ejecutar_script("/usr/bin/sort", "sort", _reader_writer, param_ordenar->mutex, param_ordenar->contador_ftok);
	//rs = ejecutar_script("/home/utnso/Escritorio/tests/mapper.sh", "mapperR", _reader_writer);

	return rs;
}

int reader_and_save_as(t_reader* reader) {
	int count, rs;
	FILE* file = fopen(reader->destino, "w");
	if (file == NULL) {
		perror("fopen reader_and_save_as");
		close(reader->fd);
		return -1;
	}
	char buffer[LEN_KEYVALUE];
	while ((count = read(reader->fd, buffer, LEN_KEYVALUE)) > 0) {
		rs = fwrite(buffer, 1, count, file);
		if (rs <= 0) {
			perror("_________fwrite reader_and_save_as");
			//exit(-1);
			return -1;
		}
	}
	if (count < 0) {
		perror("read reader_and_save_as");
		close(reader->fd);
		return -1;
	}
	fclose(file);
	close(reader->fd);
	return 0;
}

int escribir_todo(int writer, char* data, int len){
	int aux = 0;
	int bytes_escritos = 0;
	do {
		aux = write(writer, data + bytes_escritos, len - bytes_escritos);

		if (aux < 0) {
			//printf("_____________write Error\n");
			perror("write:::::::::::::::::::::::");
			//exit(-1);
			return -1;
		}
		bytes_escritos = bytes_escritos + aux;
	} while (bytes_escritos < len);
	//fsync(pipes[PARENT_WRITE_PIPE][WRITE_FD]);
	return bytes_escritos;
}


//Size of each chunk of data received, try changing this
		#define CHUNK_SIZE 512
		/*
		    Receive data in multiple chunks by checking a non-blocking socket
		    Timeout in seconds
		*/
		int recv_timeout(int s , int timeout)
		{
		    int size_recv , total_size= 0;
		    struct timeval begin , now;
		    char chunk[CHUNK_SIZE];
		    double timediff;

		    //make socket non blocking
		    fcntl(s, F_SETFL, O_NONBLOCK);

		    //beginning time
		    gettimeofday(&begin , NULL);

		    while(1)
		    {
		        gettimeofday(&now , NULL);

		        //time elapsed in seconds
		        timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);

		        //if you got some data, then break after timeout
		        if( total_size > 0 && timediff > timeout )
		        {
		            break;
		        }

		        //if you got no data at all, wait a little longer, twice the timeout
		        else if( timediff > timeout*2)
		        {
		            break;
		        }

		        memset(chunk ,0 , CHUNK_SIZE);  //clear the variable
		        if((size_recv =  recv(s , chunk , CHUNK_SIZE , 0) ) < 0)
		        {
		            //if nothing was received then we want to wait a little before trying again, 0.1 seconds
		            usleep(100000);
		        }
		        else
		        {
		            total_size += size_recv;
		            printf("%s" , chunk);
		            //reset beginning time
		            gettimeofday(&begin , NULL);
		        }
		    }

		    return total_size;
		}

size_t file_get_size(char* filename) {
	struct stat st;
	stat(filename, &st);
	return st.st_size;
}



void map_free(t_map* map){
	//FREE_NULL(map->info->nodo_base);
	FREE_NULL(map->info->resultado);
	FREE_NULL(map->info);
	//free(map->archivo_nodo_bloque);para simplificar el free lo hace el archivo_destroy
	FREE_NULL(map);

}


t_mapreduce* mapreduce_create(int id, int job_id, char* resultado){
	t_mapreduce* new = malloc(sizeof*new);
	new->id = id;
	new->resultado = string_new();
	string_append(&(new->resultado), resultado);
	new->empezo = false;
	new->termino = false;
	new->job_id = job_id;
	return new;
}

t_map* map_create(int id, int job_id, char* resultado){
	t_map* new = malloc(sizeof*new);

	new->info = mapreduce_create(id, job_id, resultado);

	return new;
}

//une alos dos string con una barra
char* file_combine(char* f1, char* f2) {
	char* p = NULL;
	p = string_new();

	string_append(&p, f1);
	string_append(&p, "/");
	string_append(&p, f2);

	return p;

}
/*
//char ip[15];

char* ip_get(){
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;

	snprintf(ifr.ifr_name, IFNAMSIZ, "eth0");

	ioctl(fd, SIOCGIFADDR, &ifr);



	strcpy(ip, inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr));
	//fprintf(ip, "%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

	printf("%s", ip);

	close(fd);
	return ip;
}
*/

int len_hasta_enter(char* strings){
		int i=0;
		while(strings[i]!='\n' && strings[i]!='\0')
			i++;

		return i+1;
	}


void file_mmap_free(char* mapped, char* filename) {
	munmap(mapped, file_get_size(filename));
}

int cant_registros(char** registros) {
	int i = 0;
	while (registros[i] != NULL) {
		i++;
	}
	return i;
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
	/*
	//el archivo ya esta creado con el size maximo
	void* mapped = NULL;
	struct stat st;
	int fd = 0;
	fd = open(filename, O_RDWR);
	if (fd == -1) {
		handle_error("open");
	}

	stat(filename, &st);
	//printf("%ld\n", st.st_size);
	size_t size = st.st_size;

	mapped = mmap(NULL, size, PROT_WRITE, MAP_SHARED | MAP_NORESERVE, fd, 0);
	close(fd);

	if (mapped == MAP_FAILED) {
		if(size==0)
			printf("el archivo tiene tamaño 0. Imposible mappear");
		handle_error("mmap");
	}

	return mapped;
	*/
}

/*
void free_null(void** data) {
	free(*data);
	*data = NULL;
	data = NULL;
}*/

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
	printf("server iniciado en %d\n", port);

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
					//char * ip;
					//newfd = accept_connection_and_get_ip(fdNuevoNodo, &ip);
					newfd = accept_connection(fdNuevoNodo);
					if (newfd < 0) {
						printf("no acepta mas conexiones\n");
					} else {
						//printf("nueva conexion desde IP: %s\n", ip);
						FD_SET(newfd, &master); // añadir al conjunto maestro
						if (newfd > fdmax) { // actualizar el máximo
							fdmax = newfd;
						}
					}

				} else { // gestionar datos de un cliente ya conectado
					t_msg *msg = recibir_mensaje(i);
					if (msg == NULL) {
						/* Socket closed connection. */
						//int status = remove_from_lists(i);
						printf("Conexion cerrada %d\n", i);
						close(i);
						FD_CLR(i, &master);
					} else {
						//print_msg(msg);
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
	//if (listen(sock_fd, BACK_LOG) < 0) {
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

	//make socket non blocking
	//fcntl(sock_fd, F_SETFL, O_NONBLOCK);

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


/*
int send_file(int socket, char* filename){
	size_t size = file_get_size(filename);

	int fd = open(filename, O_RDONLY);
	sendfile(socket, fd, NULL, size);
	close(fd);
	return 0;
}*/



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


t_mapreduce* recibir_mensaje_mapreduce(int fd){
	t_mapreduce* mr = NULL;
	t_msg* msg = NULL;
	//envio resultado, id, job_id
	msg = recibir_mensaje(fd);

	mr = mapreduce_create(msg->argv[0], msg->argv[1], msg->stream);

	destroy_message(msg);
	return mr;
}



t_archivo_nodo_bloque* archivo_nodo_bloque_create(t_nodo_base* nb, int numero_bloque){
	t_archivo_nodo_bloque* new = malloc(sizeof*new);

	new->numero_bloque = numero_bloque;
	new->base = nb;
	return new;
}


t_nodo_archivo* nodo_archivo_create(void){
	t_nodo_archivo* new = malloc(sizeof(t_nodo_archivo));
	new->nodo_base = NULL;

	return new;
}


void print_map(t_map* map){
	printf("*************************************************************\n");
	printf("Map_id: %d - Ubicacion: id_nodo: %d, %s:%d numero_bloque: %d, resultado: %s\n",
			map->info->id,
			map->archivo_nodo_bloque->base->id,
			map->archivo_nodo_bloque->base->red.ip,
			map->archivo_nodo_bloque->base->red.puerto,
			map->archivo_nodo_bloque->numero_bloque,
			map->info->resultado);
	printf("*************************************************************\n");
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
//			if ((auxInt=send(unSocket, &header, sizeof(Header), 0)) >= 0){
	auxInt = send(unSocket, bufferAux, (sizeof(t_header_base) + tamanio), 0);
	free(bufferAux);
	return auxInt;
			}

int recibir_mensaje_script(int socket, char* save_as){
	ssize_t len;
	char buffer[BUFSIZ];
	int file_size;
	FILE *received_file;
	int remain_data = 0;


	/* Receiving file size */
	recv(socket, buffer, BUFSIZ, 0);
	file_size = atoi(buffer);
	//fprintf(stdout, "\nFile size : %d\n", file_size);

	received_file = fopen(save_as, "w");
	if (received_file == NULL) {
		fprintf(stderr, "Failed to open file foo --> %s\n", strerror(errno));

		exit(EXIT_FAILURE);
	}

	remain_data = file_size;

	while (((len = recv(socket, buffer, BUFSIZ, 0)) > 0)
			&& (remain_data > 0)) {
		fwrite(buffer, sizeof(char), len, received_file);
		remain_data -= len;
		fprintf(stdout, "Receive %d bytes and we hope :- %d bytes\n", len,
				remain_data);
	}
	fclose(received_file);

	return 0;
}
//Mande un mensaje a un socket determinado
		//Recibe un mensaje del servidor - Version Lucas
int recibirMensajeConHeader(int unSocket, t_header_base* header, void** buffer) {

			int auxInt;
			if((auxInt=recv(unSocket, header, sizeof(t_header_base), 0))>=0) {
				*buffer = malloc (header->payloadlength);
				if ((auxInt=recv(unSocket, *buffer, header->payloadlength, 0)) >= 0) {
					return  auxInt;
				}
			}
			return  auxInt;

		}
int recibirMensaje(int unSocket, void** buffer) {

			t_header_base header;
			int auxInt;
			if((auxInt=recv(unSocket, &header, sizeof(t_header_base), 0))>=0) {
				*buffer = malloc (header.payloadlength);
				if ((auxInt=recv(unSocket, *buffer, header.payloadlength, 0)) >= 0) {
					return  auxInt;
				}
			}
			return  auxInt;

		}
		//Recibe un mensaje del servidor - Version Lucas
		int recibirHeader(int unSocket, t_header_base* header) {
			int auxInt;
				if((auxInt=recv(unSocket, header, sizeof(t_header_base), 0))>=0) {
					return auxInt;
					}
					return auxInt;
				}

		int recibirData(int unSocket, t_header_base header, void** buffer){
			int auxInt;
			*buffer = malloc (header.payloadlength);
					if ((auxInt=recv(unSocket, buffer, header.payloadlength, 0)) >= 0) {
						return auxInt;
					}
			return auxInt;
		}

int enviar_mensaje_script(int socket, char* path_script){
	//char file_data[CHUNK_SIZE];
	int rs =0;
	int size = file_get_size(path_script);

	/*
	FILE* file = fopen(path_script, "r");
	if(file==NULL){
		return -1;
	}
	void* data_file = malloc(size);
	rs = fwrite(data_file, size, 1, file);
	if(rs!=size){
		printf("RRRRRRRRRRRRRRRRRRRRRRRERR\n");
		return -1;
	}*/

	char* data_file = file_get_mapped(path_script);
	//enviar_mensaje_sin_header(socket, size, data_file);
	rs = mandarMensaje(socket, 0, size, data_file);

	if(rs<0){
		printf("ERRRRRRRRRRRRR\n");
		return -1;
	}

	//free(data_file);
	file_mmap_free(data_file, path_script);
	//fclose(file);
	return 0;

	/*
	ssize_t len;
	int fd;
	int sent_bytes = 0;
	char file_size[256];
	struct stat file_stat;
	int offset;
	int remain_data;

	fd = open(path_script, O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "Error opening file --> %s", strerror(errno));
		return -1;
	}

	if (fstat(fd, &file_stat) < 0) {
		fprintf(stderr, "Error fstat --> %s", strerror(errno));
		return -1;
	}

	fprintf(stdout, "File Size: \n%d bytes\n", file_stat.st_size);
	sprintf(file_size, "%d", file_stat.st_size);

	len = send(socket, file_size, sizeof(file_size), 0);
	if (len < 0) {
		fprintf(stderr, "Error on sending greetings --> %s", strerror(errno));

		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "Server sent %d bytes for the size\n", len);
*/
/*
	offset = 0;
	remain_data = file_stat.st_size;
	while (((sent_bytes = sendfile(socket, fd, &offset, BUFSIZ)) > 0)
			&& (remain_data > 0)) {
		fprintf(stdout,	"1. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n",sent_bytes, offset, remain_data);
		remain_data -= sent_bytes;
		fprintf(stdout,	"2. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n",sent_bytes, offset, remain_data);
	}
	perror("sendfile");
	*/

	return 0;

	//mandarMensaje(fd, 0, size, &buffer);
	/*
	char* script = file_get_mapped(path_script);
	t_msg* msg = string_message(JOB_SCRIPT, script, 0);
	enviar_mensaje(fd, msg);
	file_mmap_free(script, path_script);
	destroy_message(msg);
	*/
	return 0;
}

int recibir_mensaje_script_y_guardar(int socket, char* path_destino_script){
	t_header_base header;
	//recibirHeader(socket, &header);

	void* file_data ;//= malloc(header.payloadlength);
	recibirMensajeConHeader(socket, &header, &file_data);

	//recibirData(socket, header, &file_data);

	FILE* file = fopen(path_destino_script, "w");
	fwrite(file_data, header.payloadlength, 1, file);
	fclose(file);
	free(file_data);
	//settear permisos de ejecucion
	chmod(path_destino_script, S_IRWXU);

	/*ssize_t len;
	char buffer[BUFSIZ];
	int file_size;
	FILE *received_file;
	int remain_data = 0;


	recv(socket, buffer, BUFSIZ, 0);
	file_size = atoi(buffer);
	//fprintf(stdout, "\nFile size : %d\n", file_size);

	received_file = fopen(path_destino_script, "w");
	if (received_file == NULL) {
		fprintf(stderr, "Failed to open file foo --> %s\n", strerror(errno));

		exit(EXIT_FAILURE);
	}

	remain_data = file_size;

	while (((len = recv(socket, buffer, remain_data-1, 0)) > 0) && (remain_data > 0)) {
		fwrite(buffer, sizeof(char), len, received_file);
		remain_data -= len;
		fprintf(stdout, "Receive %d bytes and we hope :- %d bytes\n", len, remain_data);
	}

	fclose(received_file);
	chmod(path_destino_script, S_IRWXU);
*/
	/*
	t_msg* msg = NULL;
	//recibo el codigo del script
	msg = recibir_mensaje(fd);
	//guardo en el path destino el script
	write_file(path_destino_script,msg->stream, strlen(msg->stream));
	destroy_message(msg);
	//settear permisos de ejecucion
	chmod(path_destino_script, S_IRWXU);
	*/

	return 0;
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
	/*
	while (total < pending) {
		//int sent = send(sock_fd, buffer, msg->header.length + sizeof msg->header	+ msg->header.argc * sizeof(uint32_t), MSG_NOSIGNAL);
		int sent = send(sock_fd, buffer, msg->header.length + sizeof msg->header	+ msg->header.argc * sizeof(uint32_t), 0);
		if (sent < 0) {
			printf("sock_fd %d error\n", sock_fd);
			perror("send:::");
			FREE_NULL(buffer);
			return -1;
		}
		total += sent;
		pending -= sent;
	}*/

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
	//if (msg->header.argc && msg->argv != NULL)
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
	//perror("fopen");

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
	case NODO_CONECTAR_CON_FS:
		buf = strdup("NODO_CONECTAR_CON_FS");
		break;
	case FS_NODO_OK:
		buf = strdup("FS_NODO_OK");
		break;
	case NODO_SALIR:
		buf = strdup("NODO_SALIR");
		break;
	case CPU_NUEVO:
		buf = strdup("CPU_NUEVO");
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
/*
int convertir_path_absoluto(char**destino, char* file){
	//char* destino = malloc(PATH_MAX_LEN);
	if (getcwd(*destino, PATH_MAX_LEN) == NULL)
		handle_error("getcwd() error");
	strcat(*destino, file);

	return 0;
}
*/
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

char NODO_BASE_PRINT[30];
char* nodo_base_to_string(t_nodo_base* nb){
	memset(NODO_BASE_PRINT, 0, 30);
	sprintf(NODO_BASE_PRINT, "id:%d, %s:%d", nb->id, nb->red.ip, nb->red.puerto);
	//printf("%s\n", NODO_BASE_PRINT );
	return NODO_BASE_PRINT;
}


t_reduce* reduce_create(int id, int job_id, char* resultado, t_nodo_base* nb){
	t_reduce* new = malloc(sizeof*new);

	new->info = mapreduce_create(id,job_id, resultado);
	new->nodo_base_destino = nb;

	new->info->job_id = job_id;
	new->nodos_archivo = list_create();
	new->final = false;

	return new;
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
	//primero el pc, despues cant, a ejectuar
	new->cant_a_ejectuar = msg->argv[1];
	new->pc = msg->argv[0];
	new->cant_sentencias = msg->argv[2];

	return new;

}

int enviar_mensaje_pcb(int socket, t_pcb* pcb){
	t_msg* msg = NULL;
	int rs ;
	//primero el pc, 2 cant, a ejectuar, 3 cant sentencias,


	msg = string_message(PCB, pcb->path, 3, pcb->pc, pcb->cant_a_ejectuar, pcb->cant_sentencias);

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
