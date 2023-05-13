#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include "gol.h"

//Declaracao das var globais para percorrer a matriz
int linha_atual, coluna_atual, n_threads;

//Declaracao dos mutexes utilizados
pthread_mutex_t matrix_mutex, stats_mutex;

int main(int argc, char **argv)
{
    int size, steps;
    cell_t **prev, **next, **tmp;
    FILE *f;
    stats_t stats_step = {0, 0, 0, 0};
    stats_t stats_total = {0, 0, 0, 0};

    if (argc != 3)
    {
        printf("ERRO! Você deve digitar %s <nome do arquivo do tabuleiro> <número de threads!>\n\n", argv[0]);
        return 0;
    }

    if ((f = fopen(argv[1], "r")) == NULL)
    {
        printf("ERRO! O arquivo de tabuleiro '%s' não existe!\n\n", argv[1]);
        return 0;
    }

    n_threads = atoi(argv[2]);
    /*  Inicialização threads */
    pthread_t threads[n_threads];

    fscanf(f, "%d %d", &size, &steps);

    prev = allocate_board(size);
    next = allocate_board(size);

    read_file(f, prev, size);

    fclose(f);

#ifdef DEBUG
    printf("Initial:\n");
    print_board(prev, size);
    print_stats(stats_step);
#endif

    // Inicia mutex
    pthread_mutex_init(&matrix_mutex, NULL);
    for (int i = 0; i < steps; i++)
    {
        // Define linha e coluna inicial para cada passo do loop for
        linha_atual = 0;
        coluna_atual = 0;

        for (int j = 0; j < n_threads; j++) {
            pthread_create(&threads[j], NULL, play(prev, next, size), NULL);
        }   

        //Aguarda elas terminarem...
        for (int j = 0; j < n_threads; j++) {
            pthread_join(threads[j], NULL);
        }
        //stats_step = play(prev, next, size);
        
        stats_total.borns += stats_step.borns;
        stats_total.survivals += stats_step.survivals;
        stats_total.loneliness += stats_step.loneliness;
        stats_total.overcrowding += stats_step.overcrowding;

#ifdef DEBUG
        printf("Step %d ----------\n", i + 1);
        print_board(next, size);
        print_stats(stats_step);
#endif
        tmp = next;
        next = prev;
        prev = tmp;
    }
    //Destroi mutex
    pthread_mutex_destroy(&matrix_mutex);

#ifdef RESULT
    printf("Final:\n");
    print_board(prev, size);
    print_stats(stats_total);
#endif

    free_board(prev, size);
    free_board(next, size);
}
