#include <atender_nuevas_interfaces.h>



void atender_nueva_interfaz(void* cliente_entradasalida){
    int* cliente_momentaneo = cliente_entradasalida;
    int* cliente = malloc(sizeof(int));
    * cliente = *cliente_momentaneo;
    int cod_op = recibir_operacion(*cliente);
    if(cod_op == PRIMERA_CONEXION_IO){
        t_buffer* buffer = recibir_buffer(*cliente);
        char* nombre_interfaz = extraer_string_buffer(buffer,logger);
        char* tipo_interfaz = extraer_string_buffer(buffer,logger);
        nodo_de_diccionario_interfaz* nodo = malloc(sizeof(nodo_de_diccionario_interfaz));
        nodo ->tipo_de_interfaz = tipo_interfaz;
        nodo ->cliente = cliente;
        sem_init(&(nodo ->se_puede_enviar_proceso),0,1);
        sem_init(&(nodo ->hay_proceso_en_bloqueados),0,0);
        sem_init(&(nodo ->detener_planificacion_enviar_peticion_IO),0,0);
        sem_init(&(nodo ->detener_planificacion_recibir_respuestas_IO),0,0);
        pthread_mutex_init(&(nodo ->mutex_interfaz_siendo_usada), NULL);

        pthread_mutex_lock(&mutex_para_diccionario_entradasalida);
        dictionary_put(diccionario_entrada_salida,nombre_interfaz,nodo);
        pthread_mutex_unlock(&mutex_para_diccionario_entradasalida);

        nodo_de_diccionario_blocked* nuevo_nodo_blocked = malloc(sizeof(nodo_de_diccionario_blocked));

        nuevo_nodo_blocked ->cola_bloqueados = queue_create();
        nuevo_nodo_blocked ->cola_Variables = queue_create();

        pthread_mutex_init(&(nuevo_nodo_blocked ->mutex_para_cola_bloqueados), NULL);
        pthread_mutex_init(&(nuevo_nodo_blocked ->mutex_para_cola_variables), NULL);

        pthread_mutex_lock(&mutex_para_diccionario_blocked);
        dictionary_put(diccionario_blocked,nombre_interfaz,nuevo_nodo_blocked);
        pthread_mutex_unlock(&mutex_para_diccionario_blocked);

        nombre_y_cliente* nombre_cliente= malloc(sizeof(nombre_y_cliente));
        nombre_cliente ->cliente = cliente;
        nombre_cliente ->nombre = nombre_interfaz;
        pthread_t hilo_entradasalida_atender_mensajes;
        pthread_t hilo_entradasalida_enviar_proceso;
        pthread_create(&hilo_entradasalida_enviar_proceso,NULL, (void*)enviar_proceso_interfaz,(void*)nombre_cliente);
        pthread_detach(hilo_entradasalida_enviar_proceso);
        pthread_create(&hilo_entradasalida_atender_mensajes,NULL, (void*)atender_mensajes_interfaz,(void*)nombre_cliente);
        pthread_join(hilo_entradasalida_atender_mensajes,NULL);

    }
    else{
        log_error(logger,"Se envio el mensaje equivocado");
        liberar_conexion(*cliente);
    }
}

