CC = gcc
CFLAGS = -g -pthread -lrt -Wall

all: parque gerador

parque: info.h parque.c
				$(CC) parque.c -o parque $(CFLAGS)
gerador: info.h gerador.c
				$(CC) gerador.c -o gerador $(CFLAGS)

clean:
			rm -f parque gerador
