#include "procesoCPU.h"

///////////////////////////////////////////VARIBALES///////////////////////////////////////////////////////////////////////////////////////////////

char* CONFIG_PATH = "/home/utnso/Escritorio/git/tp-2015-2c-killthepony/procesoCPU/Debug/config.txt";
char* LOGGER_PATH = "log.txt";
//////////////////////////////////////HILO RESPONDE PORCENTAJE//////////////////////////////////////////////////////////////////////


void* hilo_responder_porcentaje() {

	t_msg* msg ;

	socket_planificador_especial = conectar_con_planificador_especial();

	if (socket_planificador_especial>0) {
		while (true) {

			log_trace(logger, "[PORCENTAJE] Esperando peticiones CPU_PORCENTAJE_UTILIZACION del planificador");
			msg = recibir_mensaje(socket_planificador_especial);

			if(msg!=NULL){												 //VALIDACION //

				if(msg->header.id == CPU_PORCENTAJE_UTILIZACION){ 		//VALIDA QUE SEA EL MENSAJE ESPERADO//

					destroy_message(msg);
					log_trace(logger, "[PORCENTAJE] Nuevo mensaje del planificador  CPU_PORCENTAJE_UTILIZACION");
					enviar_porcentaje_a_planificador();

				}else{
					log_error(logger, "[PORCENTAJE] Mensaje desconocido");
					break;
				}
			}else{
				log_error(logger, "[PORCENTAJE]: El planificador se desconecto");
				desconexion_planificador();
				break;
			}


		}
	} else
		log_trace(logger,
				"[PORCENTAJE] Error del socket especial al conectarse con el planificador. \n");

	return NULL;
}



///////////////////////////////////////FUNCION ENVIAR PORCENTAJE/////////////////////////

int enviar_porcentaje_a_planificador() {

	int i,numero_de_cpu,porcentaje;
	int retorno = 1;
	int cantidad = CANTIDAD_HILOS();

	t_msg* mensaje_a_planificador;

	for (numero_de_cpu=0;numero_de_cpu<cantidad;numero_de_cpu++){

		pthread_mutex_lock(&mutex);

		porcentaje = porcentaje_a_planificador[numero_de_cpu];
		log_trace(logger, "[HILO #%d]: CPU_PORCENTAJE_UTILIZACION: %d%c.", numero_de_cpu, porcentaje,'%');

		pthread_mutex_unlock(&mutex);

		mensaje_a_planificador = argv_message(CPU_PORCENTAJE_UTILIZACION, 2, numero_de_cpu, porcentaje);

		i = enviar_y_destroy_mensaje(socket_planificador_especial, mensaje_a_planificador);
		if (i < 0){
			retorno = -1;
			desconexion_planificador();
		}
	}
	return retorno;
}


///////////////////////////////////////PORCENTAJE////////////////////////////////////////


void inicializar_porcentajes(){

	int contador;

	for (contador = 0; contador < CANTIDAD_HILOS(); ++contador) {

		tiempo_ejecucion_ultimo_minuto[contador] = 0;
		sentencias_ejecutadas_ultimo_min[contador] = 0;

		porcentaje_a_planificador[contador] = 0;
		aux[contador] = 0;
	}
}

