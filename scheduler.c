#include <stdio.h>
#include "scheduler.h"

// Funcții interne (schelet)
// Le facem să returneze int ca să poată semnala erori pe viitor
int run_priority_scheduling(ProcessQueue *queue) {
    printf("[Scheduler] Rulare algoritm: PRIORITY SCHEDULING\n");
    
    // ... Aici va fi codul colegei ...
    // Dacă apare o eroare critică, ea va scrie: return -1;

    return 0; // Succes
}

int run_edf_scheduling(ProcessQueue *queue) {
    printf("[Scheduler] Rulare algoritm: EARLIEST DEADLINE FIRST\n");
    
    // ... Aici va fi codul colegei ...

    return 0; // Succes
}

// Funcția principală (Dispatcher)
int run_scheduler(ProcessQueue *queue, SchedulingAlgorithm algorithm) {
    printf("\n[Scheduler] Pornire motor simulare...\n");
    printf("[Scheduler] Analiza a %d procese.\n", queue->size);

    int result = 0;

    switch (algorithm) {
        case SCHED_PRIORITY:
            result = run_priority_scheduling(queue);
            break;
        
        case SCHED_EDF:
            result = run_edf_scheduling(queue);
            break;
            
        default:
            fprintf(stderr, "[Scheduler] Eroare: Algoritm intern necunoscut!\n");
            return -1; // Eroare critică
    }

    if (result == -1) {
        fprintf(stderr, "[Scheduler] Eroare fatala in timpul simularii.\n");
        return -1;
    }

    printf("[Scheduler] Simulare terminata.\n");
    return 0;
}

// MAIA TU AICI