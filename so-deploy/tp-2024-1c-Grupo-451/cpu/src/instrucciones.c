#include <instrucciones.h>


void set(char* nombre_registro, int num){
    if(strcmp(nombre_registro, "PC")==0){
        *los_registros_de_la_cpu ->PC = num - 1; //Asi, si seteas el PC; queda el numero que querias y no uno mas
    }
    else{
        int tamano = 0;
        void* registro = apuntar_a_registro(nombre_registro, &tamano);
        if (registro == NULL) {
            printf("Es NULL\n");
        }
        if (tamano == 8) {
            uint8_t* registro2 = (uint8_t*)registro;
            *registro2 = num;
        } else if (tamano == 32) {
            uint32_t* registro2 = (uint32_t*)registro;
            *registro2 = num;
        }
    }
    
}

void sum(char* nombre_registro_destino, char* nombre_registro_origen){
    int tamano_origen = 0;
    void* registro_origen = apuntar_a_registro(nombre_registro_origen, &tamano_origen);
    int contenido_origen = 0;

    if (tamano_origen == 8) {
        uint8_t* registro_origen2 = (uint8_t*)registro_origen;
        contenido_origen = *registro_origen2;
        
    } else if (tamano_origen == 32) {
        uint32_t* registro_origen2 = (uint32_t*)registro_origen;
        contenido_origen = *registro_origen2;
    }

    int tamano_destino = 0;
    void* registro_destino = apuntar_a_registro(nombre_registro_destino, &tamano_destino);

    if (tamano_destino == 8) {
        uint8_t* registro_destino2 = (uint8_t*)registro_destino;
        *registro_destino2 += contenido_origen;
        printf("%u\n", *registro_destino2);
    } else if (tamano_destino == 32) {
        uint32_t* registro_destino2 = (uint32_t*)registro_destino;
        *registro_destino2 += contenido_origen;
        printf("%u\n", *registro_destino2);
    }
}

void sub(char* nombre_registro_destino, char* nombre_registro_origen){
    int tamano_origen = 0;
    void* registro_origen = apuntar_a_registro(nombre_registro_origen, &tamano_origen);
    int contenido_origen = 0;

    if (tamano_origen == 8) {
        uint8_t* registro_origen2 = (uint8_t*)registro_origen;
        contenido_origen = *registro_origen2;
        
    } else if (tamano_origen == 32) {
        uint32_t* registro_origen2 = (uint32_t*)registro_origen;
        contenido_origen = *registro_origen2;
    }

    int tamano_destino = 0;
    void* registro_destino = apuntar_a_registro(nombre_registro_destino, &tamano_destino);

    if (tamano_destino == 8) {
        uint8_t* registro_destino2 = (uint8_t*)registro_destino;
        *registro_destino2 -= contenido_origen;
        printf("%u\n", *registro_destino2);
    } else if (tamano_destino == 32) {
        uint32_t* registro_destino2 = (uint32_t*)registro_destino;
        *registro_destino2 -= contenido_origen;
        printf("%u\n", *registro_destino2);
    }
}

void jnz(char* nombre_registro, int nuevo_pc){
    int tamano = 0;
    void* registro = apuntar_a_registro(nombre_registro, &tamano);
    int contenido = -1;
    
    if (tamano == 8) {
        uint8_t* registro2 = (uint8_t*)registro;
        contenido = *registro2;
        
    } else if (tamano == 32) {
        uint32_t* registro2 = (uint32_t*)registro;
        contenido = *registro2;
    }
    
    if(contenido){
        *los_registros_de_la_cpu -> PC = nuevo_pc -1;
    }
}

int resize(int tam){
    t_paquete* paquete = crear_paquete(RESIZE_CODE);
    agregar_int_a_paquete(paquete,pid_en_ejecucion);
    agregar_int_a_paquete(paquete,tam);
    enviar_paquete(paquete,cpu_cliente_memoria);
    eliminar_paquete(paquete);
    int ok = confirmacion_resize();
    return ok;
}

