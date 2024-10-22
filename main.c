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


Config* config;
int arreglo_frecuencias [256]; 
int respuestas_recibidas;

void enviar_arreglo_bytes_a_server(const char *ip, unsigned char *message, size_t message_size) {
    int sock;
    struct sockaddr_in serv_addr;
    int arreglo_recibido[256] = {0};

    // Crear el socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convertir la dirección IPv4 y la dirección del servidor
    
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        close(sock);
        return;
    }
    
    // Conectar al servidor
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return;
    }

    // Enviar el mensaje
    send(sock, message, message_size, 0);
    
    // Recibir el arreglo de frecuencias
    recv(sock, arreglo_recibido, sizeof(arreglo_recibido), 0);

    // Sumarlo a las frecuencias
    for (int i = 0; i < 256; i++) {
        arreglo_frecuencias[i]+=arreglo_recibido[i];
        if (arreglo_recibido[i] > 0) {
            printf("\nByte %c: %d\n", i, arreglo_recibido[i]);
        }
    }
    free(message);
    //Crear el arbol huffman y enviar de nuevo a los servidores
    
    close(sock);
    
}

int main() {
    config = leer_configuracion("configuracion.txt");
    respuestas_recibidas = 0;
    lu num_servidores = config->num_servidores;

    FILE *archivo = fopen(config->archivo_a_comprimir, "rb");
    if(archivo == NULL){ printf("No se puedo abrir el archivo %s\n",config->archivo_a_comprimir); return 1;}

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
    unsigned char juntado [100000];

    
    for (lu i = 0; i < num_servidores; i++) {
        long tamanno_bloque = bytes_por_servidor;
        //se van a leer menos bytes, estoy en el final
        if (i==num_servidores-1 && residuo>0){
            tamanno_bloque += residuo;
        }
        //num servidores = 8
        //residuo = 2
        unsigned char *buffer = calloc(tamanno_bloque,sizeof (unsigned char));
        size_t bytes_read = fread(buffer, 1, tamanno_bloque, archivo);

        //int inicio = i*bytes_por_servidor;
        //for (int x =0;x<tamanno_bloque;x++){
        //    juntado[inicio+x] = buffer[x];
        printf("ip meant: %s\n",config->ips[i]);
        printf("mandando: %s\n",buffer);
        //}
        enviar_arreglo_bytes_a_server(config->ips[i], buffer, bytes_read);
    }
    //printf("juntado: %s\n",juntado);

    return 0;
}