#include <ciclo_cpu.h>
#include <atender_mensajes.h>

void ciclo(){
        
    atender_kernel_dispatch_sin_while();
    bool correr_ciclo = true;
    int resultado_ejecucion;
    while(correr_ciclo){
        solicitar_instruccion(*los_registros_de_la_cpu -> PC);
        int cod_instruccion = decodificar_instruccion();
        resultado_ejecucion = ejecutar_instruccion(cod_instruccion);
        uint32_t programCounter = *los_registros_de_la_cpu -> PC;
        programCounter++;
        *los_registros_de_la_cpu -> PC = programCounter;
        if(resultado_ejecucion != SEGUIR_EJECUTANDO){
            correr_ciclo = false;

            pthread_mutex_lock(&mutex_para_interrupcion);
            interrupcion_recibida = NO_INTERRUPCION;
            pthread_mutex_unlock(&mutex_para_interrupcion);
        }
        else if(interrupcion_recibida != NO_INTERRUPCION && pid_en_ejecucion == pid_de_interrupcion){
            correr_ciclo = false;
        }
        else{
            string_array_destroy(instruccion_separada);
        }
    }
    if(resultado_ejecucion != SEGUIR_EJECUTANDO){
        switch (resultado_ejecucion){
            case FINALIZAR:
                t_paquete* paquete = crear_paquete(FINALIZAR_EXEC);
                cargar_registros_a_paquete(paquete);
                enviar_paquete(paquete,kernel_cliente_dispatch);
                eliminar_paquete(paquete);

                break;
            case SLEEP_GEN:
                t_paquete* paquete2 = crear_paquete(ESPERAR_GEN);
                cargar_registros_a_paquete(paquete2);
                string_append(&instruccion_separada[1],"\n");
                agregar_string_a_paquete(paquete2,instruccion_separada[1]);
                int tiempo_espera = atoi(instruccion_separada[2]);
                agregar_int_a_paquete(paquete2,tiempo_espera);
                enviar_paquete(paquete2,kernel_cliente_dispatch);
                eliminar_paquete(paquete2);

                break;
            case STD_READ:
                std_read_write(instruccion_separada[1], instruccion_separada[2], instruccion_separada[3], "stdin");

                break;
            case STD_WRITE:
                std_read_write(instruccion_separada[1], instruccion_separada[2], instruccion_separada[3], "stdout");

                break;
            case FS_CREATE:
                fs_create_delete(instruccion_separada[1], instruccion_separada[2], FS_CREATE_CODE);

                break;
            case FS_DELETE:
                fs_create_delete(instruccion_separada[1], instruccion_separada[2], FS_DELETE_CODE);

                break;
            case FS_TRUNCATE:
                fs_truncate(instruccion_separada[1], instruccion_separada[2], instruccion_separada[3]);

                break;
            case FS_READ:
                fs_read_write(instruccion_separada[1], instruccion_separada[2], instruccion_separada[3], instruccion_separada[4], instruccion_separada[5], "lectura");

                break;
            case FS_WRITE:
                fs_read_write(instruccion_separada[1], instruccion_separada[2], instruccion_separada[3], instruccion_separada[4], instruccion_separada[5], "escritura");

                break;
            case WAIT_RECURSO:
                t_paquete* paquete3 = crear_paquete(WAIT_CODE);
                cargar_registros_a_paquete(paquete3);
                agregar_string_a_paquete(paquete3,instruccion_separada[1]);
                enviar_paquete(paquete3,kernel_cliente_dispatch);
                eliminar_paquete(paquete3);
                break;
            case SIGNAL_RECURSO:
                t_paquete* paquete4 = crear_paquete(SIGNAL_CODE);
                cargar_registros_a_paquete(paquete4);
                agregar_string_a_paquete(paquete4,instruccion_separada[1]);
                enviar_paquete(paquete4,kernel_cliente_dispatch);
                eliminar_paquete(paquete4);
                break;
            case SIN_MEMORIA:
                t_paquete* paquete5 = crear_paquete(OUT_OF_MEM_CODE);
                cargar_registros_a_paquete(paquete5);
                enviar_paquete(paquete5,kernel_cliente_dispatch);
                eliminar_paquete(paquete5);
                break;
            default:
                break;
        }
    }
    else{
        t_paquete* paquete = crear_paquete(INTERRUPCION);
        cargar_registros_a_paquete(paquete);
        enviar_paquete(paquete,kernel_cliente_dispatch);
        eliminar_paquete(paquete);
            
        pthread_mutex_lock(&mutex_para_interrupcion);
        interrupcion_recibida = NO_INTERRUPCION;
        pthread_mutex_unlock(&mutex_para_interrupcion);
        
        
    }
    
    string_array_destroy(instruccion_separada);
}

