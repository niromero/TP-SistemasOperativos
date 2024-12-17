#ifndef ATENDER_MENSAJES_H
#define ATENDER_MENSAJES_H

#include <memoria_gestor.h>
#include <leer_pseudocodigo.h>
#include <memoria_usuario.h>

void atender_cpu_memoria();
void enviar_instruccion(int,int);
void crear_tdp_del_proceso(char*);
void atender_kernel_memoria();
void enviar_program_counter(int);
void atender_entradasalida_memoria();

#endif