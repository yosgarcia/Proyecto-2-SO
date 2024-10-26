#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "huffman.h"
#include "configuracion.h"
#include <sys/socket.h>

#define lu unsigned long

typedef struct datos_thread_server{
    const char* ip;
    int puerto;
    unsigned char* arreglo_bytes;
    size_t arreglo_bytes_size;
    int numero_servidor;
} datos_thread_server;

Config* config;
int arr_frec [256]; 
int frec_recibidas;
int got_all_frec;
char** tabla_huff;
int bits_compr_recibidos;
char** bits_compr_x_server;

ssize_t leer_tamanno(int socket, void* buffer, size_t longitud) {
    size_t bytes_restantes = longitud;
    unsigned char* ptr = buffer;

    while (bytes_restantes > 0) {
        ssize_t bytes_leidos = recv(socket, ptr, bytes_restantes, 0);
        if (bytes_leidos <= 0) {
            return -1; // Error o conexión cerrada
        }
        bytes_restantes -= bytes_leidos;
        ptr += bytes_leidos;
    }
    return longitud;
}

ssize_t leer_chunk_char(int socket, char *buffer, size_t bytes_esperados) {
    size_t total_leidos = 0;
    ssize_t leidos;
    while (total_leidos < bytes_esperados) {
        leidos = read(socket, buffer + total_leidos, bytes_esperados - total_leidos);
        if (leidos < 0) {
            perror("Error al leer del socket");
            return -1;  // Error de lectura
        } else if (leidos == 0) {
            break;  // El cliente cerro la conexión
        }

        total_leidos += leidos;
    }
    return total_leidos;
}

int conectar_servidor(int* sock, struct sockaddr_in* serv_addr, const char* ip, int puerto) {
    // Crear el socket
    if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(puerto);

    // Convertir la direccion IPv4 y validar
    if (inet_pton(AF_INET, ip, &serv_addr->sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        close(*sock);
        return -1;
    }

    // Conectar al servidor
    if (connect(*sock, (struct sockaddr*)serv_addr, sizeof(*serv_addr)) < 0) {
        perror("Connection failed");
        close(*sock);
        return -1;
    }
    return 0;
}
void* enviar_arreglo_bytes_a_server(void* arg) {
    datos_thread_server* datos = (datos_thread_server*)arg;
    int sock;
    struct sockaddr_in serv_addr;
    int arreglo_recibido[256] = {0};

    if (conectar_servidor(&sock, &serv_addr, datos->ip, datos->puerto) < 0) {
        free(datos->arreglo_bytes);
        free(datos);
        return NULL;
    }

    //Enviar cant de bytes del arreglo
    size_t tamanno_arreglo = datos->arreglo_bytes_size;
    send(sock,&tamanno_arreglo,sizeof(tamanno_arreglo),0);
    char tamanno_recibido [1];

    //Recibir confirmacion de mensaje
    recv(sock, tamanno_recibido, 1, 0);

    // Enviar el mensaje
    send(sock, datos->arreglo_bytes, datos->arreglo_bytes_size, 0);

    // Recibir el arreglo de frecuencias
    recv(sock, arreglo_recibido, sizeof(arreglo_recibido), 0);

    // Sumarlo a las frecuencias globales
    for (int i = 0; i < 256; i++) {
        arr_frec[i] += arreglo_recibido[i];
    }
    frec_recibidas++;

    //Esperar a que main cree el arbol_huff y la tabla_huff
    while(got_all_frec == 0){
        continue;
    }
    //Enviar tabla_huff a servidores y despues que ellos lo reciban y manden bytes
    for (int i=0;i<256;i++){
        char* msg = tabla_huff[i];
        if (tabla_huff[i] == NULL) msg = "NULO";

        //Enviar la longitud
        size_t longitud = strlen(msg);
        send (sock, &longitud, sizeof(longitud), 0);
        //Recibir OK
        char recibido_1[1];
        recv(sock, recibido_1, 1, 0);

        //Enviar el mensaje
        send(sock, msg, strlen(msg), 0);
        //Recibir OK
        char recibido_2[1];
        recv(sock, recibido_2, 1, 0);
    }

    //Get
    size_t len;
    if (leer_tamanno(sock, &len, sizeof(len)) < 0) {
            perror("Error al recibir el tamaño");
            return NULL;
    } 
    char ok_1 [1] = {'K'};
    send(sock,ok_1,sizeof(ok_1),0);
    //Get
    char* str_compressed  =  calloc(len,sizeof(char));
    ssize_t total_leidos = leer_chunk_char(sock, str_compressed, len);

    bits_compr_x_server[datos->numero_servidor] = calloc(len,sizeof(char));
    strcpy(bits_compr_x_server[datos->numero_servidor],str_compressed);

    free(datos->arreglo_bytes);  // Liberar el buffer
    free(datos);  // Liberar la estructura de datos
    close(sock);
    free(str_compressed);

    bits_compr_recibidos++;
    //shutdown(sock, SHUT_RDWR);  // Cierra la conexión de forma limpia

    return NULL;
}

// Función que imprime un byte en formato binario (8 bits)
void imprimir_byte_binario(unsigned char byte) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (byte >> i) & 1);  // Desplazar y enmascarar para imprimir cada bit
    }
    printf(" ");  // Espacio entre bytes
}

