#ifndef MAIN_KERNEL_H_
#define MAIN_KERNEL_H_

#include <kernel_gestor.h>
#include <inicializar_kernel.h>
#include <atender_mensajes.h>
#include <consola.h>
#include <atender_nuevas_interfaces.h>
#include <planificador_corto_plazo.h>
#include <planificador_largo_plazo.h>


//Variables Globales

int pid_acumulado = 0;
// int cantidad_de_proceso_en_ejecucion;
// int pid_a_eliminar;

t_log* logger;
t_log* logger_obligatorio;
t_config* config;

int kernel_server;
int entradasalida_cliente;
int kernel_cliente_dispatch;
int kernel_cliente_interrupt;
int kernel_cliente_memoria;

t_dictionary* diccionario_entrada_salida;
t_dictionary* diccionario_recursos;
pcb* proceso_en_ejecucion;


char* PUERTO_ESCUCHA;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* IP_CPU;
char* PUERTO_CPU_DISPATCH;
char* PUERTO_CPU_INTERRUPT;
char* PATH_SCRIPTS;
char* ALGORITMO_PLANIFICACION;
int QUANTUM;
char** RECURSOS;
char** INSTANCIAS_RECURSOS;
int GRADO_MULTIPROGRAMACION;

t_queue* cola_new;
t_queue* cola_ready;
t_dictionary* diccionario_blocked;
t_queue* cola_exit;
t_queue * cola_ready_prioritaria;

t_dictionary* diccionario_de_todos_los_procesos;

bool permitir_planificacion;

int espera_grado_multi;

//Semaforos
sem_t hay_proceso_en_ready;
sem_t hay_proceso_en_new;
sem_t hay_proceso_en_exit;
sem_t multiprogramacion_permite_proceso_en_ready;

sem_t detener_planificacion_exit;
sem_t detener_planificacion_to_ready;
sem_t detener_planificacion_corto_plazo;
sem_t detener_planificacion_salida_cpu;

pthread_mutex_t mutex_cola_new;
pthread_mutex_t mutex_cola_ready;
pthread_mutex_t mutex_cola_exit;
pthread_mutex_t mutex_cola_prioritaria;
pthread_mutex_t mutex_para_proceso_en_ejecucion;
pthread_mutex_t mutex_para_creacion_proceso;
pthread_mutex_t mutex_para_diccionario_entradasalida;
pthread_mutex_t mutex_para_diccionario_recursos;
pthread_mutex_t mutex_para_diccionario_blocked;
pthread_mutex_t mutex_para_eliminar_entradasalida;
pthread_mutex_t mutex_para_diccionario_de_todos_los_procesos;


#endif