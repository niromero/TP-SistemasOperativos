#include <atender_mensajes.h>

void atender_cpu_memoria(){
	t_buffer* buffer;
	char* mensaje;
    while (1) {
		//log_info(logger, "Esperando mensajes de CPU");
		int cod_op = recibir_operacion(cpu_cliente);
		buffer = recibir_buffer(cpu_cliente);
		switch (cod_op) {
		case HANDSHAKE:
			usleep(RETARDO_RESPUESTA * 1000);
			mensaje = extraer_string_buffer(buffer, logger);
			printf("Recibi un handshake de: %s, como cliente",mensaje);
			free(mensaje);
			t_paquete* paquete = crear_paquete(TAM_DE_PAG_CODE);
			agregar_int_a_paquete(paquete,TAM_PAGINA);
			enviar_paquete(paquete,cpu_cliente);
			eliminar_paquete(paquete);
			break;
		case PROTOCOLO:
			usleep(RETARDO_RESPUESTA * 1000);
			int num = extraer_int_buffer(buffer, logger);
			mensaje = extraer_string_buffer(buffer, logger);
			printf("El numero es: %d \n", num);
			printf("%s", mensaje);
			free(mensaje);
			break;
		case PEDIR_INSTRUCCION:
			usleep(RETARDO_RESPUESTA * 1000);
			int pid = extraer_int_buffer(buffer,logger);
			int pc = extraer_int_buffer(buffer,logger);
			enviar_instruccion(pc,pid);
			break;
		case PEDIR_MARCO:
			usleep(RETARDO_RESPUESTA * 1000);
			int pid2 = extraer_int_buffer(buffer,logger);
			int num_pag = extraer_int_buffer(buffer,logger);

			char* pid_clave = string_itoa(pid2);

			pthread_mutex_lock(&mutex_para_diccionario_tdp);
			nodo_dic_tdp* nodo_tdp = dictionary_get(diccionario_de_tdp,pid_clave);
			pthread_mutex_unlock(&mutex_para_diccionario_tdp);

			int* marco = list_get(nodo_tdp-> tdp_del_proceso,num_pag);

			t_paquete* paquete2 = crear_paquete(PEDIR_MARCO);
			agregar_int_a_paquete(paquete2,*marco);
			enviar_paquete(paquete2,cpu_cliente);
			eliminar_paquete(paquete2);

			free(pid_clave);
			log_info(logger_obligatorio,"PID: %d - Pagina: %d - Marco: %d",pid2, num_pag, *marco);
			break;
		
		case RESIZE_CODE:
			usleep(RETARDO_RESPUESTA * 1000);
			int pid_a_cambiar_tam = extraer_int_buffer(buffer,logger);
			int nuevo_tam_proceso = extraer_int_buffer(buffer,logger);

			int respuesta = cambiar_memoria_de_proceso(pid_a_cambiar_tam, nuevo_tam_proceso);

			
			t_paquete* paquete3 = crear_paquete(RESIZE_CODE);
			agregar_int_a_paquete(paquete3,respuesta);
			enviar_paquete(paquete3,cpu_cliente);
			eliminar_paquete(paquete3);

			break;
		case LECTURA_CODE:
			usleep(RETARDO_RESPUESTA * 1000);
			int pid_de_proceso_lectura = extraer_int_buffer(buffer, logger);
			int dir_fisica = extraer_int_buffer(buffer, logger);
			int tam_a_leer = extraer_int_buffer(buffer,logger);

			void* leido = leer_dir_fisica(dir_fisica,tam_a_leer);

			log_info(logger_obligatorio, "PID: %d - Accion: LEER - Direccion fisica: %d - Tamaño %d", pid_de_proceso_lectura, dir_fisica, tam_a_leer);

			t_paquete* paquete4 = crear_paquete(LECTURA_CODE);
			agregar_a_paquete(paquete4,leido,tam_a_leer);
			enviar_paquete(paquete4,cpu_cliente);
			eliminar_paquete(paquete4);
			free(leido);
			break;

		case ESCRITURA_CODE:
			usleep(RETARDO_RESPUESTA * 1000);
			int pid_del_proceso_escritura = extraer_int_buffer(buffer, logger);
			int dir_fisica_escritura = extraer_int_buffer(buffer, logger);
			int tamano_a_escribir = extraer_int_buffer(buffer,logger);

			void* a_escribir = extraer_contenido_buffer(buffer,logger);

			pthread_mutex_lock(&mutex_para_mem_de_usuario);
			memcpy(memoria_de_usuario + dir_fisica_escritura, a_escribir,tamano_a_escribir);
			pthread_mutex_unlock(&mutex_para_mem_de_usuario);

			free(a_escribir);

			log_info(logger_obligatorio, "PID: %d - Accion: ESCRIBIR - Direccion fisica: %d - Tamaño %d", pid_del_proceso_escritura, dir_fisica_escritura, tamano_a_escribir);

			t_paquete* paquete5 = crear_paquete(ESCRITURA_CODE);
			char* mensaje = "Ok";
			agregar_string_a_paquete(paquete5,mensaje);
			enviar_paquete(paquete5, cpu_cliente);
			eliminar_paquete(paquete5);
			break;
		case -1:
			log_error(logger, "El CPU se desconecto");
			exit(EXIT_FAILURE);
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
}

void enviar_instruccion(int pc,int pid){
	t_paquete* paquete = crear_paquete(PEDIR_INSTRUCCION);
	char* pid_clave = string_itoa(pid);

	pthread_mutex_lock(&mutex_para_diccionario_instrucciones);
	t_list* lista_instrucciones = dictionary_get(diccionario_de_instrucciones,pid_clave);
	pthread_mutex_unlock(&mutex_para_diccionario_instrucciones);

	char * instruccion = list_get(lista_instrucciones,pc);
	agregar_string_a_paquete(paquete, instruccion);
	enviar_paquete(paquete, cpu_cliente);
    eliminar_paquete(paquete);

	free(pid_clave);
}

void atender_kernel_memoria(){
    while (1) {
        log_info(logger, "Esperando mensajes de Kernel");
		int cod_op = recibir_operacion(kernel_cliente);
		t_buffer* buffer;
		switch (cod_op) {
		case HANDSHAKE:
			buffer = recibir_buffer(kernel_cliente);
			usleep(RETARDO_RESPUESTA * 1000);
			char* mensaje = extraer_string_buffer(buffer, logger);
			printf("Recibi un handshake de: %s, como cliente",mensaje);
			free(mensaje);
			break;
		case CREAR_PROCESO:
			buffer = recibir_buffer(kernel_cliente);

			usleep(RETARDO_RESPUESTA * 1000);

			int pid = extraer_int_buffer(buffer,logger);
			char* clave_pid = string_itoa(pid);
			char* ruta_pseudocodigo = extraer_string_buffer(buffer,logger);
			int programCounter = leer_archivo(ruta_pseudocodigo,diccionario_de_instrucciones,clave_pid);
			crear_tdp_del_proceso(clave_pid);
			free(clave_pid);
			enviar_program_counter(programCounter);
			free(ruta_pseudocodigo);
			break;
		case ELIMINAR_PROCESO_MEMORIA:
			buffer = recibir_buffer(kernel_cliente);
			usleep(RETARDO_RESPUESTA * 1000);
			int pid2 = extraer_int_buffer(buffer,logger);
			char* pid_clave = string_itoa(pid2);

			pthread_mutex_lock(&mutex_para_diccionario_instrucciones);
			t_list* lista_instrucciones = dictionary_remove(diccionario_de_instrucciones,pid_clave);
			pthread_mutex_unlock(&mutex_para_diccionario_instrucciones);
			
			for(int i = 0; i<list_size(lista_instrucciones); i++){
				void* instruccion = list_get(lista_instrucciones,i);
				free(instruccion);
			}
			list_destroy(lista_instrucciones);

			pthread_mutex_lock(&mutex_para_diccionario_tdp);
			nodo_dic_tdp* nodo_tdp = dictionary_get(diccionario_de_tdp, pid_clave);
			pthread_mutex_unlock(&mutex_para_diccionario_tdp);

			
			int cant_paginas = list_size(nodo_tdp ->tdp_del_proceso);
			

			for(int i = cant_paginas - 1; i >=0; i--){
				
            	int* marco = list_remove(nodo_tdp ->tdp_del_proceso,i);

            	pthread_mutex_lock(&mutex_para_marcos_libres);
            	bitarray_clean_bit(marcos_de_memoria_libres,*marco);
            	pthread_mutex_unlock(&mutex_para_marcos_libres);

            	free(marco);

        	}

			list_destroy(nodo_tdp -> tdp_del_proceso);
			log_info(logger_obligatorio, "PID: %s - Tamaño: %d", pid_clave, cant_paginas);

			free(nodo_tdp);
			free(pid_clave);
			break;
		case -1:
			log_error(logger, "El Kernel se desconecto");
			exit(EXIT_FAILURE);
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
}

void enviar_program_counter(int pc){
	t_paquete* paquete = crear_paquete(CREAR_PROCESO);
	agregar_int_a_paquete(paquete,pc);
	enviar_paquete(paquete,kernel_cliente);
	eliminar_paquete(paquete);
}

void crear_tdp_del_proceso(char* clave_pid){
	nodo_dic_tdp* nodo_tdp = malloc(sizeof(nodo_dic_tdp));
	nodo_tdp ->tdp_del_proceso = list_create();
	nodo_tdp ->tam_de_proceso = 0;

	pthread_mutex_lock(&mutex_para_diccionario_tdp);
	dictionary_put(diccionario_de_tdp,clave_pid,nodo_tdp);
	pthread_mutex_unlock(&mutex_para_diccionario_tdp);
	log_info(logger_obligatorio, "PID: %s - Tamaño: 0", clave_pid);
}

void atender_entradasalida_memoria(void* cliente){
	int* cliente_entrada_salida = cliente;
	bool continuar_while = true;
    while (continuar_while) {
        log_info(logger, "Esperando mensajes de Entrada/Salida");
		int cod_op = recibir_operacion(*cliente_entrada_salida);
		t_buffer* buffer;
		switch (cod_op) {
		case HANDSHAKE:
			buffer = recibir_buffer(*cliente_entrada_salida);
			usleep(RETARDO_RESPUESTA * 1000);
			char* mensaje = extraer_string_buffer(buffer, logger);
			printf("Recibi un handshake de: %s, como cliente",mensaje);
			free(mensaje);
			break;
		case ESCRITURA_CODE:
			buffer = recibir_buffer(*cliente_entrada_salida);
			usleep(RETARDO_RESPUESTA * 1000);
			int pid_del_proceso_escritura = extraer_int_buffer(buffer, logger);
			int dir_fisica_escritura = extraer_int_buffer(buffer, logger);
			int tamano_a_escribir = extraer_int_buffer(buffer,logger);

			void* a_escribir = extraer_contenido_buffer(buffer,logger);

			pthread_mutex_lock(&mutex_para_mem_de_usuario);
			memcpy(memoria_de_usuario + dir_fisica_escritura, a_escribir,tamano_a_escribir);
			pthread_mutex_unlock(&mutex_para_mem_de_usuario);

			free(a_escribir);

			log_info(logger_obligatorio, "PID: %d - Accion: ESCRIBIR - Direccion fisica: %d - Tamaño %d", pid_del_proceso_escritura, dir_fisica_escritura, tamano_a_escribir);

			t_paquete* paquete5 = crear_paquete(ESCRITURA_CODE);
			char* mensaje1 = "Ok";
			agregar_string_a_paquete(paquete5,mensaje1);
			enviar_paquete(paquete5, *cliente_entrada_salida);
			eliminar_paquete(paquete5);
			break;
		
		case LECTURA_CODE:
			buffer = recibir_buffer(*cliente_entrada_salida);
			usleep(RETARDO_RESPUESTA * 1000);
			int pid_de_proceso_lectura = extraer_int_buffer(buffer, logger);
			int dir_fisica = extraer_int_buffer(buffer, logger);
			int tam_a_leer = extraer_int_buffer(buffer,logger);

			void* leido = leer_dir_fisica(dir_fisica,tam_a_leer);

			log_info(logger_obligatorio, "PID: %d - Accion: LEER - Direccion fisica: %d - Tamaño %d", pid_de_proceso_lectura, dir_fisica, tam_a_leer);

			t_paquete* paquete4 = crear_paquete(LECTURA_CODE);
			agregar_a_paquete(paquete4,leido,tam_a_leer);
			enviar_paquete(paquete4,*cliente_entrada_salida);
			eliminar_paquete(paquete4);
			free(leido);
			break;

		case -1:
			log_info(logger, "La Entrada/Salida se desconecto");
			continuar_while = false;
			free(cliente_entrada_salida);
			break;
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
}