void* hilo_porcentaje(){

		int numero;

		if(PORCENTAJE_CPU() == 0){

			int numero,CANT_SENT_EN_UN_MIN;

			if (RETARDO() == 0) {
					CANT_SENT_EN_UN_MIN = dividir (60000,(RETARDO_MINIMO())); //CALCULO CANTIDAD DE SENTENCIAS MAXIMAS COMO SON MICROSEGUNDOS MULTIPLICO * 1000//
			}else{
					CANT_SENT_EN_UN_MIN = dividir (60 ,RETARDO());                 //CALCULO CANTIDAD DE SENTENCIAS MAXIMAS EN SEGUNDOS//
			}

			while(true){

				sleep(60); //ACTUALIZA A CADA MINUTO//

				pthread_mutex_lock(&mutex);
				printf( "\n*****************************************************************************\n");
				log_info(logger, "[PORCENTAJE] CALCULANDO PORCENTAJE DE UTILIZACION DE CADA HILO EN EL ULTIMO MINUTO");
				log_info(logger, "[PORCENTAJE] CANTIDAD DE SENTENCIAS MAXIMAS EN UN MIN: %d", CANT_SENT_EN_UN_MIN);


				for (numero = 0; numero < CANTIDAD_HILOS(); numero++) {

					if(CANT_SENT_EN_UN_MIN != 0){

					porcentaje_a_planificador[numero] = dividir(sentencias_ejecutadas_ultimo_min[numero]*100,CANT_SENT_EN_UN_MIN);

					log_trace(logger, "[PORCENTAJE] [HILO #%d]:SENT_EJECT_ULTIMO_MIN: %d CALCULO CPU_PORCENTAJE_UTILIZACION: %d%c.", numero,sentencias_ejecutadas_ultimo_min[numero], porcentaje_a_planificador[numero],'%');

					}else{

						porcentaje_a_planificador[numero] = 0;

						log_trace(logger, "[PORCENTAJE] [HILO #%d]:SENT_EJECT_ULTIMO_MIN: %d CALCULO CPU_PORCENTAJE_UTILIZACION: %d.", numero,sentencias_ejecutadas_ultimo_min[numero], porcentaje_a_planificador[numero]);

					}

					sentencias_ejecutadas_ultimo_min[numero] = 0;
				}

				printf("\n*****************************************************************************\n");
				pthread_mutex_unlock(&mutex);
			}



		}else{

		while(true){

			sleep(60); //ACTUALIZA A CADA MINUTO//

			pthread_mutex_lock(&mutex);
			printf( "\n*****************************************************************************\n");
			log_info(logger, "[PORCENTAJE] CALCULANDO PORCENTAJE DE UTILIZACION DE CADA HILO EN EL ULTIMO MINUTO");

			for (numero = 0; numero < CANTIDAD_HILOS(); numero++) {

				if (tiempo_ejecucion_ultimo_minuto[numero]>60){
					tiempo_ejecucion_ultimo_minuto[numero] = 60;
				}

				porcentaje_a_planificador[numero] = dividir(tiempo_ejecucion_ultimo_minuto[numero]*100,60);

				log_trace(logger, "[PORCENTAJE] [HILO #%d]:SENT_EJECT_ULTIMO_MIN: %d CALCULO CPU_PORCENTAJE_UTILIZACION: %d%c.", numero,tiempo_ejecucion_ultimo_minuto[numero], porcentaje_a_planificador[numero], '%');

				tiempo_ejecucion_ultimo_minuto[numero] = 0;

			}

			printf("\n*****************************************************************************\n");
			pthread_mutex_unlock(&mutex);

			}
		}
		return NULL;
}




int tiempo(int tiempo_inicial ,int tiempo_final,int hilo_cpu){

	if(PORCENTAJE_CPU()){

		tiempo_ejecucion_ultimo_minuto[hilo_cpu]=difftime(tiempo_final,tiempo_inicial) + tiempo_ejecucion_ultimo_minuto[hilo_cpu];

		//log_trace(logger, "[HILO #%d] SEGUNDOS DE USO: %d.\n",hilo_cpu,tiempo_ejecucion_ultimo_minuto[hilo_cpu]);

	}

	return 0;

}
///////////////////////////////////////HILO CPU ///////////////////////////////////////////////

void* hilo_cpu(int *numero_hilo) {

	int numero = *numero_hilo;
	int sock_mem,sock_planif;

	t_msg* mensaje_planificador = NULL;


	sock_mem = conectar_con_memoria(numero);
	sock_planif = conectar_con_planificador(numero);

	pthread_mutex_lock(&mutex);
	socket_memoria[numero] = sock_mem;               //AGREGO CADA SOCKET A EL VECTOR DE SOCKETS DE MEMORIA//
	socket_planificador[numero] = sock_planif;		//AGREGO CADA SOCKET A EL VECTOR DE SOCKETS DE PLANIFICADORES//
	pthread_mutex_unlock(&mutex);


	if (socket_memoria[numero]>0 && socket_planificador[numero]>0) {

		while (true) {

			log_trace(logger, "[HILO #%d] Esperando peticiones del planificador", numero);
			mensaje_planificador = recibir_mensaje(socket_planificador[numero]);

			if(mensaje_planificador!=NULL){

				log_trace(logger, "[HILO #%d] Nuevo mensaje del planificador", numero);
				procesar_mensaje_planif(mensaje_planificador, numero);

			}else{

				log_error(logger, "[HILO #%d] Error al recibir mensaje del planif", numero);
				desconexion_planificador();
				break;
			}}

	} else{
		log_trace(logger,"[HILO #%d] Error al conectarse con la memoria y el planificador.", numero);
	}

	return NULL;
}


///////////////////////////////////////////////////////////////	RETARDO /////////////////////////////////////////////////////////


void dormir2(){

	int retardo = RETARDO();

	//printf(" RETARDO %d.\n",retardo);

	if (retardo == 0) {
		usleep(RETARDO_MINIMO()*1000);
		//printf(" RETARDO MINIMO \n");
	}else{
		sleep(RETARDO());
		//printf(" RETARDO \n");
	}

}


///////////////////////////////////////////////////////////////PROCESAR MENSAJE///////////////////////////////////////////////

