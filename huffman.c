#include "huffman.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BITS 8     // Definir el tamaño máximo de bits (un byte)
#define BLOCK_SIZE 1024  // Tamaño del bloque en bytes

NodoHuff* crear_nodo_huffman(unsigned char simbolo,int frecuencia){
    NodoHuff* nodo_nvo = calloc(1,sizeof(NodoHuff));
    nodo_nvo->simbolo = simbolo;
    nodo_nvo->frecuencia = frecuencia;
    nodo_nvo->izquierda = NULL;
    nodo_nvo->derecha = NULL;
    return nodo_nvo;
}

int es_hoja(NodoHuff* nodo){
    if (nodo->derecha == NULL && nodo->izquierda == NULL){
        return 1;
    }
    return 0;
}

//a la izquierda el menor, a la derecha el mayor
NodoHuff* unificar_nodos_huffman(NodoHuff* nodo_1, NodoHuff* nodo_2){
    NodoHuff* raiz = calloc(1,sizeof(NodoHuff*));
    raiz->frecuencia = nodo_1->frecuencia + nodo_2->frecuencia;
    NodoHuff* nodo_menor;
    NodoHuff* nodo_mayor;
    if (nodo_1->frecuencia >= nodo_2->frecuencia){
        nodo_mayor = nodo_1;
        nodo_menor = nodo_2;
    }
    else{
        nodo_mayor = nodo_2;
        nodo_menor = nodo_1;
    }
    raiz->izquierda = nodo_menor;
    raiz->derecha = nodo_mayor;
    return raiz;
}

NodoHuff* crear_arbol_Huffman(int arreglo_frecuencia[256]){
    MinHeap* heap = crear_min_heap(256);
    for (int i=0; i<256;i++){
        //Si hay almenos uno, crear su nodo
        if (arreglo_frecuencia[i] > 0){
            //printf("insertando\n");
            NodoHuff* nodo_nvo = crear_nodo_huffman(i,arreglo_frecuencia[i]);
            insertar_en_heap(heap,nodo_nvo);
        }
    }
    while (heap->tamanno > 0){
        //imprimir_heap(heap);
        if (heap->tamanno == 1){  
            //imprimir_heap(heap);
            return extraer_minimo(heap);
        }
        NodoHuff* menor_1 = extraer_minimo(heap);
        NodoHuff* menor_2 = extraer_minimo(heap);
        NodoHuff* raiz_nva = unificar_nodos_huffman(menor_1,menor_2);
        insertar_en_heap(heap,raiz_nva);
    }
    return NULL; //si no metio elementos al heap
    
}



void intercambiar_nodos(NodoHuff** array, int i, int j) {
    NodoHuff* temp = array[i];
    array[i] = array[j];
    array[j] = temp;
}

MinHeap* crear_min_heap(int capacidad){
    MinHeap* nuevo_heap = calloc(1, sizeof(MinHeap));
    //El nodo en array[0] va a ser nulo para simplificar todo
    NodoHuff** array_nodos = calloc(capacidad+1, sizeof (NodoHuff*));
    nuevo_heap->tamanno = 0;
    nuevo_heap->capacidad = capacidad+1;
    nuevo_heap->elementos = array_nodos;
    return nuevo_heap;   
}

void insertar_en_heap(MinHeap* heap, NodoHuff* nodo) {
    heap->tamanno++;
    heap->elementos[heap->tamanno] = nodo; 
    int i = heap->tamanno;
    /*
    0 X
    0 -> 1 2
    2/2 = 1
    2 -> 4 5
    4/2 = 2
    5/2 = 2
    */
    //Ir escalando
    while (i>1 && heap->elementos[i]->frecuencia < heap->elementos[i/2]->frecuencia) {
        intercambiar_nodos(heap->elementos,i,i/2);
        i /= 2;
    }
}

