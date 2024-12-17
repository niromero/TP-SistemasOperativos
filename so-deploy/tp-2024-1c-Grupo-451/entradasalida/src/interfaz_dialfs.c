#include <interfaz_dialfs.h>


void atender_peticiones_dialfs(){
    while(1){
        int cod_op = recibir_operacion(entradasalida_cliente_kernel);
        t_buffer* buffer;
        switch(cod_op){
            case FS_CREATE_CODE:
                buffer = recibir_buffer(entradasalida_cliente_kernel);
                int pid1 = extraer_int_buffer(buffer, logger);
                char* nombre_Archivo1 = extraer_string_buffer(buffer, logger);

                usleep(TIEMPO_UNIDAD_TRABAJO * 1000);

                int bloque_elegido = buscar_bloque_libre();

                if(bloque_elegido == -1){
                    t_paquete* paquete = crear_paquete(FALLO_IO);
                    agregar_int_a_paquete(paquete,pid1);
                    enviar_paquete(paquete,entradasalida_cliente_kernel);
                    eliminar_paquete(paquete);
                }
                else{
                    char* path_archivo = strdup(PATH_BASE_DIALFS);
                    string_append(&path_archivo, nombre_Archivo1);

                    FILE* metadatos = fopen(path_archivo, "r");

                    if(metadatos == NULL){
                        metadatos = fopen(path_archivo, "w");
                        fclose(metadatos);

                        t_config* meta_config = config_create(path_archivo);

                        char* bloque_elegido_clave = string_itoa(bloque_elegido);

                        config_set_value(meta_config, "BLOQUE_INICIAL", bloque_elegido_clave);
                        config_set_value(meta_config, "TAMANIO_ARCHIVO", "0");

                        free(bloque_elegido_clave);

                        config_save(meta_config);

                        config_destroy(meta_config);

                        bitarray_set_bit(bitmap_bloques,bloque_elegido);

                        float cant_block_float = BLOCK_COUNT;

                        int tamanio_bitmap = ceil(cant_block_float/8);

                        msync(puntero_a_bits_de_bloques, tamanio_bitmap, MS_SYNC);

                        t_archivo* nodo_lista = malloc(sizeof(t_archivo));

                        strcpy(nodo_lista ->nombreArchivo, nombre_Archivo1);
                        nodo_lista ->posicionInicial = bloque_elegido;
                        nodo_lista ->tamanio = 0;

                        list_add(lista_archivos, nodo_lista);

                        copiar_lista_a_archivo();
                    
                    }
                    else{
                        log_info(logger, "Se intenta crear un archivo que ya existe, no se hara nada");
                    }

                    free(path_archivo);

                    log_info(logger_obligatorio, "Interfaz: %s - PID: %d - Crear Archivo: %s",nombre_interfaz,pid1,nombre_Archivo1);

                    t_paquete* paquete = crear_paquete(EXITO_IO);
                    agregar_int_a_paquete(paquete,pid1);
                    enviar_paquete(paquete,entradasalida_cliente_kernel);
                    eliminar_paquete(paquete);
                }

                free(nombre_Archivo1);
                               
                break;           
            case FS_DELETE_CODE:
                buffer = recibir_buffer(entradasalida_cliente_kernel);
                int pid2 = extraer_int_buffer(buffer, logger);
                char* nombre_Archivo2 = extraer_string_buffer(buffer, logger);

                usleep(TIEMPO_UNIDAD_TRABAJO * 1000);

                char* path_archivo2 = strdup(PATH_BASE_DIALFS);
                string_append(&path_archivo2, nombre_Archivo2);

                t_config* meta_config_delete = config_create(path_archivo2);

                if(meta_config_delete != NULL){
                    int primera_bloque = config_get_int_value(meta_config_delete, "BLOQUE_INICIAL");
                    int tamanio_total = config_get_int_value(meta_config_delete, "TAMANIO_ARCHIVO");

                    float tamanio_total_float = tamanio_total;

                    int cant_bloques = ceil(tamanio_total_float / BLOCK_SIZE);

                    for(int i = 0; i < cant_bloques; i++){
                        bitarray_clean_bit(bitmap_bloques,primera_bloque);
                        primera_bloque++;
                    }

                    float cant_block_float = BLOCK_COUNT;

                    int tamanio_bitmap = ceil(cant_block_float/8);

                    msync(puntero_a_bits_de_bloques, tamanio_bitmap, MS_SYNC);

                    config_destroy(meta_config_delete);

                    remove(path_archivo2);

                    for(int i = 0; i < list_size(lista_archivos); i++){
                        t_archivo* nodo_lista = list_get(lista_archivos,i);

                        if(strcmp(nodo_lista ->nombreArchivo,nombre_Archivo2)==0){
                            list_remove(lista_archivos, i);
                            free(nodo_lista);
                            break;
                        }
                    }

                    copiar_lista_a_archivo();

                }
                else{
                    log_info(logger, "Se intenta borrar un archivo que no existe, no se hara nada");
                }

                log_info(logger_obligatorio, "Interfaz: %s - PID: %d - Eliminar Archivo: %s",nombre_interfaz,pid2,nombre_Archivo2);


                t_paquete* paquete = crear_paquete(EXITO_IO);
                agregar_int_a_paquete(paquete,pid2);
                enviar_paquete(paquete,entradasalida_cliente_kernel);
                eliminar_paquete(paquete);

                free(nombre_Archivo2);
                free(path_archivo2);
                
                break;
            case FS_TRUNCATE_CODE:
                buffer = recibir_buffer(entradasalida_cliente_kernel);
                int pid3 = extraer_int_buffer(buffer, logger);
                char* nombre_Archivo3 = extraer_string_buffer(buffer, logger);
                int tamanio_a_truncar = extraer_int_buffer(buffer, logger);

                usleep(TIEMPO_UNIDAD_TRABAJO * 1000);

                string_append(&nombre_Archivo3, "\n");

                char* path_archivo3 = strdup(PATH_BASE_DIALFS);
                string_append(&path_archivo3, nombre_Archivo3);

                t_config* meta_config_truncate = config_create(path_archivo3);

                int tam_actual = config_get_int_value(meta_config_truncate, "TAMANIO_ARCHIVO");

                int primera_bloque = config_get_int_value(meta_config_truncate, "BLOQUE_INICIAL");

                int cant_bloques_total;

                if(tam_actual!=0){
                    cant_bloques_total = ceil((float)tam_actual/BLOCK_SIZE);
                }
                else{
                    cant_bloques_total = 1;
                }

                

                int ultimo_bloque = cant_bloques_total + primera_bloque -1;

                if(tam_actual > tamanio_a_truncar){

                    if((cant_bloques_total * BLOCK_SIZE) > tamanio_a_truncar){
                        float tam_a_reducir = (cant_bloques_total * BLOCK_SIZE) - tamanio_a_truncar;

                        int cant_bloques_a_reducir = floor(tam_a_reducir/BLOCK_SIZE);

                        for(int i = 0; i < cant_bloques_a_reducir; i++){
                            bitarray_clean_bit(bitmap_bloques,ultimo_bloque);
                            ultimo_bloque--;
                        }
                    }
                    

                }
                else if(tam_actual < tamanio_a_truncar){
                    if((cant_bloques_total * BLOCK_SIZE) <  tamanio_a_truncar){
                        float tam_a_aumentar = tamanio_a_truncar - (cant_bloques_total * BLOCK_SIZE);

                        int cant_bloques_a_aumentar = ceil(tam_a_aumentar/BLOCK_SIZE);

                        bool hay_espacio_continuo = true;

                        int pos_inicial = ultimo_bloque + 1;

                        for(int i = 0; i< cant_bloques_a_aumentar; i++){

                            if(bitarray_test_bit(bitmap_bloques, pos_inicial)){
                                hay_espacio_continuo = false;
                                break;
                            }

                            pos_inicial++;
                        }

                        if(hay_espacio_continuo){
                            pos_inicial = ultimo_bloque + 1;

                            for(int i = 0; i < cant_bloques_a_aumentar; i++){
                                bitarray_set_bit(bitmap_bloques,pos_inicial);
                                pos_inicial++;
                            }

                        }
                        else{
                            log_info(logger_obligatorio, "Interfaz: %s - PID: %d - Inicio Compactación.",nombre_interfaz, pid3);
                            int nuevo_bloque_inicial = realizar_compatacion(nombre_Archivo3);

                            usleep(1000 * RETRASO_COMPACTACION);

                            log_info(logger_obligatorio, "Interfaz: %s - PID: %d - Fin Compactación.", nombre_interfaz,pid3);

                            pos_inicial = cant_bloques_total + nuevo_bloque_inicial;

                            char* nuevo_bloque_inicial_clave = string_itoa(nuevo_bloque_inicial);

                            config_set_value(meta_config_truncate, "BLOQUE_INICIAL", nuevo_bloque_inicial_clave);

                            free(nuevo_bloque_inicial_clave);

                            for(int i = 0; i < cant_bloques_a_aumentar; i++){
                                bitarray_set_bit(bitmap_bloques,pos_inicial);
                                pos_inicial++;
                            }
                        }

                    }
                    
                }

                float cant_block_float = BLOCK_COUNT;

                int tamanio_bitmap = ceil(cant_block_float/8);

                msync(puntero_a_bits_de_bloques, tamanio_bitmap, MS_SYNC);

                log_info(logger_obligatorio, "Interfaz: %s - PID: %d - Truncar Archivo: %s - Tamaño: %d",nombre_interfaz,pid3,nombre_Archivo3,tamanio_a_truncar);

                char* tam_truncar_clave = string_itoa(tamanio_a_truncar);

                config_set_value(meta_config_truncate, "TAMANIO_ARCHIVO", tam_truncar_clave);

                free(tam_truncar_clave);

                config_save(meta_config_truncate);

                config_destroy(meta_config_truncate);

                for(int i = 0; i < list_size(lista_archivos); i++){
                    t_archivo* nodo_lista =  list_get(lista_archivos,i);
                    if(strcmp(nodo_lista ->nombreArchivo, nombre_Archivo3)==0){
                        nodo_lista ->tamanio = tamanio_a_truncar;
                    }
                }

                copiar_lista_a_archivo();

                t_paquete* paquete3 = crear_paquete(EXITO_IO);
                agregar_int_a_paquete(paquete3, pid3);
                enviar_paquete(paquete3, entradasalida_cliente_kernel);
                eliminar_paquete(paquete3);

                free(nombre_Archivo3);
                free(path_archivo3);

                break;
            case FS_READ_CODE:
                buffer = recibir_buffer(entradasalida_cliente_kernel);
                int pid4 = extraer_int_buffer(buffer,logger);
                char* nombre_Archivo4 = extraer_string_buffer(buffer, logger);
                int puntero_Arch4 = extraer_int_buffer(buffer, logger);
                int tam_total4 = extraer_int_buffer(buffer,logger);
                int cant_dir_fisicas4 = extraer_int_buffer(buffer, logger);

                usleep(TIEMPO_UNIDAD_TRABAJO * 1000);

                string_append(&nombre_Archivo4, "\n");


                void* leido_de_fs = malloc(tam_total4);

                char* path_archivo4 = strdup(PATH_BASE_DIALFS);
                string_append(&path_archivo4, nombre_Archivo4);

                t_config* meta_config_read = config_create(path_archivo4);

                int posicion_arch = config_get_int_value(meta_config_read, "BLOQUE_INICIAL");

                int desplazamiento_en_bloques = (posicion_arch * BLOCK_SIZE) + puntero_Arch4;

                memcpy(leido_de_fs, archivo_bloques_en_mem + desplazamiento_en_bloques, tam_total4);

                int desplazamiento4 = 0;

                for(int i = 0; i< cant_dir_fisicas4; i++){
                    int dir_fisca = extraer_int_buffer(buffer, logger);
                    int tam = extraer_int_buffer(buffer, logger);

                    void* copiar_a_mem = malloc(tam);

                    memcpy(copiar_a_mem,leido_de_fs + desplazamiento4, tam);

                    desplazamiento4 = desplazamiento4 + tam;

                    t_paquete* paquete1 = crear_paquete(ESCRITURA_CODE);

                    agregar_int_a_paquete(paquete1, pid4);
                    agregar_int_a_paquete(paquete1, dir_fisca);
                    agregar_int_a_paquete(paquete1,tam);

                    agregar_a_paquete(paquete1, copiar_a_mem, tam);
                    enviar_paquete(paquete1, entradasalida_cliente_memoria);
                    eliminar_paquete(paquete1);
                    free(copiar_a_mem);

                    confirmacion_escritura();
                    
                }

                

                log_info(logger_obligatorio,"Interfaz: %s - PID: %d - Leer Archivo: %s - Tamaño a Leer: %d - Puntero Archivo: %d",nombre_interfaz,pid4,nombre_Archivo4,tam_total4,puntero_Arch4);

                free(nombre_Archivo4);
                free(path_archivo4);
                free(leido_de_fs);

                config_destroy(meta_config_read);

                t_paquete* paquete4 = crear_paquete(EXITO_IO);
                agregar_int_a_paquete(paquete4, pid4);
                enviar_paquete(paquete4, entradasalida_cliente_kernel);
                eliminar_paquete(paquete4);


                break;
                
            case FS_WRITE_CODE:
                buffer = recibir_buffer(entradasalida_cliente_kernel);
                int pid5 = extraer_int_buffer(buffer,logger);
                char* nombre_Archivo5 = extraer_string_buffer(buffer, logger);
                int puntero_Arch5 = extraer_int_buffer(buffer, logger);
                int tam_total5 = extraer_int_buffer(buffer,logger);
                int cant_dir_fisicas5 = extraer_int_buffer(buffer, logger);

                usleep(TIEMPO_UNIDAD_TRABAJO * 1000);

                string_append(&nombre_Archivo5, "\n");
                
                void* contenido = malloc(tam_total5);

                int desplazamiento5 = 0;

                for(int i = 0; i< cant_dir_fisicas5; i++){
                    int dir_fisca = extraer_int_buffer(buffer, logger);
                    int tam = extraer_int_buffer(buffer, logger);

                    t_paquete* paquete1 = crear_paquete(LECTURA_CODE);

                    agregar_int_a_paquete(paquete1, pid5);
                    agregar_int_a_paquete(paquete1, dir_fisca);
                    agregar_int_a_paquete(paquete1,tam);

                    enviar_paquete(paquete1, entradasalida_cliente_memoria);
                    eliminar_paquete(paquete1);
                    
                    void* leido = recibir_lectura();

                    memcpy(contenido + desplazamiento5, leido, tam);

                    desplazamiento5 = desplazamiento5 + tam;

                    free(leido);
                }

                char* path_archivo5 = strdup(PATH_BASE_DIALFS);
                string_append(&path_archivo5, nombre_Archivo5);

                t_config* meta_config_write = config_create(path_archivo5);

                int posicion_arch2 = config_get_int_value(meta_config_write, "BLOQUE_INICIAL");

                int desplazamiento_en_bloques2 = (posicion_arch2 * BLOCK_SIZE) + puntero_Arch5;

                memcpy(archivo_bloques_en_mem + desplazamiento_en_bloques2,contenido ,tam_total5);

                msync(archivo_bloques_en_mem, (BLOCK_COUNT * BLOCK_SIZE), MS_SYNC);

                config_destroy(meta_config_write);

                free(contenido);

                log_info(logger_obligatorio,"Interfaz: %s - PID: %d - Escribir Archivo: %s - Tamaño a Escribir: %d - Puntero Archivo: %d", nombre_interfaz,pid5, nombre_Archivo5, tam_total5, puntero_Arch5);

                free(nombre_Archivo5);

                free(path_archivo5);

                t_paquete* paquete5 = crear_paquete(EXITO_IO);
                agregar_int_a_paquete(paquete5, pid5);
                enviar_paquete(paquete5, entradasalida_cliente_kernel);
                eliminar_paquete(paquete5);

                break;
            default:
                break;
        }
    }
}