void mov_in(char* reg_datos, char* reg_direccion){ //GRAN PARTE DE LO QUE ESTA ACA DEBERIA PODER MODULARIZARCE EN FUNCIONES DENTRO DE LA MMU
    int tamano_datos = 0;
    void* registro_datos = apuntar_a_registro(reg_datos, &tamano_datos);

    tamano_datos = tamano_datos / 8;

    int tamano_direccion = 0;
    void* registro_direccion = apuntar_a_registro(reg_direccion, &tamano_direccion);

    direccion_fisica* dir_fisica;

    if (tamano_direccion == 8) {
        uint8_t* registro_direccion2 = (uint8_t*)registro_direccion;
        uint8_t contenido = *registro_direccion2;
        dir_fisica = traducir_dir_logica(pid_en_ejecucion,(int)contenido);

    } else if (tamano_direccion == 32) {
        uint32_t* registro_direccion2 = (uint32_t*)registro_direccion;
        uint32_t contenido = *registro_direccion2;
        dir_fisica = traducir_dir_logica(pid_en_ejecucion,(int)contenido);
    }

    if((tam_de_pags_memoria - dir_fisica ->desplazamiento) >= tamano_datos){
        t_paquete* paquete = crear_paquete(LECTURA_CODE);
        agregar_int_a_paquete(paquete, pid_en_ejecucion);
        agregar_int_a_paquete(paquete, dir_fisica ->dir_fisica_final);
        agregar_int_a_paquete(paquete, tamano_datos);

        enviar_paquete(paquete,cpu_cliente_memoria);
        eliminar_paquete(paquete);

        void* leido = recibir_lectura();

        memcpy(registro_datos,leido,tamano_datos);

        free(leido);
    }
    else{
        float tamano_datos_sobrantes = tamano_datos - (tam_de_pags_memoria - dir_fisica ->desplazamiento);

        int cant_nuevas_pags = ceil(tamano_datos_sobrantes / tam_de_pags_memoria);

        t_paquete* paquete2 = crear_paquete(LECTURA_CODE);
        agregar_int_a_paquete(paquete2, pid_en_ejecucion);
        agregar_int_a_paquete(paquete2, dir_fisica ->dir_fisica_final);
        agregar_int_a_paquete(paquete2, (tam_de_pags_memoria - dir_fisica ->desplazamiento));

        enviar_paquete(paquete2,cpu_cliente_memoria);
        eliminar_paquete(paquete2);

        void* leido1 = recibir_lectura();

        memcpy(registro_datos,leido1,(tam_de_pags_memoria - dir_fisica ->desplazamiento));

        free(leido1);

        int nueva_pag = dir_fisica ->num_de_pag_base + 1;

        int bytes_restantes_a_copiar = tamano_datos_sobrantes;

        int desplazamiento_en_reg_datos = (tam_de_pags_memoria - dir_fisica ->desplazamiento);

        for (int i = 0; i < cant_nuevas_pags; i++){
            int nuevo_marco = solicitar_marco(pid_en_ejecucion, nueva_pag);
            nueva_pag++;

            //Como tenemos que continuar desde la anterior pagina, el desplazamiento en el nuevo marco es 0, por lo que la direccion fisica es

            int nueva_dir_fisica_final = nuevo_marco * tam_de_pags_memoria;

            if(bytes_restantes_a_copiar > tam_de_pags_memoria){
                t_paquete* paquete3 = crear_paquete(LECTURA_CODE);
                agregar_int_a_paquete(paquete3, pid_en_ejecucion);
                agregar_int_a_paquete(paquete3, nueva_dir_fisica_final);
                agregar_int_a_paquete(paquete3, tam_de_pags_memoria);

                enviar_paquete(paquete3,cpu_cliente_memoria);
                eliminar_paquete(paquete3);

                void* leido2 = recibir_lectura();

                memcpy(registro_datos + desplazamiento_en_reg_datos, leido2, tam_de_pags_memoria);

                bytes_restantes_a_copiar = bytes_restantes_a_copiar - tam_de_pags_memoria;
                desplazamiento_en_reg_datos = desplazamiento_en_reg_datos + tam_de_pags_memoria;

                free(leido2);
            }
            else{
                t_paquete* paquete4 = crear_paquete(LECTURA_CODE);
                agregar_int_a_paquete(paquete4, pid_en_ejecucion);
                agregar_int_a_paquete(paquete4, nueva_dir_fisica_final);
                agregar_int_a_paquete(paquete4, bytes_restantes_a_copiar);

                enviar_paquete(paquete4,cpu_cliente_memoria);
                eliminar_paquete(paquete4);

                void* leido3 = recibir_lectura();

                memcpy(registro_datos + desplazamiento_en_reg_datos, leido3, bytes_restantes_a_copiar);

                free(leido3);
            }
        }

    }
    
    
    if(tamano_datos == 1){
        uint8_t* registro_datos2 = (uint8_t*)registro_datos;
        log_info(logger_obligatorio, "PID: %d- Acción: LEER - Dirección Fisica: %d - Valor: %d", pid_en_ejecucion, dir_fisica ->dir_fisica_final, *registro_datos2);
    }
    else if(tamano_datos == 4){
        uint32_t* registro_datos2 = (uint32_t*)registro_datos;
        log_info(logger_obligatorio, "PID: %d- Acción: LEER - Dirección Fisica: %d - Valor: %d", pid_en_ejecucion, dir_fisica ->dir_fisica_final, *registro_datos2);
    }
    

    free(dir_fisica);
    
}

