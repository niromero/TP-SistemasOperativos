#ifndef CPU_GESTOR_H_
#define CPU_GESTOR_H_

/*
Este es el gestor del CPU, se declaran variables globales usando el extern asi todos los
modulos del CPU pueden utilizarlas. Las variables deben ser previamente declaradas en su modulo
*/

#include <stdlib.h>
#include <stdio.h>
#include <utils/utiles.h>

typedef enum{
    FINALIZAR,
    SEGUIR_EJECUTANDO,
    SLEEP_GEN,
    STD_READ,
    STD_WRITE,
    FS_CREATE,
    FS_DELETE,
    FS_TRUNCATE,
    FS_READ,
    FS_WRITE,
    WAIT_RECURSO,
    SIGNAL_RECURSO,
    SIN_MEMORIA
} salidas_cpu;

typedef enum{
    NO_INTERRUPCION,
    HUBO_INTERRUPCION
} interrupciones;

typedef struct{
    int base;
    int desplazamiento;
    int marco;
    int dir_fisica_final;
    int num_de_pag_base;
} direccion_fisica;

typedef struct {
    int pid;
    int num_pag;
    int marco;
    int ultima_vez_usada;
} entradas_tlb;


//Variables Globales

extern int cpu_server_dispatch;
extern int cpu_server_interrupt;
extern int cpu_cliente_memoria;
extern int kernel_cliente_dispatch;
extern int kernel_cliente_interrupt;

extern t_log* logger;
extern t_log* logger_obligatorio;
extern t_config* config;

extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* PUERTO_ESCUCHA_DISPATCH;
extern char* PUERTO_ESCUCHA_INTERRUPT;
extern int CANTIDAD_ENTRADAS_TLB;
extern char* ALGORITMO_TLB;

extern int tam_de_pags_memoria;

extern char* instruccion_a_decodificar;
extern char** instruccion_separada;
extern int pid_en_ejecucion;
extern int interrupcion_recibida;
extern int pid_de_interrupcion;

extern t_registros_cpu* los_registros_de_la_cpu;

extern t_list* tlb;

//Semaforos
extern pthread_mutex_t mutex_para_interrupcion;
extern pthread_mutex_t mutex_para_pid_interrupcion;

#endif