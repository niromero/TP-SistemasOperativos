#ifndef INTERFAZ_DIALFS_H_
#define INTERFAZ_DIALFS_H_

#include <entradasalida_gestor.h>
#include <atender_mensajes.h>

void atender_peticiones_dialfs();
void levantar_archivos();
int buscar_bloque_libre();
void copiar_lista_a_archivo();
int realizar_compatacion(char*);



#endif