bool mov_out(char* reg_direccion, char* reg_datos){
    int tamano_datos = 0;
    void* registro_datos = apuntar_a_registro(reg_datos, &tamano_datos);

    tamano_datos = tamano_datos / 8;

    int tamano_direccion = 0;
    void* registro_direccion = apuntar_a_registro(reg_direccion, &tamano_direccion);

    direccion_fisica* dir_fisica;

    if (tamano_direccion == 8) {
        uint8_t* registro_direccion2 = (uint8_t*)registro_direccion;
        uint8_t contenido = *registro_direccion2;
        dir_fisica = traducir_dir_logica(pid_en_ejecucion,(int)contenido);

    } else if (tamano_direccion == 32) {
        uint32_t* registro_direccion2 = (uint32_t*)registro_direccion;
        uint32_t contenido = *registro_direccion2;
        dir_fisica = traducir_dir_logica(pid_en_ejecucion,(int)contenido);
    }

    if((tam_de_pags_memoria - dir_fisica ->desplazamiento) >= tamano_datos){
        t_paquete* paquete = crear_paquete(ESCRITURA_CODE);
        agregar_int_a_paquete(paquete, pid_en_ejecucion);
        agregar_int_a_paquete(paquete, dir_fisica ->dir_fisica_final);
        agregar_int_a_paquete(paquete, tamano_datos);
        agregar_a_paquete(paquete, registro_datos, tamano_datos);

        enviar_paquete(paquete,cpu_cliente_memoria);
        eliminar_paquete(paquete);

        bool confirm = confirmacion_escritura();

        if(!confirm){
            free(dir_fisica);
            return false;
        }

    }
    else{
        float tamano_datos_sobrantes = tamano_datos - (tam_de_pags_memoria - dir_fisica ->desplazamiento);

        int cant_nuevas_pags = ceil(tamano_datos_sobrantes / tam_de_pags_memoria);

        t_paquete* paquete2 = crear_paquete(ESCRITURA_CODE);
        agregar_int_a_paquete(paquete2, pid_en_ejecucion);
        agregar_int_a_paquete(paquete2, dir_fisica ->dir_fisica_final);
        agregar_int_a_paquete(paquete2, (tam_de_pags_memoria - dir_fisica ->desplazamiento));

        void* fraccion1_reg_datos = malloc(tam_de_pags_memoria - dir_fisica ->desplazamiento);

        memcpy(fraccion1_reg_datos,registro_datos, (tam_de_pags_memoria - dir_fisica ->desplazamiento));

        agregar_a_paquete(paquete2,fraccion1_reg_datos,(tam_de_pags_memoria - dir_fisica ->desplazamiento));


        enviar_paquete(paquete2,cpu_cliente_memoria);
        eliminar_paquete(paquete2);

        bool confirm = confirmacion_escritura();

        if(!confirm){
            free(dir_fisica);
            free(fraccion1_reg_datos);
            return false;
        }

        free(fraccion1_reg_datos);


        int nueva_pag = dir_fisica ->num_de_pag_base + 1;

        int bytes_restantes_a_copiar = tamano_datos_sobrantes;

        int desplazamiento_en_reg_datos = (tam_de_pags_memoria - dir_fisica ->desplazamiento);

        for (int i = 0; i < cant_nuevas_pags; i++){
            int nuevo_marco = solicitar_marco(pid_en_ejecucion, nueva_pag);
            nueva_pag++;

            //Como tenemos que continuar desde la anterior pagina, el desplazamiento en el nuevo marco es 0, por lo que la direccion fisica es

            int nueva_dir_fisica_final = nuevo_marco * tam_de_pags_memoria;

            if(bytes_restantes_a_copiar > tam_de_pags_memoria){
                t_paquete* paquete3 = crear_paquete(ESCRITURA_CODE);
                agregar_int_a_paquete(paquete3, pid_en_ejecucion);
                agregar_int_a_paquete(paquete3, nueva_dir_fisica_final);
                agregar_int_a_paquete(paquete3, tam_de_pags_memoria);

                void* fraccionx_reg_datos = malloc(tam_de_pags_memoria);

                memcpy(fraccionx_reg_datos,registro_datos + desplazamiento_en_reg_datos, tam_de_pags_memoria);

                agregar_a_paquete(paquete3,fraccionx_reg_datos,tam_de_pags_memoria);


                enviar_paquete(paquete3,cpu_cliente_memoria);
                eliminar_paquete(paquete3);

                bool confirm = confirmacion_escritura();

                if(!confirm){
                    free(dir_fisica);
                    free(fraccionx_reg_datos);
                    return false;
                }

                free(fraccionx_reg_datos);

                bytes_restantes_a_copiar = bytes_restantes_a_copiar - tam_de_pags_memoria;
                desplazamiento_en_reg_datos = desplazamiento_en_reg_datos + tam_de_pags_memoria;
            }
            else{
                t_paquete* paquete4 = crear_paquete(ESCRITURA_CODE);
                agregar_int_a_paquete(paquete4, pid_en_ejecucion);
                agregar_int_a_paquete(paquete4, nueva_dir_fisica_final);
                agregar_int_a_paquete(paquete4, bytes_restantes_a_copiar);

                void* fraccionX_reg_datos = malloc(bytes_restantes_a_copiar);

                memcpy(fraccionX_reg_datos,registro_datos + desplazamiento_en_reg_datos, bytes_restantes_a_copiar);

                agregar_a_paquete(paquete4,fraccionX_reg_datos, bytes_restantes_a_copiar);


                enviar_paquete(paquete4,cpu_cliente_memoria);
                eliminar_paquete(paquete4);


                bool confirm = confirmacion_escritura();

                if(!confirm){
                    free(dir_fisica);
                    free(fraccionX_reg_datos);
                    return false;
                }

                free(fraccionX_reg_datos);

                
            }
        }

    }

    
    if(tamano_datos == 1){
        uint8_t* registro_datos2 = (uint8_t*)registro_datos;
        log_info(logger_obligatorio, "PID: %d- Acción: ESCRIBIR - Dirección Fisica: %d - Valor: %d", pid_en_ejecucion, dir_fisica ->dir_fisica_final, *registro_datos2);
    }
    else if(tamano_datos == 4){
        uint32_t* registro_datos2 = (uint32_t*)registro_datos;
        log_info(logger_obligatorio, "PID: %d- Acción: ESCRIBIR - Dirección Fisica: %d - Valor: %d", pid_en_ejecucion, dir_fisica ->dir_fisica_final, *registro_datos2);
    }


    free(dir_fisica);    

   return true;
}