void solicitar_instruccion(int programCounter){
    t_paquete* paquete = crear_paquete(PEDIR_INSTRUCCION);
    agregar_int_a_paquete(paquete,pid_en_ejecucion);
    agregar_int_a_paquete(paquete, programCounter);
    enviar_paquete(paquete, cpu_cliente_memoria);
    eliminar_paquete(paquete);
    atender_memoria_cpu_sin_while();
    log_info(logger_obligatorio, "PID: %d - FETCH Program Counter: %d", pid_en_ejecucion, programCounter);
}

int decodificar_instruccion(){
    // instruccion_a_decodificar
    instruccion_separada = string_split(instruccion_a_decodificar, " ");

    if (!strcmp(instruccion_separada[0],"SET")) {
        return SET;
        
    } else if (!strcmp(instruccion_separada[0],"MOV_IN")) {
        return MOV_IN;
        
    } else if (!strcmp(instruccion_separada[0], "MOV_OUT")) {
        return MOV_OUT;
        
    } else if (!strcmp(instruccion_separada[0], "SUM")) {
        return SUM;

    } else if (!strcmp(instruccion_separada[0], "SUB")) {
        return SUB;

    } else if (!strcmp(instruccion_separada[0], "JNZ")) {
        return JNZ;

    } else if (!strcmp(instruccion_separada[0], "RESIZE")) {
        return RESIZE;
        
    } else if (!strcmp(instruccion_separada[0], "COPY_STRING")) {
        return COPY_STRING;
        
    } else if (!strcmp(instruccion_separada[0], "WAIT")) {
        return WAIT;
        
    } else if (!strcmp(instruccion_separada[0],"SIGNAL")) {
        return SIGNAL;
        
    } else if (!strcmp(instruccion_separada[0],"IO_GEN_SLEEP")) {
        return IO_GEN_SLEEP;
           
    } else if (!strcmp(instruccion_separada[0],"IO_STDIN_READ")) {
        return IO_STDIN_READ;
        
    } else if (!strcmp(instruccion_separada[0],"IO_STDOUT_WRITE")) {
        return IO_STDOUT_WRITE;
        
    } else if (!strcmp(instruccion_separada[0],"IO_FS_CREATE")) {
        return IO_FS_CREATE;
        
    } else if (!strcmp(instruccion_separada[0],"IO_FS_DELETE")) {
        return IO_FS_DELETE;
        
    } else if (!strcmp(instruccion_separada[0],"IO_FS_TRUNCATE")) {
        return IO_FS_TRUNCATE;
        
    } else if (!strcmp(instruccion_separada[0],"IO_FS_WRITE")) {
        return IO_FS_WRITE;
        
    } else if (!strcmp(instruccion_separada[0],"IO_FS_READ")) {
        return IO_FS_READ;
        
    } else if (!strcmp(instruccion_separada[0], "EXIT") || !strcmp(instruccion_separada[0], "EXIT\n")) {
        return EXIT;
        
    } else {
        log_error(logger, "Decode: Comando no reconocido");
        return -1;
    }
}


