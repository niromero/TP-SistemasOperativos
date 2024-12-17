#include <interfaz_generica.h>

void atender_peticiones_generica(){
    while(1){
        int cod_op = recibir_operacion(entradasalida_cliente_kernel);
        switch(cod_op){
            case ESPERAR_GEN:
                t_buffer* buffer = recibir_buffer(entradasalida_cliente_kernel);
                 int pid = extraer_int_buffer(buffer,logger);
                int tiempo_espera = extraer_int_buffer(buffer,logger);
                log_info(logger_obligatorio, "Interfaz: %s - PID: %d - Operacion: IO_GEN_SLEEP", nombre_interfaz,pid);
                usleep(tiempo_espera*TIEMPO_UNIDAD_TRABAJO * 1000);

                t_paquete* paquete = crear_paquete(EXITO_IO);
                agregar_int_a_paquete(paquete,pid);
                enviar_paquete(paquete,entradasalida_cliente_kernel);
                eliminar_paquete(paquete);
                break;

            default:
                break;
        }
    }
}