bool copy_string(int tamanio) {
    char* contenido = malloc(tamanio);
    if (contenido == NULL) {
        log_error(logger_obligatorio, "Error al asignar memoria para contenido.");
        return false;
    }

    uint32_t registroSI_int = *los_registros_de_la_cpu->SI;
    direccion_fisica* dir_fisica_a_leer = traducir_dir_logica(pid_en_ejecucion, (int)registroSI_int);

    if ((tam_de_pags_memoria - dir_fisica_a_leer->desplazamiento) >= tamanio) {
        t_paquete* paquete = crear_paquete(LECTURA_CODE);
        agregar_int_a_paquete(paquete, pid_en_ejecucion);
        agregar_int_a_paquete(paquete, dir_fisica_a_leer->dir_fisica_final);
        agregar_int_a_paquete(paquete, tamanio);

        enviar_paquete(paquete, cpu_cliente_memoria);
        eliminar_paquete(paquete);

        void* leido = recibir_lectura();
        memcpy(contenido, leido, tamanio);
        char* para_log = strdup(contenido);
        string_append(&para_log,"\0");
        log_info(logger_obligatorio, "PID: %d- Acción: LEER - Dirección Física: %d - Valor: %s", pid_en_ejecucion, dir_fisica_a_leer->dir_fisica_final, para_log);
        free(para_log);
        free(leido);
    }
    else{
        float tamano_datos_sobrantes = tamanio - (tam_de_pags_memoria - dir_fisica_a_leer ->desplazamiento);

        int cant_nuevas_pags = ceil(tamano_datos_sobrantes / tam_de_pags_memoria);

        t_paquete* paquete2 = crear_paquete(LECTURA_CODE);
        agregar_int_a_paquete(paquete2, pid_en_ejecucion);
        agregar_int_a_paquete(paquete2, dir_fisica_a_leer ->dir_fisica_final);
        agregar_int_a_paquete(paquete2, (tam_de_pags_memoria - dir_fisica_a_leer ->desplazamiento));

        enviar_paquete(paquete2,cpu_cliente_memoria);
        eliminar_paquete(paquete2);

        void* leido1 = recibir_lectura();

        memcpy(contenido,leido1,(tam_de_pags_memoria - dir_fisica_a_leer ->desplazamiento));

        char* para_log = strdup((char*)leido1);
        string_append(&para_log,"\0");
        log_info(logger_obligatorio, "PID: %d- Acción: LEER - Dirección Física: %d - Valor: %s", pid_en_ejecucion, dir_fisica_a_leer->dir_fisica_final, para_log);
        free(para_log);

        free(leido1);

        int nueva_pag = dir_fisica_a_leer ->num_de_pag_base + 1;

        int bytes_restantes_a_copiar = tamano_datos_sobrantes;

        int desplazamiento_en_reg_datos = (tam_de_pags_memoria - dir_fisica_a_leer ->desplazamiento);

        for (int i = 0; i < cant_nuevas_pags; i++){
            int nuevo_marco = solicitar_marco(pid_en_ejecucion, nueva_pag);
            nueva_pag++;

            //Como tenemos que continuar desde la anterior pagina, el desplazamiento en el nuevo marco es 0, por lo que la direccion fisica es

            int nueva_dir_fisica_final = nuevo_marco * tam_de_pags_memoria;

            if(bytes_restantes_a_copiar > tam_de_pags_memoria){
                t_paquete* paquete3 = crear_paquete(LECTURA_CODE);
                agregar_int_a_paquete(paquete3, pid_en_ejecucion);
                agregar_int_a_paquete(paquete3, nueva_dir_fisica_final);
                agregar_int_a_paquete(paquete3, tam_de_pags_memoria);

                enviar_paquete(paquete3,cpu_cliente_memoria);
                eliminar_paquete(paquete3);

                void* leido2 = recibir_lectura();

                memcpy(contenido + desplazamiento_en_reg_datos, leido2, tam_de_pags_memoria);

                char* para_log = strdup((char*)leido2);
                string_append(&para_log,"\0");
                log_info(logger_obligatorio, "PID: %d- Acción: LEER - Dirección Física: %d - Valor: %s", pid_en_ejecucion, nueva_dir_fisica_final, para_log);
                free(para_log);

                bytes_restantes_a_copiar = bytes_restantes_a_copiar - tam_de_pags_memoria;
                desplazamiento_en_reg_datos = desplazamiento_en_reg_datos + tam_de_pags_memoria;

                free(leido2);
            }
            else{
                t_paquete* paquete4 = crear_paquete(LECTURA_CODE);
                agregar_int_a_paquete(paquete4, pid_en_ejecucion);
                agregar_int_a_paquete(paquete4, nueva_dir_fisica_final);
                agregar_int_a_paquete(paquete4, bytes_restantes_a_copiar);

                enviar_paquete(paquete4,cpu_cliente_memoria);
                eliminar_paquete(paquete4);

                void* leido3 = recibir_lectura();

                memcpy(contenido + desplazamiento_en_reg_datos, leido3, bytes_restantes_a_copiar);

                char* para_log = strdup((char*)leido3);
                string_append(&para_log,"\0");
                log_info(logger_obligatorio, "PID: %d- Acción: LEER - Dirección Física: %d - Valor: %s", pid_en_ejecucion, nueva_dir_fisica_final, para_log);
                free(para_log);

                free(leido3);
            }
        }
    }

    free(dir_fisica_a_leer);

    uint32_t registro_direccion2 = *los_registros_de_la_cpu->DI;
    direccion_fisica* dir_fisica = traducir_dir_logica(pid_en_ejecucion, (int)registro_direccion2);

    if ((tam_de_pags_memoria - dir_fisica->desplazamiento) >= tamanio) {
        t_paquete* paquete = crear_paquete(ESCRITURA_CODE);
        agregar_int_a_paquete(paquete, pid_en_ejecucion);
        agregar_int_a_paquete(paquete, dir_fisica->dir_fisica_final);
        agregar_int_a_paquete(paquete, tamanio);
        agregar_a_paquete(paquete, contenido, tamanio);

        enviar_paquete(paquete, cpu_cliente_memoria);
        eliminar_paquete(paquete);

        bool confirm = confirmacion_escritura();
        if (!confirm) {
            free(dir_fisica);
            free(contenido);
            return false;
        }

        char* para_log = strdup(contenido);
        string_append(&para_log,"\0");
        log_info(logger_obligatorio, "PID: %d- Acción: ESCRIBIR - Dirección Física: %d - Valor: %s", pid_en_ejecucion, dir_fisica->dir_fisica_final, para_log);
        free(para_log);

    } else {
        float tamano_datos_sobrantes = tamanio - (tam_de_pags_memoria - dir_fisica ->desplazamiento);

        int cant_nuevas_pags = ceil(tamano_datos_sobrantes / tam_de_pags_memoria);

        t_paquete* paquete2 = crear_paquete(ESCRITURA_CODE);
        agregar_int_a_paquete(paquete2, pid_en_ejecucion);
        agregar_int_a_paquete(paquete2, dir_fisica ->dir_fisica_final);
        agregar_int_a_paquete(paquete2, (tam_de_pags_memoria - dir_fisica ->desplazamiento));

        void* fraccion1_reg_datos = malloc(tam_de_pags_memoria - dir_fisica ->desplazamiento);

        memcpy(fraccion1_reg_datos,contenido, (tam_de_pags_memoria - dir_fisica ->desplazamiento));

        agregar_a_paquete(paquete2,fraccion1_reg_datos,(tam_de_pags_memoria - dir_fisica ->desplazamiento));


        enviar_paquete(paquete2,cpu_cliente_memoria);
        eliminar_paquete(paquete2);

        bool confirm = confirmacion_escritura();

        if(!confirm){
            free(contenido);
            free(dir_fisica);
            free(fraccion1_reg_datos);
            return false;
        }

        char* para_log = strdup((char*)fraccion1_reg_datos);
        string_append(&para_log,"\0");
        log_info(logger_obligatorio, "PID: %d- Acción: ESCRIBIR - Dirección Física: %d - Valor: %s", pid_en_ejecucion, dir_fisica->dir_fisica_final, para_log);
        free(para_log);

        free(fraccion1_reg_datos);

        int nueva_pag = dir_fisica ->num_de_pag_base + 1;

        int bytes_restantes_a_copiar = tamano_datos_sobrantes;

        int desplazamiento_en_reg_datos = (tam_de_pags_memoria - dir_fisica ->desplazamiento);

        for (int i = 0; i < cant_nuevas_pags; i++){
            int nuevo_marco = solicitar_marco(pid_en_ejecucion, nueva_pag);
            nueva_pag++;

            //Como tenemos que continuar desde la anterior pagina, el desplazamiento en el nuevo marco es 0, por lo que la direccion fisica es

            int nueva_dir_fisica_final = nuevo_marco * tam_de_pags_memoria;

            if(bytes_restantes_a_copiar > tam_de_pags_memoria){
                t_paquete* paquete3 = crear_paquete(ESCRITURA_CODE);
                agregar_int_a_paquete(paquete3, pid_en_ejecucion);
                agregar_int_a_paquete(paquete3, nueva_dir_fisica_final);
                agregar_int_a_paquete(paquete3, tam_de_pags_memoria);

                void* fraccionx_reg_datos = malloc(tam_de_pags_memoria);

                memcpy(fraccionx_reg_datos,contenido + desplazamiento_en_reg_datos, tam_de_pags_memoria);

                agregar_a_paquete(paquete3,fraccionx_reg_datos,tam_de_pags_memoria);


                enviar_paquete(paquete3,cpu_cliente_memoria);
                eliminar_paquete(paquete3);

                bool confirm = confirmacion_escritura();

                if(!confirm){
                    free(contenido);
                    free(dir_fisica);
                    free(fraccionx_reg_datos);
                    return false;
                }

                char* para_log = strdup((char*)fraccionx_reg_datos);
                string_append(&para_log,"\0");
                log_info(logger_obligatorio, "PID: %d- Acción: ESCRIBIR - Dirección Física: %d - Valor: %s", pid_en_ejecucion, nueva_dir_fisica_final, para_log);
                free(para_log);

                free(fraccionx_reg_datos);

                bytes_restantes_a_copiar = bytes_restantes_a_copiar - tam_de_pags_memoria;
                desplazamiento_en_reg_datos = desplazamiento_en_reg_datos + tam_de_pags_memoria;
            }
            else{
                t_paquete* paquete4 = crear_paquete(ESCRITURA_CODE);
                agregar_int_a_paquete(paquete4, pid_en_ejecucion);
                agregar_int_a_paquete(paquete4, nueva_dir_fisica_final);
                agregar_int_a_paquete(paquete4, bytes_restantes_a_copiar);

                void* fraccionX_reg_datos = malloc(bytes_restantes_a_copiar);

                memcpy(fraccionX_reg_datos,contenido + desplazamiento_en_reg_datos, bytes_restantes_a_copiar);

                agregar_a_paquete(paquete4,fraccionX_reg_datos, bytes_restantes_a_copiar);


                enviar_paquete(paquete4,cpu_cliente_memoria);
                eliminar_paquete(paquete4);


                bool confirm = confirmacion_escritura();

                if(!confirm){
                    free(contenido);
                    free(dir_fisica);
                    free(fraccionX_reg_datos);
                    return false;
                }

                char* para_log = strdup((char*)fraccionX_reg_datos);
                string_append(&para_log,"\0");
                log_info(logger_obligatorio, "PID: %d- Acción: ESCRIBIR - Dirección Física: %d - Valor: %s", pid_en_ejecucion, nueva_dir_fisica_final, para_log);
                free(para_log);

                free(fraccionX_reg_datos);

                
            }
        }
    }

    free(dir_fisica);

    free(contenido);
    return true;
}

