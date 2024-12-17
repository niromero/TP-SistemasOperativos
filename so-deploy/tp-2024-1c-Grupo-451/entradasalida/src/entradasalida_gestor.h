#ifndef ENTRADASALIDA_GESTOR_H_
#define ENTRADASALIDA_GESTOR_H_

/*
Este es el gestor de la Entrada y Salida, se declaran variables globales usando 
el extern asi todos los modulos de la Entrada Y Salida pueden utilizarlas. 
Las variables deben ser previamente declaradas en su modulo
*/

#include <stdlib.h>
#include <stdio.h>
#include <utils/utiles.h>

typedef enum{
    GENERICO,
    STDIN,
    STDOUT,
    DIALFS
} interfaces;

typedef struct {
    char nombreArchivo[20 + 1];
    int posicionInicial;
    int tamanio;
} t_archivo;

typedef struct {
    char nombreArchivo[20 + 1];
    int posicionInicial;
    int tamanio;
    void* datos;
} t_archivo_compactacion;

//Variables Globales

extern t_log* logger;
extern t_log* logger_obligatorio;
extern t_config* config;

extern char* nombre_interfaz;

extern int entradasalida_cliente_memoria;
extern int entradasalida_cliente_kernel;

extern pthread_mutex_t mutex_para_interfaz;

extern char* TIPO_INTERFAZ;
extern int TIEMPO_UNIDAD_TRABAJO;
extern char* IP_KERNEL;
extern char* PUERTO_KERNEL;
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* PATH_BASE_DIALFS;
extern int BLOCK_SIZE;
extern int BLOCK_COUNT;
extern int RETRASO_COMPACTACION;

extern FILE* Archivo_bloques;
extern FILE* Archivo_bitmap;
extern FILE* Archivo_lista;

extern void* archivo_bloques_en_mem;
extern void* puntero_a_bits_de_bloques;

extern t_bitarray* bitmap_bloques;

extern t_list* lista_archivos;




#endif