#include <main_cpu.h>

int main(int argc, char* argv[]) {

    char* path_config = malloc(256 + 2);
    printf("Ingrese el config con el que desea inicializar el CPU\n");
    fgets(path_config,256,stdin);

    inicializar_cpu(path_config);

    free(path_config);

    //Iniciar CPU como Server
    cpu_server_dispatch = iniciar_servidor(PUERTO_ESCUCHA_DISPATCH, logger);
    cpu_server_interrupt = iniciar_servidor(PUERTO_ESCUCHA_INTERRUPT, logger);

    //Conectarse a la memoria
    cpu_cliente_memoria = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA);
    

    //Esperar que se conecte el kernel
    kernel_cliente_dispatch = esperar_cliente(cpu_server_dispatch, logger, "Kernel dispatch conectado");
    kernel_cliente_interrupt = esperar_cliente(cpu_server_interrupt, logger, "Kernel interrupt conectado");


    //Atender mensajes del Kernel Interrupt
    pthread_t hilo_kernel_interrupt;
	pthread_create(&hilo_kernel_interrupt,NULL,(void*)atender_kernel_interrupt, NULL);
	pthread_detach(hilo_kernel_interrupt);


    enviar_handshake("CPU", cpu_cliente_memoria);
    atender_memoria_cpu_sin_while(); //Se recibe el tamaño de las paginas de memoria

    
    while(true){
        ciclo();
    }
    

    liberar_conexion(cpu_cliente_memoria);
    liberar_conexion(cpu_server_dispatch);
    liberar_conexion(cpu_server_interrupt);

    terminar_programa(logger, config);
    return 0;
}