int procesar_mensaje_planif(t_msg* msg, int numero) {

	int retorno = 1;

	t_pcb* pcb = NULL;
	t_resultado_pcb mensaje_planificador;

	switch (msg->header.id) {

	case PCB_A_EJECUTAR:

		destroy_message(msg);

		pcb = recibir_mensaje_pcb(socket_planificador[numero]);

		printf("\n\n");

		log_trace(logger, "[HILO #%d] -Ejecutando: %s.\n-PID: %d.\n-PC: %d.\n-Sentencias a Ejecutar: %d.\n-Cantidad sentencias: %d.",
				numero, pcb->path, pcb->pid, pcb->pc, pcb->cant_a_ejectuar,pcb->cant_sentencias);

		mensaje_planificador = ejecutar(pcb, socket_memoria[numero], numero);

		printf("\n");

		log_trace(logger, "[HILO #%d] -Fin ejecucion: %s.\n-PID: %d.\n.PC: %d.\n Sentencias a Ejecutar: %d.\n-Cantidad de sentencias: %d",
						numero, pcb->path, pcb->pid, pcb->pc, pcb->cant_a_ejectuar,pcb->cant_sentencias);

		printf("\n\n");

		retorno = avisar_a_planificador(mensaje_planificador, socket_planificador[numero], numero);

		FREE_NULL(pcb);



		return retorno;

	default :											 //EN CASO DE ERROR//

		destroy_message(msg);

		log_trace(logger,"[HILO #%d] MENSAJE DESCONOCIDO.");
		//mensaje_error_planificador();

		FREE_NULL(pcb);

		return (-1);

	}

	return 0;
}

int pcb_tiene_que_seguir_ejecutando(t_pcb* pcb) {
	return pcb->pc < pcb->cant_sentencias;
}

/*
 * formatear para que quede solo texto
 */

char* get_texto_solo(char* texto) {
	char* txt;
	//borro la commilla de principio y final
	int len = strlen(texto) - 2;
	txt = malloc(len + 1);
	memset(txt, 0, len); //inicializo en 0
	memcpy(txt, texto +1 , len);
	txt[len - 1] = '\0';
	return txt;
}

char* leer_hasta_el_final(char* texto){

	int i=0;

	while(texto[i]!='\n' && texto[i]!='\0'){
		i++;
	}

	char* rs = string_substring_until(texto, i);
	return rs;
}

char* leer_hasta_espacio(char* texto){
	int i=0;
	while(texto[i]!='\n' && texto[i]!='\0' && texto[i]!=' ' ){
		i++;
	}

	char* rs = string_substring_until(texto, i);
	return rs;
}

char** splitear_sentencia(char* sent){
	//char** rs = string_n_split(sent, 2, " ");

	char**rs = malloc(4*(sizeof(char*)));

	rs[0] = NULL;rs[1] = NULL;rs[2] = NULL;
	rs[3] = NULL;				//PARA SABER CUANDO CORTAR AL HACER EL FREE_SPLIT


	rs[0] = leer_hasta_espacio(sent);//nombre ej: iniciar, leer, escribir, entrada-salida, finalizar

								//si no es el finalizar, el segundo param si o si es un nro
	if(!string_starts_with(sent, "finalizar")){
		rs[1] = leer_hasta_espacio(sent+strlen(rs[0])+1);

								//si comienza con escribir leo el siguiente param que es el contenido de la pagina
							//ej: escribir 1 "hola que talco!"
		if(string_starts_with(sent, "escribir")){

			char* texto_con_commillas = leer_hasta_el_final(sent + strlen(rs[0]) + strlen(rs[1]) + 2);
			rs[2] = get_texto_solo(texto_con_commillas);
			free(texto_con_commillas);
		}
	}
	return rs;
}


t_sentencia* sent_crear(char* sentencia, int pid, int hilo) {

	t_sentencia* sent = malloc(sizeof(*sent));
	sent->hilo = hilo;
	sent->pagina = -1;
	sent->pid = pid;
	sent->tiempo = -1;
	sent->texto = NULL;
	sent->cant_paginas = -1;
	sent->sentencia = -1;

	char** split = splitear_sentencia(sentencia);

	if (string_starts_with(sentencia, "iniciar ")) {
		sent->sentencia = iniciar;
		sent->cant_paginas = atoi(split[1]);

	} else if (string_starts_with(sentencia, "leer ")) {
		sent->sentencia = leer;
		sent->pagina = atoi(split[1]);


	} else if (string_starts_with(sentencia, "escribir ")) {
		sent->sentencia = escribir;
		sent->pagina = atoi(split[1]);
		sent->texto = string_duplicate(split[2]);

	} else if (string_starts_with(sentencia, "entrada-salida ")) {
		sent->sentencia = io;
		sent->tiempo = atoi(split[1]);

	} else if (string_starts_with(sentencia, "finalizar")) {
		sent->sentencia = final;

	} else {
		printf("Error para desconocido.\n");
		sent->sentencia = error;
	}

	free_split(split);
	return sent;
}

////////////////////////////////////////////////////////EJECUTAR INICIAR///////////////////////////////////

