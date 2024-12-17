#ifndef MEMORIA_USUARIO_H
#define MEMORIA_USUARIO_H

#include <memoria_gestor.h>

int cambiar_memoria_de_proceso(int, int);
bool hay_marcos_suficientes(int);
void* leer_dir_fisica(int, int);

#endif