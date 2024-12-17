#include <consola.h>

void consola_kernel(){
    
    log_info(logger, "INICIANDO CONSOLA \n");
    char* leido;
    leido = readline(">");

    while(strcmp(leido,"EXIT")!=0){
        add_history(leido);
        char** comando_por_partes = string_split(leido, " "); //Se divide lo leido, separandolos por los espacios
        validar_y_ejecutar_comando(comando_por_partes);
        string_array_destroy(comando_por_partes);
        free(leido);
        leido = readline(">");
    }
    free(leido);
}

void validar_y_ejecutar_comando(char** comando_por_partes){
    
    if(strcmp(comando_por_partes[0],"EJECUTAR_SCRIPT")==0 && (string_array_size(comando_por_partes)==2) && (strcmp(comando_por_partes[1],"")!=0)){
        ejecutar_script(comando_por_partes[1]);
    }
    else if((strcmp(comando_por_partes[0],"INICIAR_PROCESO")==0) && (string_array_size(comando_por_partes)==2) && (strcmp(comando_por_partes[1],"")!=0)){
        char* ruta = strdup(comando_por_partes[1]);
	crear_proceso((void*)ruta);

    }
    else if(strcmp(comando_por_partes[0],"FINALIZAR_PROCESO")==0){
        

        if(permitir_planificacion){
            permitir_planificacion = false;
            //usleep(500 * 1000);
            pthread_mutex_lock(&mutex_para_diccionario_de_todos_los_procesos);
            pcb* el_pcb_a_eliminar = dictionary_get(diccionario_de_todos_los_procesos,comando_por_partes[1]);
            pthread_mutex_unlock(&mutex_para_diccionario_de_todos_los_procesos);

            if(el_pcb_a_eliminar!= NULL){

                eliminar_proceso_por_usuario(el_pcb_a_eliminar);
            }

            iniciar_planificacion();
        }
        else{
            //usleep(500 * 1000);
            pthread_mutex_lock(&mutex_para_diccionario_de_todos_los_procesos);
            pcb* el_pcb_a_eliminar = dictionary_get(diccionario_de_todos_los_procesos,comando_por_partes[1]);
            pthread_mutex_unlock(&mutex_para_diccionario_de_todos_los_procesos);

            if(el_pcb_a_eliminar!= NULL){
                eliminar_proceso_por_usuario(el_pcb_a_eliminar); 
            }
        }


        
    }
    else if(strcmp(comando_por_partes[0],"DETENER_PLANIFICACION")==0 && (string_array_size(comando_por_partes)==1)){
        permitir_planificacion = false;

        if ((strcmp(ALGORITMO_PLANIFICACION,"rr")==0 || strcmp(ALGORITMO_PLANIFICACION,"vrr")==0) && proceso_en_ejecucion != NULL) {
            pthread_mutex_lock(&mutex_para_proceso_en_ejecucion);

            pthread_cancel(proceso_en_ejecucion->hilo_quantum);
            temporal_stop(proceso_en_ejecucion->tiempo_en_ejecucion);
            int64_t tiempo_ejecutado = temporal_gettime(proceso_en_ejecucion ->tiempo_en_ejecucion);
            if (tiempo_ejecutado < proceso_en_ejecucion->quantum_restante) {
                proceso_en_ejecucion->quantum_restante = proceso_en_ejecucion->quantum_restante - tiempo_ejecutado;
            }

            pthread_mutex_unlock(&mutex_para_proceso_en_ejecucion);
        }
    }
    else if(strcmp(comando_por_partes[0],"INICIAR_PLANIFICACION")==0 && (string_array_size(comando_por_partes)==1)){
        
        iniciar_planificacion();
        if ((strcmp(ALGORITMO_PLANIFICACION,"rr")==0 || strcmp(ALGORITMO_PLANIFICACION,"vrr")==0) && proceso_en_ejecucion != NULL) {
            pthread_mutex_lock(&mutex_para_proceso_en_ejecucion);
            pthread_create(&(proceso_en_ejecucion->hilo_quantum), NULL, (void*)esperar_quantum, (void*)proceso_en_ejecucion);
            pthread_detach(proceso_en_ejecucion->hilo_quantum);
            temporal_resume(proceso_en_ejecucion->tiempo_en_ejecucion);
            pthread_mutex_unlock(&mutex_para_proceso_en_ejecucion);
        }
    }
    else if(strcmp(comando_por_partes[0],"MULTIPROGRAMACION")==0 && (string_array_size(comando_por_partes)==2) && (strcmp(comando_por_partes[1],"")!=0)){
        int nuevo_grado_multi = atoi(comando_por_partes[1]);
        int diferencia = nuevo_grado_multi - GRADO_MULTIPROGRAMACION;
        if(diferencia > 0){
            for(int i = 0; i< diferencia; i++){
                sem_post(&(multiprogramacion_permite_proceso_en_ready));
            }
        }
        else if(diferencia <0){
            espera_grado_multi = (-1)*diferencia;

            int valor_semaforo;
            sem_getvalue(&(multiprogramacion_permite_proceso_en_ready), &valor_semaforo);
            for(int i = 0; i < (-diferencia) && i < valor_semaforo; i++){
                sem_wait(&(multiprogramacion_permite_proceso_en_ready));
            }
        }
        GRADO_MULTIPROGRAMACION = nuevo_grado_multi;
    }
    else if(strcmp(comando_por_partes[0],"PROCESO_ESTADO")==0 && (string_array_size(comando_por_partes)==1)){
         printf("Hola, soy proceso estado\n");
    pcb* pcb_revisar;
    int largo_cola;

    // COLA NEW
    printf("COLA NEW: \n");
    pthread_mutex_lock(&mutex_cola_new);
    largo_cola = queue_size(cola_new);
    for(int i = 0; i < largo_cola; i++) {
        pcb_revisar = list_get(cola_new->elements, i);
        printf("%d\n", pcb_revisar->PID);
    }
    pthread_mutex_unlock(&mutex_cola_new);

    // COLA READY
    printf("COLA READY: \n");
    pthread_mutex_lock(&mutex_cola_ready);
    largo_cola = queue_size(cola_ready);
    for(int i = 0; i < largo_cola; i++) {
        pcb_revisar = list_get(cola_ready->elements, i);
        printf("%d\n", pcb_revisar->PID);
    }
    pthread_mutex_unlock(&mutex_cola_ready);

    // COLA READY PRIORITARIA (si aplica)
    if(strcmp(ALGORITMO_PLANIFICACION, "vrr") == 0) {
        printf("COLA READY PRIORITARIA: \n");
        pthread_mutex_lock(&mutex_cola_prioritaria);
        largo_cola = queue_size(cola_ready_prioritaria);
        for(int i = 0; i < largo_cola; i++) {
            pcb_revisar = list_get(cola_ready_prioritaria->elements, i);
            printf("%d\n", pcb_revisar->PID);
        }
        pthread_mutex_unlock(&mutex_cola_prioritaria);
    }

    // COLA EXEC
    printf("COLA EXEC: \n");
    if(proceso_en_ejecucion != NULL) {
        printf("%d\n", proceso_en_ejecucion->PID);
    } else {
        printf("\n");
    }

    // COLAS BLOCKED
    printf("COLAS BLOCKED: \n");
    pthread_mutex_lock(&mutex_para_diccionario_blocked);
    bool diccionario_vacio = dictionary_is_empty(diccionario_blocked);
    if(!diccionario_vacio) {
        t_list* lista_keys = dictionary_keys(diccionario_blocked);
        pthread_mutex_unlock(&mutex_para_diccionario_blocked);

        for(int i = 0; i < list_size(lista_keys); i++) {
            char* interfaz_o_recurso = list_get(lista_keys, i);
            pthread_mutex_lock(&mutex_para_diccionario_blocked);
            nodo_de_diccionario_blocked* nodo = dictionary_get(diccionario_blocked, interfaz_o_recurso);
            pthread_mutex_unlock(&mutex_para_diccionario_blocked);

            printf("COLA BLOCKED DE %s\n", interfaz_o_recurso);
            pthread_mutex_lock(&(nodo->mutex_para_cola_bloqueados));
            largo_cola = queue_size(nodo->cola_bloqueados);
            for(int p = 0; p < largo_cola; p++) {
                pcb_revisar = list_get(nodo->cola_bloqueados->elements, p);
                printf("%d\n", pcb_revisar->PID);
            }
            pthread_mutex_unlock(&(nodo->mutex_para_cola_bloqueados));
        }
        list_destroy(lista_keys);
    } else {
        pthread_mutex_unlock(&mutex_para_diccionario_blocked);
    }

    // COLAS BLOCKED POR RECURSOS
    pthread_mutex_lock(&mutex_para_diccionario_recursos);
    diccionario_vacio = dictionary_is_empty(diccionario_recursos);
    if(!diccionario_vacio) {
        t_list* lista_keys_recurso = dictionary_keys(diccionario_recursos);
        pthread_mutex_unlock(&mutex_para_diccionario_recursos);

        for(int i = 0; i < list_size(lista_keys_recurso); i++) {
            char* recurso = list_get(lista_keys_recurso, i);
            pthread_mutex_lock(&mutex_para_diccionario_recursos);
            nodo_recursos* nodo_del_recurso = dictionary_get(diccionario_recursos, recurso);
            pthread_mutex_unlock(&mutex_para_diccionario_recursos);

            printf("COLA BLOCKED DE %s\n", recurso);
            pthread_mutex_lock(&(nodo_del_recurso->mutex_del_recurso));
            largo_cola = queue_size(nodo_del_recurso->cola_bloqueados_recurso);
            for(int p = 0; p < largo_cola; p++) {
                pcb_revisar = list_get(nodo_del_recurso->cola_bloqueados_recurso->elements, p);
                printf("%d\n", pcb_revisar->PID);
            }
            pthread_mutex_unlock(&(nodo_del_recurso->mutex_del_recurso));
        }
        list_destroy(lista_keys_recurso);
    } else {
        pthread_mutex_unlock(&mutex_para_diccionario_recursos);
    }

    // COLA EXIT
    printf("COLA EXIT: \n");
    pthread_mutex_lock(&mutex_cola_exit);
    largo_cola = queue_size(cola_exit);
    for(int i = 0; i < largo_cola; i++) {
        pcb_revisar = list_get(cola_exit->elements, i);
        printf("%d\n", pcb_revisar->PID);
    }
    pthread_mutex_unlock(&mutex_cola_exit);
} else {
    log_error(logger, "ERROR. COMANDO NO RECONOCIDO O SINTAXIS ERRONEA");
}
    
}

