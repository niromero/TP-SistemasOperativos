#ifndef CICLO_CPU_H_
#define CICLO_CPU_H_

#include <utils/utiles.h>
#include <cpu_gestor.h>
#include <atender_mensajes.h>
#include <MMU.h>
#include <instrucciones.h>

void ciclo();
void solicitar_instruccion(int);
int decodificar_instruccion();
int ejecutar_instruccion (int);
void cargar_registros_a_paquete(t_paquete* );

#endif