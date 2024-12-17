#include <planificador_largo_plazo.h>

void iniciar_planificacion_largo_plazo(){
    pthread_t hilo_new_to_ready;
    pthread_create(&hilo_new_to_ready,NULL,(void*)planificador_new_to_ready,NULL);
    pthread_detach(hilo_new_to_ready);

    pthread_t hilo_exit;
    pthread_create(&hilo_exit,NULL,(void*)planificador_exit,NULL);
    pthread_detach(hilo_exit);
}

void planificador_new_to_ready(){
    while(true){
        sem_wait(&multiprogramacion_permite_proceso_en_ready);
        sem_wait(&hay_proceso_en_new);

        if(!permitir_planificacion){
            sem_wait(&detener_planificacion_to_ready);
        }

        pcb* un_pcb = NULL;
        pthread_mutex_lock(&mutex_cola_new);
        if(!queue_is_empty(cola_new)){
            un_pcb = queue_pop(cola_new);
        }
        pthread_mutex_unlock(&mutex_cola_new);

        if(un_pcb != NULL){
            un_pcb ->estado_proceso = READY;
            log_info(logger_obligatorio, "PID: %d - Estado Anterior: NEW - Estado Actual: READY", un_pcb->PID);

            pthread_mutex_lock(&mutex_cola_ready);
            queue_push(cola_ready,un_pcb);
            log_de_lista_de_ready();
            pthread_mutex_unlock(&mutex_cola_ready);

            sem_post(&hay_proceso_en_ready);
        }

    }
}

void planificador_exit(){
   while(true){
    sem_wait(&hay_proceso_en_exit);
    if(!permitir_planificacion){
        sem_wait(&detener_planificacion_exit);
    }
    
    pcb* un_pcb = NULL;
    
    pthread_mutex_lock(&mutex_cola_exit);
    if(!queue_is_empty(cola_exit)){
        un_pcb = queue_pop(cola_exit);
    }
    pthread_mutex_unlock(&mutex_cola_exit);

    if(un_pcb != NULL){
        eliminar_el_proceso(un_pcb);
    }
    
   }
}