// Función que imprime todos los bytes del archivo en formato binario
void imprimir_bytes_archivo_binario(const char* nombre_archivo) {
    FILE* archivo = fopen(nombre_archivo, "rb");  // Abrir en modo binario
    if (archivo == NULL) {
        printf("No se pudo abrir el archivo %s\n", nombre_archivo);
        return;
    }

    unsigned char byte;
    size_t contador = 0;

    printf("Contenido del archivo %s en binario:\n", nombre_archivo);
    while (fread(&byte, sizeof(unsigned char), 1, archivo) == 1) {
        imprimir_byte_binario(byte);
        contador++;

        // Opcional: Formatear la salida en bloques de 8 bytes por línea
        if (contador % 8 == 0) {
            printf("\n");
        }
    }

    printf("\nTotal de bytes leídos: %zu\n", contador);
    fclose(archivo);
}

void comprimir_local(){
    FILE* archivo = fopen(config->archivo_a_comprimir, "rb");  // Abrir en modo binario
    fseek(archivo, 0, SEEK_END);
    size_t longitud = ftell(archivo);  // Obtener la posición actual (tamaño del archivo)
    rewind(archivo);  // Volver al inicio del archivo

    // Reservar memoria para almacenar los bytes
    unsigned char* buffer = (unsigned char*)malloc(longitud);
    if (buffer == NULL) {
        perror("Error al reservar memoria");
        fclose(archivo);
        return NULL;
    }

    // Leer los bytes del archivo
    size_t bytes_leidos = fread(buffer, 1, longitud, archivo);
    int frec_table[256] = {0};
    for (size_t i = 0; i< longitud; i++){
        frec_table[buffer[i]]++;
    }
    NodoHuff* arbol = crear_arbol_Huffman (frec_table);
    char** tabla_huff =  generar_tabla_codigos(arbol);
    imprimir_tabla(tabla_huff);
    fclose(archivo);
    comprimir_archivo(arbol,config->archivo_a_comprimir,config->archivo_comprimido,config->archivo_tabla_huff);
}