void levantar_archivos(){
    int tamanio_archivo = BLOCK_SIZE * BLOCK_COUNT;
    char* nombre_archivo_bloques = strdup(PATH_BASE_DIALFS);
    string_append(&nombre_archivo_bloques, "bloques.dat");

    Archivo_bloques = fopen(nombre_archivo_bloques, "r+b");

    if(Archivo_bloques == NULL){
        Archivo_bloques = fopen(nombre_archivo_bloques, "w+b");

        truncate(nombre_archivo_bloques,tamanio_archivo);
    }
    else{
        fseek(Archivo_bloques, 0, SEEK_END);
        int tam = ftell(Archivo_bloques);
        fseek(Archivo_bloques, 0, SEEK_SET);
        
        if(tam != tamanio_archivo){
            truncate(nombre_archivo_bloques, tamanio_archivo);
        }
    }

    int fd_bloques = fileno(Archivo_bloques);

    archivo_bloques_en_mem = mmap(NULL,tamanio_archivo, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bloques, 0);

    fclose(Archivo_bloques);

    free(nombre_archivo_bloques);

    char* nombre_archivo_bitmap = strdup(PATH_BASE_DIALFS);
    string_append(&nombre_archivo_bitmap, "bitmap.dat");

    Archivo_bitmap = fopen(nombre_archivo_bitmap, "r+b");

    float cant_block_float = BLOCK_COUNT;

    int tamanio_bitmap = ceil(cant_block_float/8);

    bool creacion = false;

    if(Archivo_bitmap == NULL){
        Archivo_bitmap = fopen(nombre_archivo_bitmap, "w+b");

        truncate(nombre_archivo_bitmap,tamanio_bitmap);

        creacion = true;
    }

    int fd_bitmap = fileno(Archivo_bitmap);

    puntero_a_bits_de_bloques = mmap(NULL, tamanio_bitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bitmap, 0);

    fclose(Archivo_bitmap);

    bitmap_bloques = bitarray_create_with_mode(puntero_a_bits_de_bloques, tamanio_bitmap, LSB_FIRST);

    if(creacion){
        for(int i = 0; i < BLOCK_COUNT; i++){
            bitarray_clean_bit(bitmap_bloques,i);
        }
    }

    free(nombre_archivo_bitmap);

    char* nombre_archivo_lista = strdup(PATH_BASE_DIALFS);
    string_append(&nombre_archivo_lista, "lista_archivos.dat");

    Archivo_lista = fopen(nombre_archivo_lista, "r+b");

    if(Archivo_lista == NULL){
        Archivo_lista = fopen(nombre_archivo_lista, "wb");
        
    }
    else{
        fseek(Archivo_lista, 0, SEEK_END); 
        float size = ftell(Archivo_lista);  
        rewind(Archivo_lista);

        int cant_archs = floor(size/sizeof(t_archivo));

        int i = 0;
        while(!feof(Archivo_lista) && i <cant_archs){
            t_archivo* nodo_archivo = malloc(sizeof(t_archivo));
            fread(nodo_archivo, sizeof(t_archivo), 1, Archivo_lista);
            list_add(lista_archivos,nodo_archivo);
            i++;

            //log_info(logger, "Nombre: %s. Tam: %d. Bloque Ini: %d", nodo_archivo ->nombreArchivo, nodo_archivo ->tamanio, nodo_archivo ->posicionInicial);
        }


    }
    
    fclose(Archivo_lista);

    free(nombre_archivo_lista);
}