void imprimir_heap(MinHeap* heap){
    if(heap->tamanno == 0){
        printf("La cola de priorida esta vacia\n");
        return;
    }
    for(int i = 1; i <= heap->tamanno; i++){
        printf("Pos_Nodo: %d, Frecuencia: %d\n", i, heap->elementos[i]->frecuencia);
    }
    printf("\n");
}

NodoHuff* extraer_minimo(MinHeap* heap) {
    if (heap->tamanno == 0) return NULL;
    if (heap->tamanno == 1) {
        NodoHuff* nodoMinimo = heap->elementos[1];
        heap->tamanno--;
        heap->elementos[1] = NULL;
        return nodoMinimo;
    }

    NodoHuff* nodoMinimo = heap->elementos[1];
    NodoHuff* nodoMayor = heap->elementos[heap->tamanno];
    //Subir el mayor
    heap->elementos[1] = nodoMayor;
    heap->elementos[heap->tamanno] = NULL; //pop
    heap->tamanno--;

    //Percolate down
    int idx = 1;
    //We have a left child
    int idx_izq = idx*2;
    int idx_der = idx*2+1;
    while (idx_izq <= heap->tamanno){
        //Derecho existe, derecho mas pequenno que izquierdo, papa mas grande que derecho
        if (idx_der <= heap->tamanno && heap->elementos[idx_der]->frecuencia < heap->elementos[idx_izq]->frecuencia
            && heap->elementos[idx]->frecuencia > heap->elementos[idx_der]->frecuencia){
                intercambiar_nodos(heap->elementos,idx,idx_der);
                idx = idx_der; //Papa se movio a hijo derecho
            }
        //One child
        //Papa mayor a hijo izquierdo
        else if (heap->elementos[idx]->frecuencia > heap->elementos[idx_izq]->frecuencia){
            intercambiar_nodos(heap->elementos,idx,idx_izq);
            //Papa se movio a hijo izquierdo
            idx = idx_izq;
        }
        else{
            //Papa es menor a ambos hijos
            break;
        }
        idx_izq = idx*2;
        idx_der = idx*2+1;
    }
    return nodoMinimo;
}

void imprimir_arbol_huffman(NodoHuff* raiz, int profundidad){
    if (raiz == NULL) {
        return;
    }

    // Indentación basada en la profundidad del nodo
    for (int i = 0; i < profundidad; i++) {
        printf("    ");  // Añadir 4 espacios por cada nivel de profundidad
    }

    // Imprimir el símbolo y la frecuencia
    if (raiz->izquierda == NULL && raiz->derecha == NULL) {
        // Si es una hoja, imprimir el símbolo
        if (raiz->simbolo=='\n') printf("Símbolo:'\\n', Frecuencia: %d\n",  raiz->frecuencia);
        else printf("Símbolo: '%c', Frecuencia: %d\n", raiz->simbolo, raiz->frecuencia);
    } else {
        // Si no es una hoja, solo imprimir la frecuencia
        printf("Nodo interno: Frecuencia: %d\n", raiz->frecuencia);
    }

    // Recorrer el subárbol izquierdo
    imprimir_arbol_huffman(raiz->izquierda, profundidad + 1);

    // Recorrer el subárbol derecho
    imprimir_arbol_huffman(raiz->derecha, profundidad + 1);
}

char** generar_tabla_codigos(NodoHuff* Arbol){
    char** array = calloc(256,sizeof(char* ));
    generar_codigos(Arbol,array,"");
    return array;
}

void generar_codigos(NodoHuff* raiz,char** tabla_codigo, char* camino_actual){
    if(raiz == NULL){
        return;
    }
    //hoja
    if(raiz->izquierda == NULL && raiz->derecha == NULL){
        tabla_codigo[raiz->simbolo] = camino_actual;
        return;
    }

    char* nuevo_codigo_izq = calloc(256,sizeof(char));
    strcpy(nuevo_codigo_izq, camino_actual);
    strcat(nuevo_codigo_izq, "0");
    generar_codigos(raiz->izquierda, tabla_codigo, nuevo_codigo_izq);

    char* nuevo_codigo_der = calloc(256, sizeof(char));
    strcpy(nuevo_codigo_der, camino_actual);
    strcat(nuevo_codigo_der, "1");
    generar_codigos(raiz->derecha, tabla_codigo, nuevo_codigo_der);

}