int ejecutar_instruccion (int codigo_instruccion) {
    

    switch (codigo_instruccion)
    {
    case SET: // SET (Registro, Valor)
        int numero = atoi(instruccion_separada[2]);
        set(instruccion_separada[1], numero);
        log_info(logger_obligatorio, "PID: %d - EJECUTANDO: %s - %s %s",pid_en_ejecucion, instruccion_separada[0], instruccion_separada[1], 
        instruccion_separada[2]);
        return SEGUIR_EJECUTANDO;

    case SUM: // SUM (Registro Destino, Registro Origen)
        sum(instruccion_separada[1], instruccion_separada[2]);
        log_info(logger_obligatorio, "PID: %d - EJECUTANDO: %s - %s %s",pid_en_ejecucion, instruccion_separada[0], instruccion_separada[1], 
        instruccion_separada[2]);
        return SEGUIR_EJECUTANDO;

    case SUB: // SUB (Registro Destino, Registro Origen)
        sub(instruccion_separada[1], instruccion_separada[2]);
        log_info(logger_obligatorio, "PID: %d - EJECUTANDO: %s - %s %s",pid_en_ejecucion, instruccion_separada[0], instruccion_separada[1], 
        instruccion_separada[2]);
        return SEGUIR_EJECUTANDO;

    case JNZ: // JNZ (Registro, Instrucción)
        int nuevo_pc = atoi(instruccion_separada[2]);
        jnz(instruccion_separada[1], nuevo_pc);
        log_info(logger_obligatorio, "PID: %d - EJECUTANDO: %s - %s %s",pid_en_ejecucion, instruccion_separada[0], instruccion_separada[1], 
        instruccion_separada[2]);
        return SEGUIR_EJECUTANDO;

    case IO_GEN_SLEEP: // IO_GEN_SLEEP (Interfaz, Unidades de trabajo)
        log_info(logger_obligatorio, "PID: %d - EJECUTANDO: %s - %s %s",pid_en_ejecucion, instruccion_separada[0], instruccion_separada[1], 
        instruccion_separada[2]);
        return SLEEP_GEN;

    case WAIT: // WAIT (RECURSO)
        log_info(logger_obligatorio, "PID: %d - EJECUTANDO: %s - %s",pid_en_ejecucion, instruccion_separada[0], instruccion_separada[1]);
        return WAIT_RECURSO;

    case SIGNAL: //SIGNAL (RECURSO)
        log_info(logger_obligatorio, "PID: %d - EJECUTANDO: %s - %s",pid_en_ejecucion, instruccion_separada[0], instruccion_separada[1]);
        return SIGNAL_RECURSO;

    case RESIZE: //RESIZE (TAMAÑO)
        log_info(logger_obligatorio, "PID: %d - EJECUTANDO: %s - %s",pid_en_ejecucion, instruccion_separada[0], instruccion_separada[1]);
        int tam_a_resize = atoi(instruccion_separada[1]);
        int ok = resize(tam_a_resize);
        if(ok){
            return SEGUIR_EJECUTANDO;
        }
        else{
            return SIN_MEMORIA;
        }

    case MOV_IN: //MOV IN (Registro Datos, Registro Dirección)
        mov_in(instruccion_separada[1],instruccion_separada[2]);
        log_info(logger_obligatorio, "PID: %d - EJECUTANDO: %s - %s %s",pid_en_ejecucion, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2]);
        return SEGUIR_EJECUTANDO;

    case MOV_OUT: //MOV OUT (Registro Dirección, Registro Datos)
        bool ok2 = mov_out(instruccion_separada[1],instruccion_separada[2]);
        log_info(logger_obligatorio, "PID: %d - EJECUTANDO: %s - %s %s",pid_en_ejecucion, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2]);

        if(ok2){
            return SEGUIR_EJECUTANDO;
        }
        else{
            return SIN_MEMORIA;
        }
        
    case COPY_STRING: //COPY_STRING (Tamaño)
        int tamanio= atoi(instruccion_separada[1]);
        bool ok3 = copy_string(tamanio);
        log_info(logger_obligatorio,"PID: %d - EJECUTANDO: %s - %s",pid_en_ejecucion, instruccion_separada[0], instruccion_separada[1]);

        if(ok3){
            return SEGUIR_EJECUTANDO;
        }
        else{
            return SIN_MEMORIA;
        }
    
    case IO_STDIN_READ: //IO_STDIN_READ (Interfaz, Registro Dirección, Registro Tamaño)
        log_info(logger_obligatorio, "PID: %d - EJECUTANDO: %s - %s %s %s",pid_en_ejecucion, instruccion_separada[0], instruccion_separada[1], 
        instruccion_separada[2], instruccion_separada[3]);
        return STD_READ;

    case IO_STDOUT_WRITE: //IO_STDOUT_WRITE (Interfaz, Registro Dirección, Registro Tamaño)
        log_info(logger_obligatorio, "PID: %d - EJECUTANDO: %s - %s %s %s",pid_en_ejecucion, instruccion_separada[0], instruccion_separada[1], 
        instruccion_separada[2], instruccion_separada[3]);
        return STD_WRITE;

    case IO_FS_CREATE: //IO_FS_CREATE (Interfaz, Nombre Archivo)
        log_info(logger_obligatorio, "PID: %d - EJECUTANDO: %s - %s %s", pid_en_ejecucion, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2]);
        return FS_CREATE;
    
    case IO_FS_DELETE: //IO_FS_DELETE (Interfaz, Nombre Archivo)
        log_info(logger_obligatorio, "PID: %d - EJECUTANDO: %s - %s %s", pid_en_ejecucion, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2]);
        return FS_DELETE;

    case IO_FS_TRUNCATE: //IO_FS_TRUNCATE (Interfaz, Nombre Archivo, Registro Tamaño)
        log_info(logger_obligatorio, "PID: %d - EJECUTANDO: %s - %s %s %s", pid_en_ejecucion, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2], instruccion_separada[3]);
        return FS_TRUNCATE;

    case IO_FS_READ: //IO_FS_READ (Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)
        log_info(logger_obligatorio, "PID: %d - EJECUTANDO: %s - %s %s %s %s %s", pid_en_ejecucion, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2], instruccion_separada[3],
        instruccion_separada[4], instruccion_separada[5]);
        return FS_READ;

    case IO_FS_WRITE: //IO_FS_WRITE (Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)
        log_info(logger_obligatorio, "PID: %d - EJECUTANDO: %s - %s %s %s %s %s", pid_en_ejecucion, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2], instruccion_separada[3],
        instruccion_separada[4], instruccion_separada[5]);
        return FS_WRITE;

    case EXIT:
        log_info(logger_obligatorio, "PID: %d - EJECUTANDO: %s",pid_en_ejecucion, instruccion_separada[0]);
        return FINALIZAR;

    default:
        printf("Execute: Comando no reconocido");
        return 1;
    }

    return -1;
}


void cargar_registros_a_paquete(t_paquete* paquete){
    agregar_a_paquete(paquete,los_registros_de_la_cpu->PC,sizeof(uint32_t));
    agregar_a_paquete(paquete,los_registros_de_la_cpu->AX,sizeof(uint8_t));
    agregar_a_paquete(paquete,los_registros_de_la_cpu->BX,sizeof(uint8_t));
    agregar_a_paquete(paquete,los_registros_de_la_cpu->CX,sizeof(uint8_t));
    agregar_a_paquete(paquete,los_registros_de_la_cpu->DX,sizeof(uint8_t));
    agregar_a_paquete(paquete,los_registros_de_la_cpu->EAX,sizeof(uint32_t));
    agregar_a_paquete(paquete,los_registros_de_la_cpu->EBX,sizeof(uint32_t)); 
    agregar_a_paquete(paquete,los_registros_de_la_cpu->ECX,sizeof(uint32_t));
    agregar_a_paquete(paquete,los_registros_de_la_cpu->EDX,sizeof(uint32_t));
    agregar_a_paquete(paquete,los_registros_de_la_cpu->SI,sizeof(uint32_t)); 
    agregar_a_paquete(paquete,los_registros_de_la_cpu->DI,sizeof(uint32_t));
}