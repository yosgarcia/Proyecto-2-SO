#ifndef descompresor_h
#define descompresor_h

#include "huffman.h"

void agregar_nodo_arbol(NodoHuff* raiz, unsigned char simbolo, char* codigo, int pos_codigo );

NodoHuff* generar_arbol_desde_tabla(char** tabla_codigo);

void clean_buffer(char* buffer);

char** generar_tabla_desde_archivo(FILE* archivo_tabla);

int char_to_int(char c);

int leer_bit(FILE *archivo_input, int *bitBuffer, int *contador_bits);

void descomprimir(char* nombre_entrada, char* nombre_salida, char* nombre_tabla);

#endif