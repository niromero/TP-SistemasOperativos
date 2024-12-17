#include <inicializar_memoria.h>

void inicializar_memoria(char* path_config){
    logger = iniciar_logger("./memoria.log", "Memoria_Logger", LOG_LEVEL_DEBUG, 1);
    logger_obligatorio = iniciar_logger("./memoriaObligatorio.log", "Memoria_Logger_Obligatorio", LOG_LEVEL_INFO,1);
    inicializar_config_memoria(path_config);


    if(TAM_MEMORIA % TAM_PAGINA != 0){
		log_error(logger, "Tamanio de memoria no es multiplo de tamanio de pagina");
		exit(EXIT_FAILURE);
	}
	memoria_de_usuario = malloc(TAM_MEMORIA);

    cant_marcos_totales = TAM_MEMORIA/TAM_PAGINA;
    int cant_marcos_para_bitarray = ceil(cant_marcos_totales/8);

    puntero_a_bits_de_los_marcos = malloc(cant_marcos_para_bitarray);
    marcos_de_memoria_libres = bitarray_create_with_mode(puntero_a_bits_de_los_marcos,cant_marcos_para_bitarray,LSB_FIRST);

    for(int i = 0; i< cant_marcos_totales; i++){
        bitarray_clean_bit(marcos_de_memoria_libres,i);
    }

    diccionario_de_instrucciones = dictionary_create();
    diccionario_de_tdp = dictionary_create();

    pthread_mutex_init(&mutex_para_leer_pseudo, NULL);
    pthread_mutex_init(&mutex_para_diccionario_instrucciones, NULL);
    pthread_mutex_init(&mutex_para_diccionario_tdp, NULL);
    pthread_mutex_init(&mutex_para_marcos_libres, NULL);
    pthread_mutex_init(&mutex_para_mem_de_usuario, NULL);

}

void inicializar_config_memoria(char* path_config){
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
    TAM_MEMORIA = config_get_int_value(config, "TAM_MEMORIA");
    TAM_PAGINA = config_get_int_value(config, "TAM_PAGINA"); 
    PATH_INSTRUCCIONES = config_get_string_value(config, "PATH_INSTRUCCIONES");
    RETARDO_RESPUESTA = config_get_int_value(config, "RETARDO_RESPUESTA");
}