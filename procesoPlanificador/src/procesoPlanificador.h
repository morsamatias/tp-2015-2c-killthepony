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
#include "time.h"

int correr_proceso(char* path);
int iniciar_server_select();
void cambiar_a_exec(int pid);

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

typedef struct {
	int pid;

}t_ready;

typedef struct {
	int pid;
}t_exec;




typedef struct {
	int pid;
	int tiempoIO;
	int estado;
}t_block;




typedef struct {
	int pid;
}t_finish;



t_list* list_ready;/*lista de procesos listos para ejecutar*/

t_list* list_exec;/*lista de procesos en ejecución*/

t_list* list_block;/*lista de procesos bloqueados*/

t_list* list_finish;/*lista de procesos terminados*/




typedef struct{
	int pid;
	int cant_sentencias;
	int tiempo_retorno;
	int tiempo_respuesta;
	int tiempo_espera;
	int tiempo_total;
}t_pcb_finalizado;

// tiempo de retorno: tiempo que tarda en terminar su ejecución un proceso

// tiempo de respuesta: tiempo que transcurre desde que se inicia una solicitud hasta que se recibe la primer respuesta

// tiempo de espero: tiempo total que pasa un proceso en la cola de listos

/*typedef struct{
	int id;
}t_cpu_especial;*/

void pcb_agregar(t_pcb* pcb){
	list_add(pcbs, (void*)pcb);
}

typedef struct{
	//t_cpu_base *base;
	int id;
	int socket;
	int usoUltimoMinuto;
	int estado;
}t_cpu;

int cpu_especial;

t_list* procesosFinalizados;

/*cpu*/
t_list* cpus;

t_cpu* cpu_buscar(int id);
t_cpu* cpu_buscar_por_socket(int socket);

bool _estado_blockeado(t_block block);
int esta_ocupado_IO();
bool cpu_existe(int id);
t_cpu* cpu_nuevo(int id);
int cpu_disponible();
t_cpu* cpu_seleccionar();
int cpu_ejecutar(t_cpu* cpu, t_pcb* pcb);
int procesar_mensaje_cpu(int socket, t_msg* msg);
t_pcb* pcb_buscar_por_cpu(int cpu);

int gl_id;
int gl_cpu;
int gl_socket;

bool _pcb_buscar_por_cpu(t_pcb* pcb){

		return pcb->cpu_asignado == gl_cpu;
	}

t_pcb* pcb_buscar_por_cpu(int cpu){

	gl_cpu=cpu;
	return list_find(pcbs, (void*)_pcb_buscar_por_cpu);
}

bool _cpu_buscar_por_socket(t_cpu* cpu){
		return cpu->socket == gl_socket;
	}
t_cpu* cpu_buscar_por_socket(int socket){

	gl_socket=socket;
	return list_find(cpus, (void*)_cpu_buscar_por_socket);
}


int cpu_ejecutar(t_cpu* cpu, t_pcb* pcb){

	t_msg* msg = argv_message(PCB_A_EJECUTAR, 0);
	enviar_y_destroy_mensaje(cpu->socket, msg);

	enviar_mensaje_pcb(cpu->socket, pcb);

	cambiar_a_exec(pcb->pid);

	return 0;
}

/*
 * seleecciona el mejor cpu segun el algoritmo
 */

int cpu_libre (t_cpu* cpu){
	return (cpu->estado==1);
}

t_cpu* cpu_seleccionar(){
	if(list_any_satisfy(cpus,(void*) cpu_libre)){
	return list_find(cpus, (void*) cpu_libre);}
	else{
		return NULL;
	}
}



//int cpu_libre (t_cpu* cpu);


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

bool _estado_bloqueado(t_block block){
	return block.estado==1;

}
bool _cpu_buscar(t_cpu* cpu){
		return cpu->id == gl_id;
	}
t_cpu* cpu_buscar(int id){
	gl_id=id;
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





t_pcb* es_el_pcb_buscado_por_id(int pid);

t_pcb* es_el_pcb_buscado_struct(t_pcb* pcb);

//t_pcb* es_el_pcb_buscado();

void Hilo_IO(int pid);
void controlar_IO (char* pid_string);

//void controlar_IO (int pid);

int es_el_pid_en_block(int pid, t_list* list_block);

int es_el_pcb_buscado_en_exec(t_exec* exec);

int es_el_pcb_buscado_en_ready(t_ready* ready);

int es_el_pcb_buscado_en_block(t_block* block);

int pos_del_pcb(int pid);

void procesar_msg_consola(char* msg);

double round_2(double X, int k);

int cpus_sin_dato_uso( t_cpu* cpu);

#endif /* PROCESOPLANIFICADOR_H_ */
