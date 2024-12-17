#include <main_entradasalida.h>

int main(int argc, char* argv[]) {
     
    nombre_interfaz = malloc(50 + 2);
    char* path_config = malloc(256 + 2);
    printf("Ingrese el nombre de la interfaz\n");
    fgets(nombre_interfaz,50,stdin);
    printf("Ingrese el config con el que desea inicializar la interfaz\n");
    fgets(path_config,256,stdin);
    inicializar_entradasalida(path_config);
    free(path_config);

    tipo_de_interfaz = definir_tipo_interfaz();
    if(tipo_de_interfaz == -1){
        log_error(logger,"TIPO DE INTERFAZ INEXISTENTE, REVISE EL CONFIG");
        exit (EXIT_FAILURE);
    }

    //Se conecta al Kernel
    entradasalida_cliente_kernel = crear_conexion(IP_KERNEL, PUERTO_KERNEL);
    t_paquete* paquete = crear_paquete(PRIMERA_CONEXION_IO);
    agregar_string_a_paquete(paquete,nombre_interfaz);
    agregar_string_a_paquete(paquete,TIPO_INTERFAZ);
    enviar_paquete(paquete,entradasalida_cliente_kernel);
    eliminar_paquete(paquete);
    
    if(strcmp(TIPO_INTERFAZ,"Generica")!=0){
        //Se conecta al Memoria 
        entradasalida_cliente_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
        enviar_handshake("Entrada/Salida", entradasalida_cliente_memoria);
    }
    
    enviar_handshake("Entrada/Salida", entradasalida_cliente_kernel);


    for(int i = 0; i < strlen(nombre_interfaz); i++){
        if(nombre_interfaz[i] == '\n'){
            nombre_interfaz[i] = '\0';
            break;
        }
    }


    switch (tipo_de_interfaz){
        case GENERICO:
            atender_peticiones_generica();
            break;
        case STDIN:
            atender_peticiones_stdin();
            break;
        case STDOUT:
            atender_peticiones_stdout();
            break;
        case DIALFS:
            levantar_archivos();
            atender_peticiones_dialfs();
            break;
        default:
            break;
    }


    liberar_conexion(entradasalida_cliente_kernel);
    //liberar_conexion(entradasalida_cliente_memoria);
    
    terminar_programa(logger, config);


    return 0;
    
}

int definir_tipo_interfaz(){
    if(strcmp(TIPO_INTERFAZ,"Generica")==0){
        return GENERICO;
    }
    else if(strcmp(TIPO_INTERFAZ,"stdin")==0){
        return STDIN;
    }
    else if(strcmp(TIPO_INTERFAZ,"stdout")==0){
        return STDOUT;
    }
    else if(strcmp(TIPO_INTERFAZ,"dialfs")==0){
        
        return DIALFS;
    }
    else{
        return -1;
    }
}