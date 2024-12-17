#include <main_kernel.h>

int main(int argc, char* argv[]) {

    char* path_config = malloc(256 + 2);
    printf("Ingrese el config con el que desea inicializar el Kernel\n");
    fgets(path_config,256,stdin);

    inicializar_kernel(path_config);

    free(path_config);
    
    //Se inicia al kernel como servidor
    kernel_server = iniciar_servidor(PUERTO_ESCUCHA, logger);
    
    //Se conecta Kernel a memoria
    kernel_cliente_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);

    //Se conecta el kernel con el CPU dispatch
    kernel_cliente_dispatch = crear_conexion(IP_CPU, PUERTO_CPU_DISPATCH);

    //Se conecta el kernel con el CPU interrupt
    kernel_cliente_interrupt = crear_conexion(IP_CPU, PUERTO_CPU_INTERRUPT);

    //Esperar conexion de Entrada Salida
    
    pthread_t hilo_entradasalida_kernel;
	pthread_create(&hilo_entradasalida_kernel,NULL,(void*)atender_las_nuevas_interfaces, NULL);
	pthread_detach(hilo_entradasalida_kernel);

    enviar_handshake("Kernel",kernel_cliente_memoria);
    enviar_handshake("Kernel Interrupt", kernel_cliente_interrupt);

    iniciar_planificacion_largo_plazo();
    iniciar_planificacion_corto_plazo();

    consola_kernel();
    

    liberar_conexion(kernel_cliente_dispatch);
    liberar_conexion(kernel_cliente_interrupt);
    liberar_conexion(kernel_server);
    terminar_programa(logger, config);
    return 0;
}