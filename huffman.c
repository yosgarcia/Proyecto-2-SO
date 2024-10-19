#include "huffman.h"
#include <stdio.h>
#include <stdlib.h>



NodoHuff* crear_nodo_Huffman(unsigned char simbolo,int frecuencia){
    NodoHuff* nodo_nvo = calloc(1,sizeof(NodoHuff));
    nodo_nvo->simbolo = simbolo;
    nodo_nvo->frecuencia = frecuencia;
    nodo_nvo->izquierda = NULL;
    nodo_nvo->derecha = NULL;
    return nodo_nvo;
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
            NodoHuff* nodo_nvo = crear_nodo_Huffman(i,arreglo_frecuencia[i]);
            insertar_en_heap(heap,nodo_nvo);
        }
    }
    while (heap->tamanno > 0){
        imprimir_heap(heap);
        if (heap->tamanno == 1){  
            imprimir_heap(heap);
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
