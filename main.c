#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_FILOSOFOS 5
#define NUM_REFEICOES 3

sem_t hashis[NUM_FILOSOFOS];
sem_t garcom;
sem_t mutexPrint;

int qntRefeicoes[NUM_FILOSOFOS] = {0};
double tempoEspera[NUM_FILOSOFOS] = {0};

// Função para simular um tempo de espera aleatório
void esperarTempoAleatorio(unsigned int *seed, int minimoMs, int maximoMs) {
    int tempo = minimoMs + rand_r(seed) % (maximoMs - minimoMs + 1);
    usleep(tempo * 1000);
}

// Função para imprimir o estado do filósofo de forma sincronizada
void imprimirEstado(int id, const char *estado) {
    sem_wait(&mutexPrint);
    printf("Filosofo %d %s\n", id, estado);
    sem_post(&mutexPrint);
}

int main() {
    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        sem_init(&hashis[i], 0, 1);
    }

    sem_init(&garcom, 0, NUM_FILOSOFOS - 1);
    sem_init(&mutexPrint, 0, 1);

    printf("Iniciando o jantar dos filosofos...\n\n");

    #pragma omp parallel num_threads(NUM_FILOSOFOS)
    {
        int id = omp_get_thread_num();

        int hashiEsq = id;
        int hashiDir = (id + 1) % NUM_FILOSOFOS;

        unsigned int seed = time(NULL) + id * 100; // Semente para geração de números aleatórios

        for (int refeicao = 0; refeicao < NUM_REFEICOES; refeicao++) {
            
            #pragma omp single
            {
                printf("\n--- Rodada %d ---\n", refeicao + 1);
            }
            
            imprimirEstado(id, "esta pensando");
            esperarTempoAleatorio(&seed, 500, 1500);

            imprimirEstado(id, "esta com fome");

            double inicioEspera = omp_get_wtime();

            sem_wait(&garcom); // Solicita permissão ao garçom para tentar pegar os hashis

            sem_wait(&hashis[hashiEsq]); // Pega o hashi esquerdo
            sem_wait(&hashis[hashiDir]); // Pega o hashi direito

            double fimEspera = omp_get_wtime();
            tempoEspera[id] += fimEspera - inicioEspera; // Acumula o tempo de espera para o filósofo

            imprimirEstado(id, "esta comendo");
            esperarTempoAleatorio(&seed, 500, 1200);

            qntRefeicoes[id]++;

            sem_post(&hashis[hashiEsq]);
            sem_post(&hashis[hashiDir]);

            sem_post(&garcom);

            imprimirEstado(id, "terminou de comer");
            
            #pragma omp barrier
        }

        imprimirEstado(id, "saiu da mesa");
    }

    printf("\nJantar finalizado.\n\n");
    printf("Relatorio final:\n");

    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        printf("Filosofo %d comeu %d vezes | Tempo total esperando: %.4f segundos\n",
               i,
               qntRefeicoes[i],
               tempoEspera[i]);
    }

    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        sem_destroy(&hashis[i]);
    }

    sem_destroy(&garcom);
    sem_destroy(&mutexPrint);

    return 0;
}