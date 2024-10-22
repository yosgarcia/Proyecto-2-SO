#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "configuracion.h"

#define MAX_LINEA 100

Config* leer_configuracion(char* nombre_archivo){
    Config* config = calloc(1, sizeof(Config));
    config->archivo_a_comprimir = calloc(50,sizeof(char));
    config->archivo_comprimido = calloc(50,sizeof(char));
    config->archivo_descomprimido = calloc(50,sizeof(char));
    config->archivo_tabla_huff = calloc(50,sizeof(char));
    FILE* archivo = fopen(nombre_archivo, "r");
    if(archivo == NULL){
        printf("Error al abrir el archivo de configuracion\n");
        exit(1);
    }
    int ips_leidas = -1;
    char linea[MAX_LINEA];
    while(fgets(linea, MAX_LINEA, archivo) != NULL){
        if(sscanf(linea, "cantidad_de_servidores = %lu", &config->num_servidores) == 1){
            config->ips = calloc(config->num_servidores,sizeof(char*));
            continue;
        } 

        if(sscanf(linea, "nombre_archivo_a_comprimir = %s", config->archivo_a_comprimir) == 1) continue;

        if(sscanf(linea, "nombre_archivo_comprimido = %s", config->archivo_comprimido) == 1) continue;

        if(sscanf(linea, "nombre_archivo_descomprimido = %s", config->archivo_descomprimido) == 1) continue;

        if(sscanf(linea, "nombre_archivo_tabla_huff = %s", config->archivo_tabla_huff) == 1) continue;

        if (ips_leidas == -1){
            ips_leidas++;
            continue;
        }        
        //leyendo ip
        char* ip = calloc(11,sizeof(char));
        for (int i=0;i<11;i++){
            if (linea[i] == '\0') continue;
            if (linea[i] == '\n') linea[i] = '\0';
        }
        strcpy(ip,linea);
        config->ips[ips_leidas] = ip;
        ips_leidas++;     
    }
    fclose(archivo);
    return config;
}