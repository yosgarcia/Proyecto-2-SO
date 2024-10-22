
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "descompresor.h"
#include "huffman.h"
#include "configuracion.h"



#define DERECHA '1'
#define IZQUIEDA '0'
#define STRING_ARCHIVO_TABLA "tabla.txt"
#define STRING_ARCHIVO_COMPRIDO "archivo_comprimido"

Config* config;

void agregar_nodo_arbol(NodoHuff* raiz, unsigned char simbolo, char* codigo, int pos_codigo ){
    int tamanno_codigo = strlen(codigo);
    //estoy nodo final, ejemplo camino = 1, y estoy en raiz (vacia)
    if (pos_codigo == tamanno_codigo - 1){
        if (codigo[pos_codigo] == DERECHA){
            NodoHuff* nodo_nvo = crear_nodo_huffman(simbolo,-1);    
            raiz->derecha = nodo_nvo;
            return;
        }
        else {
            NodoHuff* nodo_nvo = crear_nodo_huffman(simbolo,-2);
            raiz->izquierda = nodo_nvo;
            return;
        }
    }
    //No estoy en el final
    //voy a derecha
    if(codigo[pos_codigo] == DERECHA){
        if(raiz->derecha == NULL){
            raiz->derecha = crear_nodo_huffman('\0',-3);
        }
        agregar_nodo_arbol(raiz->derecha, simbolo,codigo,pos_codigo+1);
        return;
    }
    //voy a la izquierda
    if(raiz->izquierda == NULL){
        raiz->izquierda = crear_nodo_huffman('\0',-3);
    }
    agregar_nodo_arbol(raiz->izquierda, simbolo,codigo,pos_codigo+1);
}


NodoHuff* generar_arbol_desde_tabla(char** tabla_codigo){
    NodoHuff* raiz = calloc(1,sizeof(NodoHuff));
    for (int i =0;i<256;i++){
        if (tabla_codigo[i] != NULL){
            agregar_nodo_arbol(raiz, i,tabla_codigo[i],0);
        }
    }
    return raiz;
}
/*
bits sobrantes
codigo
codigo
....
*/

void clean_buffer(char* buffer){
    for (int i =0;i<256;i++){
        buffer[i] = '\0';
    }
}

char** generar_tabla_desde_archivo(FILE* archivo_tabla){
    char** tabla_codigos = calloc(256,sizeof(char*));
    char buffer[256]; 
    for (int i =0;i<256;i++){
        if (fgets(buffer,sizeof(buffer),archivo_tabla) !=NULL ){}
        if (strlen(buffer) == 1){ //solo cambio de linea
            buffer[0] = '\0';
            continue;
        }
        //quitar el cambio de linea
        if (buffer[strlen(buffer) - 1] == '\n'){
            buffer[strlen(buffer) - 1] = '\0';
        }
        int cant_chars = strlen(buffer);
        char* codigo = calloc(cant_chars,sizeof(char));
        strcpy(codigo,buffer);
        tabla_codigos[i] = codigo;
    }
    return tabla_codigos;
}

int char_to_int(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else {
        return -1;
    }
}


int leer_bit(FILE *archivo_input, int *bitBuffer, int *contador_bits) {
    // Si no quedan bits en el buffer, leer un nuevo byte del archivo
    if (*contador_bits == 0) {
        *bitBuffer = fgetc(archivo_input);  // Leer un byte
        if (*bitBuffer == EOF) {
            return -1;  // Fin del archivo
        }
        *contador_bits = 8;  // Reiniciar el contador a 8 bits
    }

    // Extraer el bit más significativo del buffer
    int bit = (*bitBuffer >> 7) & 1;

    // Desplazar el buffer hacia la izquierda y reducir el contador de bits
    *bitBuffer <<= 1;
    (*contador_bits)--;

    return bit;
}
/*
    -Asegurarse de que el archivo salida no exista al ejecutarse
*/
void descomprimir(char* nombre_entrada, char* nombre_salida, char* nombre_tabla){ 
    FILE* archivo_input = fopen(nombre_entrada, "rb");
    FILE* archivo_output = fopen(nombre_salida, "wb");
    FILE* archivo_tabla = fopen(nombre_tabla,"r");
    if(archivo_input == NULL || archivo_output == NULL || archivo_tabla == NULL){
        printf("Error al abrir los archivos\n");
        return;
    }
    //archivos abiertos
    int bits_sobrantes; // por ejemplo, el comprido es 1111 1111 1111, se agregaron 0000 que hay que ignorar
    char buffer_1[4];
    if (fgets(buffer_1,sizeof(buffer_1),archivo_tabla) != NULL) {}
    bits_sobrantes = char_to_int(buffer_1[0]);

    char** tabla_codigos = generar_tabla_desde_archivo(archivo_tabla);
    //imprimir_tabla(tabla_codigos);
    
    NodoHuff* raiz = generar_arbol_desde_tabla(tabla_codigos);
    NodoHuff* nodo_act = raiz;

    int byte;  // Variable para almacenar el byte leído
    int byte2;
    byte = fgetc(archivo_input);

    while (byte != EOF) {  // Leer byte por byte
        byte2 = fgetc(archivo_input);
        int extras = 0;
        //Ultimo byte
        
        if (feof(archivo_input)){
            //Sobraron (5) 1111 1   000  
            //             7654 3   210
            // 8-5 = 3 
            extras = bits_sobrantes;
        }
        // Recorrer el byte de izquierda a derecha (de 7 a 0)
        for (int i = 7; i >= 0+extras; i--) {
            // Usar una máscara para verificar el bit en la posición i
            if (byte & (1 << i)) {
                // Si el bit es 1, derecha
                nodo_act = nodo_act-> derecha;
                //printf("Derecha ");
                if (es_hoja(nodo_act)){
                    //printf("Simbolo: %c\n", nodo_act->simbolo);
                    fputc(nodo_act->simbolo,archivo_output);
                    nodo_act = raiz;
                }
            } else {
                // Si el bit es 0, izquierda
                nodo_act = nodo_act-> izquierda;
                //printf("Izquierda ");
                if (es_hoja(nodo_act)){
                    //printf("Simbolo: %c\n", nodo_act->simbolo);
                    fputc(nodo_act->simbolo,archivo_output);
                    nodo_act = raiz;
                }
            }
        }
        byte = byte2;
    }
    fclose(archivo_input);  // Cerrar el archivo
    fclose(archivo_output);
    return;
}

int main(){
    config = leer_configuracion("configuracion.txt");
    descomprimir(config->archivo_comprimido,config->archivo_descomprimido,config->archivo_tabla_huff); 
    return 0;
}