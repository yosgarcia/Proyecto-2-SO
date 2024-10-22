// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFFER_SIZE 1024
#define FREQUENCY_SIZE 256

void start_server(const char *ip){
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    unsigned char buffer[BUFFER_SIZE] = {0};
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

    // Aceptar conexiones y recibir mensajes
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        // Leer el mensaje del cliente
        ssize_t read_size = read(new_socket, buffer, BUFFER_SIZE);
        if (read_size > 0) {
            // Calcular la frecuencia de bytes
            for (int i = 0; i < read_size; i++) {
                frequency[buffer[i]]++;
            }
        }

        // Enviar el arreglo de frecuencias al cliente
        send(new_socket, frequency, sizeof(frequency), 0);
        close(new_socket);
        for (int i =0;i<256;i++){
            frequency[i] = 0;
        }
    }

    close(server_fd);
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