int sent_ejecutar_iniciar(t_sentencia* sent, int socket_mem) {

	int rs = 0;
	int i;

	t_msg* msg = NULL;

	msg = argv_message(MEM_INICIAR, 2, sent->pid, sent->cant_paginas);

	log_trace(logger, "[HILO #%d] Enviando mensaje MemIniciar %d.", sent->hilo, sent->cant_paginas);

	i = enviar_y_destroy_mensaje(socket_mem, msg);

	if (i < 0){							//CAPTURA ERRORES//
			desconexion_memoria();
			return i;
	}

	log_trace(logger, "[HILO #%d] Esperando Rta MemIniciar %d", sent->hilo, sent->cant_paginas);
	msg = recibir_mensaje(socket_mem);

	if (msg != NULL) {						//NULL//

			if (msg->header.id == MEM_OK) {			//HEADER//
													//el argv[0] es el estado//
				log_trace(logger, "[HILO #%d] Rta mensaje MemIniciar %d OK", sent->hilo,  msg->argv[0]);
				rs = 0; // OK

			} else {
				rs = -1; // NO OK
				//error_memoria();
			}

			destroy_message(msg);
			return rs;

	} else {
		log_trace(logger, "[HILO #%d] RTA MemIniciar NULL", sent->hilo);
		return -2;
	}
}

///////////////////////////////////////////////////////////////EJECUTAR FINALIZAR//////////////////////////////////////////////////////////////////


int sent_ejecutar_finalizar(t_sentencia* sent, int socket_mem) {

	int i,rs = 0;
	t_msg* msg = NULL;
	msg = argv_message(MEM_FINALIZAR, 1, sent->pid);
	log_trace(logger, "[HILO #%d] Enviando mensaje MEM_FINALIZAR ", sent->hilo);

	i= enviar_y_destroy_mensaje(socket_mem, msg);

	if (i < 0){
		i = -1;
		desconexion_memoria();
	}

	log_trace(logger, "[HILO #%d] Esperando Rta MEM_FINALIZAR", sent->hilo);

	msg = recibir_mensaje(socket_mem);

	if (msg != NULL) {

		if (msg->header.id == MEM_OK) {
			//el argv[0] es el estado
			log_trace(logger, "[HILO #%d] Rta mensaje MEM_FINALIZAR OK", sent->hilo);
			rs = 0; // OK

		} else {
			rs = -1; // NO OK
		}

		destroy_message(msg);
		return rs;

	} else {

		log_trace(logger, "[HILO #%d] RTA MEM_FINALIZAR NULL", sent->hilo);
		return -2;
	}
}


//////////////////////////////////////////////////////////EJECUTAR ESCRIBIR/////////////////////////////////////////////

int sent_ejecutar_escribir(t_sentencia* sent, int socket_mem) {

	int i,rs = 0;
	t_msg* msg = NULL;

	msg = string_message(MEM_ESCRIBIR, sent->texto, 2, sent->pid, sent->pagina);

	log_trace(logger, "[HILO #%d] Enviando mensaje MEM_ESCRIBIR %d %s", sent->hilo, sent->pagina,
			sent->texto);

	i = enviar_y_destroy_mensaje(socket_mem, msg);

	if (i < 0){
			i = -1;
			desconexion_memoria();
	}

	log_trace(logger, "[HILO #%d] Esperando rta mensaje MEM_ESCRIBIR %d",sent->hilo, sent->pagina,sent->texto);

	msg = recibir_mensaje(socket_mem);

	if (msg != NULL) {
		if (msg->header.id == MEM_OK) {
			//en el stream esta el contenido de la pagina
			log_trace(logger, "[HILO #%d] Rta mensaje MEM_ESCRIBIR %d", sent->hilo, msg->argv[0]);
			rs = 0; // OK
		} else {
			rs = -1;
		}
		destroy_message(msg);

		return rs;
	} else {
		log_trace(logger, "[HILO #%d] RTA MEM_ESCRIBIR NULL", sent->hilo);
		return -2;
	}
}

char* sent_ejecutar_leer(t_sentencia* sent, int socket_mem) {

	char* pagina = NULL;
	int i;

	t_msg* msg = NULL;
	msg = argv_message(MEM_LEER, 2, sent->pid, sent->pagina);
	log_trace(logger, "[HILO #%d] Enviando mensaje MEM_LEER %d", sent->hilo,  sent->pagina);

	i = enviar_y_destroy_mensaje(socket_mem, msg);

	if (i < 0){
			i = -1;
			desconexion_memoria();
	}

	log_trace(logger, "[HILO #%d] Esperando rta mensaje MEM_LEER %d",sent->hilo, sent->pagina);
	msg = recibir_mensaje(socket_mem);
	if (msg != NULL) {
		if (msg->header.id == MEM_OK) {
			//en el stream esta el contenido de la pagina
			log_trace(logger, "[HILO #%d] Rta mensaje: MEM_LEER %s", sent->hilo, msg->stream);
			pagina = string_duplicate(msg->stream);

		} else {
			pagina = NULL;
		}
		destroy_message(msg);

		return pagina;
	} else {
		log_trace(logger, "[HILO #%d] RTA MEM_LEER NULL", sent->hilo);
		return NULL;
	}
}

