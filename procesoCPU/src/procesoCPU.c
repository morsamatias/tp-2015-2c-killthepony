
#include "procesoCPU.h"

int main(void) {

	int contador=CANTIDAD_HILOS();

	int vector[contador];



	pthread_t *hilo=(pthread_t*)malloc((contador)*sizeof(pthread_t));

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
