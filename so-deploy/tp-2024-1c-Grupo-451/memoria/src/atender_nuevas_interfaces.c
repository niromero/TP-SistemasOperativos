#include <atender_nuevas_interfaces.h>

void atender_conexion_de_interfaces(){
    while(1){
        int* cliente = malloc(sizeof(int));
        *cliente = esperar_cliente(memoria_server,logger,"Nueva Interfaz Entrada Salida conectada");
        pthread_t hilo_atender_entradasalida;
        pthread_create(&hilo_atender_entradasalida,NULL, (void*)atender_entradasalida_memoria,(void*)cliente);
        pthread_detach(hilo_atender_entradasalida);
    }
}