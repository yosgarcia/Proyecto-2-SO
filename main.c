#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"

int arreglo_frecuencias [256]; 


void mostrarByteEnBinario(unsigned char byte){
    for (int i = 7; i >= 0; i--){
        printf("%d", (byte >> i) & 1);
    }
}
void imprimir_bytes_archivo(FILE* archivo){
    fseek(archivo, 0, SEEK_END);
    unsigned long tamanoArchivo = ftell(archivo);
    rewind(archivo);

    unsigned char *buffer = (unsigned char*) malloc(sizeof(unsigned char) * tamanoArchivo);
    if (buffer == NULL) {
        printf("No se pudo asignar memoria para leer el archivo\n");
        fclose(archivo);
        return;
    }

    size_t bytesLeidos = fread(buffer, sizeof(unsigned char), tamanoArchivo, archivo);
    if (bytesLeidos != tamanoArchivo) {
        printf("Error al leer el archivo\n");
        free(buffer);
        fclose(archivo);
        return;
    }

    // Mostrar los bytes leídos (en formato hexadecimal)
    for (unsigned long i = 0; i < tamanoArchivo; i++) {
        mostrarByteEnBinario(buffer[i]);
        printf(" ");
    }
    printf("\n");
}
//archivoprueba.txt
int main(){
    const char* nombre_archivo = "archivoprueba.txt";
    FILE *archivo = fopen(nombre_archivo, "rb");
    /*
    const char* nombre_archivo = "comprimido";
    FILE *archivo = fopen(nombre_archivo, "rb");
    imprimir_bytes_archivo(archivo);
    return 0;
    */
    if(archivo == NULL){
        printf("No se puedo abrir el archivo %s\n", nombre_archivo);
        return 1;
    }

    fseek(archivo, 0, SEEK_END);
    unsigned long tamanoArchivo = ftell(archivo);
    rewind(archivo);

    unsigned char *buffer = (unsigned char*) malloc(sizeof(unsigned char) * tamanoArchivo);
    if (buffer == NULL) {
        printf("No se pudo asignar memoria para leer el archivo\n");
        fclose(archivo);
        return 1;
    }
    size_t bytesLeidos = fread(buffer, sizeof(unsigned char), tamanoArchivo, archivo);
    if (bytesLeidos != tamanoArchivo) {
        printf("Error al leer el archivo\n");
        free(buffer);
        fclose(archivo);
        return 1;
    }
    // Mostrar los bytes leídos (en formato hexadecimal)
    for (unsigned long i = 0; i < tamanoArchivo; i++) {
        //mostrarByteEnBinario(buffer[i]);
        //printf(" ");
        arreglo_frecuencias[buffer[i]] ++;
        
    }
    
    //imprimir el arreglo frecuencias
    for (int i =0; i<256;i++){
        //printf("Arreglo[%d] = %d \n",i,arreglo_frecuencias[i]);
    }
    // Liberar la memoria y cerrar el archivo
    
    free(buffer);
    fclose(archivo);

    NodoHuff* raiz = crear_arbol_Huffman(arreglo_frecuencias);
    
    imprimir_arbol_huffman(raiz,0);
    comprimir_archivo(raiz,nombre_archivo,"comprimido");
    return 0;    
}