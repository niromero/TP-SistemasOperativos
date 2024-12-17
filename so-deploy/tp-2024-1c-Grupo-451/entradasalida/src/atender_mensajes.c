#include <atender_mensajes.h>

bool confirmacion_escritura(){
	t_buffer* buffer;
	int cod_op = recibir_operacion(entradasalida_cliente_memoria);
	switch (cod_op) {
		case ESCRITURA_CODE:
			buffer = recibir_buffer(entradasalida_cliente_memoria);
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

void* recibir_lectura(){
	t_buffer* buffer;
	int cod_op = recibir_operacion(entradasalida_cliente_memoria);
	switch (cod_op) {
		case LECTURA_CODE:
			buffer = recibir_buffer(entradasalida_cliente_memoria);
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