int buscar_bloque_libre(){
    int i;

    for(i = 0; i < BLOCK_COUNT; i++){
        if(!bitarray_test_bit(bitmap_bloques,i)){
            return i;
        }
    }

    return -1;
}

void copiar_lista_a_archivo(){
    char* nombre_archivo_lista = strdup(PATH_BASE_DIALFS);
    string_append(&nombre_archivo_lista, "lista_archivos.dat");

    Archivo_lista = fopen(nombre_archivo_lista, "wb");

    for(int i = 0; i < list_size(lista_archivos); i++){
        t_archivo* nodo_lista = list_get(lista_archivos,i);

        fwrite(nodo_lista, sizeof(t_archivo), 1, Archivo_lista);
    }

    fclose(Archivo_lista);

    free(nombre_archivo_lista);
}

int realizar_compatacion(char* nombre_arch_a_expandirse){
    t_list* lista_aux_compactacion = list_create();

    for(int i = 0; i <list_size(lista_archivos); i++){
        t_archivo* nodo_lista = list_get(lista_archivos,i);

        t_archivo_compactacion* nodo_lista_aux = malloc(sizeof(t_archivo_compactacion));

        strcpy(nodo_lista_aux ->nombreArchivo, nodo_lista ->nombreArchivo);
        nodo_lista_aux ->posicionInicial = nodo_lista ->posicionInicial;
        nodo_lista_aux ->tamanio = nodo_lista ->tamanio;

        nodo_lista_aux ->datos = malloc(nodo_lista_aux ->tamanio);

        memcpy(nodo_lista_aux ->datos, archivo_bloques_en_mem + (nodo_lista_aux ->posicionInicial * BLOCK_SIZE), nodo_lista_aux ->tamanio);

        list_add(lista_aux_compactacion, nodo_lista_aux);
        
    }

    int desplazamiento = 0;

    for(int i = 0; i < BLOCK_COUNT; i++){
        bitarray_clean_bit(bitmap_bloques,i);
    }

    for(int i = 0; i < list_size(lista_aux_compactacion); i++){
        t_archivo_compactacion* nodo_aux = list_get(lista_aux_compactacion, i);

        if(strcmp(nombre_arch_a_expandirse, nodo_aux ->nombreArchivo)){
            memcpy(archivo_bloques_en_mem + desplazamiento, nodo_aux ->datos, nodo_aux ->tamanio);
            free(nodo_aux ->datos);

            int nueva_pos_inicial = desplazamiento/BLOCK_SIZE;

            float tam_block_float = BLOCK_SIZE;

            if(nodo_aux ->tamanio != 0){
                desplazamiento = desplazamiento + (ceil(nodo_aux ->tamanio/tam_block_float) * BLOCK_SIZE);
            }
            else{
                desplazamiento = desplazamiento + BLOCK_SIZE;
            }

            

            t_archivo* nodo_lista = list_get(lista_archivos,i);

            nodo_lista ->posicionInicial = nueva_pos_inicial;

            char* path_archivo = strdup(PATH_BASE_DIALFS);
            string_append(&path_archivo, nodo_lista ->nombreArchivo);

            t_config* meta_config = config_create(path_archivo);

            char* nueva_pos_inicial_clave = string_itoa(nueva_pos_inicial);

            config_set_value(meta_config, "BLOQUE_INICIAL", nueva_pos_inicial_clave);

            free(nueva_pos_inicial_clave);

            config_save(meta_config);

            config_destroy(meta_config);

            int cant_bloques_a_marcar;

            if(nodo_aux ->tamanio !=0){
                cant_bloques_a_marcar = ceil(nodo_aux ->tamanio/tam_block_float);
            }
            else{
                cant_bloques_a_marcar = 1;
            }

            for(int i = 0; i <cant_bloques_a_marcar; i++){
                bitarray_set_bit(bitmap_bloques,nueva_pos_inicial);
                nueva_pos_inicial++;
            }

            free(path_archivo);


        }
    }

    int nueva_pos_inicial_arch_expa = desplazamiento/BLOCK_SIZE;

    for(int i = 0; i < list_size(lista_aux_compactacion); i++){
        t_archivo_compactacion* nodo_aux = list_get(lista_aux_compactacion, i);

        if(strcmp(nodo_aux ->nombreArchivo, nombre_arch_a_expandirse)==0){
            memcpy(archivo_bloques_en_mem + desplazamiento, nodo_aux ->datos, nodo_aux ->tamanio);
            free(nodo_aux ->datos);

            float tam_block_float = BLOCK_SIZE;

            t_archivo* nodo_lista = list_get(lista_archivos,i);

            nodo_lista ->posicionInicial = nueva_pos_inicial_arch_expa;

            char* path_archivo = strdup(PATH_BASE_DIALFS);
            string_append(&path_archivo, nodo_lista ->nombreArchivo);

            t_config* meta_config = config_create(path_archivo);

            char* nueva_pos_inicial_arch_expa_clave = string_itoa(nueva_pos_inicial_arch_expa);

            config_set_value(meta_config, "BLOQUE_INICIAL", nueva_pos_inicial_arch_expa_clave);

            free(nueva_pos_inicial_arch_expa_clave);

            config_save(meta_config);

            config_destroy(meta_config);

            int nueva_pos_inicial = nueva_pos_inicial_arch_expa;

            int cant_bloques_a_marcar;

            if(nodo_aux ->tamanio !=0){
                cant_bloques_a_marcar = ceil(nodo_aux ->tamanio/tam_block_float);
            }
            else{
                cant_bloques_a_marcar = 1;
            }

            for(int i = 0; i <cant_bloques_a_marcar; i++){
                bitarray_set_bit(bitmap_bloques,nueva_pos_inicial);
                nueva_pos_inicial++;
            }

            free(path_archivo);

        }
    }

    list_destroy_and_destroy_elements(lista_aux_compactacion, free);

    msync(archivo_bloques_en_mem, (BLOCK_COUNT * BLOCK_SIZE), MS_SYNC);

    return nueva_pos_inicial_arch_expa;
}

