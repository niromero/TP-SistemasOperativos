#include <MMU.h>

int solicitar_marco(int pid, int num_de_pag){

    int marco;

    if(CANTIDAD_ENTRADAS_TLB <= 0){
        t_paquete* paquete = crear_paquete(PEDIR_MARCO);
        agregar_int_a_paquete(paquete, pid);
        agregar_int_a_paquete(paquete, num_de_pag);
        enviar_paquete(paquete,cpu_cliente_memoria);
        eliminar_paquete(paquete);
        marco = recibir_marco();
        log_info(logger_obligatorio, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pid, num_de_pag, marco);
    }
    else{
        marco = buscar_en_tlb(pid, num_de_pag);

        if(marco < 0){
            t_paquete* paquete = crear_paquete(PEDIR_MARCO);
            agregar_int_a_paquete(paquete, pid);
            agregar_int_a_paquete(paquete, num_de_pag);
            enviar_paquete(paquete,cpu_cliente_memoria);
            eliminar_paquete(paquete);
            marco = recibir_marco();
            log_info(logger_obligatorio, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pid, num_de_pag, marco);
            agregar_en_tlb(pid, num_de_pag, marco);
        }
    }

    return marco;
}

direccion_fisica* traducir_dir_logica(int pid ,int dir_logica){
    direccion_fisica* nueva_dir_fisica = malloc(sizeof(direccion_fisica));

    float dir_log_float = dir_logica;
    int num_de_pag = floor(dir_log_float / tam_de_pags_memoria);
    nueva_dir_fisica ->desplazamiento = dir_logica - num_de_pag * tam_de_pags_memoria;

    nueva_dir_fisica ->marco = solicitar_marco(pid,num_de_pag);

    nueva_dir_fisica ->base = nueva_dir_fisica ->marco * tam_de_pags_memoria;

    nueva_dir_fisica ->dir_fisica_final = nueva_dir_fisica ->base + nueva_dir_fisica ->desplazamiento;

    nueva_dir_fisica ->num_de_pag_base = num_de_pag;

    return nueva_dir_fisica;
}

int buscar_en_tlb(int pid, int num_pag){

    for(int i = 0; i< list_size(tlb); i++){
        entradas_tlb* entrada = list_get(tlb, i);

        if(entrada ->pid == pid && entrada ->num_pag == num_pag){
            log_info(logger_obligatorio, "PID: %d - TLB HIT - Pagina: %d", pid, num_pag);
            entrada ->ultima_vez_usada = time(NULL);
            return entrada ->marco;
        }
    }
    log_info(logger_obligatorio, "PID: %d - TLB MISS - Pagina: %d", pid, num_pag);
    return -1;
}

void agregar_en_tlb(int pid, int num_pag, int marco){

    entradas_tlb* nueva_entrada = malloc(sizeof(entradas_tlb));
    nueva_entrada ->marco = marco;
    nueva_entrada ->num_pag = num_pag;
    nueva_entrada ->pid = pid;
    nueva_entrada ->ultima_vez_usada = time(NULL);

    if(CANTIDAD_ENTRADAS_TLB > list_size(tlb)){
        list_add(tlb,nueva_entrada);
    }
    else if(strcmp(ALGORITMO_TLB, "fifo")== 0){
        entradas_tlb* entrada_a_remplazar = list_remove(tlb,0);
        log_info(logger_obligatorio, "PID ANTIGUO: %d - PAGINA ANTIGUA: %d - MARCO ANTIGUO: %d - PID NUEVO: %d - PAGINA NUEVA: %d - MARCO NUEVO: %d ", entrada_a_remplazar ->pid, entrada_a_remplazar ->num_pag, 
        entrada_a_remplazar ->marco, nueva_entrada ->pid, nueva_entrada ->num_pag, nueva_entrada ->marco);
        free(entrada_a_remplazar);
        list_add(tlb, nueva_entrada);
    }
    else{
        int posicion_a_remplazar = 0;
        entradas_tlb* entrada_a_remplazar = list_get(tlb, 0);

        for(int i = 1; i<list_size(tlb); i++){
            entradas_tlb* otra_entrada = list_get(tlb,i);

            if(entrada_a_remplazar->ultima_vez_usada > otra_entrada ->ultima_vez_usada){
                entrada_a_remplazar = otra_entrada;
                posicion_a_remplazar = i;
            }
        }

        list_remove(tlb,posicion_a_remplazar);
        log_info(logger_obligatorio, "PID ANTIGUO: %d - PAGINA ANTIGUA: %d - MARCO ANTIGUO: %d - PID NUEVO: %d - PAGINA NUEVA: %d - MARCO NUEVO: %d ", entrada_a_remplazar ->pid, entrada_a_remplazar ->num_pag, 
        entrada_a_remplazar ->marco, nueva_entrada ->pid, nueva_entrada ->num_pag, nueva_entrada ->marco);
        free(entrada_a_remplazar);
        list_add(tlb, nueva_entrada);
    }
}



