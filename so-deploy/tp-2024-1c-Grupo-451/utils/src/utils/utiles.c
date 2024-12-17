#include <utils/utiles.h>

void decir_hola(char* quien) {
    printf("Hola desde %s!!\n", quien);
}

t_config* iniciar_config(char* ruta)
{
	t_config* nuevo_config = config_create(ruta);
	if(nuevo_config == NULL){
		perror("Error al intentar cargar el config");
		exit(EXIT_FAILURE);
	}

	return nuevo_config;
}

t_log* iniciar_logger(char* ruta, char* nombre, int nivel, int mostrar)
{
	t_log* nuevo_logger = log_create(ruta, nombre, mostrar, nivel);

	if(nuevo_logger == NULL){
		printf("Error con el logger");
		exit(1);
	}

	return nuevo_logger;
}

void terminar_programa(t_log* logger_destruir, t_config* config_destruir)
{
	log_destroy(logger_destruir);
	config_destroy(config_destruir);
}

//Funciones de Cliente


int crear_conexion(char *ip, char* puerto) 
{
	int err;
	struct addrinfo hints;
	struct addrinfo *server_info;

	if (ip == NULL || puerto == NULL) {
        fprintf(stderr, "IP o puerto no pueden ser nulos.\n");
        exit(EXIT_FAILURE);
    }

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	err = getaddrinfo(ip, puerto, &hints, &server_info);
	if (err != 0) {
		perror("Error al ejecutar getaddrinfo()");
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

	if (server_info == NULL) {
        fprintf(stderr, "No se pudo obtener la información del servidor.");
		freeaddrinfo(server_info);
        exit(EXIT_FAILURE);
    }

	// Ahora vamos a crear el socket.
	int socket_cliente  = socket(server_info->ai_family,
                    	server_info->ai_socktype,
                    	server_info->ai_protocol);

	if (socket_cliente == -1) {
        perror("Error al crear el socket");
        freeaddrinfo(server_info);
        exit(EXIT_FAILURE);
    }

	// Ahora que tenemos el socket, vamos a conectarlo	
	err = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

	if (err == -1) {
        perror("Error al crear el socket");
        freeaddrinfo(server_info);
        exit(EXIT_FAILURE);
    }
	
	freeaddrinfo(server_info);

	return socket_cliente;
}

//Funciones de Server

int iniciar_servidor(char* puerto_de_escucha, t_log* logger)
{

	int socket_servidor;
	struct addrinfo hints, *servinfo;

	// Cargamos la estructura hints con la familia de direcciones que queremos
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	// Obtenemos la lista de direcciones que cumplen con los parametros de hints
	getaddrinfo(NULL, puerto_de_escucha, &hints, &servinfo); 

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family,
                         servinfo->ai_socktype,
                         servinfo->ai_protocol);

	if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) 
    	error("setsockopt(SO_REUSEADDR) failed");
	// Asociamos el socket a un puerto
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);
	// Escuchamos las conexiones entrantes

	listen(socket_servidor, SOMAXCONN);
	log_info(logger, "Listo para escuchar a mi cliente");

	freeaddrinfo(servinfo);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log* logger, char* mensaje)
{
	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(logger, mensaje);

	return socket_cliente;
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

//-------------------- Protoloco de Comunicacion --------------------//

void enviar_handshake(char* origen, int socket_cliente)
{
	t_paquete* paquete = crear_paquete(HANDSHAKE);
	agregar_a_paquete(paquete, origen, strlen(origen)+1);
    enviar_paquete(paquete, socket_cliente);
    eliminar_paquete(paquete);
	
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(int codigo_de_operacion)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo_de_operacion;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	//Reserva en el stream del buffer el espacio necesario para poder almacenar el nuevo contenido.
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	//Copia primero el tamanio del nuevo contenido, luego el contenido. Se usa algebra de punteros para no pisar lo ya guardado
	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	//Se establece el nuevo tamanio
	paquete->buffer->size += tamanio + sizeof(int);
	return;
}

void agregar_int_a_paquete(t_paquete* paquete, int num){
	agregar_a_paquete(paquete, &num, sizeof(int));
}

void agregar_string_a_paquete(t_paquete* paquete, char* el_String){
	agregar_a_paquete(paquete,el_String,strlen(el_String)+1);
}

void* serializar_paquete(t_paquete* paquete, int tamanio_paquete)
{
	void * paquete_serializado = malloc(tamanio_paquete);
	int desplazamiento = 0;

	//Se copia todo al void*, siguiendo el orden establecido
	memcpy(paquete_serializado + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(paquete_serializado + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(paquete_serializado + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return paquete_serializado;
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{	
	//El tamanio del buffer + el int del codigo de operacion + el int que indica el tamanio del buffer
	int tamanio_paquete = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, tamanio_paquete);

	send(socket_cliente, a_enviar, tamanio_paquete, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	eliminar_buffer(paquete -> buffer);
	free(paquete);
}

void eliminar_buffer(t_buffer* buffer){
	if(buffer -> stream != NULL){
		free(buffer -> stream);
	}
	free(buffer);
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

t_buffer* recibir_buffer(int socket_cliente)
{
	t_buffer* buffer = malloc(sizeof(t_buffer));

	//Se recibe primero el tamanio del buffer, luego el contenido de este

	recv(socket_cliente, &(buffer -> size), sizeof(int), MSG_WAITALL);
	buffer -> stream = malloc(buffer -> size);
	recv(socket_cliente, buffer -> stream, buffer -> size, MSG_WAITALL);

	return buffer;
}

void* extraer_contenido_buffer(t_buffer* buffer, t_log* logger){
	if(buffer -> size <= 0 ){
		log_error(logger, "ERROR, se intento extraer contenido de un buffer vacio");
		exit (EXIT_FAILURE);
	}
	int size_contenido;
	memcpy(&size_contenido, buffer ->stream, sizeof(int)); //Se copia el tamanio de lo que se quiere extraer
	void* contenido = malloc(size_contenido);
	memcpy(contenido, buffer -> stream + sizeof(int), size_contenido); //Se copia el contenido, utilizando el tamanio que copiamos antes

	int nuevo_size = buffer -> size - size_contenido - sizeof(int);
	if(nuevo_size == 0){ //El buffer esta vacio
		eliminar_buffer(buffer); //SE libera el espacio de memoria que tenia el stream del buffer y se pone en NULL el puntero
		return contenido;
	}
	if(nuevo_size <0){
		log_error(logger, "ERROR, el nuevo size quedo negativo");
		exit (EXIT_FAILURE);
	}
	
	//Se reserva un espacio de memoria para el nuevo stream, se copia el stream anterior
	//evitando lo ya extraido y se libera el espacio de memoria que estaba usando el stream anteior

	void* nuevo_stream = malloc(nuevo_size);
	memcpy(nuevo_stream, buffer -> stream + size_contenido + sizeof(int), nuevo_size);
	free(buffer -> stream);
	buffer -> stream = nuevo_stream;
	buffer -> size = nuevo_size;

	return contenido;
}

int extraer_int_buffer(t_buffer* buffer, t_log* logger){
	int * un_int = extraer_contenido_buffer(buffer, logger);
	int numero_a_retornar = *un_int;
	free(un_int);
	return numero_a_retornar;
}

char* extraer_string_buffer(t_buffer* buffer, t_log* logger){
	char* un_string = extraer_contenido_buffer(buffer, logger);
	return un_string;
}

//No hacian falt

uint32_t extraer_uint32_buffer(t_buffer* buffer, t_log* logger){
	uint32_t* un_uint32 = extraer_contenido_buffer(buffer, logger);
	uint32_t numero_a_retornar = *un_uint32;
	free (un_uint32);
	return numero_a_retornar;
}

uint8_t extraer_uint8_buffer(t_buffer* buffer, t_log* logger){
	uint8_t* un_uint8 = extraer_contenido_buffer(buffer, logger);
	uint8_t numero_a_retornar = *un_uint8;
	free (un_uint8);
	return numero_a_retornar;
}