void crear_proceso(void* ruta_pseudocodigo){
    // Recibira un path por consola.
    // Crear PCB del proceso.
    pthread_mutex_lock(&mutex_para_creacion_proceso);
    pcb* pcb_proceso = creacion_pcb((char*)ruta_pseudocodigo);
    if(pcb_proceso != NULL){
        pthread_mutex_lock(&mutex_cola_new);
        queue_push(cola_new,pcb_proceso);
        pthread_mutex_unlock(&mutex_cola_new);
        
        sem_post(&hay_proceso_en_new);

    }
    else{
        log_error(logger,"No existe ese archivo de Pseudocodigo");
    }
    free(ruta_pseudocodigo);

    pthread_mutex_unlock(&mutex_para_creacion_proceso);
}

void iniciar_planificacion(){
    if(!permitir_planificacion){
	permitir_planificacion = true;
        sem_post(&detener_planificacion_corto_plazo);
        sem_post(&detener_planificacion_exit);
        sem_post(&detener_planificacion_salida_cpu);
        sem_post(&detener_planificacion_to_ready);
        bool diccionario_vacio;

        pthread_mutex_lock(&mutex_para_diccionario_entradasalida);
        diccionario_vacio = dictionary_is_empty(diccionario_entrada_salida);
        pthread_mutex_unlock(&mutex_para_diccionario_entradasalida);

        if(!diccionario_vacio){
            nodo_de_diccionario_interfaz* nodo;

            pthread_mutex_lock(&mutex_para_diccionario_entradasalida);
            t_list* lista_keys = dictionary_keys(diccionario_entrada_salida);
            pthread_mutex_unlock(&mutex_para_diccionario_entradasalida);

            for(int i = 0; i<list_size(lista_keys); i++){
                char* interfaz_o_recurso = list_get(lista_keys,i);

                pthread_mutex_lock(&mutex_para_diccionario_entradasalida);
                nodo = dictionary_get(diccionario_entrada_salida,interfaz_o_recurso);
                pthread_mutex_unlock(&mutex_para_diccionario_entradasalida);

                sem_post(&(nodo ->detener_planificacion_enviar_peticion_IO));
                sem_post(&(nodo ->detener_planificacion_recibir_respuestas_IO));

            }

            list_destroy(lista_keys);
        }
    }
}