void atender_mensajes_interfaz(void* nombre_interfaz_y_cliente){
    nombre_y_cliente* nombre_cliente = nombre_interfaz_y_cliente;
    bool continuar_while = true;
    t_buffer* buffer;
    while(continuar_while){

        int cod_op = recibir_operacion(*nombre_cliente ->cliente);

        if(!permitir_planificacion){
            pthread_mutex_lock(&mutex_para_diccionario_entradasalida);
            nodo_de_diccionario_interfaz* nodo_para_detener = dictionary_get(diccionario_entrada_salida,nombre_cliente ->nombre);
            pthread_mutex_unlock(&mutex_para_diccionario_entradasalida);


            sem_wait(&(nodo_para_detener ->detener_planificacion_recibir_respuestas_IO));
        }

        switch (cod_op){
        case HANDSHAKE:
            buffer = recibir_buffer(*nombre_cliente ->cliente);
			char* mensaje = extraer_string_buffer(buffer, logger);
			log_info(logger,mensaje);
			free(mensaje);
			break;  
        case PRIMERA_CONEXION_IO:
            break;
        case EXITO_IO:
            buffer = recibir_buffer(*nombre_cliente ->cliente);
			int pid = extraer_int_buffer(buffer,logger);


			pthread_mutex_lock(&mutex_para_diccionario_blocked);
			nodo_de_diccionario_blocked* nodo_bloqueados = dictionary_get(diccionario_blocked, nombre_cliente ->nombre);
			pthread_mutex_unlock(&mutex_para_diccionario_blocked);

            pcb* un_pcb = buscar_proceso_en_cola(pid,nodo_bloqueados);

            pthread_mutex_lock(&mutex_para_diccionario_entradasalida);
            nodo_de_diccionario_interfaz* nodo_interfaz = dictionary_get(diccionario_entrada_salida,nombre_cliente ->nombre);
            pthread_mutex_unlock(&mutex_para_diccionario_entradasalida);

            if(un_pcb != NULL){
                eliminar_variable(nodo_interfaz, nodo_bloqueados);
                pthread_mutex_lock(&(nodo_bloqueados ->mutex_para_cola_bloqueados));
                eliminar_pcb_cola(nodo_bloqueados -> cola_bloqueados, un_pcb);
                pthread_mutex_unlock(&(nodo_bloqueados ->mutex_para_cola_bloqueados));

                un_pcb ->estado_proceso = READY;
                log_info(logger_obligatorio,"PID: %d - Estado Anterior: BLOCKED - Estado Actual: READY", un_pcb ->PID);
                strcpy(un_pcb ->interfaz_bloqueante, "No");

                if(strcmp(ALGORITMO_PLANIFICACION, "vrr")==0){
                    int64_t tiempo_ejecutado = temporal_gettime(un_pcb ->tiempo_en_ejecucion);
					free(un_pcb ->tiempo_en_ejecucion);
                    un_pcb ->tiempo_en_ejecucion = NULL;
                    if(tiempo_ejecutado < un_pcb -> quantum_restante){
                        un_pcb ->quantum_restante = un_pcb ->quantum_restante - tiempo_ejecutado;

                        pthread_mutex_lock(&mutex_cola_prioritaria);
                        queue_push(cola_ready_prioritaria,un_pcb);
                        log_de_lista_de_ready_prioritaria();
                        pthread_mutex_unlock(&mutex_cola_prioritaria);
                    }
                    else{
                        un_pcb ->quantum_restante = QUANTUM;
                        log_info(logger_obligatorio,"PID: %d - Desalojado por fin de Quantum",un_pcb ->PID);

                        pthread_mutex_lock(&mutex_cola_ready);
                        queue_push(cola_ready,un_pcb);
                        log_de_lista_de_ready(); 
                        pthread_mutex_unlock(&mutex_cola_ready);
                    }
                }
                else{
                    if(un_pcb ->tiempo_en_ejecucion != NULL){
                        free(un_pcb ->tiempo_en_ejecucion);
                        un_pcb ->tiempo_en_ejecucion = NULL;
                    }
                    pthread_mutex_lock(&mutex_cola_ready);
			        queue_push(cola_ready,un_pcb);
                    log_de_lista_de_ready();
			        pthread_mutex_unlock(&mutex_cola_ready);
                }
			    

                sem_post(&hay_proceso_en_ready);
            }

            sem_post(&(nodo_interfaz ->se_puede_enviar_proceso));
            break;

        case FALLO_IO:
            buffer = recibir_buffer(*nombre_cliente ->cliente);
			int pid2 = extraer_int_buffer(buffer,logger);


			pthread_mutex_lock(&mutex_para_diccionario_blocked);
			nodo_de_diccionario_blocked* nodo_bloqueados2 = dictionary_get(diccionario_blocked, nombre_cliente ->nombre);
			pthread_mutex_unlock(&mutex_para_diccionario_blocked);

            pthread_mutex_lock(&mutex_para_diccionario_entradasalida);
            nodo_de_diccionario_interfaz* nodo_interfaz2 = dictionary_get(diccionario_entrada_salida,nombre_cliente ->nombre);
            pthread_mutex_unlock(&mutex_para_diccionario_entradasalida);

            eliminar_variable(nodo_interfaz2, nodo_bloqueados2);

            pcb* un_pcb2 = buscar_proceso_en_cola(pid2,nodo_bloqueados);

            if(un_pcb2 != NULL){
                pthread_mutex_lock(&(nodo_bloqueados2 ->mutex_para_cola_bloqueados));
                eliminar_pcb_cola(nodo_bloqueados2 -> cola_bloqueados, un_pcb2);
                pthread_mutex_unlock(&(nodo_bloqueados2 ->mutex_para_cola_bloqueados));

                un_pcb2 ->estado_proceso = EXIT_PROCESS;
                un_pcb2 ->razon_salida = FALLO_EN_IO;

                mandar_a_exit(un_pcb2);
                log_info(logger_obligatorio,"PID: %d - Estado Anterior: BLOCKED - Estado Actual: EXIT", un_pcb2 ->PID);

            }

            sem_post(&(nodo_interfaz2 ->se_puede_enviar_proceso));


            break;

        case -1:
            pthread_mutex_lock(&mutex_para_eliminar_entradasalida);

            pthread_mutex_lock(&mutex_para_diccionario_entradasalida);
            nodo_de_diccionario_interfaz* nodo = dictionary_remove(diccionario_entrada_salida,nombre_cliente ->nombre);
            pthread_mutex_unlock(&mutex_para_diccionario_entradasalida);
            

            pcb* el_pcb;

            pthread_mutex_lock(&mutex_para_diccionario_blocked);
            nodo_de_diccionario_blocked* nodo_de_bloqueados = dictionary_remove(diccionario_blocked,nombre_cliente ->nombre);
            pthread_mutex_unlock(&mutex_para_diccionario_blocked);

            int cantidad_procesos_bloqueados;

            pthread_mutex_lock(&(nodo_de_bloqueados->mutex_para_cola_bloqueados));
            cantidad_procesos_bloqueados = queue_size(nodo_de_bloqueados -> cola_bloqueados);
            pthread_mutex_unlock(&(nodo_de_bloqueados->mutex_para_cola_bloqueados));

            for(int i = 0; i<cantidad_procesos_bloqueados; i++){
                pthread_mutex_lock(&(nodo_de_bloqueados->mutex_para_cola_bloqueados));
                el_pcb = queue_pop(nodo_de_bloqueados -> cola_bloqueados);
                pthread_mutex_unlock(&(nodo_de_bloqueados->mutex_para_cola_bloqueados));

                eliminar_variable(nodo, nodo_de_bloqueados);

                el_pcb ->razon_salida = INTERFAZ_INVALIDA;

                mandar_a_exit(el_pcb);
                log_info(logger_obligatorio, "PID: %d - Estado Anterior: BLOCKED - Estado Actual: EXIT", el_pcb->PID);

            }
            queue_destroy(nodo_de_bloqueados ->cola_bloqueados);
            queue_destroy(nodo_de_bloqueados ->cola_Variables);
            pthread_mutex_destroy(&(nodo_de_bloqueados ->mutex_para_cola_bloqueados));
            pthread_mutex_destroy(&(nodo_de_bloqueados ->mutex_para_cola_variables));
            free(nodo_de_bloqueados);

            free(nodo ->tipo_de_interfaz);
            liberar_conexion(*nodo ->cliente);
            free(nodo ->cliente);
            sem_destroy(&(nodo ->hay_proceso_en_bloqueados));
            sem_destroy(&(nodo ->se_puede_enviar_proceso));
            pthread_mutex_destroy(&(nodo ->mutex_interfaz_siendo_usada));
            free(nodo);

            continuar_while = false;

            pthread_mutex_unlock(&mutex_para_eliminar_entradasalida);
            break;
    
        default:
            log_info(logger,"Nada paso");
            break;
        }
    }

}