void std_read_write(char* interfaz, char* registro_direccion, char* registro_tam, char* tipo_interfaz){
    io_std_fs* conjunto_dir_fisicas = io_std_fs_get_dir_fis(interfaz, registro_direccion, registro_tam);

    t_paquete* paquete6 = crear_paquete(STD_READ_WRITE_CODE);
    cargar_registros_a_paquete(paquete6);
    agregar_string_a_paquete(paquete6, tipo_interfaz);
    agregar_string_a_paquete(paquete6, conjunto_dir_fisicas ->interfaz);
    agregar_int_a_paquete(paquete6, conjunto_dir_fisicas ->tam);
    agregar_int_a_paquete(paquete6, conjunto_dir_fisicas ->cant_dir_fisicas);

    for(int i = 0; i < conjunto_dir_fisicas ->cant_dir_fisicas; i++){
        dir_fis_y_tam*  dir_fisica_y_tam = list_get(conjunto_dir_fisicas ->lista_dir_fisicas, i);
        agregar_int_a_paquete(paquete6, dir_fisica_y_tam ->dir_fisica);
        agregar_int_a_paquete(paquete6, dir_fisica_y_tam ->tam);
    }

    enviar_paquete(paquete6, kernel_cliente_dispatch);
    eliminar_paquete(paquete6);

    list_destroy_and_destroy_elements(conjunto_dir_fisicas ->lista_dir_fisicas, free);
    free(conjunto_dir_fisicas ->interfaz);
    free(conjunto_dir_fisicas);
}

