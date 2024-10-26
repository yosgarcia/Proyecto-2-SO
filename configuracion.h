#ifndef configuracion_h
#define configuracion_h
#include <stdio.h>
#include <stdlib.h>

typedef struct Config{
    unsigned long num_servidores;
    char* archivo_a_comprimir;
    char* archivo_descomprimido;
    char* archivo_comprimido;
    char* archivo_tabla_huff;
    char** ips;
    int* puertos;
} Config;

Config* leer_configuracion(char* nombre_archivo);


#endif