void enviar_proceso_interfaz(void* nombre_interfaz_y_cliente){
    
    nombre_y_cliente* nombre_cliente = nombre_interfaz_y_cliente;
    bool continuar_while = true;

    pthread_mutex_lock(&mutex_para_diccionario_entradasalida);
    nodo_de_diccionario_interfaz* nodo_interfaz = dictionary_get(diccionario_entrada_salida,nombre_cliente ->nombre);
    pthread_mutex_unlock(&mutex_para_diccionario_entradasalida);
   

    pthread_mutex_lock(&mutex_para_diccionario_blocked);
    nodo_de_diccionario_blocked* nodo_blocked = dictionary_get(diccionario_blocked,nombre_cliente->nombre);
    pthread_mutex_unlock(&mutex_para_diccionario_blocked);
    
    
    while(continuar_while){
        
        sem_wait(&(nodo_interfaz ->hay_proceso_en_bloqueados));
        sem_wait(&(nodo_interfaz ->se_puede_enviar_proceso));

        if(!permitir_planificacion){
            
            sem_wait(&(nodo_interfaz ->detener_planificacion_enviar_peticion_IO));
        }
        
        pcb* pcb_a_enviar = NULL;


        pthread_mutex_lock(&(nodo_blocked ->mutex_para_cola_bloqueados));
        if(!queue_is_empty(nodo_blocked ->cola_bloqueados)){
            pcb_a_enviar = queue_peek(nodo_blocked ->cola_bloqueados);
        }
        pthread_mutex_unlock(&(nodo_blocked ->mutex_para_cola_bloqueados));

        if(pcb_a_enviar != NULL){
            if(strcmp(nodo_interfaz ->tipo_de_interfaz, "Generica")==0){
                pthread_mutex_lock(&(nodo_blocked ->mutex_para_cola_variables));
                int* tiempo_espera = queue_peek(nodo_blocked ->cola_Variables);
                pthread_mutex_unlock(&(nodo_blocked ->mutex_para_cola_variables));

                t_paquete* paquete = crear_paquete(ESPERAR_GEN);
                agregar_int_a_paquete(paquete,pcb_a_enviar ->PID);
                agregar_int_a_paquete(paquete,*tiempo_espera);
                enviar_paquete(paquete,*nodo_interfaz ->cliente);
                eliminar_paquete(paquete);
            }
            else if(strcmp(nodo_interfaz ->tipo_de_interfaz, "stdin")==0){
                pthread_mutex_lock(&(nodo_blocked ->mutex_para_cola_variables));
                io_std_fs* dir_fisicas = queue_peek(nodo_blocked ->cola_Variables);
                pthread_mutex_unlock(&(nodo_blocked ->mutex_para_cola_variables));

                t_paquete* paquete2 = crear_paquete(STD_READ_CODE);
                agregar_int_a_paquete(paquete2, pcb_a_enviar ->PID);
                agregar_int_a_paquete(paquete2, dir_fisicas ->tam);
                agregar_int_a_paquete(paquete2, dir_fisicas ->cant_dir_fisicas);

                for(int i = 0; i < list_size(dir_fisicas ->lista_dir_fisicas); i++){
                    dir_fis_y_tam* dir_fisica_y_tam = list_get(dir_fisicas ->lista_dir_fisicas, i);
                    agregar_int_a_paquete(paquete2, dir_fisica_y_tam ->dir_fisica);
                    agregar_int_a_paquete(paquete2, dir_fisica_y_tam ->tam);
                }
                enviar_paquete(paquete2, *nodo_interfaz ->cliente);
                eliminar_paquete(paquete2);
                
            }
            else if(strcmp(nodo_interfaz ->tipo_de_interfaz, "stdout")==0){
                pthread_mutex_lock(&(nodo_blocked ->mutex_para_cola_variables));
                io_std_fs* dir_fisicas = queue_peek(nodo_blocked ->cola_Variables);
                pthread_mutex_unlock(&(nodo_blocked ->mutex_para_cola_variables));

                t_paquete* paquete2 = crear_paquete(STD_WRITE_CODE);
                agregar_int_a_paquete(paquete2, pcb_a_enviar ->PID);
                agregar_int_a_paquete(paquete2, dir_fisicas ->tam);
                agregar_int_a_paquete(paquete2, dir_fisicas ->cant_dir_fisicas);

                for(int i = 0; i < list_size(dir_fisicas ->lista_dir_fisicas); i++){
                    dir_fis_y_tam* dir_fisica_y_tam = list_get(dir_fisicas ->lista_dir_fisicas, i);
                    agregar_int_a_paquete(paquete2, dir_fisica_y_tam ->dir_fisica);
                    agregar_int_a_paquete(paquete2, dir_fisica_y_tam ->tam);
                }
                enviar_paquete(paquete2, *nodo_interfaz ->cliente);
                eliminar_paquete(paquete2);

            }
            else if(strcmp(nodo_interfaz ->tipo_de_interfaz, "dialfs")==0){
                pthread_mutex_lock(&(nodo_blocked ->mutex_para_cola_variables));
                var_fs* variable_fs = queue_peek(nodo_blocked ->cola_Variables);
                pthread_mutex_unlock(&(nodo_blocked ->mutex_para_cola_variables));

                switch(variable_fs ->tipo_variable){

                    case VAR_FS_CREATE:
                        enviar_fs_create_delete(nodo_interfaz, variable_fs, FS_CREATE_CODE, pcb_a_enviar->PID);
                        break;
                    
                    case VAR_FS_DELETE:
                        enviar_fs_create_delete(nodo_interfaz, variable_fs, FS_DELETE_CODE, pcb_a_enviar->PID);                 
                        break;
                    
                    case VAR_FS_TRUNCATE:
                        t_paquete* paquete = crear_paquete(FS_TRUNCATE_CODE);
                        agregar_int_a_paquete(paquete, pcb_a_enviar ->PID);
                        agregar_string_a_paquete(paquete,variable_fs ->nombre_Archivo);
                        agregar_int_a_paquete(paquete, variable_fs ->tam_truncate);
                        enviar_paquete(paquete, *nodo_interfaz ->cliente);
                        eliminar_paquete(paquete);
                        
                        break;

                    case VAR_FS_READ:
                        enviar_fs_read_write(nodo_interfaz, variable_fs, FS_READ_CODE, pcb_a_enviar->PID);
                        break;
                    
                    case VAR_FS_WRITE:
                        enviar_fs_read_write(nodo_interfaz, variable_fs, FS_WRITE_CODE, pcb_a_enviar->PID);
                        break;
                }
            }
        }
        else{
            sem_post(&(nodo_interfaz ->se_puede_enviar_proceso));
        }
        

        
    }
}