io_std_fs* io_std_fs_get_dir_fis(char* interfaz, char* registro_direccion, char* registro_tam){
    io_std_fs* nueva_peticion = malloc(sizeof(io_std_fs));

    nueva_peticion ->interfaz = strdup(interfaz);
    string_append(&nueva_peticion ->interfaz, "\n");

    nueva_peticion ->lista_dir_fisicas = list_create();

    int tamano_reg_tam;

    void* reg_tamano = apuntar_a_registro(registro_tam, &tamano_reg_tam);

    if(tamano_reg_tam == 8){
        uint8_t* reg_tamano2 = (uint8_t* )reg_tamano;
        uint8_t contenido_reg = *reg_tamano2;

        nueva_peticion ->tam = (int)contenido_reg;
    }
    else if(tamano_reg_tam == 32){
        uint32_t* reg_tamano2 = (uint32_t* )reg_tamano;
        uint32_t contenido_reg = *reg_tamano2;

        nueva_peticion ->tam = (int)contenido_reg;
    }

    direccion_fisica* dir_fisica;

    int tamano_reg_dir;

    void* reg_direccion = apuntar_a_registro(registro_direccion, &tamano_reg_dir);

    if (tamano_reg_dir == 8) {
        uint8_t* registro_direccion2 = (uint8_t*)reg_direccion;
        uint8_t contenido = *registro_direccion2;
        dir_fisica = traducir_dir_logica(pid_en_ejecucion,(int)contenido);

    } else if (tamano_reg_dir == 32) {
        uint32_t* registro_direccion2 = (uint32_t*)reg_direccion;
        uint32_t contenido = *registro_direccion2;
        dir_fisica = traducir_dir_logica(pid_en_ejecucion,(int)contenido);
    }

    if((tam_de_pags_memoria - dir_fisica ->desplazamiento) >= nueva_peticion ->tam){
        nueva_peticion ->cant_dir_fisicas = 1;

        dir_fis_y_tam* direccion_y_tam_0 = malloc(sizeof(dir_fis_y_tam));

        direccion_y_tam_0 ->dir_fisica = dir_fisica ->dir_fisica_final;

        direccion_y_tam_0 ->tam = nueva_peticion ->tam;

        list_add(nueva_peticion ->lista_dir_fisicas,direccion_y_tam_0);
    }
    else{
        float tamano_datos_sobrantes = nueva_peticion ->tam - (tam_de_pags_memoria - dir_fisica ->desplazamiento);

        dir_fis_y_tam* direccion_y_tam_1 = malloc(sizeof(dir_fis_y_tam));

        direccion_y_tam_1 ->dir_fisica = dir_fisica ->dir_fisica_final;

        direccion_y_tam_1 ->tam = (tam_de_pags_memoria - dir_fisica ->desplazamiento);

        list_add(nueva_peticion ->lista_dir_fisicas,direccion_y_tam_1);

        nueva_peticion ->cant_dir_fisicas = 1;

        int cant_nuevas_pags = ceil(tamano_datos_sobrantes / tam_de_pags_memoria);

        int nueva_pag = dir_fisica ->num_de_pag_base + 1;

        int bytes_restantes = tamano_datos_sobrantes;

        for (int i = 0; i < cant_nuevas_pags; i++){
            int nuevo_marco = solicitar_marco(pid_en_ejecucion, nueva_pag);
            nueva_pag++;

            nueva_peticion ->cant_dir_fisicas ++;

            //Como tenemos que continuar desde la anterior pagina, el desplazamiento en el nuevo marco es 0, por lo que la direccion fisica es

            int nueva_dir_fisica_final = nuevo_marco * tam_de_pags_memoria;

            if(bytes_restantes > tam_de_pags_memoria){

                dir_fis_y_tam* direccion_y_tam_x = malloc(sizeof(dir_fis_y_tam));
                
                direccion_y_tam_x ->dir_fisica = nueva_dir_fisica_final;

                direccion_y_tam_x ->tam = tam_de_pags_memoria;



                list_add(nueva_peticion ->lista_dir_fisicas,direccion_y_tam_x);

                bytes_restantes = bytes_restantes - tam_de_pags_memoria;
            
            }
            else{
                dir_fis_y_tam* direccion_y_tam_X = malloc(sizeof(dir_fis_y_tam));

                direccion_y_tam_X ->dir_fisica = nueva_dir_fisica_final;

                direccion_y_tam_X ->tam = bytes_restantes;

                list_add(nueva_peticion ->lista_dir_fisicas, direccion_y_tam_X);
            }
        }

    }

    nueva_peticion ->cant_dir_fisicas = list_size(nueva_peticion ->lista_dir_fisicas);
    free(dir_fisica);

    return nueva_peticion;

}

