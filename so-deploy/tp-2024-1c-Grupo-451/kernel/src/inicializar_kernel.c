#include <inicializar_kernel.h>

void inicializar_kernel(char* path_config){
    inicializar_config_kernel(path_config);

    pid_acumulado = 0;
    // pid_a_eliminar = -1;
    espera_grado_multi = -1;
    // cantidad_de_proceso_en_ejecucion = GRADO_MULTIPROGRAMACION;
    permitir_planificacion = true;

    logger = iniciar_logger("./kernel.log", "Kernel_Logger", LOG_LEVEL_INFO, 1);
    logger_obligatorio = iniciar_logger("./kernelObligatorio.log", "Kernel_Logger_Obligatorio", LOG_LEVEL_INFO, 0);
    cola_new = queue_create();
    cola_ready = queue_create();
    diccionario_blocked = dictionary_create();
    cola_exit = queue_create();
    cola_ready_prioritaria = queue_create();

    diccionario_entrada_salida = dictionary_create();
    diccionario_recursos = dictionary_create();

    diccionario_de_todos_los_procesos = dictionary_create();

    //Semaforos
    sem_init(&hay_proceso_en_ready,0,0);
    sem_init(&hay_proceso_en_new,0,0);
    sem_init(&hay_proceso_en_exit,0,0);
    sem_init(&multiprogramacion_permite_proceso_en_ready,0,GRADO_MULTIPROGRAMACION);
    
    sem_init(&detener_planificacion_corto_plazo,0,0);
    sem_init(&detener_planificacion_exit,0,0);
    sem_init(&detener_planificacion_salida_cpu,0,0);
    sem_init(&detener_planificacion_to_ready,0,0);

    pthread_mutex_init(&mutex_cola_new, NULL);
    pthread_mutex_init(&mutex_cola_ready, NULL);
    pthread_mutex_init(&mutex_cola_exit, NULL);
    pthread_mutex_init(&mutex_cola_prioritaria, NULL);
    pthread_mutex_init(&mutex_para_proceso_en_ejecucion, NULL);
    pthread_mutex_init(&mutex_para_creacion_proceso, NULL);
    pthread_mutex_init(&mutex_para_diccionario_entradasalida, NULL);
    pthread_mutex_init(&mutex_para_diccionario_recursos, NULL);
    pthread_mutex_init(&mutex_para_diccionario_blocked, NULL);
    pthread_mutex_init(&mutex_para_eliminar_entradasalida, NULL);
    pthread_mutex_init(&mutex_para_diccionario_de_todos_los_procesos, NULL);

    inicializar_recursos();
    
}

void inicializar_config_kernel(char* path_config){
    char** string_sin_contra_barra = string_split(path_config,"\n");
    char* path_configs = strdup("./");
    string_append(&path_configs, string_sin_contra_barra[0]);
    config = iniciar_config(path_configs);

    string_array_destroy(string_sin_contra_barra);
    free(path_configs);

    if(config == NULL){
        log_error(logger, "CONFIG INEXISTENTE");
        exit(EXIT_FAILURE);
    }

    PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");
    IP_MEMORIA = config_get_string_value(config,"IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
    IP_CPU = config_get_string_value(config,"IP_CPU");
    PUERTO_CPU_DISPATCH = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    PUERTO_CPU_INTERRUPT = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
    PATH_SCRIPTS = config_get_string_value(config, "PATH_SCRIPTS");
    ALGORITMO_PLANIFICACION = config_get_string_value(config,"ALGORITMO_PLANIFICACION");
    QUANTUM = config_get_int_value(config, "QUANTUM");
    RECURSOS = config_get_array_value(config,"RECURSOS");
    INSTANCIAS_RECURSOS = config_get_array_value(config,"INSTANCIAS_RECURSOS");
    GRADO_MULTIPROGRAMACION = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
}

void inicializar_recursos(){
    if(!string_array_is_empty(RECURSOS)){
        for(int i = 0; i< string_array_size(RECURSOS); i++){
            nodo_recursos* nodo = malloc(sizeof(nodo_recursos));
            nodo ->cola_bloqueados_recurso = queue_create();
            nodo ->instancias = atoi(INSTANCIAS_RECURSOS[i]);
            string_append(&RECURSOS[i],"\n");
            pthread_mutex_init(&(nodo ->mutex_del_recurso), NULL);
            pthread_mutex_lock(&mutex_para_diccionario_recursos);
            dictionary_put(diccionario_recursos,RECURSOS[i], nodo);
            pthread_mutex_unlock(&mutex_para_diccionario_recursos);
            
        }
    }
}
