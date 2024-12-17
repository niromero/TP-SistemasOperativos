#include <memoria_usuario.h>

int cambiar_memoria_de_proceso(int pid, int nuevo_tam){ 
    char* clave_pid = string_itoa(pid);

    pthread_mutex_lock(&mutex_para_diccionario_tdp);
    nodo_dic_tdp* nodo_tdp = dictionary_get(diccionario_de_tdp,clave_pid);
    pthread_mutex_unlock(&mutex_para_diccionario_tdp);

    free(clave_pid);

    int tam_actual = list_size(nodo_tdp -> tdp_del_proceso) * TAM_PAGINA;

    if(nodo_tdp ->tam_de_proceso > nuevo_tam){

        if(tam_actual > nuevo_tam){
            float Bytes_A_Eliminar = tam_actual - nuevo_tam;

            int cant_pags_a_borrar = floor(Bytes_A_Eliminar / TAM_PAGINA);
            int ultima_pag = list_size(nodo_tdp ->tdp_del_proceso) - 1;

            for(int i = 0; i< cant_pags_a_borrar; i++){
                int* marco = list_remove(nodo_tdp ->tdp_del_proceso,ultima_pag);

                pthread_mutex_lock(&mutex_para_marcos_libres);
                bitarray_clean_bit(marcos_de_memoria_libres,*marco);
                pthread_mutex_unlock(&mutex_para_marcos_libres);

                free(marco);
                ultima_pag--;

            }
        }
        
        log_info(logger_obligatorio, "PID: %d - Tamaño Actual: %d - Tamaño a Reducir: %d", pid, nodo_tdp ->tam_de_proceso, (nodo_tdp ->tam_de_proceso - nuevo_tam));

        nodo_tdp ->tam_de_proceso = nuevo_tam;

        return true;
    }
    else if(nodo_tdp ->tam_de_proceso < nuevo_tam){

        if(tam_actual < nuevo_tam){
            float bytesNecesarios = nuevo_tam - tam_actual;

            int cant_marcos_nuevos = ceil(bytesNecesarios / TAM_PAGINA);

            log_info(logger_obligatorio, "PID: %d - Tamaño Actual: %d - Tamaño a Ampliar: %d", pid, nodo_tdp ->tam_de_proceso, (nuevo_tam - nodo_tdp ->tam_de_proceso));

            if(hay_marcos_suficientes(cant_marcos_nuevos)){

                int nueva_pag = list_size(nodo_tdp ->tdp_del_proceso); //Las paginas comienzan en 0, por lo que al hacer el list size obtenemos el valor de la siguiente pagina
                int paginas_reservadas = 0;

                for(int i = 0; i< cant_marcos_totales; i++){

                    pthread_mutex_lock(&mutex_para_marcos_libres);
                    bool esta_ocupado = bitarray_test_bit(marcos_de_memoria_libres,i);
                    pthread_mutex_unlock(&mutex_para_marcos_libres);

                    if(!esta_ocupado){

                        pthread_mutex_lock(&mutex_para_marcos_libres);
                        bitarray_set_bit(marcos_de_memoria_libres,i);
                        pthread_mutex_unlock(&mutex_para_marcos_libres);

                        int* marco_a_cargar = malloc(sizeof(int));
                        *marco_a_cargar = i;

                        list_add_in_index(nodo_tdp ->tdp_del_proceso,nueva_pag,marco_a_cargar);
                        nueva_pag++;
                        paginas_reservadas++;
                    }
                    if(paginas_reservadas == cant_marcos_nuevos){
                        nodo_tdp ->tam_de_proceso = nuevo_tam;
                        return true;
                    }

                }

                return false; //NO deberia devolver nunca este false, porque se supone que ya comprobaste antes que hay marcos disponibles

            
            }

            return false;
        }
        
        
    }

    log_info(logger_obligatorio, "PID: %d - Tamaño Actual: %d - Tamaño a Ampliar: 0", pid, nodo_tdp ->tam_de_proceso);

    return true;
}

bool hay_marcos_suficientes(int cant_marcos_necesarios){
    int marcos_libres = 0;

    for(int i = 0; i< cant_marcos_totales; i++){

        pthread_mutex_lock(&mutex_para_marcos_libres);
        bool marco = bitarray_test_bit(marcos_de_memoria_libres,i);
        pthread_mutex_unlock(&mutex_para_marcos_libres);

        //Se va a considerar el bit en 0 como espacio libre y el bit en 1 como ocupado

        if(!marco){
            marcos_libres++;
        }

        if(marcos_libres == cant_marcos_necesarios){
            return true;
        }
    }

    return false;
}

void* leer_dir_fisica(int dir_fisica, int tam_a_leer){
    void* leido = malloc(tam_a_leer);

    pthread_mutex_lock(&mutex_para_mem_de_usuario);
    memcpy(leido,memoria_de_usuario + dir_fisica,tam_a_leer);
    pthread_mutex_unlock(&mutex_para_mem_de_usuario);

    return leido;
}