#ifndef LOG_LISTAR_READY_H_
#define LOG_LISTAR_READY_H_

#include <kernel_gestor.h>

//UTILIZAR SIEMPRE ENTRE MUTEX DE LA COLA READY
void log_de_lista_de_ready();

//UTILIZAR SIEMPRE ENTRE MUTEX DE LA COLA READY PRIORITARIA
void log_de_lista_de_ready_prioritaria();




#endif