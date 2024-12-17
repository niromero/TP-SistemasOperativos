#ifndef ATENDER_MENSAJES_H
#define ATENDER_MENSAJES_H

#include <cpu_gestor.h>

void recibir_contexto_de_CPU(t_buffer*);
void atender_kernel_dispatch_sin_while();
void atender_kernel_interrupt();
void atender_memoria_cpu_sin_while();
int recibir_marco();
int confirmacion_resize();
void* recibir_lectura();
bool confirmacion_escritura();

#endif