pcb* buscar_proceso_en_cola(int pid, nodo_de_diccionario_blocked* nodo){
    int cantidad_procesos;
    pthread_mutex_lock(&(nodo ->mutex_para_cola_bloqueados));
    cantidad_procesos = queue_size(nodo ->cola_bloqueados);
    pthread_mutex_unlock(&(nodo ->mutex_para_cola_bloqueados));
    pcb* pcb_revisar;
    for(int i = 0; i<cantidad_procesos; i++){
        pthread_mutex_lock(&(nodo ->mutex_para_cola_bloqueados));
        pcb_revisar = list_get(nodo -> cola_bloqueados -> elements,i);
        pthread_mutex_unlock(&(nodo ->mutex_para_cola_bloqueados));
        if(pcb_revisar->PID == pid){
            return pcb_revisar;
        }
    }
    return NULL;

}

void atender_las_nuevas_interfaces(){

    while(1){
        int cliente = esperar_cliente(kernel_server, logger, "Entrada Salida Conectado");
        pthread_t hilo_atender_entradasalida;
        pthread_create(&hilo_atender_entradasalida,NULL, (void*)atender_nueva_interfaz,(void*)&cliente);
        pthread_detach(hilo_atender_entradasalida);
    }
    
}

void enviar_fs_create_delete(nodo_de_diccionario_interfaz* nodo_interfaz, var_fs* variable_fs, int cod_op, int PID){
    t_paquete* paquete = crear_paquete(cod_op);
    agregar_int_a_paquete(paquete, PID);
    agregar_string_a_paquete(paquete, variable_fs ->nombre_Archivo);
    enviar_paquete(paquete, *nodo_interfaz ->cliente);
    eliminar_paquete(paquete);                                             
}

