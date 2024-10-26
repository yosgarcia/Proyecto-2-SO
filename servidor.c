#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "huffman.h"

#define BUFFER_CODIGO_SIZE 9
#define FREQUENCY_SIZE 256

//puede ser que no se envien todos los bytes y se lean de una vez
//ya que pueden haber interrupciones en medio
ssize_t leer_chunk(int socket, unsigned char *buffer, size_t bytes_esperados) {
    size_t total_leidos = 0;
    ssize_t leidos;

    while (total_leidos < bytes_esperados) {
        leidos = read(socket, buffer + total_leidos, bytes_esperados - total_leidos);

        if (leidos < 0) {
            perror("Error al leer del socket");
            return -1;  // Error de lectura
        } else if (leidos == 0) {
            break;  // El cliente cerro la conexion
        }

        total_leidos += leidos;
    }

    return total_leidos;
}

ssize_t leer_tamanno(int socket, void* buffer, size_t longitud) {
    size_t bytes_restantes = longitud;
    unsigned char* ptr = buffer;

    while (bytes_restantes > 0) {
        ssize_t bytes_leidos = recv(socket, ptr, bytes_restantes, 0);
        if (bytes_leidos <= 0) {
            return -1;
        }
        bytes_restantes -= bytes_leidos;
        ptr += bytes_leidos;
    }
    return longitud;
}

void imprimir_bytes_binario(const unsigned char* bytes, size_t longitud) {
    for (size_t i = 0; i < longitud; i++) {
        unsigned char byte_actual = bytes[i];

        // Recorre los 8 bits del byte desde el mas significativo al menos significativo
        for (int bit = 7; bit >= 0; bit--) {
            printf("%c", (byte_actual & (1 << bit)) ? '1' : '0');
        }

        printf(" ");
    }
    printf("\n");
}

// Inicializa el servidor con una IP y puerto especificos.
int inicializar_servidor(const char *ip, int puerto, int *server_fd, struct sockaddr_in *address) {
    if ((*server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        return -1;
    }

    address->sin_family = AF_INET;
    address->sin_addr.s_addr = inet_addr(ip);
    address->sin_port = htons(puerto);

    if (bind(*server_fd, (struct sockaddr *)address, sizeof(*address)) < 0) {
        perror("Bind failed");
        close(*server_fd);
        return -1;
    }

    if (listen(*server_fd, 3) < 0) {
        perror("Listen failed");
        close(*server_fd);
        return -1;
    }

    printf("Servidor escuchando en %s:%d\n", ip, puerto);
    return 0;
}

void start_server(const char *ip, int puerto) {
    char **tabla_huff = calloc(256, sizeof(char *));
    size_t cant_bytes_archivos;
    int server_fd, sock;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer_codigo[BUFFER_CODIGO_SIZE] = {0};
    unsigned char *bytes_archivo;
    int frequency_table[FREQUENCY_SIZE] = {0};

    // Inicializar el servidor
    if (inicializar_servidor(ip, puerto, &server_fd, &address) < 0) {
        exit(EXIT_FAILURE); // Terminar si hubo error
    }

    // Recibir chunk de bytes
    while (1) {  
        if ((sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }
        if (leer_tamanno(sock, &cant_bytes_archivos, sizeof(cant_bytes_archivos)) < 0) {
            perror("Error al recibir el tamaño");
            close(sock);
            return;
        }     
        //Leer la cantidad de bytes recibidos
        printf("Cant de bytes recibidos: %lu\n",cant_bytes_archivos);

        bytes_archivo = calloc(cant_bytes_archivos,sizeof(unsigned char)); 
        //Notificar cant_bytes_leida.
        char recibido_1[1] = {"Y"};
        send(sock,recibido_1,1,0);

        // Leer el mensaje del cliente
        ssize_t total_leidos = leer_chunk(sock, bytes_archivo, cant_bytes_archivos);
        if (total_leidos > 0) {
            // Calcular la frecuencia de bytes
            for (int i = 0; i < total_leidos; i++) {
                frequency_table[bytes_archivo[i]]++;
            }
        }
        //imprimir_bytes_binario(bytes_archivo,cant_bytes_archivos);
        // Enviar el arreglo de frecuencias al cliente
        send(sock, frequency_table, sizeof(frequency_table), 0);
        printf("Tabla de frecuencias enviado al cliente\n");
        break;
    }

    //Recibir tabla_huff del cliente
    for (int i = 0; i<256; i++){
        for (int x = 0; x<BUFFER_CODIGO_SIZE; x++){
            buffer_codigo[x] = '\0';
        }

        //get
        size_t len;
        if (leer_tamanno(sock, &len, sizeof(len)) < 0) {
            perror("Error al recibir el tamaño");
            close(sock);
            return;
        }

        //send
        char ok_1 [1] = {'K'};
        send(sock,ok_1,sizeof(ok_1),0);

        //get
        ssize_t read_size = recv(sock, buffer_codigo, BUFFER_CODIGO_SIZE,0);
        if (buffer_codigo[0] == '0' || buffer_codigo[0] == '1'){
            tabla_huff[i] = calloc(len,sizeof(char));
            strcpy(tabla_huff[i],buffer_codigo);
        }

        //send
        char ok_2[1] = {'1'};
        send(sock,ok_2, sizeof(ok_2), 0);
    }
    size_t cant_bits_compress = 0;
    for (int i = 0; i <cant_bytes_archivos; i++){
        unsigned char byte_actual = bytes_archivo[i];
        cant_bits_compress += strlen(tabla_huff[byte_actual]);
    }


    char* bits_comprimidos = calloc(cant_bits_compress,sizeof(char));

    // Recorrer la parte de archivo que se le asigno al servidor
    // y escribir en un nuevo buffer con los codigos cambiados

    int pos_bits = 0;
    for(int i = 0;i < cant_bytes_archivos; i++){

        unsigned char byte_actual = bytes_archivo[i];
        const char* codigo_huffman = tabla_huff[byte_actual];
        int tamanno_codigo_huffman = strlen(codigo_huffman);

        for (int x=0;x<tamanno_codigo_huffman;x++){
            bits_comprimidos[pos_bits] = codigo_huffman[x];
            pos_bits++;
        }
    }

    //Send len
    send (sock, &cant_bits_compress, sizeof(cant_bits_compress), 0);
    //Get OK
    char recibido[1];
    recv(sock, recibido, 1, 0);
    //Send
    send(sock, bits_comprimidos, cant_bits_compress, 0);
    printf("Bits comprimidos enviados.\n");
    //printf("{%s}\n enviados\n",bits_comprimidos);

    close(sock);
    close(server_fd);
    free(bits_comprimidos);
    return;
}



int main(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <IP:puerto>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char ip[16];
    int puerto;

    // Extraer IP y puerto de la entrada
    if (sscanf(argv[1], "%15[^:]:%d", ip, &puerto) != 2) {
        fprintf(stderr, "Formato inválido. Uso: <IP:puerto>\n");
        return EXIT_FAILURE;
    }

    start_server(ip, puerto);
    return 0;
}