#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "generator.h"

#define MAX_ARRIVAL_GAP 5
#define MAX_BURST 20
#define MAX_PRIORITY 5
#define MIN_SLACK 2
#define MAX_SLACK 30

int generate_file(int num_processes, const char *filename) {
    printf("[Generator] Am intrat cu %d procese...\n", num_processes);
    // Validare defensivă a argumentelor
    if (num_processes <= 0) {
        fprintf(stderr, "[Eroare][Generator] Numar invalid de procese (%d).\n", num_processes);
        return -1;
    }
    // Deschidere fisier pentru scriere
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        perror("[Eroare][Generator] Crearea fisierului a esuat");
        return -1; 
    }

    // Initializare generator numere aleatoare
    srand(time(NULL));
    // Variabila pentru timpul curent de sosire
    int current_time = 0;

    printf("[Generator] Generare %d procese in '%s'...\n", num_processes, filename);

    // Generare procese
    for (int i = 0; i < num_processes; i++) {
        int gap = rand() % MAX_ARRIVAL_GAP; 
        current_time += gap;
        
        int arrival = current_time;
        int burst = (rand() % MAX_BURST) + 1;
        int priority = (rand() % MAX_PRIORITY) + 1;
        
        // Deadline-ul trebuie sa fie realizabil (Arrival + Burst + Slack)
        int slack = (rand() % (MAX_SLACK - MIN_SLACK + 1)) + MIN_SLACK;
        int deadline = arrival + burst + slack;

        fprintf(f, "%d %d %d %d\n", arrival, burst, priority, deadline);
    }

    printf("[Generator] Scriere in fisier finalizata.\n");

    // Verificare erori de scriere (ex: Disk Full)
    if (ferror(f)) {
        perror("[Eroare][Generator] Nu s-a putut scrie in fisier");
        fclose(f);
        return -1;
    }

    fclose(f);
    printf("[Generator] Fisier generat cu succes.\n");
    return 0; // Succes
}