int main() {
    config = leer_configuracion("configuracion.txt");

    FILE *archivo = fopen(config->archivo_a_comprimir, "rb");
    if(archivo == NULL){ printf("No se puedo abrir el archivo %s\n",config->archivo_a_comprimir); return 1;}
    FILE* archivo_comprimido = fopen(config->archivo_comprimido, "wb");
    if(archivo == NULL){ printf("No se puedo abrir el archivo %s\n",config->archivo_a_comprimir); return 1;}


    frec_recibidas = 0;
    got_all_frec = 0;
    int num_servidores = config->num_servidores;

    fseek(archivo, 0, SEEK_END);
    lu cant_bytes_archivo = ftell(archivo);
    rewind(archivo);
    fseek(archivo, 0, SEEK_SET); // Volver al inicio del archivo

    lu bytes_por_servidor = cant_bytes_archivo/num_servidores;
    lu residuo = cant_bytes_archivo%num_servidores;

    if  (num_servidores>cant_bytes_archivo){
        bytes_por_servidor = 1;
        num_servidores = cant_bytes_archivo;
    }

    bits_compr_x_server = calloc(num_servidores, sizeof(char*));


    pthread_t threads[num_servidores];  // Arreglo de hilos
    for (lu i = 0; i < num_servidores; i++) {
        lu tamanno_bloque = bytes_por_servidor;
        //se van a leer menos bytes, estoy en el final
        if (i==num_servidores-1 && residuo>0){
            tamanno_bloque += residuo;
        }

        unsigned char *buffer = calloc(tamanno_bloque,sizeof (unsigned char));
        size_t bytes_read = fread(buffer, 1, tamanno_bloque, archivo);

        datos_thread_server* datos = calloc(1,sizeof(datos_thread_server));
        datos->ip = config->ips[i];
        datos->puerto = config->puertos[i];
        datos->arreglo_bytes = buffer;
        datos->arreglo_bytes_size = bytes_read;
        datos->numero_servidor = i;
        // Crear el hilo para enviar los datos al servidor
        if (pthread_create(&threads[i], NULL, enviar_arreglo_bytes_a_server, datos) != 0) {
            perror("Error al crear el thread");
            free(buffer);
            free(datos);
            continue;
        }
    }
    while (frec_recibidas != num_servidores){
        continue;
    }

    NodoHuff* arbol_huff = crear_arbol_Huffman(arr_frec);
    tabla_huff = generar_tabla_codigos(arbol_huff);

    got_all_frec = 1;
    // Esperar a que todos los hilos terminen de comprimir;
    while(bits_compr_recibidos != num_servidores){
    }    
    lu cant_bits_juntados = 0;
    for (int i=0; i<num_servidores; i++){
        cant_bits_juntados += strlen(bits_compr_x_server[i]);
    }
    //Hacer cant multiplo de 8
    lu faltante = (8-cant_bits_juntados%8)%8;
    lu cant_bits_archivo_comprimido = cant_bits_juntados + faltante;
    lu cant_bytes_archivo_comprimido = cant_bits_archivo_comprimido/ 8;

    //juntar todos los bits en un string de '0's y '1's
    char* string_bits_juntados = calloc(cant_bits_juntados,sizeof(char));
    int pos_bit = 0;
    for (int i=0; i<num_servidores; i++){
        lu cant_bits = strlen(bits_compr_x_server[i]);
        for (lu j=0; j<cant_bits; j++){
            string_bits_juntados[pos_bit] = bits_compr_x_server[i][j];
            pos_bit++;
        }
    }
    // Rellenar con '0's si faltan bits para completar un múltiplo de 8
    while (pos_bit < cant_bits_archivo_comprimido) {
        string_bits_juntados[pos_bit] = '0';
        pos_bit++;
    }

    // Convertir el string de '0's y '1's a bytes
    unsigned char* buffer = calloc(cant_bytes_archivo_comprimido, sizeof(unsigned char));

    if (buffer == NULL) {
        printf("Error al asignar memoria.\n");
        return 1;
    }
    // Llenar el buffer con los bits convertidos en bytes
    for (lu i = 0; i < cant_bits_archivo_comprimido; i++) {
        int byte_pos = i / 8;
        int bit_en_byte = 7 - (i % 8);  // Bit más significativo primero

        if (string_bits_juntados[i] == '1') {
            buffer[byte_pos] |= (1 << bit_en_byte);  // Poner el bit en 1
        }
    }

    // Escribir los bytes en el archivo
    size_t bytes_escritos = fwrite(buffer,sizeof(unsigned char),
                            cant_bytes_archivo_comprimido, archivo_comprimido);
    escribir_tabla(faltante,tabla_huff,config->archivo_tabla_huff);

    printf("Archivo comprimido escrito correctamente.\n");
    free(buffer);
    fclose(archivo_comprimido);
    fclose(archivo);
    
    return 0;
}

