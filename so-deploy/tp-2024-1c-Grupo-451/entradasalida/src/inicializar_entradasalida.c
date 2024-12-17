#include <inicializar_entradasalida.h>

void inicializar_entradasalida(char* path_config){
    logger = iniciar_logger("./entradasalida.log", "EntradaSalida_Logger", LOG_LEVEL_INFO, 1);
    logger_obligatorio = iniciar_logger("./entradasalidaObligatorio.log", "EntradaSalida_Logger_Obligatorio", LOG_LEVEL_INFO,1);
    inicializar_config_entradasalida(path_config);

    lista_archivos = list_create();

    pthread_mutex_init(&mutex_para_interfaz, NULL);
}

void inicializar_config_entradasalida(char* path_config){
    char** string_sin_contra_barra = string_split(path_config,"\n");
    char* path_configs = strdup("./Configs/");
    string_append(&path_configs,string_sin_contra_barra[0]);

    config = iniciar_config(path_configs);
    string_array_destroy(string_sin_contra_barra);
    free(path_configs);
    
    TIPO_INTERFAZ = config_get_string_value(config, "TIPO_INTERFAZ");
    TIEMPO_UNIDAD_TRABAJO = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
    PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");
    IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
    PUERTO_MEMORIA =  config_get_string_value(config, "PUERTO_MEMORIA");
    PATH_BASE_DIALFS = config_get_string_value(config, "PATH_BASE_DIALFS");
    BLOCK_SIZE = config_get_int_value(config, "BLOCK_SIZE");
    BLOCK_COUNT = config_get_int_value(config, "BLOCK_COUNT");
    RETRASO_COMPACTACION = config_get_int_value(config, "RETRASO_COMPACTACION");
}