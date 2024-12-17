#include <crear_pcb.h>

pcb* creacion_pcb(char* ruta_pseudocodigo){
    pcb* el_pcb = malloc(sizeof(pcb));
    el_pcb ->estado_proceso = NEW;
    el_pcb ->PID = pid_acumulado; 
    pid_acumulado++;
    el_pcb ->quantum_restante = QUANTUM;
    el_pcb ->lista_recursos_tomados = list_create();
    el_pcb ->tiempo_en_ejecucion = NULL;
    t_paquete* paquete_codeop_ruta = crear_paquete(CREAR_PROCESO);
    agregar_int_a_paquete(paquete_codeop_ruta,el_pcb->PID);
    agregar_string_a_paquete(paquete_codeop_ruta,ruta_pseudocodigo);
    enviar_paquete(paquete_codeop_ruta,kernel_cliente_memoria);
    eliminar_paquete(paquete_codeop_ruta);
    int programCounter = recibir_PC_memoria();
    if(programCounter <0){
        pid_acumulado--;
        return NULL;
    }
    el_pcb ->registros_cpu_en_pcb = malloc(sizeof(t_registros_cpu));
    el_pcb ->registros_cpu_en_pcb->PC = malloc(sizeof(uint32_t));
    *el_pcb->registros_cpu_en_pcb->PC = programCounter;
    el_pcb ->registros_cpu_en_pcb->AX = calloc(1,sizeof(uint8_t));
    el_pcb ->registros_cpu_en_pcb->BX = calloc(1,sizeof(uint8_t));
    el_pcb ->registros_cpu_en_pcb->CX = calloc(1,sizeof(uint8_t));
    el_pcb ->registros_cpu_en_pcb->DX = calloc(1,sizeof(uint8_t));
    el_pcb ->registros_cpu_en_pcb->EAX = calloc(1,sizeof(uint32_t));
    el_pcb ->registros_cpu_en_pcb->EBX = calloc(1,sizeof(uint32_t));
    el_pcb ->registros_cpu_en_pcb->ECX = calloc(1,sizeof(uint32_t));
    el_pcb ->registros_cpu_en_pcb->EDX = calloc(1,sizeof(uint32_t));
    el_pcb ->registros_cpu_en_pcb->DI = calloc(1,sizeof(uint32_t));
    el_pcb ->registros_cpu_en_pcb->SI = calloc(1,sizeof(uint32_t));

    pthread_mutex_lock(&mutex_para_diccionario_de_todos_los_procesos);
    char* pid_clave = string_itoa(el_pcb ->PID);
    dictionary_put(diccionario_de_todos_los_procesos,pid_clave, el_pcb);
    pthread_mutex_unlock(&mutex_para_diccionario_de_todos_los_procesos);

    strcpy(el_pcb ->interfaz_bloqueante, "No");
    strcpy(el_pcb ->recurso_bloqueante, "No");
    

    log_info(logger_obligatorio, "Se crea el proceso %d en NEW", el_pcb->PID);
    free(pid_clave);
    return el_pcb;

}