int sent_ejecutar(t_sentencia* sent, int socket_mem,int numero) {				//TINE INCLUIDO EL RETARDO//


	time_t tiempo_inicial,tiempo_final;
	pthread_mutex_lock(&mutex);
	sentencias_ejecutadas_ultimo_min[sent->hilo] ++;
	pthread_mutex_unlock(&mutex);

	tiempo_inicial=time(NULL);

	dormir2();

	int st = 0;
	switch (sent->sentencia) {
	case iniciar:
		st = sent_ejecutar_iniciar(sent, socket_mem);
		break;
	case leer:
		sent->texto = sent_ejecutar_leer(sent, socket_mem);
		if(sent->texto == NULL)
			st = -1;
		break;
	case escribir:
		st = sent_ejecutar_escribir(sent, socket_mem);
		break;
	case io:
		log_trace(logger, "[HILO #%d] Entrada-Salida de %d", sent->hilo, sent->tiempo);
		break;
	case error:
		log_trace(logger, "[HILO #%d] Error de sentencia en archivo", sent->hilo);
		st = -1;
		break;
	case final:
		sent_ejecutar_finalizar(sent, socket_mem);
		break;
	default:
		log_trace(logger, "[HILO #%d] case default", sent->hilo);
		break;
	}

	tiempo_final = time(NULL);
	tiempo(tiempo_inicial,tiempo_final,numero);

	return st;

	//return 0;
}

void sent_free(t_sentencia* sent) {

	if(sent->sentencia == escribir || sent->sentencia == leer){
		FREE_NULL(sent->texto);
	}

	FREE_NULL(sent);
}

t_resultado_pcb ejecutar(t_pcb* pcb, int socket_mem, int hilo) {

	int st = 0;
	int cantidad_a_ejecutar = pcb->cant_a_ejectuar;
	int sentencias_ejecutadas = 0;

	char* mcod = file_get_mapped(pcb->path);
	char** sents = string_split(mcod, "\n");

	t_resultado_pcb resultado;
	resultado.resultados_sentencias = list_create();


	t_sentencia* sent = NULL;
	e_sentencia ultima_sentencia_ejecutada;

	sent = sent_crear(sents[pcb->pc], pcb->pid, hilo);

	while ((sent->sentencia != final) && (sent->sentencia != io)                       //QUE NO SEA IO NI FINAL//
			&& (cantidad_a_ejecutar != sentencias_ejecutadas) && (st == 0)) {		   //NI SE HAYAN EJECUTADO TODAS//


		ultima_sentencia_ejecutada = sent->sentencia;

		st = sent_ejecutar(sent, socket_mem,hilo);
		sentencias_ejecutadas = sentencias_ejecutadas + 1;
		pcb->pc++;
		list_add(resultado.resultados_sentencias, sent);

		//sent_free(sent);
		sent = sent_crear(sents[pcb->pc], pcb->pid, hilo);

	}

	if (((sent->sentencia == final) && (cantidad_a_ejecutar != sentencias_ejecutadas)) || sent->sentencia == io){

		//O YA SE EJECUTARON TODAS
		st = sent_ejecutar(sent, socket_mem,hilo);
		ultima_sentencia_ejecutada = sent->sentencia;
		pcb->pc++;
		sentencias_ejecutadas++;
		list_add(resultado.resultados_sentencias, sent);
	}


	//si es una io, voy a la siguiente posicion y hago que esta ejecutando
	/*
	if(sent->sentencia == io){
		log_trace(logger, "[HILO #%d] Entrada-Salida de %d", hilo, sent->tiempo);
		sleep(RETARDO());
		ultima_sentencia_ejecutada = sent->sentencia;
		sentencias_ejecutadas = sentencias_ejecutadas + 1;
		pcb->pc++;
		list_add(resultado.resultados_sentencias, sent);
	} */

	file_mmap_free(mcod, pcb->path);

	free_split(sents);

	////////////////////////////////////
	resultado.pcb = pcb;
	resultado.ejecuto_ok = st == 0;//si es igual a cero ejecuto OK
	resultado.sentencia = ultima_sentencia_ejecutada;
	resultado.tiempo = sent->tiempo;
	resultado.cantidad_sentencias = sentencias_ejecutadas;
	//sent_free(sent);
	return resultado;

}

