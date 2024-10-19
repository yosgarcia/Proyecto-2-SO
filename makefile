TARGET = compresor

SRC = main.c huffman.c

OBJ = $(SRC:.c=.o)

CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS = -lm -lpthread

# Regla por defecto (compilar y ejecutar)
all: $(TARGET)
	./$(TARGET)

# Regla para compilar el ejecutable
$(TARGET): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

# Regla para compilar archivos fuente a objetos
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regla para limpiar archivos generados
clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean