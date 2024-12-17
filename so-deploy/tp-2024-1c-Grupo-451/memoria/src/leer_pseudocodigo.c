#include <leer_pseudocodigo.h>

int leer_archivo(char *nombArch, t_dictionary* diccionario,char* clave_pid){
    pthread_mutex_lock(&mutex_para_leer_pseudo);
    char * archivo = strdup(PATH_INSTRUCCIONES);
    string_append(&archivo, nombArch);
    FILE* archivo_pseudo = fopen(archivo, "r");
    if(archivo_pseudo != NULL){
        int programCounter = 0;
        t_list* lista_de_intrucciones = list_create();
        while (!feof(archivo_pseudo))
        {
        
            char instruccion[256];

            fgets(instruccion,256,archivo_pseudo);
            int i = strlen(instruccion);

            char* a_guardar = malloc(sizeof(char)*i + 1);
            strcpy(a_guardar,instruccion);

            list_add(lista_de_intrucciones,a_guardar);


        
        }

        pthread_mutex_lock(&mutex_para_diccionario_instrucciones);
        dictionary_put(diccionario_de_instrucciones,clave_pid,lista_de_intrucciones);
        pthread_mutex_unlock(&mutex_para_diccionario_instrucciones);

        fclose(archivo_pseudo);
        free(archivo);
        pthread_mutex_unlock(&mutex_para_leer_pseudo);
        return programCounter;
    }
    else{
       pthread_mutex_unlock(&mutex_para_leer_pseudo);
       free(archivo);
       return -1; 
    }
    
    
}