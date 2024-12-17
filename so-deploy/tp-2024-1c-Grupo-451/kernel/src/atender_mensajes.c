#include <atender_mensajes.h>

void atender_memoria(){
    while (1) {
		int cod_op = recibir_operacion(kernel_cliente_memoria);
		switch (cod_op) {
		case HANDSHAKE:
			t_buffer* buffer = recibir_buffer(kernel_cliente_memoria);
			char* mensaje = extraer_string_buffer(buffer, logger);
			printf("Recibi un handshake de: %s, como cliente",mensaje);
			free(mensaje);
			break;
		case -1:
			log_error(logger, "Se desconceto la Memoria");
			exit(EXIT_FAILURE);
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
}

int recibir_PC_memoria(){
	int cod_op = recibir_operacion(kernel_cliente_memoria);
	switch (cod_op) {
	case CREAR_PROCESO:
		t_buffer* buffer = recibir_buffer(kernel_cliente_memoria);
		int pc = extraer_int_buffer(buffer, logger);
		return pc;
		break;
	case -1:
		log_error(logger, "Se desconceto la Memoria");
		exit(EXIT_FAILURE);
	default:
		log_warning(logger,"La operacion no es la de crear proceso");
		return -1;
		break;
	}
}

void atender_cpu_dispatch(){
		int cod_op = recibir_operacion(kernel_cliente_dispatch);

		if(proceso_en_ejecucion != NULL && (strcmp(ALGORITMO_PLANIFICACION,"rr")==0 || strcmp(ALGORITMO_PLANIFICACION,"vrr")==0)){
			pthread_mutex_lock(&mutex_para_proceso_en_ejecucion);
			pthread_cancel(proceso_en_ejecucion ->hilo_quantum);
			temporal_stop(proceso_en_ejecucion ->tiempo_en_ejecucion);
			pthread_mutex_unlock(&mutex_para_proceso_en_ejecucion);
		}


		if(!permitir_planificacion){
            sem_wait(&detener_planificacion_salida_cpu);
        }

		t_buffer* buffer;
		if(proceso_en_ejecucion != NULL){
			switch (cod_op) {
				case HANDSHAKE:
					buffer = recibir_buffer(kernel_cliente_dispatch);
					char* mensaje = extraer_string_buffer(buffer, logger);
					printf("Recibi un handshake de: %s, como cliente",mensaje);
					free(mensaje);
					break;
				case FINALIZAR_EXEC:
					buffer = recibir_buffer(kernel_cliente_dispatch);

					pthread_mutex_lock(&mutex_para_proceso_en_ejecucion);
					pcb* pcb_a_finalizar = proceso_en_ejecucion;
					pthread_mutex_unlock(&mutex_para_proceso_en_ejecucion);
				

					recibir_contexto_de_ejecucion(buffer,pcb_a_finalizar);

					pcb_a_finalizar ->razon_salida = EXITO;
					mandar_a_exit(pcb_a_finalizar);
					log_info(logger_obligatorio, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: EXIT", pcb_a_finalizar ->PID);
					
					break;
				case OUT_OF_MEM_CODE:
					buffer = recibir_buffer(kernel_cliente_dispatch);

					pthread_mutex_lock(&mutex_para_proceso_en_ejecucion);
					pcb* pcb_a_finalizar_por_mem = proceso_en_ejecucion;
					pthread_mutex_unlock(&mutex_para_proceso_en_ejecucion);

					recibir_contexto_de_ejecucion(buffer,pcb_a_finalizar_por_mem);

					pcb_a_finalizar_por_mem ->razon_salida = SIN_MEMORIA;
					mandar_a_exit(pcb_a_finalizar_por_mem);
					log_info(logger_obligatorio, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: EXIT", pcb_a_finalizar_por_mem ->PID);

					break;
				case ESPERAR_GEN:
					buffer = recibir_buffer(kernel_cliente_dispatch);

					pthread_mutex_lock(&mutex_para_proceso_en_ejecucion);
					pcb* pcb_del_proceso = proceso_en_ejecucion;
					pthread_mutex_unlock(&mutex_para_proceso_en_ejecucion);

					recibir_contexto_de_ejecucion(buffer,pcb_del_proceso);


					char* interfaz = extraer_string_buffer(buffer,logger);
					char* tipo_interfaz = "Generica";
					int* tiempo_espera = malloc(sizeof(int));
					*tiempo_espera = extraer_int_buffer(buffer,logger);

					pthread_mutex_lock(&mutex_para_eliminar_entradasalida);
					nodo_de_diccionario_interfaz* nodo_de_interfaz = comprobrar_existencia_de_interfaz(pcb_del_proceso,interfaz,tipo_interfaz);

					if(nodo_de_interfaz != NULL){
						pthread_mutex_lock(&mutex_para_diccionario_blocked);
						nodo_de_diccionario_blocked* nodo_bloqueados = dictionary_get(diccionario_blocked,interfaz);
						pthread_mutex_unlock(&mutex_para_diccionario_blocked);

						pthread_mutex_lock(&(nodo_bloqueados ->mutex_para_cola_variables));
						queue_push(nodo_bloqueados ->cola_Variables,tiempo_espera);
						pthread_mutex_unlock(&(nodo_bloqueados ->mutex_para_cola_variables));

						sem_post(&(nodo_de_interfaz ->hay_proceso_en_bloqueados));

					}
					else{
						free(tiempo_espera);
					}
					pthread_mutex_unlock(&mutex_para_eliminar_entradasalida);
					free(interfaz);
					break;

				case STD_READ_WRITE_CODE:
					buffer = recibir_buffer(kernel_cliente_dispatch);

					pthread_mutex_lock(&mutex_para_proceso_en_ejecucion);
					pcb* pcb_a_std_read_write = proceso_en_ejecucion;
					pthread_mutex_unlock(&mutex_para_proceso_en_ejecucion);

					recibir_contexto_de_ejecucion(buffer, pcb_a_std_read_write);

					char* tipo_interfaz1 = extraer_string_buffer(buffer, logger);

					io_std_fs* dir_fisicas = extraer_dir_fisicas_de_buffer(buffer);

					pthread_mutex_lock(&mutex_para_eliminar_entradasalida);
					nodo_de_diccionario_interfaz* nodo_interfaz1 = comprobrar_existencia_de_interfaz(pcb_a_std_read_write, dir_fisicas ->interfaz, tipo_interfaz1);

					if(nodo_interfaz1 != NULL){
						pthread_mutex_lock(&mutex_para_diccionario_blocked);
						nodo_de_diccionario_blocked* nodo_bloqueados1 = dictionary_get(diccionario_blocked,dir_fisicas ->interfaz);
						pthread_mutex_unlock(&mutex_para_diccionario_blocked);

						pthread_mutex_lock(&(nodo_bloqueados1 ->mutex_para_cola_variables));
						queue_push(nodo_bloqueados1 ->cola_Variables, dir_fisicas);
						pthread_mutex_unlock(&(nodo_bloqueados1 ->mutex_para_cola_variables));

						sem_post(&(nodo_interfaz1 ->hay_proceso_en_bloqueados));

					}
					else{
						free(dir_fisicas ->interfaz);
						list_destroy_and_destroy_elements(dir_fisicas ->lista_dir_fisicas, free);
						free(dir_fisicas);
					}
					pthread_mutex_unlock(&mutex_para_eliminar_entradasalida);

					free(tipo_interfaz1);

					break;

				case FS_CREATE_CODE:
					buffer = recibir_buffer(kernel_cliente_dispatch);

					pthread_mutex_lock(&mutex_para_proceso_en_ejecucion);
					pcb* pcb_create = proceso_en_ejecucion;
					pthread_mutex_unlock(&mutex_para_proceso_en_ejecucion);

					recibir_contexto_de_ejecucion(buffer, pcb_create);

					char* interfaz1 = extraer_string_buffer(buffer, logger);
					char* nombre_Arch1 = extraer_string_buffer(buffer, logger);

					char* tipo_interfaz2 = "dialfs";

					pthread_mutex_lock(&mutex_para_eliminar_entradasalida);
					nodo_de_diccionario_interfaz* nodo_interfaz2 = comprobrar_existencia_de_interfaz(pcb_create, interfaz1, tipo_interfaz2);

					if(nodo_interfaz2 != NULL){
						pthread_mutex_lock(&mutex_para_diccionario_blocked);
						nodo_de_diccionario_blocked* nodo_bloqueados2 = dictionary_get(diccionario_blocked, interfaz1);
						pthread_mutex_unlock(&mutex_para_diccionario_blocked);

						var_fs* variable_para_cola = malloc(sizeof(var_fs));

						variable_para_cola ->tipo_variable = VAR_FS_CREATE;
						variable_para_cola ->nombre_Archivo = nombre_Arch1;

						pthread_mutex_lock(&(nodo_bloqueados2 ->mutex_para_cola_variables));
						queue_push(nodo_bloqueados2 ->cola_Variables, variable_para_cola);
						pthread_mutex_unlock(&(nodo_bloqueados2 ->mutex_para_cola_variables));


						sem_post(&(nodo_interfaz2 ->hay_proceso_en_bloqueados));

					}
					else{
						free(nombre_Arch1);
					}
					free(interfaz1);
					pthread_mutex_unlock(&mutex_para_eliminar_entradasalida);

					break;
				
				case FS_DELETE_CODE:
					buffer = recibir_buffer(kernel_cliente_dispatch);

					pthread_mutex_lock(&mutex_para_proceso_en_ejecucion);
					pcb* pcb_delete = proceso_en_ejecucion;
					pthread_mutex_unlock(&mutex_para_proceso_en_ejecucion);

					recibir_contexto_de_ejecucion(buffer, pcb_delete);

					char* interfaz2 = extraer_string_buffer(buffer, logger);
					char* nombre_Arch2 = extraer_string_buffer(buffer, logger);

					char* tipo_interfaz3 = "dialfs";

					pthread_mutex_lock(&mutex_para_eliminar_entradasalida);
					nodo_de_diccionario_interfaz* nodo_interfaz3 = comprobrar_existencia_de_interfaz(pcb_delete, interfaz2, tipo_interfaz3);

					if(nodo_interfaz3 != NULL){
						pthread_mutex_lock(&mutex_para_diccionario_blocked);
						nodo_de_diccionario_blocked* nodo_bloqueados3 = dictionary_get(diccionario_blocked, interfaz2);
						pthread_mutex_unlock(&mutex_para_diccionario_blocked);

						var_fs* variable_para_cola = malloc(sizeof(var_fs));

						variable_para_cola ->tipo_variable = VAR_FS_DELETE;
						variable_para_cola ->nombre_Archivo = nombre_Arch2;

						pthread_mutex_lock(&(nodo_bloqueados3 ->mutex_para_cola_variables));
						queue_push(nodo_bloqueados3 ->cola_Variables, variable_para_cola);
						pthread_mutex_unlock(&(nodo_bloqueados3 ->mutex_para_cola_variables));


						sem_post(&(nodo_interfaz3 ->hay_proceso_en_bloqueados));

					}
					else{
						free(nombre_Arch2);
					}
					free(interfaz2);
					pthread_mutex_unlock(&mutex_para_eliminar_entradasalida);

					break;

				case FS_TRUNCATE_CODE:
					buffer = recibir_buffer(kernel_cliente_dispatch);

					pthread_mutex_lock(&mutex_para_proceso_en_ejecucion);
					pcb* pcb_truncate = proceso_en_ejecucion;
					pthread_mutex_unlock(&mutex_para_proceso_en_ejecucion);

					recibir_contexto_de_ejecucion(buffer, pcb_truncate);

					char* interfaz3 = extraer_string_buffer(buffer, logger);
					char* nombre_Arch3 = extraer_string_buffer(buffer, logger);

					int tam_a_truncar = extraer_int_buffer(buffer, logger);

					char* tipo_interfaz4 = "dialfs";

					pthread_mutex_lock(&mutex_para_eliminar_entradasalida);
					nodo_de_diccionario_interfaz* nodo_interfaz4 = comprobrar_existencia_de_interfaz(pcb_truncate, interfaz3, tipo_interfaz4);

					if(nodo_interfaz4 != NULL){
						pthread_mutex_lock(&mutex_para_diccionario_blocked);
						nodo_de_diccionario_blocked* nodo_bloqueados4 = dictionary_get(diccionario_blocked, interfaz3);
						pthread_mutex_unlock(&mutex_para_diccionario_blocked);

						var_fs* variable_para_cola = malloc(sizeof(var_fs));

						variable_para_cola ->tipo_variable = VAR_FS_TRUNCATE;
						variable_para_cola ->nombre_Archivo = nombre_Arch3;
						variable_para_cola ->tam_truncate = tam_a_truncar;

						pthread_mutex_lock(&(nodo_bloqueados4 ->mutex_para_cola_variables));
						queue_push(nodo_bloqueados4 ->cola_Variables, variable_para_cola);
						pthread_mutex_unlock(&(nodo_bloqueados4 ->mutex_para_cola_variables));
						

						sem_post(&(nodo_interfaz4 ->hay_proceso_en_bloqueados));

					}
					else{
						free(nombre_Arch3);
					}
					free(interfaz3);
					pthread_mutex_unlock(&mutex_para_eliminar_entradasalida);

					break;

				case FS_READ_WRITE_CODE:
					buffer = recibir_buffer(kernel_cliente_dispatch);

					pthread_mutex_lock(&mutex_para_proceso_en_ejecucion);
					pcb* pcb_fs_read_write = proceso_en_ejecucion;
					pthread_mutex_unlock(&mutex_para_proceso_en_ejecucion);

					recibir_contexto_de_ejecucion(buffer, pcb_fs_read_write);

					char* nombre_Arch4 = extraer_string_buffer(buffer, logger);
					int puntero_arch = extraer_int_buffer(buffer, logger);
					char* lectura_o_escritura = extraer_string_buffer(buffer, logger);

					char* tipo_interfaz5 = "dialfs";

					io_std_fs* dir_fisicas_fs = extraer_dir_fisicas_de_buffer(buffer);

					pthread_mutex_lock(&mutex_para_eliminar_entradasalida);
					nodo_de_diccionario_interfaz* nodo_interfaz5 = comprobrar_existencia_de_interfaz(pcb_fs_read_write, dir_fisicas_fs ->interfaz, tipo_interfaz5);

					if(nodo_interfaz5 != NULL){
						pthread_mutex_lock(&mutex_para_diccionario_blocked);
						nodo_de_diccionario_blocked* nodo_bloqueados5 = dictionary_get(diccionario_blocked,dir_fisicas_fs ->interfaz);
						pthread_mutex_unlock(&mutex_para_diccionario_blocked);

						var_fs* variable_para_cola = malloc(sizeof(var_fs));
						variable_para_cola ->nombre_Archivo = nombre_Arch4;
						variable_para_cola ->puntero_Arch = puntero_arch;
						variable_para_cola ->dir_fisicas = dir_fisicas_fs;
						
						if(strcmp(lectura_o_escritura, "lectura")==0){
							variable_para_cola ->tipo_variable = VAR_FS_READ;
						}
						else if(strcmp(lectura_o_escritura, "escritura")==0){
							variable_para_cola ->tipo_variable = VAR_FS_WRITE;
						}
						
						free(lectura_o_escritura);

						pthread_mutex_lock(&(nodo_bloqueados5 ->mutex_para_cola_variables));
						queue_push(nodo_bloqueados5 ->cola_Variables, variable_para_cola);
						pthread_mutex_unlock(&(nodo_bloqueados5 ->mutex_para_cola_variables));

						sem_post(&(nodo_interfaz5 ->hay_proceso_en_bloqueados));

					}
					else{
						free(nombre_Arch4);
						free(lectura_o_escritura);
						free(dir_fisicas_fs ->interfaz);
						list_destroy_and_destroy_elements(dir_fisicas_fs ->lista_dir_fisicas, free);
						free(dir_fisicas_fs);
					}
					pthread_mutex_unlock(&mutex_para_eliminar_entradasalida);

					break;

				case WAIT_CODE:
					buffer = recibir_buffer(kernel_cliente_dispatch);

					pthread_mutex_lock(&mutex_para_proceso_en_ejecucion);
					pcb* pcb_a_esperar = proceso_en_ejecucion;
					pthread_mutex_unlock(&mutex_para_proceso_en_ejecucion);

					recibir_contexto_de_ejecucion(buffer,pcb_a_esperar);

					char* recurso = extraer_string_buffer(buffer,logger);

					bool existe_recurso;

					pthread_mutex_lock(&mutex_para_diccionario_recursos);
					existe_recurso = dictionary_has_key(diccionario_recursos,recurso);
					pthread_mutex_unlock(&mutex_para_diccionario_recursos);

					if(existe_recurso){
						pthread_mutex_lock(&mutex_para_diccionario_recursos);
						nodo_recursos* nodo_del_recurso = dictionary_get(diccionario_recursos,recurso);
						pthread_mutex_unlock(&mutex_para_diccionario_recursos);

						pthread_mutex_lock(&(nodo_del_recurso ->mutex_del_recurso));
						nodo_del_recurso ->instancias = nodo_del_recurso ->instancias -1;
						

						if(nodo_del_recurso -> instancias < 0){
							pcb_a_esperar ->estado_proceso = BLOCKED;
							free(pcb_a_esperar ->tiempo_en_ejecucion);
							pcb_a_esperar ->tiempo_en_ejecucion = NULL;
							log_info(logger_obligatorio,"PID: %d - Estado Anterior: EXECUTE - Estado Actual: BLOCKED", pcb_a_esperar->PID);
							log_info(logger_obligatorio, "PID: %d - Bloqueado por: %s",pcb_a_esperar ->PID, recurso);
							strcpy(pcb_a_esperar ->recurso_bloqueante, recurso);
							queue_push(nodo_del_recurso ->cola_bloqueados_recurso,pcb_a_esperar);
							pthread_mutex_unlock(&(nodo_del_recurso ->mutex_del_recurso));
						}
						else if(strcmp(ALGORITMO_PLANIFICACION,"rr")==0 || strcmp(ALGORITMO_PLANIFICACION,"vrr")==0){ //ROUND ROBIN O VIRTUAL ROUND ROBIN
							char* recurso_lista = strdup(recurso);
							list_add(pcb_a_esperar ->lista_recursos_tomados,recurso_lista);
							int64_t tiempo_ejecutado = temporal_gettime(pcb_a_esperar ->tiempo_en_ejecucion);
							free(pcb_a_esperar ->tiempo_en_ejecucion);
							pcb_a_esperar ->tiempo_en_ejecucion = NULL;
							if(tiempo_ejecutado < pcb_a_esperar ->quantum_restante){
								pcb_a_esperar ->quantum_restante = pcb_a_esperar ->quantum_restante - tiempo_ejecutado;

								t_paquete* paquete_pcb_a_enviar = crear_paquete(INICIAR_EXEC);
								agregar_int_a_paquete(paquete_pcb_a_enviar,pcb_a_esperar->PID);
								serializar_registros_procesador(paquete_pcb_a_enviar, pcb_a_esperar->registros_cpu_en_pcb);
			
								enviar_paquete(paquete_pcb_a_enviar, kernel_cliente_dispatch);
								eliminar_paquete(paquete_pcb_a_enviar);
								pcb_a_esperar ->tiempo_en_ejecucion = temporal_create();
								pthread_create(&(pcb_a_esperar ->hilo_quantum),NULL,(void*)esperar_quantum,(void*)pcb_a_esperar);
								pthread_detach(pcb_a_esperar ->hilo_quantum);
								pthread_mutex_unlock(&(nodo_del_recurso ->mutex_del_recurso));
								atender_cpu_dispatch();
							}
							else{
								char* recurso_lista = strdup(recurso);
								list_add(pcb_a_esperar ->lista_recursos_tomados,recurso_lista);
								log_info(logger_obligatorio,"PID: %d - Desalojado por fin de Quantum",pcb_a_esperar ->PID);
								pcb_a_esperar->estado_proceso = READY;
								log_info(logger_obligatorio, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: READY", pcb_a_esperar->PID);
								pcb_a_esperar ->quantum_restante = QUANTUM;


								pthread_mutex_lock(&mutex_cola_ready);
								queue_push(cola_ready,pcb_a_esperar);
								log_de_lista_de_ready();
								pthread_mutex_unlock(&mutex_cola_ready);

								sem_post(&hay_proceso_en_ready);
								pthread_mutex_unlock(&(nodo_del_recurso ->mutex_del_recurso));
							}
						}
						else{ //ES FIFO
							char* recurso_lista = strdup(recurso);
							list_add(pcb_a_esperar ->lista_recursos_tomados,recurso_lista);
							t_paquete* paquete_pcb_a_enviar = crear_paquete(INICIAR_EXEC);
							agregar_int_a_paquete(paquete_pcb_a_enviar,pcb_a_esperar->PID);
							serializar_registros_procesador(paquete_pcb_a_enviar, pcb_a_esperar->registros_cpu_en_pcb);
			
							enviar_paquete(paquete_pcb_a_enviar, kernel_cliente_dispatch);
							eliminar_paquete(paquete_pcb_a_enviar);
							pthread_mutex_unlock(&(nodo_del_recurso ->mutex_del_recurso));
							atender_cpu_dispatch();
						}
					}
					else{
						pcb_a_esperar ->razon_salida = RECURSO_INVALIDO;
						mandar_a_exit(pcb_a_esperar);
						
					}
					free(recurso);


					break;
				case SIGNAL_CODE:
					buffer = recibir_buffer(kernel_cliente_dispatch);

					pthread_mutex_lock(&mutex_para_proceso_en_ejecucion);
					pcb* pcb_a_senial = proceso_en_ejecucion;
					pthread_mutex_unlock(&mutex_para_proceso_en_ejecucion);

					recibir_contexto_de_ejecucion(buffer,pcb_a_senial);

					char* recurso_signal = extraer_string_buffer(buffer,logger);

					bool existe_recurso_signal;

					pthread_mutex_lock(&mutex_para_diccionario_recursos);
					existe_recurso_signal = dictionary_has_key(diccionario_recursos,recurso_signal);
					pthread_mutex_unlock(&mutex_para_diccionario_recursos);

					if(existe_recurso_signal){
						pthread_mutex_lock(&mutex_para_diccionario_recursos);
						nodo_recursos* nodo_del_recurso = dictionary_get(diccionario_recursos,recurso_signal);
						pthread_mutex_unlock(&mutex_para_diccionario_recursos);

						for (int i = 0; i<list_size(pcb_a_senial ->lista_recursos_tomados); i++){
							char* recurso_lista = list_get(pcb_a_senial ->lista_recursos_tomados,i);
							if(strcmp(recurso_lista,recurso_signal)==0){
								list_remove(pcb_a_senial ->lista_recursos_tomados,i);
								free(recurso_lista);
								break;
							}
						}

						pthread_mutex_lock(&(nodo_del_recurso ->mutex_del_recurso));
						nodo_del_recurso ->instancias ++;

						if(nodo_del_recurso ->instancias <= 0){
							pcb* pcb_a_desbloquear = queue_pop(nodo_del_recurso ->cola_bloqueados_recurso);
							pcb_a_desbloquear->estado_proceso = READY;
							log_info(logger_obligatorio, "PID: %d - Estado Anterior: BLOCKED - Estado Actual: READY", pcb_a_desbloquear->PID);
							pcb_a_desbloquear ->quantum_restante = QUANTUM;
							strcpy(pcb_a_desbloquear ->recurso_bloqueante, "No");

							char* recurso_lista = strdup(recurso_signal);
							list_add(pcb_a_desbloquear ->lista_recursos_tomados,recurso_lista);

							if(pcb_a_desbloquear ->tiempo_en_ejecucion != NULL){
								free(pcb_a_desbloquear ->tiempo_en_ejecucion);
								pcb_a_desbloquear ->tiempo_en_ejecucion = NULL;
							}


							pthread_mutex_lock(&mutex_cola_ready);
							queue_push(cola_ready,pcb_a_desbloquear);
							log_de_lista_de_ready();
							pthread_mutex_unlock(&mutex_cola_ready);

							sem_post(&hay_proceso_en_ready);
						}
						if(strcmp(ALGORITMO_PLANIFICACION,"rr")==0 || strcmp(ALGORITMO_PLANIFICACION,"vrr")==0){ //ROUND ROBIN O VIRTUAL ROUND ROBIN
							int64_t tiempo_ejecutado = temporal_gettime(pcb_a_senial ->tiempo_en_ejecucion);
							free(pcb_a_senial ->tiempo_en_ejecucion);
							pcb_a_senial ->tiempo_en_ejecucion = NULL;
							if(tiempo_ejecutado < pcb_a_senial ->quantum_restante){
								pcb_a_senial ->quantum_restante = pcb_a_senial ->quantum_restante - tiempo_ejecutado;

								t_paquete* paquete_pcb_a_enviar = crear_paquete(INICIAR_EXEC);
								agregar_int_a_paquete(paquete_pcb_a_enviar,pcb_a_senial->PID);
								serializar_registros_procesador(paquete_pcb_a_enviar, pcb_a_senial->registros_cpu_en_pcb);
			
								enviar_paquete(paquete_pcb_a_enviar, kernel_cliente_dispatch);
								eliminar_paquete(paquete_pcb_a_enviar);
								pcb_a_senial ->tiempo_en_ejecucion = temporal_create();
								pthread_create(&(pcb_a_senial ->hilo_quantum),NULL,(void*)esperar_quantum,(void*)pcb_a_senial);
								pthread_detach(pcb_a_senial ->hilo_quantum);
								pthread_mutex_unlock(&(nodo_del_recurso ->mutex_del_recurso));
								atender_cpu_dispatch();
							}
							else{
								log_info(logger_obligatorio,"PID: %d - Desalojado por fin de Quantum",pcb_a_senial ->PID);
								pcb_a_senial->estado_proceso = READY;
								log_info(logger_obligatorio, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: READY", pcb_a_senial->PID);
								pcb_a_senial ->quantum_restante = QUANTUM;


								pthread_mutex_lock(&mutex_cola_ready);
								queue_push(cola_ready,pcb_a_senial);
								log_de_lista_de_ready();
								pthread_mutex_unlock(&mutex_cola_ready);

								sem_post(&hay_proceso_en_ready);
								pthread_mutex_unlock(&(nodo_del_recurso ->mutex_del_recurso));
							}
						}
						else{ //ES FIFO
							t_paquete* paquete_pcb_a_enviar = crear_paquete(INICIAR_EXEC);
							agregar_int_a_paquete(paquete_pcb_a_enviar,pcb_a_senial->PID);
							serializar_registros_procesador(paquete_pcb_a_enviar, pcb_a_senial->registros_cpu_en_pcb);
			
							enviar_paquete(paquete_pcb_a_enviar, kernel_cliente_dispatch);
							eliminar_paquete(paquete_pcb_a_enviar);
							if(pcb_a_senial ->tiempo_en_ejecucion != NULL){
								free(pcb_a_senial ->tiempo_en_ejecucion);
								pcb_a_senial ->tiempo_en_ejecucion = NULL;
							}
							pcb_a_senial ->tiempo_en_ejecucion = temporal_create();
							pthread_mutex_unlock(&(nodo_del_recurso ->mutex_del_recurso));
							atender_cpu_dispatch();
						}
					}
					else{
						pcb_a_senial -> razon_salida = RECURSO_INVALIDO;
						mandar_a_exit(pcb_a_senial);
					}
					free(recurso_signal);
					break;
				case INTERRUPCION:
					buffer = recibir_buffer(kernel_cliente_dispatch);

					pthread_mutex_lock(&mutex_para_proceso_en_ejecucion);
					pcb* pcb_a_guardar = proceso_en_ejecucion;
					pthread_mutex_unlock(&mutex_para_proceso_en_ejecucion);


					recibir_contexto_de_ejecucion(buffer,pcb_a_guardar);

					if(pcb_a_guardar ->tiempo_en_ejecucion != NULL){
						free(pcb_a_guardar ->tiempo_en_ejecucion);
						pcb_a_guardar ->tiempo_en_ejecucion = NULL;
					}

					log_info(logger_obligatorio,"PID: %d - Desalojado por fin de Quantum",pcb_a_guardar ->PID);
					pcb_a_guardar->estado_proceso = READY;
					log_info(logger_obligatorio, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: READY", pcb_a_guardar->PID);
					pcb_a_guardar -> quantum_restante = QUANTUM;

					pthread_mutex_lock(&mutex_cola_ready);
					queue_push(cola_ready,pcb_a_guardar);
					log_de_lista_de_ready();
					pthread_mutex_unlock(&mutex_cola_ready);

					sem_post(&hay_proceso_en_ready);
					break;
				case -1:
					log_error(logger, "El CPU Dispatch se desconecto");
					exit(EXIT_FAILURE);
				default:
					log_warning(logger,"Operacion desconocida. No quieras meter la pata");
					break;
				}
			}
		else{
			buffer = recibir_buffer(kernel_cliente_dispatch);
			eliminar_buffer(buffer);
		}
		
	} 

nodo_de_diccionario_interfaz* comprobrar_existencia_de_interfaz(pcb* el_pcb, char* interfaz,char* tipo_interfaz){
	bool tiene_la_interfaz;

	pthread_mutex_lock(&mutex_para_diccionario_entradasalida);
	tiene_la_interfaz = dictionary_has_key(diccionario_entrada_salida,interfaz);
	pthread_mutex_unlock(&mutex_para_diccionario_entradasalida);

	if(tiene_la_interfaz){

		pthread_mutex_lock(&mutex_para_diccionario_entradasalida);
		nodo_de_diccionario_interfaz* nodo_de_interfaz = dictionary_get(diccionario_entrada_salida,interfaz);
		pthread_mutex_unlock(&mutex_para_diccionario_entradasalida);

		if(nodo_de_interfaz != NULL && strcmp(nodo_de_interfaz->tipo_de_interfaz,tipo_interfaz)==0){
			el_pcb->estado_proceso = BLOCKED;
			log_info(logger_obligatorio,"PID: %d - Estado Anterior: EXECUTE - Estado Actual: BLOCKED", el_pcb->PID);
			log_info(logger_obligatorio, "PID: %d - Bloqueado por: %s",el_pcb ->PID, interfaz);
			strcpy(el_pcb ->interfaz_bloqueante,interfaz);

			pthread_mutex_lock(&mutex_para_diccionario_blocked);
			nodo_de_diccionario_blocked* nodo_bloqueados = dictionary_get(diccionario_blocked,interfaz);
			pthread_mutex_unlock(&mutex_para_diccionario_blocked);

			pthread_mutex_lock(&(nodo_bloqueados ->mutex_para_cola_bloqueados));
			queue_push(nodo_bloqueados ->cola_bloqueados,el_pcb);
			pthread_mutex_unlock(&(nodo_bloqueados ->mutex_para_cola_bloqueados));
			return nodo_de_interfaz;
		}
	}
	el_pcb ->razon_salida = INTERFAZ_INVALIDA;

	log_info(logger_obligatorio, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: EXIT", el_pcb ->PID);
	mandar_a_exit(el_pcb);
	

	return NULL;
}

io_std_fs* extraer_dir_fisicas_de_buffer(t_buffer* buffer){
	io_std_fs* conjunto_de_dir_fisicas = malloc(sizeof(io_std_fs));
	conjunto_de_dir_fisicas ->interfaz = extraer_string_buffer(buffer, logger);
	conjunto_de_dir_fisicas ->tam = extraer_int_buffer(buffer, logger);
	conjunto_de_dir_fisicas ->cant_dir_fisicas = extraer_int_buffer(buffer, logger);

	conjunto_de_dir_fisicas ->lista_dir_fisicas = list_create();

	for(int i = 0; i < conjunto_de_dir_fisicas ->cant_dir_fisicas; i++){
		dir_fis_y_tam* dir_fisica_y_tam = malloc(sizeof(dir_fis_y_tam));
		dir_fisica_y_tam ->dir_fisica = extraer_int_buffer(buffer, logger);
		dir_fisica_y_tam ->tam = extraer_int_buffer(buffer, logger);
		list_add(conjunto_de_dir_fisicas ->lista_dir_fisicas, dir_fisica_y_tam);
	}	

	return conjunto_de_dir_fisicas;
}



void recibir_contexto_de_ejecucion(t_buffer* buffer,pcb* el_pcb) {
	*el_pcb->registros_cpu_en_pcb->PC = extraer_uint32_buffer(buffer, logger);
	*el_pcb->registros_cpu_en_pcb->AX = extraer_uint8_buffer(buffer, logger);	
	*el_pcb->registros_cpu_en_pcb->BX = extraer_uint8_buffer(buffer, logger);
	*el_pcb->registros_cpu_en_pcb->CX = extraer_uint8_buffer(buffer, logger);
	*el_pcb->registros_cpu_en_pcb->DX = extraer_uint8_buffer(buffer, logger);
	*el_pcb->registros_cpu_en_pcb->EAX = extraer_uint32_buffer(buffer, logger);
	*el_pcb->registros_cpu_en_pcb->EBX = extraer_uint32_buffer(buffer, logger);
	*el_pcb->registros_cpu_en_pcb->ECX = extraer_uint32_buffer(buffer, logger);
	*el_pcb->registros_cpu_en_pcb->EDX = extraer_uint32_buffer(buffer, logger);
	*el_pcb->registros_cpu_en_pcb->SI = extraer_uint32_buffer(buffer, logger);
	*el_pcb->registros_cpu_en_pcb->DI = extraer_uint32_buffer(buffer, logger);
}


