
#include "procesoCPU.h"



int main(void) {
	inicializar();
	int contador=CANTIDAD_HILOS();
	

	socket_planificador = malloc(contador*sizeof(int));
	socket_memoria = malloc(contador*sizeof(int));
	
	porcentaje = malloc(contador*sizeof(int));
	porcentaje_a_planificador = malloc (contador*sizeof(int));

	int vector[contador];



	pthread_t *hilo=(pthread_t*)malloc((contador+2)*sizeof(pthread_t));
	pthread_create(&(hilo[contador+1]), NULL,(void*)&hilo_responder_porcentaje,NULL );

	for( ;contador>0;contador=contador-1){
		vector[contador]=contador;

		pthread_create(&(hilo[contador-1]), NULL,(void*)&hilo_cpu,(void*)&vector[contador] );
	}

	for(contador=CANTIDAD_HILOS();contador>0;contador=contador-1){

		pthread_join(hilo[contador-1],NULL);
	}

	finalizar();
	return EXIT_SUCCESS;
}
