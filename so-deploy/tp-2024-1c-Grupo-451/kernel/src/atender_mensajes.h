#ifndef ATENDER_MENSAJES_H_
#define ATENDER_MENSAJES_H_

#include <kernel_gestor.h>
#include <eliminar_elemento_de_cola.h>
#include <planificador_corto_plazo.h>
#include <eliminar_proceso.h>

typedef struct 
{
    nodo_de_diccionario_interfaz* nodo;
    int pid;
    int tiempo_espera;
} estructura_esperar_gen;



void atender_memoria();
int recibir_PC_memoria();
void atender_cpu_dispatch();
nodo_de_diccionario_interfaz* comprobrar_existencia_de_interfaz(pcb*, char* ,char* );
void recibir_contexto_de_ejecucion(t_buffer* ,pcb*); 
void atender_cpu_interrupt();
io_std_fs* extraer_dir_fisicas_de_buffer(t_buffer*);
void atender_entradasalida_kernel(void*);


#endif