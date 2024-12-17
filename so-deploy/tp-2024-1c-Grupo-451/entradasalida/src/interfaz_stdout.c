#include <interfaz_stdout.h>

void atender_peticiones_stdout(){
    while(1){
        int cod_op = recibir_operacion(entradasalida_cliente_kernel);
        switch(cod_op){
            case STD_WRITE_CODE:
                t_buffer* buffer = recibir_buffer(entradasalida_cliente_kernel);
                int pid = extraer_int_buffer(buffer,logger);
                int tam_total = extraer_int_buffer(buffer,logger);
                int cant_dir_fisicas = extraer_int_buffer(buffer, logger);
                
                char* texto = malloc(tam_total + 1);

                int desplazamiento = 0;

                for(int i = 0; i< cant_dir_fisicas; i++){
                    int dir_fisca = extraer_int_buffer(buffer, logger);
                    int tam = extraer_int_buffer(buffer, logger);

                    t_paquete* paquete1 = crear_paquete(LECTURA_CODE);

                    agregar_int_a_paquete(paquete1, pid);
                    agregar_int_a_paquete(paquete1, dir_fisca);
                    agregar_int_a_paquete(paquete1,tam);

                    enviar_paquete(paquete1, entradasalida_cliente_memoria);
                    eliminar_paquete(paquete1);
                    
                    void* leido = recibir_lectura();

                    memcpy(texto + desplazamiento, leido, tam );

                    desplazamiento = desplazamiento + tam;

                    free(leido);
                }

                texto[tam_total] = '\0';

                printf("El texto leido correspondiente al PID %d es: %s \n",pid, texto);
                free(texto);

                log_info(logger_obligatorio, "Interfaz: %s - PID: %d - Operacion: IO_STDOUT_WRITE", nombre_interfaz,pid);
                
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