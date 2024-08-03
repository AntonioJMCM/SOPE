#include "info.h"

#define LOGNAME "gerador.log"

unsigned int idViatura;

static sem_t *semaforo;
static FILE *parqueLog;
static clock_t ticksInicial;

void logGerador(int ticks, int id, char porta, int tempoEstacionar, int tempoVida, char* mensagem) {
  if (tempoVida == -1)
    fprintf(parqueLog, "%-8d ; %-7d ; %-6c ; %-10d ; %-8c ; %s\n", ticks, id, porta, tempoEstacionar, '?', mensagem);
  else
    fprintf(parqueLog, "%-8d ; %-7d ; %-6c ; %-10d ; %-8d ; %s\n", ticks, id, porta, tempoEstacionar, tempoVida, mensagem);

  return;
}

void *gestaoEntrada(void * arg) {

    pthread_detach(pthread_self());

    clock_t tempoInicial = clock();
    clock_t ticks;

    info * infoVeiculo = (info *) arg;

    char pathname[7];
    sprintf(pathname, "fifo%c", infoVeiculo->acesso);

    char fifoPrivado[20];
    sprintf(fifoPrivado, "fifo%d", infoVeiculo->id);

   /*if (mkfifo(fifoPrivado, PERMISSIONS) == -1) {
      perror(fifoPrivado);
      return NULL;
    }*/

    sem_wait(semaforo);

    int fd;

    if ((fd = open(pathname, O_WRONLY)) == -1) {
      sem_post(semaforo);
      unlink(fifoPrivado);
      free(infoVeiculo);
      return NULL;
    }

    if (write(fd, infoVeiculo, sizeof(info)) == -1) {
      sem_post(semaforo);
      close(fd);
      unlink(fifoPrivado);
      free(infoVeiculo);
      return NULL;
    }
    close(fd);

    sem_post(semaforo);

    if ((fd = open(fifoPrivado, O_RDONLY)) == -1) {
      unlink(fifoPrivado);
      free(infoVeiculo);
      return NULL;
    }

    char mensagem[BUF_SIZE];

    read(fd, mensagem, BUF_SIZE);

    if (strcmp(mensagem, "entrou") == 0) {

      ticks = clock() - ticksInicial;
      logGerador((int) ticks, infoVeiculo->id, infoVeiculo->acesso, (int) infoVeiculo->tempoEstacionar, -1, mensagem);

      read(fd, mensagem, BUF_SIZE);

      ticks = clock() - ticksInicial;
      clock_t tempoVida = clock() - tempoInicial;
      logGerador((int) ticks, infoVeiculo->id, infoVeiculo->acesso, (int) infoVeiculo->tempoEstacionar, (int) tempoVida, mensagem);
    }

    else if (strcmp(mensagem, "cheio") == 0) {
      
      ticks = clock() - ticksInicial;
      logGerador((int) ticks, infoVeiculo->id, infoVeiculo->acesso, (int) infoVeiculo->tempoEstacionar, -1, mensagem);
    }

    close(fd);
    free(infoVeiculo);

    return NULL;
}

int main (int argc, char* argv[]) {
    int tempoGeracao;
    int uniRelogio;
    char acessos[4] = {'N', 'E', 'S', 'O'};
    int probabilidades[10] = {0, 0, 0, 0, 0, 1, 1, 1, 2, 2};
    idViatura = 1;

    if (argc != 3)
    {
        printf("Usage: %s <T_GERACAO> <U_RELOGIO>\n", argv[0]);
        exit(1);
    }

    tempoGeracao = (int) strtol(argv[1], NULL, 10);
    uniRelogio = (int) strtol(argv[2], NULL, 10);

    if ((semaforo = sem_open(SEMNAME, O_CREAT, PERMISSIONS, 1)) == SEM_FAILED) {
      perror("Semaforo");
      exit(2);
    }

    if ((parqueLog = fopen(LOGNAME, "w")) == NULL) {
      perror(LOGNAME);
      exit(3);
    }

    fprintf(parqueLog, "%-8s ; %-7s ; %-6s ; %-10s ;  %-6s  ; %s\n", "t(ticks)", "id_viat", "destin", "t_estacion", "t_vida", "observ");

    srand(time(NULL));

    clock_t duracao = tempoGeracao * CLOCKS_PER_SEC;
    clock_t fim;
    clock_t inicio = clock();
    clock_t anterior = inicio;

    int indiceProximo;
    int proximo = 0;

    do {
        fim = clock();

        if ((fim - anterior) >= proximo) {
            int indiceAcesso = rand() % 4;
            int tempoEstacionar = rand() % 10 + 1;

            info *viatura = malloc(sizeof(viatura));
            viatura->acesso = acessos[indiceAcesso];
            viatura->tempoEstacionar = tempoEstacionar * uniRelogio;
            viatura->id = idViatura++;

            pthread_t entrada;
            pthread_create(&entrada, NULL, gestaoEntrada, (void *) viatura);

            anterior = fim;

            indiceProximo = rand() % 10;
            proximo = probabilidades[indiceProximo] * uniRelogio;
        }

    } while(fim - inicio < duracao);

    sem_close(semaforo);
    sem_unlink(SEMNAME);

    pthread_exit(0);
  }
