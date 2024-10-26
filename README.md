# Proyecto-2-SO
Compresor de archivos con árboles de Hauffman utilizando varios servidores
Integrantes:
- Santiago Ramos Arroyo
- Yosward García Tellez

## Considraciones antes de ejecutar el programa
Poner nombres de los archivos en configuracion.txt
Poner al final de configuracion.txt en cada linea la ip:puerto de cada servidor a usar
Se debe ejecutar primero los servidores


## Para ejecutar cada uno de los servidores:
Compilar:
`gcc servidor.c -o servidor`
Ejecutar (poner ip y puerto real):
`./servidor 127.100.100.100:1024`

## Para comprimir un archivo
Compilar y ejecutar:
`make MODE=compress`

## Para descomprimir un archivo
Compilar y ejecutar:
`make MODE=decompress`