void ejecutar_script(char* nombre_archivo){
    char* archivo = strdup(PATH_SCRIPTS);
    string_append(&archivo, nombre_archivo);

    FILE* archivo_script = fopen(archivo, "r");

    if(archivo_script != NULL){
	permitir_planificacion = false;

        if ((strcmp(ALGORITMO_PLANIFICACION,"rr")==0 || strcmp(ALGORITMO_PLANIFICACION,"vrr")==0) && proceso_en_ejecucion != NULL) {
            pthread_mutex_lock(&mutex_para_proceso_en_ejecucion);

            pthread_cancel(proceso_en_ejecucion->hilo_quantum);
            temporal_stop(proceso_en_ejecucion->tiempo_en_ejecucion);
            int64_t tiempo_ejecutado = temporal_gettime(proceso_en_ejecucion ->tiempo_en_ejecucion);
            if (tiempo_ejecutado < proceso_en_ejecucion->quantum_restante) {
                proceso_en_ejecucion->quantum_restante = proceso_en_ejecucion->quantum_restante - tiempo_ejecutado;
            }

            pthread_mutex_unlock(&mutex_para_proceso_en_ejecucion);
        }
        while(!feof(archivo_script)){
            //usleep(500 * 1000);
            char instruccion_consola[256];
            fgets(instruccion_consola,256,archivo_script);

            if(instruccion_consola[strlen(instruccion_consola)-1] == '\n'){
                instruccion_consola[strlen(instruccion_consola)-1] = '\0';
            }
            

            char** comando_por_partes = string_split(instruccion_consola, " ");

            validar_y_ejecutar_comando(comando_por_partes);

            string_array_destroy(comando_por_partes);

        }
	 iniciar_planificacion();
        if ((strcmp(ALGORITMO_PLANIFICACION,"rr")==0 || strcmp(ALGORITMO_PLANIFICACION,"vrr")==0) && proceso_en_ejecucion != NULL) {
            pthread_mutex_lock(&mutex_para_proceso_en_ejecucion);
            pthread_create(&(proceso_en_ejecucion->hilo_quantum), NULL, (void*)esperar_quantum, (void*)proceso_en_ejecucion);
            pthread_detach(proceso_en_ejecucion->hilo_quantum);
            temporal_resume(proceso_en_ejecucion->tiempo_en_ejecucion);
            pthread_mutex_unlock(&mutex_para_proceso_en_ejecucion);
        }
        fclose(archivo_script);
    }
    else{
        log_error(logger,"No existe ese archivo de Script");
    }
	
    free(archivo);
}

