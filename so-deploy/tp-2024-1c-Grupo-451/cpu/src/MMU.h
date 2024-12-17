#ifndef MMU_H_
#define MMU_H_

#include <cpu_gestor.h>
#include <atender_mensajes.h>

int solicitar_marco(int, int);
direccion_fisica* traducir_dir_logica(int ,int);
int buscar_en_tlb(int, int);
void agregar_en_tlb(int, int, int);


#endif