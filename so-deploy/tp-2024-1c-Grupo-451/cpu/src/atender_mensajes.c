#include <atender_mensajes.h>

// Atender mensajes enviados por el Kernel



void recibir_contexto_de_CPU(t_buffer* buffer) {
	buffer = recibir_buffer(kernel_cliente_dispatch);
	pid_en_ejecucion = extraer_int_buffer(buffer,logger);
	*los_registros_de_la_cpu->PC = extraer_uint32_buffer(buffer, logger);
	*los_registros_de_la_cpu->AX = extraer_uint8_buffer(buffer, logger);	
	*los_registros_de_la_cpu->BX = extraer_uint8_buffer(buffer, logger);
	*los_registros_de_la_cpu->CX = extraer_uint8_buffer(buffer, logger);
	*los_registros_de_la_cpu->DX = extraer_uint8_buffer(buffer, logger);
	*los_registros_de_la_cpu->EAX = extraer_uint32_buffer(buffer, logger);
	*los_registros_de_la_cpu->EBX = extraer_uint32_buffer(buffer, logger);
	*los_registros_de_la_cpu->ECX = extraer_uint32_buffer(buffer, logger);
	*los_registros_de_la_cpu->EDX = extraer_uint32_buffer(buffer, logger);
	*los_registros_de_la_cpu->SI = extraer_uint32_buffer(buffer, logger);
	*los_registros_de_la_cpu->DI = extraer_uint32_buffer(buffer, logger);

	
}



void atender_kernel_dispatch_sin_while(){
	log_info(logger, "Esperando mensajes de Kernel");
	t_buffer* buffer = NULL;
	int cod_op = recibir_operacion(kernel_cliente_dispatch);
	switch (cod_op) {
	case HANDSHAKE:
		buffer = recibir_buffer(kernel_cliente_dispatch);
		char* mensaje = extraer_string_buffer(buffer, logger);
		printf("Recibi un handshake de %s, como cliente", mensaje);
		free(mensaje);
		break;
	case INICIAR_EXEC:
		recibir_contexto_de_CPU(buffer);
		break;
	case -1:
		log_error(logger, "El Kernel Dispatch se desconecto");
		exit(EXIT_FAILURE);
	default:
		log_warning(logger,"Operacion desconocida. No quieras meter la pata");
		break;
	}
}

void atender_kernel_interrupt(){
    while (1) {
        log_info(logger, "Esperando mensajes de de Kernel cliente interrupt");
		int cod_op = recibir_operacion(kernel_cliente_interrupt);
		t_buffer* buffer = NULL;
		switch (cod_op) {
		case HANDSHAKE:
			buffer = recibir_buffer(kernel_cliente_interrupt);
			char* mensaje = extraer_string_buffer(buffer, logger);
			printf("Recibi un handshake de: %s, como cliente",mensaje);
			free(mensaje);
			break;
		case INTERRUPCION:
			buffer = recibir_buffer(kernel_cliente_interrupt);
			int pid = extraer_int_buffer(buffer,logger);

			pthread_mutex_lock(&mutex_para_pid_interrupcion);
			pid_de_interrupcion = pid;
			pthread_mutex_unlock(&mutex_para_pid_interrupcion);

			pthread_mutex_lock(&mutex_para_interrupcion);
			interrupcion_recibida = HUBO_INTERRUPCION;
			pthread_mutex_unlock(&mutex_para_interrupcion);
			
			break;
		case -1:
			log_error(logger, "El Kernel Interrupt se desconecto");
			exit(EXIT_FAILURE);
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}  
}

void atender_memoria_cpu_sin_while(){
        log_info(logger, "Esperando mensajes de memoria");
		t_buffer* buffer;
		int cod_op = recibir_operacion(cpu_cliente_memoria);
		switch (cod_op) {
		case HANDSHAKE:
			buffer = recibir_buffer(cpu_cliente_memoria);
			char* mensaje = extraer_string_buffer(buffer, logger);
			printf("Recibi un handshake de: %s, como cliente",mensaje);
			free(mensaje);
			break;
		case PEDIR_INSTRUCCION: //CASE para recibir la instruccion de memoria
			buffer = recibir_buffer(cpu_cliente_memoria);
			char* instruccion = extraer_string_buffer(buffer,logger);
			//Libera la anterior instruccion, solicita espacio para la nueva y la almacena
			free(instruccion_a_decodificar); 
			instruccion_a_decodificar = malloc(strlen(instruccion)+1);
			strcpy(instruccion_a_decodificar,instruccion);
			free(instruccion);
			break;
		case TAM_DE_PAG_CODE: //case para recibir el tamaño de las paginas
			buffer = recibir_buffer(cpu_cliente_memoria);
			tam_de_pags_memoria = extraer_int_buffer(buffer,logger);
			break;
		case -1:
			log_error(logger, "La Memoria se desconecto");
			exit(EXIT_FAILURE);
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
}

int recibir_marco(){
	t_buffer* buffer;
		int cod_op = recibir_operacion(cpu_cliente_memoria);
		switch (cod_op) {
		case PEDIR_MARCO:
			buffer = recibir_buffer(cpu_cliente_memoria);
			int marco = extraer_int_buffer(buffer,logger);
			return marco;
			break;
		case -1:
			log_error(logger, "La Memoria se desconecto");
			exit(EXIT_FAILURE);
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	return -1;
}

int confirmacion_resize(){
	t_buffer* buffer;
	int cod_op = recibir_operacion(cpu_cliente_memoria);
	switch (cod_op) {
		case RESIZE_CODE:
			buffer = recibir_buffer(cpu_cliente_memoria);
			int confirmacion = extraer_int_buffer(buffer,logger);
			return confirmacion;
			break;
		case -1:
			log_error(logger, "La Memoria se desconecto");
			exit(EXIT_FAILURE);
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	return 0;
}

void* recibir_lectura(){
	t_buffer* buffer;
	int cod_op = recibir_operacion(cpu_cliente_memoria);
	switch (cod_op) {
		case LECTURA_CODE:
			buffer = recibir_buffer(cpu_cliente_memoria);
			void* leido = extraer_contenido_buffer(buffer, logger);
			return leido;
			break;
		case -1:
			log_error(logger, "La Memoria se desconecto");
			exit(EXIT_FAILURE);
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
	}
}

bool confirmacion_escritura(){
	t_buffer* buffer;
	int cod_op = recibir_operacion(cpu_cliente_memoria);
	switch (cod_op) {
		case ESCRITURA_CODE:
			buffer = recibir_buffer(cpu_cliente_memoria);
			char* confirm = extraer_string_buffer(buffer, logger);

			if(strcmp(confirm, "Ok")==0){
				free(confirm);
				return true;
			}
			
			free(confirm);
			return false;

			break;
		case -1:
			log_error(logger, "La Memoria se desconecto");
			exit(EXIT_FAILURE);
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
	}

	return false;
}


	  

