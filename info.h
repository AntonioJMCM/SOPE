#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

#define PERMISSIONS S_IRUSR | S_IWUSR
#define SEMNAME "/Semaforo"
#define BUF_SIZE 16

typedef struct {
    int id;
    char acesso;
    clock_t tempoEstacionar;
} info;