void eliminar_proceso_por_usuario(pcb* el_pcb_a_eliminar){
    switch(el_pcb_a_eliminar ->estado_proceso){

        case NEW:

            pthread_mutex_lock(&mutex_cola_new);
            eliminar_pcb_cola(cola_new,el_pcb_a_eliminar);
            if(!queue_is_empty(cola_new)){
                sem_wait(&hay_proceso_en_new);
            }
            pthread_mutex_unlock(&mutex_cola_new);
            
            el_pcb_a_eliminar ->razon_salida = FINALIZADO_POR_USUARIO;

            el_pcb_a_eliminar ->estado_proceso = EXIT_PROCESS;

            log_info(logger_obligatorio, "PID: %d - Estado Anterior: NEW - Estado Actual: EXIT", el_pcb_a_eliminar->PID);
            eliminar_el_proceso_nuevo(el_pcb_a_eliminar);

            break;

        case READY:
            //Como eliminar_pcb_cola no hace nada si el pcb no esta en esa cola, lo ejecutamos 2 veces pero con las 2 colas de ready
            pthread_mutex_lock(&mutex_cola_ready);
            eliminar_pcb_cola(cola_ready, el_pcb_a_eliminar);
            pthread_mutex_unlock(&mutex_cola_ready);

            pthread_mutex_lock(&mutex_cola_prioritaria);
            eliminar_pcb_cola(cola_ready_prioritaria, el_pcb_a_eliminar);
            pthread_mutex_unlock(&mutex_cola_prioritaria);
        
            if(!queue_is_empty(cola_ready) && !queue_is_empty(cola_ready_prioritaria)){
                sem_wait(&hay_proceso_en_ready);
            }

            log_info(logger_obligatorio, "PID: %d - Estado Anterior: BLOCKED - Estado Actual: EXIT", el_pcb_a_eliminar->PID);

            el_pcb_a_eliminar ->estado_proceso = EXIT_PROCESS;

            el_pcb_a_eliminar ->razon_salida = FINALIZADO_POR_USUARIO;

            eliminar_el_proceso(el_pcb_a_eliminar);
                    

            break;

        case BLOCKED:
            el_pcb_a_eliminar ->razon_salida = FINALIZADO_POR_USUARIO;

            el_pcb_a_eliminar ->estado_proceso = EXIT_PROCESS;

            log_info(logger_obligatorio, "PID: %d - Estado Anterior: BLOCKED - Estado Actual: EXIT", el_pcb_a_eliminar->PID);

            if(strcmp(el_pcb_a_eliminar ->interfaz_bloqueante, "No")!=0){

                pthread_mutex_lock(&mutex_para_diccionario_blocked);
                nodo_de_diccionario_blocked* nodo_blocked = dictionary_get(diccionario_blocked, el_pcb_a_eliminar ->interfaz_bloqueante);
                pthread_mutex_unlock(&mutex_para_diccionario_blocked);

                pthread_mutex_lock(&mutex_para_diccionario_entradasalida);
		        nodo_de_diccionario_interfaz* nodo_de_interfaz = dictionary_get(diccionario_entrada_salida,el_pcb_a_eliminar ->interfaz_bloqueante);
		        pthread_mutex_unlock(&mutex_para_diccionario_entradasalida);

                int i;

                        
                pthread_mutex_lock(&(nodo_blocked ->mutex_para_cola_bloqueados));
                int largo_cola = queue_size(nodo_blocked ->cola_bloqueados);
                pcb* pcb_revisar;
                for(i = 0; i<largo_cola; i++){
                    pcb_revisar = list_get(nodo_blocked ->cola_bloqueados->elements,i);
                    if(pcb_revisar->PID == el_pcb_a_eliminar ->PID){
                        list_remove(nodo_blocked ->cola_bloqueados->elements,i);
                        break;
                    }
                }
                pthread_mutex_unlock(&(nodo_blocked ->mutex_para_cola_bloqueados));


                pthread_mutex_lock(&(nodo_blocked ->mutex_para_cola_variables));
                void* variable = list_remove(nodo_blocked ->cola_Variables ->elements,i);
                pthread_mutex_unlock(&(nodo_blocked ->mutex_para_cola_variables));
                        
                if(strcmp(nodo_de_interfaz ->tipo_de_interfaz, "Generica")==0){
                    free(variable);
                }
                else if(strcmp(nodo_de_interfaz ->tipo_de_interfaz, "stdin")==0 || strcmp(nodo_de_interfaz ->tipo_de_interfaz, "stdout")==0){
                    io_std_fs* dir_fisicas = variable;
                    free(dir_fisicas ->interfaz);

                    list_destroy_and_destroy_elements(dir_fisicas ->lista_dir_fisicas, free);
                    free(dir_fisicas);
                }
                else if(strcmp(nodo_de_interfaz ->tipo_de_interfaz, "dialfs")==0){
                    var_fs* variable_fs = variable;
                    
                    if(variable_fs ->tipo_variable == VAR_FS_READ || variable_fs ->tipo_variable == VAR_FS_WRITE){
                        list_destroy_and_destroy_elements(variable_fs->dir_fisicas ->lista_dir_fisicas, free);
                        free(variable_fs ->dir_fisicas ->interfaz);
                        free(variable_fs->dir_fisicas);
                    }
                    free(variable_fs ->nombre_Archivo);
                    free(variable_fs);
                }  
               
                
                if(queue_size(nodo_blocked ->cola_bloqueados) > 1){
                    sem_wait(&(nodo_de_interfaz ->hay_proceso_en_bloqueados));
                }

            }
                else if(strcmp(el_pcb_a_eliminar ->recurso_bloqueante, "No")!=0){
                        
                    pthread_mutex_lock(&mutex_para_diccionario_recursos);
                    nodo_recursos* nodo_del_recurso = dictionary_get(diccionario_recursos, el_pcb_a_eliminar ->recurso_bloqueante);
                    pthread_mutex_unlock(&mutex_para_diccionario_recursos);

                    pthread_mutex_lock(&(nodo_del_recurso ->mutex_del_recurso));
                    eliminar_pcb_cola(nodo_del_recurso ->cola_bloqueados_recurso, el_pcb_a_eliminar);
                    nodo_del_recurso ->instancias++;
                    pthread_mutex_unlock(&(nodo_del_recurso ->mutex_del_recurso));

                }

                eliminar_el_proceso(el_pcb_a_eliminar);
                break;

            case EXIT_PROCESS:

                pthread_mutex_lock(&mutex_cola_exit);
                eliminar_pcb_cola(cola_exit,el_pcb_a_eliminar);
                pthread_mutex_unlock(&mutex_cola_exit);

                el_pcb_a_eliminar ->razon_salida = FINALIZADO_POR_USUARIO;

                if(!queue_is_empty(cola_exit)){
                    sem_wait(&hay_proceso_en_exit);
                }
                

                eliminar_el_proceso(el_pcb_a_eliminar);


                break;

            case EXEC:
                if(strcmp(ALGORITMO_PLANIFICACION,"rr")==0 || strcmp(ALGORITMO_PLANIFICACION,"vrr")==0){
                    pthread_mutex_lock(&mutex_para_proceso_en_ejecucion);
                    pthread_cancel(proceso_en_ejecucion ->hilo_quantum);
                    temporal_stop(proceso_en_ejecucion ->tiempo_en_ejecucion);
                    pthread_mutex_unlock(&mutex_para_proceso_en_ejecucion);
                }

                t_paquete* paquete = crear_paquete(INTERRUPCION);
                agregar_int_a_paquete(paquete, el_pcb_a_eliminar ->PID);
                enviar_paquete(paquete,kernel_cliente_interrupt);
                eliminar_paquete(paquete); 

                el_pcb_a_eliminar ->razon_salida = FINALIZADO_POR_USUARIO;

                log_info(logger_obligatorio, "PID: %d - Estado Anterior: EXEC - Estado Actual: EXIT", el_pcb_a_eliminar->PID);

                eliminar_el_proceso(el_pcb_a_eliminar);
                proceso_en_ejecucion = NULL;

                break;

    }
}
