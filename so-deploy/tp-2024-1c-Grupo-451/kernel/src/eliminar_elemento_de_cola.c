#include <eliminar_elemento_de_cola.h>

void eliminar_pcb_cola(t_queue* cola,pcb* el_pcb){
    int largo_cola = queue_size(cola);
    pcb* pcb_revisar;
    for(int i = 0; i<largo_cola; i++){
        pcb_revisar = list_get(cola->elements,i);
        if(pcb_revisar->PID == el_pcb ->PID){
            list_remove(cola->elements,i);
            break;
        }
    }
}