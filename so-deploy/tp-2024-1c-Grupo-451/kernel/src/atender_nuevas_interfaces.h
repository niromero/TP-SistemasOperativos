#ifndef ATENDER_NUEVAS_INTERFACES_H_
#define ATENDER_NUEVAS_INTERFACES_H_

#include <kernel_gestor.h>
#include <atender_mensajes.h>

typedef struct {
    char* nombre;
    int* cliente;
} nombre_y_cliente;


void atender_nueva_interfaz(void*);
void atender_mensajes_interfaz(void*);
void enviar_proceso_interfaz(void* );
pcb* buscar_proceso_en_cola(int , nodo_de_diccionario_blocked*);
void atender_las_nuevas_interfaces();
void enviar_fs_create_delete(nodo_de_diccionario_interfaz*, var_fs*, int, int);
void enviar_fs_read_write(nodo_de_diccionario_interfaz*, var_fs*, int, int);
void eliminar_variable(nodo_de_diccionario_interfaz*, nodo_de_diccionario_blocked*);




#endif