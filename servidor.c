// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFFER_SIZE 228760
#define FREQUENCY_SIZE 256

void start_server(const char *ip){
    char** tabla_huff = calloc(256,sizeof(char*));
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    unsigned char buffer_bytes_archivo[BUFFER_SIZE] = {0};
    
    int frequency[FREQUENCY_SIZE] = {0};

    // Crear el socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Asignar la dirección y el puerto
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(PORT);

    // Asignar el socket a la dirección y puerto
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor escuchando en %s:%d\n", ip, PORT);

    // Recibir chunk de bytes
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        // Leer el mensaje del cliente
        ssize_t read_size = read(new_socket, buffer_bytes_archivo, BUFFER_SIZE);
        if (read_size > 0) {
            // Calcular la frecuencia de bytes
            for (int i = 0; i < read_size; i++) {
                frequency[buffer_bytes_archivo[i]]++;
            }
        }
        // Enviar el arreglo de frecuencias al cliente
        send(new_socket, frequency, sizeof(frequency), 0);
        printf("Tabla de frecuencias enviado al cliente\n");
        break;
    }
    //Recibir tabla_huff del cliente
    int codigos_recibidos = 0;
    while (1) {
        if (codigos_recibidos == 256){
            printf("ya recibi todos los codigos\n");
            break;
        }
        //Limpiar buffer
        for (int i=0;i<BUFFER_SIZE;i++){
            buffer[i] = '\0';
        }
        //Recibir buffer
        ssize_t read_size = recv(new_socket, buffer, BUFFER_SIZE,0);
        if (buffer[0] != '0' && buffer[0] != '1'){
        }
        else{
            tabla_huff[codigos_recibidos] = calloc(read_size,sizeof(char));
            strcpy(tabla_huff[codigos_recibidos],buffer);
        }
        char mensaje[1] = {'1'};
        send(new_socket,mensaje, 1, 0);
        codigos_recibidos++;
    }

    char* bits_comprimidos = calloc(BUFFER_SIZE,sizeof(char));
    if (bits_comprimidos == NULL){
        printf("outofmemory\n");
    }

    // Recorrer la parte de archivo que se le asigno al servidor
    // y escribir en un nuevo buffer con los codigos cambiados
    int pos_bits = 0;
    for(int i = 0;i < strlen(buffer_bytes_archivo); i++){
        unsigned char byte_actual = buffer_bytes_archivo[i];
        const char* codigo_huffman = tabla_huff[byte_actual];
        int tamanno_codigo_huffman = strlen(codigo_huffman);
        for (int x=0;x<tamanno_codigo_huffman;x++){
            bits_comprimidos[pos_bits] = codigo_huffman[x];
            pos_bits++;
        }
    }

    printf("Bits que voy a enviar: %s\n",bits_comprimidos);

    // Enviar bits del fragemento de archivo a comprimir
    while(1){
        send(new_socket, bits_comprimidos, strlen(bits_comprimidos), 0);
        printf("bits comprimidos enviados\n");
        break;

    }
    close(new_socket);
    close(server_fd);
    free(bits_comprimidos);
    while(1){
    }
}



int main(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <IP>\n", argv[0]);
        return EXIT_FAILURE;
    }
    start_server(argv[1]);
    return 0;
}




/*
127.0.0.1
127.0.0.2
127.0.0.3

        "Hola desde el cliente a 127.0.0.1:12345",
        "Mensaje para 127.0.0.2:12345",
        "Saludos al servidor en 127.0.0.3:12345"
*/