int avisar_a_planificador(t_resultado_pcb respuesta, int socket_planif, int hilo) {
	t_msg* mensaje_a_planificador;

	int i = 0;
	//printf("Sentencia: %d - IO: %d - FINAL: %d - ERROR: %d ",respuesta.sentencia,io,final,error);

	/*
	 * PRIMERO PID
	 * SEGUNDO SENTENCIA
	 * TERCERO TIEMPO
	 * CUARTO CANT_SENTENCIAS_EJECUTADAS
	 */
	//primero verifico si no ejecuto bien, para enviarle la sentencia que dio error (respuesta.sentencia)
	if (!respuesta.ejecuto_ok) {
		mensaje_a_planificador = argv_message(PCB_ERROR, 4, respuesta.pcb->pid,
				respuesta.sentencia, respuesta.tiempo,respuesta.cantidad_sentencias);
		log_trace(logger,
				"[HILO #%d] EnviarAlPlanif PCB_ERROR, PID:%d, sent:%d, t: %d, cant_sent_ejec:%d",
				hilo, respuesta.pcb->pid, respuesta.sentencia, respuesta.tiempo,
		respuesta.cantidad_sentencias);
			} else {
		//si ejecuto bien, le paso la info
		switch (respuesta.sentencia) {
		case io:
			//el planif recibe primero pid, despues tiempo
			mensaje_a_planificador = argv_message(PCB_IO, 4, respuesta.pcb->pid,
					respuesta.sentencia, respuesta.tiempo,
					respuesta.cantidad_sentencias);
			log_trace(logger,
					"[HILO #%d] EnviarAlPlanif PCB_IO, PID:%d, sent:%d, t: %d, cant_sent_ejec:%d",
					hilo, respuesta.pcb->pid, respuesta.sentencia,
					respuesta.tiempo, respuesta.cantidad_sentencias);
			if (avisar_a_memoria_io(socket_memoria[hilo],respuesta)<0)
							desconexion_memoria();
			break;
		case final:
			mensaje_a_planificador = argv_message(PCB_FINALIZAR, 4,
					respuesta.pcb->pid, respuesta.sentencia, respuesta.tiempo,
					respuesta.cantidad_sentencias);
			log_trace(logger,
					"[HILO #%d] ************** PCB_FINALIZAR, PID:%d *****************",
					hilo, respuesta.pcb->pid);
			log_trace(logger,
					"[HILO #%d] EnviarAlPlanif PCB_FINALIZAR, PID:%d, sent:%d, t: %d, cant_sent_ejec:%d",
					hilo, respuesta.pcb->pid, respuesta.sentencia,
					respuesta.tiempo, respuesta.cantidad_sentencias);
			break;
		case error:
			mensaje_a_planificador = argv_message(PCB_ERROR, 4,
					respuesta.pcb->pid, respuesta.sentencia, respuesta.tiempo,
					respuesta.cantidad_sentencias);
			log_trace(logger,
					"[HILO #%d] EnviarAlPlanif PCB_ERROR, PID:%d, sent:%d, t: %d, cant_sent_ejec:%d",
					hilo, respuesta.pcb->pid, respuesta.sentencia,
					respuesta.tiempo, respuesta.cantidad_sentencias);
			break;
		default: //si no entra a ninguno de los cases, significa que termino por fin de quantum
			mensaje_a_planificador = argv_message(PCB_FIN_QUANTUM, 4,
					respuesta.pcb->pid, respuesta.sentencia, respuesta.tiempo,
					respuesta.cantidad_sentencias);
			log_trace(logger,
					"[HILO #%d] EnviarAlPlanif PCB_FIN_QUANTUM, PID:%d, sent:%d, t: %d, cant_sent_ejec:%d",
					hilo, respuesta.pcb->pid, respuesta.sentencia,
					respuesta.tiempo, respuesta.cantidad_sentencias);
			break;
		}
	}

	i = enviar_y_destroy_mensaje(socket_planif, mensaje_a_planificador);

	if(i<0){
		log_error(logger, "[HILO #%d] ERROR al enviar el resultado al planificador. ", hilo);
		desconexion_planificador();
		return -1;
	}

	enviar_logs(socket_planif, respuesta.resultados_sentencias);


	//limpio la lista
	list_destroy_and_destroy_elements(respuesta.resultados_sentencias, (void*)sent_free);

	return 0;
}

int enviar_logs(int socket, t_list* resultados_sentencias){

	t_msg* msg;
	int i, rs = 0;
	t_sentencia* sent = NULL;

	for (i = 0; i < list_size(resultados_sentencias); ++i) {
		sent = (t_sentencia*) list_get(resultados_sentencias, i);

		msg = sent_to_msg(sent);

		if(enviar_mensaje(socket, msg)<0){

			log_error(logger, "[HILO #%d] ERROR AL ENVIAR LOG AL PLANIFICADOR", sent->hilo);
			desconexion_planificador();
			destroy_message(msg);
			rs = -1;
			return -1;

		}else{
		destroy_message(msg);
	}}
	return rs;
}

t_msg* sent_to_msg(t_sentencia* sent){

	t_msg* msg;

	switch (sent->sentencia) {
		case iniciar:
			msg = argv_message(PCB_LOGUEO, 3, sent->pid, sent->sentencia, sent->cant_paginas);
			log_trace(logger, "[HILO #%d] LOG INICIAR PID:%d Paginas: %d", sent->hilo, sent->pid, sent->cant_paginas);
			break;
		case final:
			msg = argv_message(PCB_LOGUEO, 2, sent->pid, sent->sentencia);
			log_trace(logger, "[HILO #%d] LOG FINAL PID:%d", sent->hilo, sent->pid);
			break;
		case leer:
			msg = string_message(PCB_LOGUEO, sent->texto, 3, sent->pid, sent->sentencia, sent->pagina);
			log_trace(logger, "[HILO #%d] LOG LEER PID:%d, Pagina: %d, Texto: \"%s\"", sent->hilo, sent->pid, sent->pagina, sent->texto);
			break;
		case escribir:
			msg = string_message(PCB_LOGUEO, sent->texto, 3, sent->pid, sent->sentencia, sent->pagina);
			log_trace(logger, "[HILO #%d] LOG ESCRIBIR PID:%d, Pagina: %d, Texto: \"%s\"", sent->hilo, sent->pid, sent->pagina, sent->texto);
			break;
		case io:
			msg = argv_message(PCB_LOGUEO, 3, sent->pid, sent->sentencia, sent->tiempo);
			log_trace(logger, "[HILO #%d] LOG ENVIAR ENTRADA-SALIDA PID:%d, Tiempo: %d", sent->hilo, sent->pid, sent->tiempo);
			break;
		default:
			log_error(logger, "[HILO #%d] ERROR", sent->hilo);
			msg = NULL;
			break;
	}

	return msg;
}



