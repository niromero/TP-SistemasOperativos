#ifndef PLANIFICADOR_CORTO_PLAZO_H_
#define PLANIFICADOR_CORTO_PLAZO_H_

#include <kernel_gestor.h>
#include <atender_mensajes.h>

void iniciar_planificacion_corto_plazo();
void algoritmo_fifo();
void serializar_registros_procesador (t_paquete* , t_registros_cpu* );
void algoritmo_round_robin();
void esperar_quantum(void*);
void algoritmo_virtual_round_robin();
void algoritmo_round_robin();

#endif