void imprimir_tabla(char** tabla_codigo){
    printf("Imprimiendo tabla de codigos\n");
    for(int i = 0; i < 256; i++){
        if (tabla_codigo[i] == NULL) continue;
        printf("Simbolo: %c  Codigo: %s\n",i, tabla_codigo[i]);
    }
}

// Función para escribir un bit en el archivo
void escribir_bit(FILE *archivo_output, int bit, int *bitBuffer, int *contador_bits) {
    *bitBuffer = (*bitBuffer << 1) | bit;  // Desplazar el buffer hacia la izquierda y agregar el nuevo bit
    (*contador_bits)++;
    // Si el buffer tiene 8 bits, escribirlo en el archivo
    if (*contador_bits == MAX_BITS) {
        //printf("a escribir: %d", *bitBuffer);
        fputc(*bitBuffer, archivo_output);
        *bitBuffer = 0;  // Reiniciar el buffer
        *contador_bits = 0;   // Reiniciar el contador de bits
    }
}

void comprimir_archivo(NodoHuff* Arbol, const char* nombre_input, char* nombre_output,
                       char* archivo_tabla) {
    char** tabla_codigos = generar_tabla_codigos(Arbol);
    FILE *input = fopen(nombre_input, "rb");
    if (input == NULL) {
        perror("Error al abrir archivo de entrada"); return;
    }
    
    FILE *output = fopen(nombre_output, "wb");
    if (output == NULL) {
        perror("Error al abrir archivo de salida"); fclose(input); return;
    }
    
    int bitBuffer = 0;  // Buffer para almacenar los bits
    int contador_bits = 0;   // Contador de bits escritos
    unsigned char block[BLOCK_SIZE]; // Buffer para almacenar el bloque leído
    size_t bytes_leidos;

    // Leer el archivo en bloques
    while ((bytes_leidos = fread(block, sizeof(unsigned char), BLOCK_SIZE, input)) > 0) {
        // Procesar cada byte en el bloque
        for (size_t i = 0; i < bytes_leidos; i++) {
            unsigned char byte = block[i];
            const char *codigoHuffman = tabla_codigos[byte];
            // Escribir cada bit del código en el archivo de salida
            for (int j = 0; codigoHuffman[j] != '\0'; j++) {
                int bit = codigoHuffman[j] - '0';  // Convertir '0' o '1' a 0 o 1
                escribir_bit(output, bit, &bitBuffer, &contador_bits);
            }
        }
    }

    // Si quedan bits en el buffer, escribirlos
    int bits_sobrantes = MAX_BITS - contador_bits;
    
    if (contador_bits > 0) {
        bitBuffer <<= (MAX_BITS - contador_bits);  // Desplazar para llenar el byte
        fputc(bitBuffer, output);
    }
    escribir_tabla(bits_sobrantes,tabla_codigos, archivo_tabla);
    fclose(output);
    fclose(input);
}

void escribir_tabla(int bits_sobrantes, char** tabla_codigos, char* nombre_archivo_tabla){
    FILE* archivo_tabla = fopen(nombre_archivo_tabla,"w");
    fprintf(archivo_tabla,"%d\n",bits_sobrantes);
    for (int i=0;i<256;i++){
        if (tabla_codigos[i] == NULL){
            fprintf(archivo_tabla,"\n");
            continue;
        }
        fprintf(archivo_tabla,"%s\n",tabla_codigos[i]);
    }
    fprintf(archivo_tabla,"fin\n");
    fclose(archivo_tabla);
}