int avisar_a_memoria_io(int sock_m,t_resultado_pcb respuesta){

	t_msg* msg = NULL;
	int rs ;

	msg = string_message(MEM_IO, respuesta.pcb->path, 1,respuesta.pcb->pid);

	rs = enviar_y_destroy_mensaje(sock_m, msg);

	return rs;
}
/////////////////////////////////////////////////DESCONEXION DE PROCESOS/////////////////////

int desconexion_planificador(){

	int i = 0;

	if(flag_error_planificador == 0){

			t_msg* mensaje_a_memoria;

			pthread_mutex_lock(&mutex);
			log_trace(logger, "DESCONEXION DEL PLANIFICADOR SE LE AVISA AL ADMINISTRADOR DE MEMORIA.");
			pthread_mutex_unlock(&mutex);

			mensaje_a_memoria = argv_message(CAIDA_PLANIFICADOR, 0);
			i = enviar_y_destroy_mensaje(socket_memoria[0], mensaje_a_memoria);

			flag_error_planificador = 1;

			//abort();
			exit(-1);
	}

	if ((i== -1)&&(flag_error_memoria == 0)){
		log_trace(logger, "DESCONEXION DEL ADMINISTRADOR DE MEMORIA.");
		flag_error_memoria = 1;
		//abort();
		exit(-1);
	}

	return i;
}

int desconexion_memoria(){

	int i = 0;
	int flag_error_memoria;
	int flag_error_planificador;

	if (flag_error_memoria == 0){

			t_msg* mensaje_a_planificador;
			pthread_mutex_lock(&mutex);
			log_trace(logger, "Desconexion del administrador de memoria se avisa al planificador.");
			pthread_mutex_unlock(&mutex);

			mensaje_a_planificador = argv_message(CAIDA_MEMORIA, 0);

			i = enviar_y_destroy_mensaje(socket_planificador[0], mensaje_a_planificador);

			flag_error_memoria = 1;
	}

	if ((i== -1)&&(flag_error_planificador == 0)){

		log_trace(logger, "Desconexion del planificador.");
		flag_error_planificador = 1;
	}

return i;

}

//////////////////////////////////////////////////CONEXION CON MEMORIA Y PLANIFICADOR//////////

int conectar_con_memoria(int numero) {

	int sock;
	int retorno;
	sock = client_socket(IP_MEMORIA(), PUERTO_MEMORIA());


	pthread_mutex_lock(&mutex);
	if (sock < 0) {
		log_trace(logger, "[HILO #%d] Error al conectar con  admin Mem. %s:%d",numero,IP_MEMORIA(), PUERTO_MEMORIA());
		retorno = -1;
	} else {
		log_trace(logger, "[HILO #%d] Conectado con admin Mem. %s:%d", numero, IP_MEMORIA(), PUERTO_MEMORIA());
		retorno = sock;
	}
	pthread_mutex_unlock(&mutex);

	//envio handshake
	//envio un msj con el id del proceso
	t_msg* msg = string_message(CPU_NUEVO, "[HILO #%d] Hola soy un CPU ", 1, numero);
	int rs;
	rs = enviar_mensaje(sock, msg);
	pthread_mutex_lock(&mutex);
	if (rs > 0) {
		log_trace(logger, "[HILO #%d] Mensaje enviado OK CPU_NUEVO", numero);
	} else {
		log_trace(logger, "[HILO #%d] Error al enviar mensaje CPU_NUEVO", numero);
		retorno = -1;
	}
	pthread_mutex_unlock(&mutex);
	destroy_message(msg);

	//enviar_mensaje_cpu(sock);

	return retorno;
}

