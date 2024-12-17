#include <interfaz_stdin.h>

void atender_peticiones_stdin(){
    while(1){
        int cod_op = recibir_operacion(entradasalida_cliente_kernel);
        switch(cod_op){
            case STD_READ_CODE:
                t_buffer* buffer = recibir_buffer(entradasalida_cliente_kernel);
                int pid = extraer_int_buffer(buffer,logger);
                int tam_total = extraer_int_buffer(buffer,logger);
                int cant_dir_fisicas = extraer_int_buffer(buffer, logger);
                
                char* texto = malloc(tam_total + 2);
                printf("Ingrese el texto correspondiente al PID %d: \n", pid);
                fgets(texto,1000 + tam_total, stdin);
                //texto[strcspn(texto, "\n")] = '\0';


                int desplazamiento = 0;
                bool conf_escritura;

                for(int i = 0; i< cant_dir_fisicas; i++){
                    int dir_fisca = extraer_int_buffer(buffer, logger);
                    int tam = extraer_int_buffer(buffer, logger);

                    void* copiar_a_mem = malloc(tam);

                    memcpy(copiar_a_mem,texto + desplazamiento, tam);

                    desplazamiento = desplazamiento + tam;

                    t_paquete* paquete1 = crear_paquete(ESCRITURA_CODE);

                    agregar_int_a_paquete(paquete1, pid);
                    agregar_int_a_paquete(paquete1, dir_fisca);
                    agregar_int_a_paquete(paquete1,tam);

                    agregar_a_paquete(paquete1, copiar_a_mem, tam);
                    enviar_paquete(paquete1, entradasalida_cliente_memoria);
                    eliminar_paquete(paquete1);
                    free(copiar_a_mem);

                    conf_escritura = confirmacion_escritura();
                    if(!conf_escritura){
                        t_paquete* paquete_fallo = crear_paquete(FALLO_IO);
                        agregar_int_a_paquete(paquete_fallo, pid);
                        enviar_paquete(paquete_fallo, entradasalida_cliente_kernel);
                        eliminar_paquete(paquete_fallo);

                        break;
                    }
                }

                free(texto);

                log_info(logger_obligatorio, "Interfaz: %s - PID: %d - Operacion: IO_STDIN_READ", nombre_interfaz,pid);
                


                if(conf_escritura){
                    t_paquete* paquete = crear_paquete(EXITO_IO);
                    agregar_int_a_paquete(paquete,pid);
                    enviar_paquete(paquete,entradasalida_cliente_kernel);
                    eliminar_paquete(paquete);
                }
                
                
                break;

            default:
                break;
        }
    }
}