void fs_create_delete(char* interfaz, char* nombre_Arch, int cod_op){
    t_paquete* paquete = crear_paquete(cod_op);
    cargar_registros_a_paquete(paquete);
    char* interfaz_nuevo = strdup(interfaz);
    string_append(&interfaz_nuevo,"\n");
    agregar_string_a_paquete(paquete,interfaz_nuevo);
    agregar_string_a_paquete(paquete,nombre_Arch);
    enviar_paquete(paquete, kernel_cliente_dispatch);
    eliminar_paquete(paquete);
    free(interfaz_nuevo);
}

void fs_truncate(char* interfaz, char* nombre_arch, char* registro_tam){
    t_paquete* paquete = crear_paquete(FS_TRUNCATE_CODE);
    cargar_registros_a_paquete(paquete);
    char* interfaz_nuevo = strdup(interfaz);
    string_append(&interfaz_nuevo,"\n");
    agregar_string_a_paquete(paquete, interfaz_nuevo);
    agregar_string_a_paquete(paquete, nombre_arch);

    int tam_registro;
    int tam_a_pasar;
    void* registro = apuntar_a_registro(registro_tam, &tam_registro);

    if(tam_registro == 8){
        uint8_t* registro2 = registro;
        tam_a_pasar = (int)(*registro2);
    }
    else if(tam_registro == 32){
        uint32_t* registro2 = registro;
        tam_a_pasar = (int)(*registro2);
    }
    agregar_int_a_paquete(paquete, tam_a_pasar);

    enviar_paquete(paquete, kernel_cliente_dispatch);
    eliminar_paquete(paquete);

    free(interfaz_nuevo);

}

