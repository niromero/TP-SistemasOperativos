#include <main_memoria.h>

int main(int argc, char* argv[]) {

	char* path_config = malloc(256 + 2);
    printf("Ingrese el config con el que desea inicializar la Memoria\n");
    fgets(path_config,256,stdin);

    inicializar_memoria(path_config);

	free(path_config);
		
	//Se inicia la memoria como servidor
    memoria_server = iniciar_servidor(PUERTO_ESCUCHA, logger);
	
	//Esperar la conexion del cpu
	cpu_cliente = esperar_cliente(memoria_server, logger, "Cpu conectada");
	

	//Esperar la conexion del kernel
	kernel_cliente = esperar_cliente(memoria_server,logger, "Kernel Conectado");

	//Se esperan las conexiones de las nuevas interfaces de Entrada/Salida
	pthread_t hilo_entradasalida;
	pthread_create(&hilo_entradasalida,NULL,(void*)atender_conexion_de_interfaces, NULL);
	pthread_detach(hilo_entradasalida);

	//Atender mensajes del cpu
	pthread_t hilo_cpu;
	pthread_create(&hilo_cpu,NULL,(void*)atender_cpu_memoria, NULL);
	pthread_detach(hilo_cpu);

	//Atender mensajes del kernel
	pthread_t hilo_kernel;
	pthread_create(&hilo_kernel,NULL,(void*)atender_kernel_memoria, NULL);
	pthread_join(hilo_kernel,NULL);

	

	liberar_conexion(cpu_cliente);
	liberar_conexion(kernel_cliente);
	liberar_conexion(memoria_server);
	
	terminar_programa(logger, config);
	free(memoria_de_usuario);
	bitarray_destroy(marcos_de_memoria_libres);
	return EXIT_SUCCESS;

	
}
