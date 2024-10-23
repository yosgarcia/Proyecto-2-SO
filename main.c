#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "huffman.h"
#include "configuracion.h"
#include <sys/socket.h>


#define PORT 12345
#define lu unsigned long

typedef struct datos_thread_server{
    const char* ip;
    unsigned char* arreglo_bytes;
    size_t arreglo_bytes_size;
    int numero_servidor;
} datos_thread_server;

Config* config;
int arreglo_frecuencias [256]; 
int respuestas_recibidas;
int recibi_todas_frequencias;
char** tabla_huff;
char** bits_comprimidos_x_servidor;

void* enviar_arreglo_bytes_a_server(void* arg) {
    datos_thread_server* datos = (datos_thread_server*)arg;
    int sock;
    struct sockaddr_in serv_addr;
    int arreglo_recibido[256] = {0};

    // Crear el socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return NULL;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convertir la dirección IPv4 y la dirección del servidor
    if (inet_pton(AF_INET, datos->ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        close(sock);
        free(datos->arreglo_bytes);
        free(datos);
        return NULL;
    }

    // Conectar al servidor
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        free(datos->arreglo_bytes);
        free(datos);
        return NULL;
    }

    // Enviar el mensaje
    send(sock, datos->arreglo_bytes, datos->arreglo_bytes_size, 0);

    // Recibir el arreglo de frecuencias
    recv(sock, arreglo_recibido, sizeof(arreglo_recibido), 0);

    // Sumarlo a las frecuencias globales
    for (int i = 0; i < 256; i++) {
        arreglo_frecuencias[i] += arreglo_recibido[i];
    }
    respuestas_recibidas++;

    //Esperar a que main cree el arbol_huff y la tabla_huff
    while(recibi_todas_frequencias == 0){
        continue;
    }
    //Enviar tabla_huff a servidores y despues que ellos lo reciban y manden bytes
    for (int i=0;i<256;i++){
        if (tabla_huff[i] == NULL){
            char* msg = "NULO";
            send(sock, msg, strlen(msg), 0);
        }
        else{
            send(sock, tabla_huff[i], strlen(tabla_huff[i]), 0);
        }
        char mensaje_recibido[1];
        recv(sock, mensaje_recibido, 1, 0);
    }
    //printf("Esperando bits comprimidos\n");
    //Recibir los bits comprimidos
    char buffer [228760];
    ssize_t tamanno = recv(sock, buffer, 228760, 0);
    
    bits_comprimidos_x_servidor[datos->numero_servidor] = calloc(tamanno,sizeof(char));
    strcpy(bits_comprimidos_x_servidor[datos->numero_servidor],buffer);
    //printf("bits comprimidos recibidos: {%s}\n",bits_comprimidos_x_servidor[datos->numero_servidor]);

    free(datos->arreglo_bytes);  // Liberar el buffer
    free(datos);  // Liberar la estructura de datos
    close(sock);
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

int main() {
    config = leer_configuracion("configuracion.txt");
    //imprimir_bytes_archivo_binario(config->archivo_comprimido);
    //return 0;

    FILE *archivo = fopen(config->archivo_a_comprimir, "rb");
    if(archivo == NULL){ printf("No se puedo abrir el archivo %s\n",config->archivo_a_comprimir); return 1;}
    FILE* archivo_comprimido = fopen(config->archivo_comprimido, "wb");
    if(archivo == NULL){ printf("No se puedo abrir el archivo %s\n",config->archivo_a_comprimir); return 1;}


    respuestas_recibidas = 0;
    recibi_todas_frequencias = 0;
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

    bits_comprimidos_x_servidor = calloc(num_servidores, sizeof(char*));


    pthread_t threads[num_servidores];  // Arreglo de hilos
//Bytes por servidor{27845} bits por servidor{222760}
    for (lu i = 0; i < num_servidores; i++) {
        printf("Bytes por servidor{%lu} bits por servidor{%lu}\n",bytes_por_servidor,bytes_por_servidor*8);
        long tamanno_bloque = bytes_por_servidor;
        //se van a leer menos bytes, estoy en el final
        if (i==num_servidores-1 && residuo>0){
            tamanno_bloque += residuo;
        }

        unsigned char *buffer = calloc(tamanno_bloque,sizeof (unsigned char));
        size_t bytes_read = fread(buffer, 1, tamanno_bloque, archivo);

        datos_thread_server* datos = calloc(1,sizeof(datos_thread_server));
        datos->ip = config->ips[i];
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
    while (respuestas_recibidas != num_servidores){
        continue;
    }

    NodoHuff* arbol_huff = crear_arbol_Huffman(arreglo_frecuencias);
    tabla_huff = generar_tabla_codigos(arbol_huff);
    recibi_todas_frequencias = 1;
    
    printf("main> threads no unidos\n");
    // Esperar a que todos los hilos terminen de comprimir;
    for (lu i = 0; i < num_servidores; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("main> threads unidos\n");
    
    lu cant_bits_juntados = 0;
    for (int i=0; i<num_servidores; i++){
        cant_bits_juntados += strlen(bits_comprimidos_x_servidor[i]);
    }
    //Hacer cant multiplo de 8
    lu faltante = cant_bits_juntados%8;
    lu cant_bits_archivo_comprimido = cant_bits_juntados + faltante;
    lu cant_bytes_archivo_comprimido = cant_bits_archivo_comprimido/ 8;

    //juntar todos los bits en un string de '0's y '1's
    printf("voy a hacer calloc bits juntados\n");
    char* string_bits_juntados = calloc(cant_bits_juntados,sizeof(char));
    printf("done\n");
    int pos_bit = 0;
    for (int i=0; i<num_servidores; i++){
        lu cant_bits = strlen(bits_comprimidos_x_servidor[i]);
        for (lu j=0; j<cant_bits; j++){
            string_bits_juntados[pos_bit] = bits_comprimidos_x_servidor[i][j];
            pos_bit++;
        }
    }
    // Rellenar con '0's si faltan bits para completar un múltiplo de 8
    while (pos_bit < cant_bits_archivo_comprimido) {
        string_bits_juntados[pos_bit] = '0';
        pos_bit++;
    }

    // Convertir el string de '0's y '1's a bytes
    printf("voy a hacer calloc al buffer\n");
    unsigned char* buffer = calloc(cant_bytes_archivo_comprimido, sizeof(unsigned char));
    printf("done\n");

    if (buffer == NULL) {
        printf("Error al asignar memoria.\n");
        //free(string_bits_juntados);
        //fclose(archivo_comprimido);
        return 1;
    }
    printf("voy a llenar el buffer\n");
    // Llenar el buffer con los bits convertidos en bytes
    for (lu i = 0; i < cant_bits_archivo_comprimido; i++) {
        int byte_pos = i / 8;
        int bit_en_byte = 7 - (i % 8);  // Bit más significativo primero

        if (string_bits_juntados[i] == '1') {
            buffer[byte_pos] |= (1 << bit_en_byte);  // Poner el bit en 1
        }
    }
    printf("done.\n");

    // Escribir los bytes en el archivo
    size_t bytes_escritos = fwrite(buffer,sizeof(unsigned char),
                            cant_bytes_archivo_comprimido, archivo_comprimido);
    escribir_tabla(faltante,tabla_huff,config->archivo_tabla_huff);

    printf("Archivo comprimido escrito correctamente.\n");
    free(buffer);
    //free(string_bits_juntados);
    fclose(archivo_comprimido);
    fclose(archivo);
    
    return 0;
}

