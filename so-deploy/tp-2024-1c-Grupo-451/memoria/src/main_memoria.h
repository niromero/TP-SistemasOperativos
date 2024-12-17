#ifndef MAIN_MEMORIA_H_
#define MAIN_MEMORIA_H_

#include <memoria_gestor.h>
#include <inicializar_memoria.h>
#include <atender_mensajes.h>
#include <leer_pseudocodigo.h>
#include <atender_nuevas_interfaces.h>

t_log* logger;
t_log* logger_obligatorio;
t_config* config;

void* memoria_de_usuario;
float cant_marcos_totales;
void* puntero_a_bits_de_los_marcos;
t_bitarray* marcos_de_memoria_libres;


int memoria_server;
int cpu_cliente;
int kernel_cliente;

char* PUERTO_ESCUCHA;
int TAM_MEMORIA;
int TAM_PAGINA;
char* PATH_INSTRUCCIONES;
int RETARDO_RESPUESTA;

t_dictionary* diccionario_de_instrucciones;
t_dictionary* diccionario_de_tdp;

//Semaforos
pthread_mutex_t mutex_para_leer_pseudo;
pthread_mutex_t mutex_para_diccionario_instrucciones;
pthread_mutex_t mutex_para_diccionario_tdp;
pthread_mutex_t mutex_para_marcos_libres;
pthread_mutex_t mutex_para_mem_de_usuario;



#endif