void enviar_fs_read_write(nodo_de_diccionario_interfaz* nodo_interfaz, var_fs* variable_fs, int cod_op, int PID){
    t_paquete* paquete = crear_paquete(cod_op);
    agregar_int_a_paquete(paquete, PID);
    agregar_string_a_paquete(paquete,variable_fs ->nombre_Archivo);
    agregar_int_a_paquete(paquete, variable_fs ->puntero_Arch);

    agregar_int_a_paquete(paquete, variable_fs-> dir_fisicas ->tam);
    agregar_int_a_paquete(paquete, variable_fs->dir_fisicas ->cant_dir_fisicas);

    for(int i = 0; i < list_size(variable_fs->dir_fisicas ->lista_dir_fisicas); i++){
        dir_fis_y_tam* dir_fisica_y_tam = list_get(variable_fs ->dir_fisicas ->lista_dir_fisicas, i);
        agregar_int_a_paquete(paquete, dir_fisica_y_tam ->dir_fisica);
        agregar_int_a_paquete(paquete, dir_fisica_y_tam ->tam);
    }
    enviar_paquete(paquete, *nodo_interfaz ->cliente);
    eliminar_paquete(paquete);
   
}

void eliminar_variable(nodo_de_diccionario_interfaz* nodo_interfaz, nodo_de_diccionario_blocked* nodo_blocked){
    if(strcmp(nodo_interfaz ->tipo_de_interfaz, "Generica")==0){
        pthread_mutex_lock(&(nodo_blocked ->mutex_para_cola_variables));
        int* tiempo_espera = queue_pop(nodo_blocked ->cola_Variables);
        pthread_mutex_unlock(&(nodo_blocked ->mutex_para_cola_variables));
        free(tiempo_espera);
    }
    else if(strcmp(nodo_interfaz ->tipo_de_interfaz, "stdin")==0 || strcmp(nodo_interfaz ->tipo_de_interfaz, "stdout")==0){
        pthread_mutex_lock(&(nodo_blocked ->mutex_para_cola_variables));
        io_std_fs* dir_fisicas = queue_pop(nodo_blocked ->cola_Variables);
        pthread_mutex_unlock(&(nodo_blocked ->mutex_para_cola_variables));
        list_destroy_and_destroy_elements(dir_fisicas ->lista_dir_fisicas, free);
        free(dir_fisicas ->interfaz);
        free(dir_fisicas);
                    
    }
    else if(strcmp(nodo_interfaz ->tipo_de_interfaz, "dialfs")==0){
        pthread_mutex_lock(&(nodo_blocked ->mutex_para_cola_variables));
        var_fs* variable_fs = queue_pop(nodo_blocked ->cola_Variables);
        pthread_mutex_unlock(&(nodo_blocked ->mutex_para_cola_variables));

        if(variable_fs ->tipo_variable == VAR_FS_READ || variable_fs ->tipo_variable == VAR_FS_WRITE){
            list_destroy_and_destroy_elements(variable_fs->dir_fisicas ->lista_dir_fisicas, free);
            free(variable_fs ->dir_fisicas ->interfaz);
            free(variable_fs->dir_fisicas);
        }
        free(variable_fs ->nombre_Archivo);
        free(variable_fs);
    }
}