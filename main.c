#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
    const char* nombre_archivo = "archivo.txt";
    FILE *archivo = fopen(nombre_archivo, "rb");

    if(archivo == NULL){
        printf("No se puedo abrir el archivo %s\n", nombre_archivo);
        return 1;
    }

    fseek(archivo, 0, SEEK_END);
    long tamanArchivo = ftell(archivo);
    rewind(archivo);

    
}