void fs_read_write(char* interfaz,char* nombre_arch, char* registro_direccion, char* registro_tam, char* registro_puntero, char* lectura_o_escritura){
    io_std_fs* conjunto_dir_fisicas = io_std_fs_get_dir_fis(interfaz, registro_direccion, registro_tam);

    t_paquete* paquete6 = crear_paquete(FS_READ_WRITE_CODE);
    cargar_registros_a_paquete(paquete6);

    agregar_string_a_paquete(paquete6,nombre_arch);

    int tam_registro;
    int puntero;
    void* registro_punt = apuntar_a_registro(registro_puntero, &tam_registro);

    if(tam_registro == 8){
        uint8_t* registro2 = registro_punt;
        puntero = (int)(*registro2);
    }
    else if(tam_registro == 32){
        uint8_t* registro2 = registro_punt;
        puntero = (int)(*registro2);
    }
    agregar_int_a_paquete(paquete6, puntero);

    agregar_string_a_paquete(paquete6, lectura_o_escritura);
    
    agregar_string_a_paquete(paquete6, conjunto_dir_fisicas ->interfaz);
    agregar_int_a_paquete(paquete6, conjunto_dir_fisicas ->tam);
    agregar_int_a_paquete(paquete6, conjunto_dir_fisicas ->cant_dir_fisicas);

    for(int i = 0; i < conjunto_dir_fisicas ->cant_dir_fisicas; i++){
        dir_fis_y_tam*  dir_fisica_y_tam = list_get(conjunto_dir_fisicas ->lista_dir_fisicas, i);
        agregar_int_a_paquete(paquete6, dir_fisica_y_tam ->dir_fisica);
        agregar_int_a_paquete(paquete6, dir_fisica_y_tam ->tam);
    }

    enviar_paquete(paquete6, kernel_cliente_dispatch);
    eliminar_paquete(paquete6);

    list_destroy_and_destroy_elements(conjunto_dir_fisicas ->lista_dir_fisicas, free);
    free(conjunto_dir_fisicas ->interfaz);
    free(conjunto_dir_fisicas);
}

void* apuntar_a_registro (char* regist, int* puntero_a_tamano) {
    if (!strcmp(regist, "PC") || !strcmp(regist, "PC\n")) {
        *puntero_a_tamano = 32;
        return los_registros_de_la_cpu->PC;
        
    } else if (!strcmp(regist, "AX") || !strcmp(regist, "AX\n")) {
        *puntero_a_tamano = 8;
        return los_registros_de_la_cpu->AX;
    } else if (!strcmp(regist, "BX") || !strcmp(regist, "BX\n")) {
        *puntero_a_tamano = 8;
        return los_registros_de_la_cpu->BX;
    } else if (!strcmp(regist, "CX") || !strcmp(regist, "CX\n")) {
        *puntero_a_tamano = 8;
        return los_registros_de_la_cpu->CX;
    } else if (!strcmp(regist, "DX") || !strcmp(regist, "DX\n")) {
        *puntero_a_tamano = 8;
        return los_registros_de_la_cpu->DX;
    } else if (!strcmp(regist, "EAX") || !strcmp(regist, "EAX\n")) {
        *puntero_a_tamano = 32;
        return los_registros_de_la_cpu->EAX;
    } else if (!strcmp(regist, "EBX") || !strcmp(regist, "EBX\n")) {
        *puntero_a_tamano = 32;
        return los_registros_de_la_cpu->EBX;
    } else if (!strcmp(regist, "ECX") || !strcmp(regist, "ECX\n")) {
        *puntero_a_tamano = 32;
        return los_registros_de_la_cpu->ECX;
    } else if (!strcmp(regist, "EDX") || !strcmp(regist, "EDX\n")) {
        *puntero_a_tamano = 32;
        return los_registros_de_la_cpu->EDX;
    } else if (!strcmp(regist, "SI") || !strcmp(regist, "SI\n")) {
        *puntero_a_tamano = 32;
        return los_registros_de_la_cpu->SI;
    } else if (!strcmp(regist, "DI") || !strcmp(regist, "DI\n")) {
        *puntero_a_tamano = 32;
        return los_registros_de_la_cpu->DI;
    } else {
        log_error(logger, "Error al identificar");
        return NULL;
    }

    return NULL;
}