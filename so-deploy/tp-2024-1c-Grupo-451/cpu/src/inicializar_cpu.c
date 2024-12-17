#include <inicializar_cpu.h>

void inicializar_cpu(char* path_config){
    logger = iniciar_logger("./cpu.log", "Cpu_Logger", LOG_LEVEL_INFO, 1);
    logger_obligatorio = iniciar_logger("./cpuObligatorio.log", "Cpu_Logger_Obligatorio", LOG_LEVEL_INFO,1);
    pid_en_ejecucion = 0;
    interrupcion_recibida = NO_INTERRUPCION;
    pid_de_interrupcion = 0;
    inicializar_config_cpu(path_config);

    los_registros_de_la_cpu = iniciar_registros_cpu();

    tlb = list_create();

    pthread_mutex_init(&mutex_para_interrupcion, NULL);
    pthread_mutex_init(&mutex_para_pid_interrupcion, NULL);

}

void inicializar_config_cpu(char* path_config){

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

    IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
    PUERTO_ESCUCHA_DISPATCH = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");
    CANTIDAD_ENTRADAS_TLB = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
    ALGORITMO_TLB = config_get_string_value(config, "ALGORITMO_TLB");
}

t_registros_cpu* iniciar_registros_cpu(){
    t_registros_cpu* registro = malloc(sizeof(t_registros_cpu));

    registro->PC = calloc(1,sizeof(uint32_t));
    registro->AX = calloc(1,sizeof(uint8_t));
    registro->BX = calloc(1,sizeof(uint8_t));
    registro->CX = calloc(1,sizeof(uint8_t));
    registro->DX = calloc(1,sizeof(uint8_t));
    /*
    registro->AX = malloc(sizeof(uint8_t));
    registro->BX = malloc(sizeof(uint8_t));
    registro->CX = malloc(sizeof(uint8_t));
    registro->DX = malloc(sizeof(uint8_t));
    */
    registro->EAX = calloc(1,sizeof(uint32_t));
    registro->EBX = calloc(1,sizeof(uint32_t));
    registro->ECX = calloc(1,sizeof(uint32_t));
    registro->EDX = calloc(1,sizeof(uint32_t));
    registro->SI = calloc(1,sizeof(uint32_t));
    registro->DI = calloc(1,sizeof(uint32_t));

    return registro;

}
