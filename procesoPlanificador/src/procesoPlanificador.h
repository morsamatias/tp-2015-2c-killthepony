/*
 * procesoPlanificador.h
 *
 *  Created on: 2/9/2015
 *      Author: utnso
 */

#ifndef PROCESOPLANIFICADOR_H_
#define PROCESOPLANIFICADOR_H_

#define NEW 0;
#define READY 1;
#define BLOCK 2;
#define EXEC 3;
#define FINISH 4;


#include <commons/log.h>
#include <pthread.h>

#include <util.h>
#include "consola.h"

#include "config_planif.h"
#include <commons/collections/list.h>


int correr_proceso(char* path);
int iniciar_server_select();

t_list* pcbs;/*lista de pcbs*/

/*
typedef struct {
	char path[MAX_PATH];
	int pc;
	int cant_a_ejectuar;
	int cant_sentencias;
	int pid;
	char* nombre_archivo_mcod;
	int estado_proceso;
}t_pcb;
*/

void pcb_agregar(t_pcb* pcb){
	list_add(pcbs, (void*)pcb);
}

/*cpu*/
t_list* cpus;

t_cpu* cpu_buscar(int id);
bool cpu_existe(int id);
t_cpu* cpu_nuevo(int id);
int cpu_disponible();
t_cpu* cpu_seleccionar();
int cpu_ejecutar(t_cpu* cpu, t_pcb* pcb);
int procesar_mensaje_cpu(int socket, t_msg* msg);

int cpu_ejecutar(t_cpu* cpu, t_pcb* pcb){


	t_msg* msg = argv_message(PCB_A_EJECUTAR, 0);
	enviar_y_destroy_mensaje(cpu->socket, msg);

	enviar_mensaje_pcb(cpu->socket, pcb);

	return 0;
}

/*
 * seleecciona el mejor cpu segun el algoritmo
 */
t_cpu* cpu_seleccionar(){
	return list_get(cpus, 0);
}

/*
 * verifica si hay cpus disponibles para ejecutar un proceso
 */
int cpu_disponible(){
	return list_size(cpus);
}

bool cpu_existe(int id){
	t_cpu* cpu = cpu_buscar(id);
	return cpu !=NULL;
}

t_cpu* cpu_buscar(int id){
	bool _cpu_buscar(t_cpu* cpu){
		return cpu->id == id;
	}
	return list_find(cpus, (void*)_cpu_buscar);
}

t_cpu* cpu_nuevo(int id){
	t_cpu* new = malloc(sizeof(*new));
	new->id = id;
	return new;
}

int cpu_agregar(t_cpu* cpu){
	list_add(cpus, (void*)cpu);

	return 0;
}

typedef struct {
	int pid;
}t_ready;

typedef struct {
	int pid;
}t_exec;

typedef struct {
	int pid;
}t_block;

typedef struct {
	int pid;
}t_new;

typedef struct {
	int pid;
}t_finish;

t_list* list_ready;/*lista de procesos listos para ejecutar*/

t_list* list_exec;/*lista de procesos en ejecución*/

t_list* list_block;/*lista de procesos bloqueados*/

t_list* list_new;/*lista de procesos nuevos*/

t_list* list_finish;/*lista de procesos terminados*/



#endif /* PROCESOPLANIFICADOR_H_ */
