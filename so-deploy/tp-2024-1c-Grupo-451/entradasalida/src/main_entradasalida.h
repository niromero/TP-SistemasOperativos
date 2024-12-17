#ifndef MAIN_ENTRADASALIDA_H_
#define MAIN_ENTRADASALIDA_H_

#include <entradasalida_gestor.h>
#include <inicializar_entradasalida.h>
#include <atender_mensajes.h>
#include <interfaz_generica.h>
#include <interfaz_stdin.h>
#include <interfaz_stdout.h>
#include <interfaz_dialfs.h>

t_log* logger;
t_log* logger_obligatorio;
t_config* config;

char* nombre_interfaz;

int entradasalida_cliente_memoria;
int entradasalida_cliente_kernel;

int tipo_de_interfaz;

pthread_mutex_t mutex_para_interfaz;

char* TIPO_INTERFAZ;
int TIEMPO_UNIDAD_TRABAJO;
char* IP_KERNEL;
char* PUERTO_KERNEL;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PATH_BASE_DIALFS;
int BLOCK_SIZE;
int BLOCK_COUNT;
int RETRASO_COMPACTACION;

FILE* Archivo_bloques;
FILE* Archivo_bitmap;
FILE* Archivo_lista;

void* archivo_bloques_en_mem;
void* puntero_a_bits_de_bloques;

t_bitarray* bitmap_bloques;

t_list* lista_archivos;


int definir_tipo_interfaz();

#endif