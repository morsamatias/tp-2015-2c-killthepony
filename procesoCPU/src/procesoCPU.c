
#include "procesoCPU.h"


int main(void) {
	inicializar();
	int contador=CANTIDAD_HILOS();
	

	socket_planificador = malloc(contador*sizeof(int));
	socket_memoria = malloc(contador*sizeof(int));
	
	sentencias_ejecutadas_ultimo_min = malloc(contador*sizeof(int));
	porcentaje_a_planificador = malloc (contador*sizeof(int));
	inicializar_porcentajes();

	//hilo que escucha las peticiones del planificador

	pthread_t th_responder_porcentaje;
	pthread_create(&th_responder_porcentaje, NULL,(void*)&hilo_responder_porcentaje,NULL );
	pthread_detach(th_responder_porcentaje);

	//hilo que va calculando el % de cpu cada minuto
	pthread_t th_calcular_porcentaje;
	pthread_create(&th_calcular_porcentaje, NULL,(void*)&hilo_porcentaje,NULL );
	pthread_detach(th_calcular_porcentaje);

	pthread_t *hilo=(pthread_t*)malloc((contador)*sizeof(pthread_t));


	int* vector= malloc (contador*sizeof(int));
	int* numero ;

	for( ;contador>0;contador=contador-1){
		//vector[contador-1]=contador-1;
		//pthread_create(&(hilo[contador-1]), NULL,(void*)&hilo_cpu,(void*)&(vector[contador-1]) );
		numero = malloc(sizeof(int));
		*numero = contador-1;
		pthread_create(&(hilo[contador - 1]), NULL, (void*) &hilo_cpu, (void*) numero);
	}

	for(contador=CANTIDAD_HILOS();contador>0;contador=contador-1){

		pthread_join(hilo[contador-1],NULL);
	}
	free(vector);

	finalizar();
	return EXIT_SUCCESS;
}
