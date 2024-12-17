#include <log_listar_ready.h>

void log_de_lista_de_ready(){
    //UTILIZAR SIEMPRE ENTRE MUTEX DE LA COLA READY
    char* lista = malloc(3);
    strcpy(lista,"[");
    for(int i = 0; i < queue_size(cola_ready); i++){
        pcb* un_pcb = list_get(cola_ready ->elements,i);
        char* pid = string_itoa(un_pcb ->PID);
        string_append(&lista, pid);
        if(i != (queue_size(cola_ready)-1)){
            string_append(&lista, ", ");
        }
        free(pid);
            
    }
    string_append(&lista, "]");
    log_info(logger_obligatorio, "Cola Ready %s", lista);
    free(lista);
}

void log_de_lista_de_ready_prioritaria(){
    char* lista = malloc(3);
    strcpy(lista,"[");
    for(int i = 0; i < queue_size(cola_ready_prioritaria); i++){
        pcb* un_pcb = list_get(cola_ready_prioritaria ->elements,i);
        char* pid = string_itoa(un_pcb ->PID);
        string_append(&lista, pid);
        if(i != (queue_size(cola_ready_prioritaria)-1)){
            string_append(&lista, ", ");
        }

        free(pid);
            
    }
    string_append(&lista, "]");
    log_info(logger_obligatorio, "Cola Ready Prioritaria %s", lista);
    free(lista);
}