int conectar_con_planificador(int numero) {

	int sock;
	int retorno;

	sock = client_socket(IP_PLANIFICADOR(), PUERTO_PLANIFICADOR());

	if (sock < 0) {
		log_trace(logger, "[HILO #%d] Error al conectar con el planificador. %s:%d",numero,
				IP_PLANIFICADOR(), PUERTO_PLANIFICADOR());
	} else {
		log_trace(logger, "[HILO #%d] Conectado con planificador. %s:%d",numero,
				IP_PLANIFICADOR(), PUERTO_PLANIFICADOR());
				retorno = sock;
	}

	//envio handshake
	//envio un msj con el id del proceso
	t_msg* msg = string_message(CPU_NUEVO, "[HILO #%d] Hola soy un CPU", 1, numero);
	pthread_mutex_lock(&mutex);
	if (enviar_mensaje(sock, msg) > 0) {
		log_trace(logger, "[HILO #%d] Mensaje enviado OK", numero);
	}else{
		log_trace(logger, "[HILO #%d] Error al enviar mensaje CPU_NUEVO", numero);
		retorno = -1;
	}
	pthread_mutex_unlock(&mutex);
	destroy_message(msg);

	//enviar_mensaje_cpu(sock);

	return retorno;
}

int conectar_con_planificador_especial() {
	int sock;

	sock = client_socket(IP_PLANIFICADOR(), PUERTO_PLANIFICADOR());

	pthread_mutex_lock(&mutex);
	if (sock < 0) {
		log_trace(logger, "[PORCENTAJE] Error al conectar con el planificador especial. %s:%d",	IP_PLANIFICADOR(), PUERTO_PLANIFICADOR());
	} else {
		log_trace(logger, "[PORCENTAJE] Conectado con planificador especial. %s:%d",IP_PLANIFICADOR(), PUERTO_PLANIFICADOR());
	}
	pthread_mutex_unlock(&mutex);

	//envio handshake
	//envio un msj con el id del proceso
	t_msg* msg = string_message(CPU_ESPECIAL, "[PORCENTAJE] Hola soy un CPU", 1, -1);
	int rs;
	rs = enviar_mensaje(sock, msg);

	pthread_mutex_lock(&mutex);
	if (rs > 0) {
		log_trace(logger, "[PORCENTAJE] Mensaje enviado OK");
	} else {
		log_trace(logger, "[PORCENTAJE] Error al enviar mensaje CPU_ESPECIAL");
		sock = -1;
	}
	pthread_mutex_unlock(&mutex);
	destroy_message(msg);

	//enviar_mensaje_cpu(sock);

	return sock;
}
///////////////////////////////////////////////////INICIAZACION////////////////////////////////////
void config_inicializar(){


	clean_file(LOGGER_PATH);
	memset(ipmem, 0, 15);
	strcpy(ipmem, config_get_string_value(cfg, "IP_MEMORIA"));
	puertomem = config_get_int_value(cfg, "PUERTO_MEMORIA");

	memset(ipplanif, 0, 15);
	strcpy(ipplanif, config_get_string_value(cfg, "IP_PLANIFICADOR"));
	puertoplanif = config_get_int_value(cfg, "PUERTO_PLANIFICADOR");

	cant_hilos = config_get_int_value(cfg, "CANTIDAD_HILOS");

	printf("\n \n CONFIGURACION:\n - IP MEMORIA: %s.\n - PUERTO MEMORIA: %d.\n - IP PLANIFICADOR: %s.\n - PUERTO PLANIFICADOR: %d.\n - CANTIDAD HILOS: %d.\n - CONFIG PATH: %s.\n - LOGGER PATH: %s.\n\n",ipmem,puertomem,ipplanif,puertoplanif,cant_hilos,CONFIG_PATH,LOGGER_PATH);
}

int inicializar() {

	flag_error_memoria = 0;
	flag_error_planificador = 0;

	cfg = config_create(CONFIG_PATH);
	config_inicializar();

	clean_file(LOGGER_PATH);
	logger = log_create(LOGGER_PATH, "procesoCPU", true, LOG_LEVEL_TRACE);

	pthread_mutex_init(&mutex, NULL);

	return 0;
}

///////////////////////////////////////////////////FINALIZAR////////////////////////////////

int finalizar() {

	free(sentencias_ejecutadas_ultimo_min);
	free(porcentaje_a_planificador);

	config_destroy(cfg);
	log_destroy(logger);
	pthread_mutex_destroy(&mutex);
	return 0;
}

/////////////////////////////////////////////////DIVISION DE FLOATS////////////////////////////////

int dividir(unsigned long int sent_ejecutadas,int sent_maximas)
{
	unsigned long int valor;
	valor = (sent_ejecutadas/sent_maximas);
	return(valor);
}





//////////////////////////////////////////////////FUNCIONES CONFIGURACION////////////////////////


char* IP_MEMORIA() {
	return ipmem;
}
int PUERTO_MEMORIA() {
	return puertomem;
}

char* IP_PLANIFICADOR() {
	return ipplanif;
}
int PUERTO_PLANIFICADOR() {
	return puertoplanif;
}

int CANTIDAD_HILOS() {
	return cant_hilos;
}

int RETARDO() {
	return config_get_int_value(cfg, "RETARDO_SEGUNDOS");
}

int RETARDO_MINIMO() {
	return config_get_int_value(cfg, "RETARDO_MILISEGUNDOS");
}

int PORCENTAJE_CPU(){
	return config_get_int_value(cfg,"CALCULO_PORCENTAJE_CPU");
}

