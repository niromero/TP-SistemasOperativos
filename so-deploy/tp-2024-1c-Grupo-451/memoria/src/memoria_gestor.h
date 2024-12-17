#ifndef MEMORIA_GESTOR_H_
#define MEMORIA_GESTOR_H_

/*
Este es el gestor de la Memoria, se declaran variables globales usando el extern asi todos los
modulos de la Memoria pueden utilizarlas. Las variables deben ser previamente declaradas en su modulo
*/

#include <stdlib.h>
#include <stdio.h>
#include <utils/utiles.h>

typedef struct{
    t_list* tdp_del_proceso;
    int tam_de_proceso;
} nodo_dic_tdp;

//Variables Globales

extern t_log* logger;
extern t_log* logger_obligatorio;
extern t_config* config;

extern void* memoria_de_usuario;
extern float cant_marcos_totales;
extern void* puntero_a_bits_de_los_marcos;
extern t_bitarray* marcos_de_memoria_libres;

extern int memoria_server;
extern int cpu_cliente;
extern int kernel_cliente;

extern char* PUERTO_ESCUCHA;
extern int TAM_MEMORIA;
extern int TAM_PAGINA;
extern char* PATH_INSTRUCCIONES;
extern int RETARDO_RESPUESTA;


extern t_dictionary* diccionario_de_instrucciones;
extern t_dictionary* diccionario_de_tdp;

//Semaforos
extern pthread_mutex_t mutex_para_leer_pseudo;
extern pthread_mutex_t mutex_para_diccionario_instrucciones;
extern pthread_mutex_t mutex_para_diccionario_tdp;
extern pthread_mutex_t mutex_para_marcos_libres;
extern pthread_mutex_t mutex_para_mem_de_usuario;





#endif