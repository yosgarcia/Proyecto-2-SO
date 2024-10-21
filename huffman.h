
#ifndef huffman_h
#define huffman_h
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


typedef struct nodo_huffman{
    unsigned char simbolo;  // Símbolo (byte) (Si no es hoja, no tiene simbolo)
    int frecuencia;          // Frecuencia del símbolo
    struct nodo_huffman* izquierda;  // Hijo izquierdo
    struct nodo_huffman* derecha;    // Hijo derecho
} NodoHuff;


typedef struct MinHeap {
    int tamanno;               // Número actual de elementos en el heap
    int capacidad;            // Capacidad máxima del heap, 256
    NodoHuff** elementos;  // Array de punteros a nodos de Huffman
} MinHeap;



NodoHuff* crear_nodo_huffman(unsigned char simbolo,int frecuencia);
int es_hoja(NodoHuff* nodo);

NodoHuff* unificar_nodos_huffman(NodoHuff* nodo_1, NodoHuff* nodo_2);

NodoHuff* crear_arbol_Huffman(int arreglo_frecuencia[256]);

void intercambiar_nodos(NodoHuff** array, int i, int j);

MinHeap* crear_min_heap(int capacidad);

void insertar_en_heap(MinHeap* heap, NodoHuff* elemento);

void imprimir_heap(MinHeap* heap);

NodoHuff* extraer_minimo(MinHeap* heap);

void imprimir_arbol_huffman(NodoHuff* raiz, int profundidad);

char** generar_tabla_codigos(NodoHuff* Arbol);

void generar_codigos(NodoHuff* raiz,char** tabla_codigo, char* camino_actual);

void imprimir_tabla(char** tabla_codigo);

void comprimir_archivo(NodoHuff* Arbol, const char* nombre_input, char* nombre_output);

void escribir_bit(FILE *archivo_output, int bit, int *bitBuffer, int *contador_bits);

void escribir_tabla(int bits_sobrantes, char** tabla_codigos);
#endif

