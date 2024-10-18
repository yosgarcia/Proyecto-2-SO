#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "configuracion.h"

#define MAX_LINEA 100

Config* leer_configuracion(char* nombre_archivo){
    Config* config = calloc(1, sizeof(Config));
    FILE* archivo = fopen(nombre_archivo, "r");
    if (archivo == NULL) {
        printf("Error al abrir el archivo de configuracion\n");
        exit(1);
    }

    char linea[MAX_LINEA];

    while(fgets(linea, MAX_LINEA, archivo) != NULL){
        /* if (sscanf(linea, "longitud_puente = %d", &config->longitud_puente) == 1) continue;

        if (sscanf(linea, "media_llegadas_este = %d", &config->media_llegadas_este) == 1) continue;
        if (sscanf(linea, "media_llegadas_oeste = %d", &config->media_llegadas_oeste) == 1) continue;

        if (sscanf(linea, "velocidad_promedio = %d", &config->velocidad_promedio) == 1) continue;

        // Rango de velocidad (Este)
        if (sscanf(linea, "rango_velocidad_inferior_este = %d", &config->rango_velocidad_inferior_este) == 1) continue;
        if (sscanf(linea, "rango_velocidad_superior_este = %d", &config->rango_velocidad_superior_este) == 1) continue;

        // Rango de velocidad (Oeste)
        if (sscanf(linea, "rango_velocidad_inferior_oeste = %d", &config->rango_velocidad_inferior_oeste) == 1) continue;
        if (sscanf(linea, "rango_velocidad_superior_oeste = %d", &config->rango_velocidad_superior_oeste) == 1) continue;

        // Duración de la luz verde
        if (sscanf(linea, "duracion_luz_verde_este = %d", &config->duracion_luz_verde_este) == 1) continue;
        if (sscanf(linea, "duracion_luz_verde_oeste = %d", &config->duracion_luz_verde_oeste) == 1) continue;

        if (sscanf(linea, "porcentaje_ambulancias = %d", &config->porcentaje_ambulancias) == 1) continue;

        // Valores de K1 y K2
        if (sscanf(linea, "k1 = %d", &config->k1) == 1) continue;
        if (sscanf(linea, "k2 = %d", &config->k2) == 1) continue; */
    }

    fclose(archivo);
    return config;
          
}

