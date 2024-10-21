# Variables para los nombres de los ejecutables
TARGET_COMPRESSOR = compresor
TARGET_DECOMPRESSOR = descompresor
ARCHIVO_TABLA = tabla.txt

# Fuentes comunes
SRC = huffman.c

# Fuentes específicas para el compresor y el descompresor
SRC_COMPRESSOR = main.c $(SRC)
SRC_DECOMPRESSOR = descompresor.c $(SRC)

# Objetos específicos para cada uno
OBJ_COMPRESSOR = $(SRC_COMPRESSOR:.c=.o)
OBJ_DECOMPRESSOR = $(SRC_DECOMPRESSOR:.c=.o)

# Compilador y banderas
CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS = -lm -lpthread

# Variable MODE por defecto, puede ser sobrescrita por la línea de comandos
MODE ?= compress

# Regla principal (dependiendo del modo)
all: $(MODE)

# Reglas para compilar y ejecutar el compresor
compress: $(TARGET_COMPRESSOR)
	./$(TARGET_COMPRESSOR)

$(TARGET_COMPRESSOR): $(OBJ_COMPRESSOR)
	$(CC) -o $@ $(OBJ_COMPRESSOR) $(LDFLAGS)

# Reglas para compilar y ejecutar el descompresor
decompress: $(TARGET_DECOMPRESSOR)
	./$(TARGET_DECOMPRESSOR)

$(TARGET_DECOMPRESSOR): $(OBJ_DECOMPRESSOR)
	$(CC) -o $@ $(OBJ_DECOMPRESSOR) $(LDFLAGS)

# Regla para compilar archivos fuente a objetos
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regla para limpiar archivos generados
clean:
	rm -f $(TARGET_COMPRESSOR) $(TARGET_DECOMPRESSOR) $(OBJ_COMPRESSOR) $(OBJ_DECOMPRESSOR) $(ARCHIVO_TABLA)

.PHONY: all clean compress decompress
