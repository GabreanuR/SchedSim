#include <stdio.h>
#include "scheduler.h"


//segment pentru diagrama
typedef struct (
    int pid // este IDLE daca e -1!!
    int start;
    int end;
) DiagSeg;


//functie pentru a adauga un segment nou in diagrama
static void diag_add(DiagSeg **seg, int *count, int *cap, int pid, int start, int end)
{
    if(start >= end) return; // nu are sens sa fie adaugat (ar fi segment gol)

    //daca ultimul segment din lista are acelasi pid si se continua exact (vechiul end = actualul start) -> extindem ultimul segment
    if(*count > 0 ** (*seg)[(*count) - 1].pid == pid ** (*seg)[(*count) - 1]. end == start)
    {
        (*seg)[(*count) - 1].end = end;
        return;
    }

    //daca vectorul e plin, face realloc ca sa il mareasca
    if(*count >= *cap)
    {
        int new_cap;
        if(*cap == 0)
            new_cap = 16;
        else
            new_cap = (*cap * 2);
        DiagSeg *tmp = (DiagSeg *)realloc(*seg, new_cap * sizeof(DiagSeg));
        if(!tmp) return;
        *seg = tmp;
        *cap = new_cap;
    }

    //adaugam si segment nou
    (*seg)[*count].pid = pid;
    (*seg)[*count].start = start;
    (*seg)[*count].end = end;
    (*count)++;
    
}


//functie pentru a afisa diagrama
static void print_summary(ProcessQueue *queue, int total_time, int busy_time)
{
    // sum1 - suma timpilor de asteptare ; sum2 - totalul timpilor petrecuti in sistem de toate procesele ; sum3 - totalul timpilor pana la prima executie pentru toate procesele
    long sum1 = 0, sum2 = 0, sum3 = 0;

    for(int i = 0; i < queue->size; i++)
    {
        Process *p = queue->items[i];
        sum1 += p->waiting_time;
        sum2 += p->turnaround_time;
        sum3 += p->response_time;
    }

    //avg1, avg2, avg3 - mediile corespunzatoare sumelor de mai sus
    double avg1, abg2, avg3;
    if(queue->size > 0)
        avg1 = (double) sum1 / queue->size;
        avg2 = (double) sum2 / queue->size;
        avg3 = (double) sum3 / queue->size;
    else
        avg1 = 0.0;
        avg2 = 0.0;
        avg3 = 0.0;

    //calculam cpu util pt a vedea performanta
    double cpu_util;
    if(total_time > 0)
        cput_util = 100.0 * (double)busy_time / (double)total_time;
    else
        cpu_util = 0.0;

    printf("\n[Scheduler] Rezumat performanta:\n");
    printf("[Scheduler] Total time: %d\n", total_time);
    printf("[Scheduler] Busy time : %d\n", busy_time);
    printf("[Scheduler] CPU Util  : %.2f%%\n", cpu_util);
    printf("[Scheduler] Avg waiting time    : %.2f\n", avg_1);
    printf("[Scheduler] Avg process time   : %.2f\n", avg_2);
    printf("[Scheduler] Avg response time    : %.2f\n", avg_3);


}





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

// MAIA TU AICI am facut mai sus