#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <kernel_gestor.h>
#include <inicializar_kernel.h>
#include <crear_pcb.h>
#include <planificador_corto_plazo.h>
#include <eliminar_elemento_de_cola.h>
#include <eliminar_proceso.h>


void consola_kernel();
void validar_y_ejecutar_comando(char**);
void crear_proceso(void*);
void iniciar_planificacion();
void ejecutar_script(char*);
void eliminar_proceso_por_usuario(pcb* );



#endif