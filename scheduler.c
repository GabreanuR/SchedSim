#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"

typedef struct {
    int pid; // -1 pentru IDLE
    int start;
    int end;
} DiagSeg;

// Adauga un segment nou in diagrama (cu realocare dinamica)
static void diag_add(DiagSeg **seg, int *count, int *cap, int pid, int start, int end) {
    if (start >= end) return; // Ignoram segmentele de lungime 0

    // Optimizare: Daca ultimul segment e acelasi proces, il extindem
    if (*count > 0 && (*seg)[(*count) - 1].pid == pid && (*seg)[(*count) - 1].end == start) {
        (*seg)[(*count) - 1].end = end;
        return;
    }

    // Realocare daca vectorul e plin
    if (*count >= *cap) {
        int new_cap = (*cap == 0) ? 16 : (*cap * 2);
        DiagSeg *tmp = (DiagSeg *)realloc(*seg, new_cap * sizeof(DiagSeg));
        if (!tmp) {
            perror("[Eroare][Scheduler] Nu s-a putut realoca memoria pentru diagrama");
            exit(1); // Eroare critica de sistem
        }
        *seg = tmp;
        *cap = new_cap;
    }

    // Adaugare segment nou
    (*seg)[*count].pid = pid;
    (*seg)[*count].start = start;
    (*seg)[*count].end = end;
    (*count)++;
}

// Afiseaza Diagrama Gantt Compacta
static void diag_print(DiagSeg *seg, int count) {
    if (count == 0) return;

    printf("\n[Scheduler] Diagrama Gantt:\n");
    printf("[Timeline] ");

    // Linia cu procesele
    for (int i = 0; i < count; i++) {
        if (seg[i].pid == -1)
            printf("| %-4s ", "IDLE");
        else
            printf("| P%-3d ", seg[i].pid);
    }
    printf("|\n");

    // Linia cu timpii
    printf("[Time]     ");
    printf("%-7d", seg[0].start); // Primul timp

    for (int i = 0; i < count; i++) {
        // Calculam spatierea pentru a alinia numerele sub barele verticale
        printf("%-7d", seg[i].end);
    }
    printf("\n\n");
}

// Tie-breaker: 1. Arrival Time, 2. PID
static int tie(Process *a, Process *b) {
    if(a->arrival_time != b->arrival_time)
        return a->arrival_time < b->arrival_time;
    return a->pid < b->pid;
}

// Selectie Priority: Cel mai mic numar = prioritate maxima
static int priority(Process **r, int r_size) {
    int best = 0;
    for(int i = 1; i < r_size; i++) {
        if(r[i]->priority < r[best]->priority) best = i;
        else if (r[i]->priority == r[best]->priority && tie(r[i], r[best])) best = i;
    }
    return best;
}

// Selectie EDF: Cel mai mic deadline = urgenta maxima
static int edf(Process **r, int r_size) {
    int best = 0;
    for(int i = 1; i < r_size; i++) {
        if(r[i]->deadline < r[best]->deadline) best = i;
        else if (r[i]->deadline == r[best]->deadline && tie(r[i], r[best])) best = i;
    }
    return best;
}

// Resetare metrici inainte de rulare
static void init_metrics(ProcessQueue *queue) {
    for(int i = 0; i < queue->size; i++) {
        Process *p = queue->items[i];
        p->remaining_time = p->burst_time;
        p->state = STATE_NEW; // Resetam starea
        p->start_time = -1;
        p->finish_time = 0;
        p->waiting_time = 0;
        p->turnaround_time = 0;
        p->response_time = 0;
    }
}

static void print_table(ProcessQueue *queue, int is_edf) {
    printf("[Scheduler] Tabel statistici finale:\n");
    
    if (is_edf) {
        printf("-----------------------------------------------------------------------------\n");
        printf("| PID | Arr  | Burst | Pri  | Dead  | Start | Finish | Wait | TAT  | Miss |\n");
        printf("-----------------------------------------------------------------------------\n");
    } else {
        printf("----------------------------------------------------------------------\n");
        printf("| PID | Arr  | Burst | Pri  | Start | Finish | Wait | TAT  | Resp |\n");
        printf("----------------------------------------------------------------------\n");
    }

    for (int i = 0; i < queue->size; i++) {
        Process *p = queue->items[i];
        
        if (is_edf) {
            int miss = (p->finish_time > p->deadline) ? 1 : 0;
            printf("| P%-3d| %-5d| %-6d| %-5d| %-6d| %-6d| %-7d| %-5d| %-5d| %-5s|\n",
                   p->pid, p->arrival_time, p->burst_time, p->priority, p->deadline,
                   p->start_time, p->finish_time,
                   p->waiting_time, p->turnaround_time, 
                   miss ? "YES" : "NO");
        } else {
            printf("| P%-3d| %-5d| %-6d| %-5d| %-6d| %-7d| %-5d| %-5d| %-5d|\n",
                   p->pid, p->arrival_time, p->burst_time, p->priority,
                   p->start_time, p->finish_time,
                   p->waiting_time, p->turnaround_time, p->response_time);
        }
    }
    printf("----------------------------------------------------------------------\n");
}

static void print_summary(ProcessQueue *queue, int total_time, int busy_time) {
    long sum_wait = 0, sum_tat = 0, sum_resp = 0;

    for(int i = 0; i < queue->size; i++) {
        Process *p = queue->items[i];
        sum_wait += p->waiting_time;
        sum_tat += p->turnaround_time;
        sum_resp += p->response_time;
    }

    double avg_wait = (queue->size > 0) ? (double)sum_wait / queue->size : 0.0;
    double avg_tat = (queue->size > 0) ? (double)sum_tat / queue->size : 0.0;
    double avg_resp = (queue->size > 0) ? (double)sum_resp / queue->size : 0.0;
    
    double cpu_util = (total_time > 0) ? 100.0 * (double)busy_time / total_time : 0.0;

    printf("\n[Scheduler] Rezumat performanta:\n");
    printf("   > Timp Total Simulare : %d unitati\n", total_time);
    printf("   > Timp CPU Activ      : %d unitati\n", busy_time);
    printf("   > Utilizare CPU       : %.2f%%\n", cpu_util);
    printf("   > Avg Turnaround Time : %.2f\n", avg_tat);
    printf("   > Avg Waiting Time    : %.2f\n", avg_wait);
    if(avg_resp > 0) printf("   > Avg Response Time   : %.2f\n", avg_resp);
}

// Definim tipul functiei pointer
typedef int(*SelectFunc)(Process **r, int r_size);

static int run(ProcessQueue *queue, SelectFunc select_func, int is_edf_mode) {
    init_metrics(queue); // Resetam procesele
    
    int n = queue->size;
    int completed = 0;
    int current_time = 0;
    int busy_time = 0;

    // Ready Queue (Vector de pointeri catre procese)
    Process **ready_queue = (Process **)malloc(n * sizeof(Process *));
    int rq_size = 0;

    if (!ready_queue) {
        fprintf(stderr, "[Eroare][Scheduler] Nu s-a putut aloca memoria interna (Ready Queue).\n");
        return -1;
    }

    // Variabile pentru Gantt
    DiagSeg *segments = NULL;
    int seg_count = 0;
    int seg_cap = 0;
    
    int current_pid = -1; // -1 = IDLE
    int seg_start_time = 0;

    // --- BUCLA PRINCIPALA (Tick-by-Tick) ---
    while (completed < n) {
        
        // 1. Verificam ce procese noi au sosit
        for (int i = 0; i < n; i++) {
            Process *p = queue->items[i];
            // Folosim p->state pentru a sti daca l-am bagat deja in coada
            if (p->state == STATE_NEW && p->arrival_time <= current_time) {
                ready_queue[rq_size++] = p;
                p->state = STATE_READY; 
            }
        }

        // 2. CPU IDLE?
        if (rq_size == 0) {
            if (current_pid != -1) {
                // Tocmai am terminat un proces, intram in IDLE
                diag_add(&segments, &seg_count, &seg_cap, current_pid, seg_start_time, current_time);
                current_pid = -1;
                seg_start_time = current_time;
            }
            current_time++; // Timpul trece chiar daca nu facem nimic
            continue; 
        }

        // 3. Selectia Procesului (Priority sau EDF)
        int selected_idx = select_func(ready_queue, rq_size);
        Process *p = ready_queue[selected_idx];

        // 4. Context Switch (Schimbare proces in executie)
        if (current_pid != p->pid) {
            diag_add(&segments, &seg_count, &seg_cap, current_pid, seg_start_time, current_time);
            current_pid = p->pid;
            seg_start_time = current_time;
        }

        // 5. Start Logic
        if (p->start_time == -1) {
            p->start_time = current_time;
            p->response_time = p->start_time - p->arrival_time;
        }
        p->state = STATE_RUNNING;

        // 6. Executie (1 unitate de timp)
        p->remaining_time--;
        busy_time++;
        current_time++;

        // 7. Terminare Proces
        if (p->remaining_time == 0) {
            p->state = STATE_FINISHED;
            p->finish_time = current_time;
            
            p->turnaround_time = p->finish_time - p->arrival_time;
            p->waiting_time = p->turnaround_time - p->burst_time;

            // Eliminare din Ready Queue (Shift stanga)
            for (int k = selected_idx; k < rq_size - 1; k++) {
                ready_queue[k] = ready_queue[k + 1];
            }
            rq_size--;
            completed++;
        }
    }

    // Inchidem ultimul segment ramas deschis
    diag_add(&segments, &seg_count, &seg_cap, current_pid, seg_start_time, current_time);

    // --- AFISARE REZULTATE ---
    diag_print(segments, seg_count);
    print_table(queue, is_edf_mode);
    print_summary(queue, current_time, busy_time);

    // Curatenie memorie interna
    free(segments);
    free(ready_queue);
    
    return 0; // Succes
}

int run_priority_scheduling(ProcessQueue *queue) {
    if (!queue) return -1;
    printf("\n[Scheduler] >>> Rulare Algoritm: PRIORITY SCHEDULING (Preemptiv) <<<\n");
    
    // Apelam motorul cu functia de selectie 'priority' si modul 0 (Non-EDF)
    if (run(queue, priority, 0) == -1) { 
        fprintf(stderr, "[Eroare][Scheduler] Algoritmul Priority a esuat intern.\n");
        return -1;
    }
    return 0;
}

int run_edf_scheduling(ProcessQueue *queue) {
    if (!queue) return -1;
    printf("\n[Scheduler] >>> Rulare Algoritm: REAL-TIME (Earliest Deadline First) <<<\n");
    
    // Apelam motorul cu functia de selectie 'edf' si modul 1 (EDF)
    if (run(queue, edf, 1) == -1) {
        return -1;
    }

    // Analiza specifica Real-Time (Post-Processing)
    int misses = 0;
    for(int i = 0; i < queue->size; i++) {
         Process *p = queue->items[i];
        if(p->finish_time > p->deadline) misses++;
    }

    printf("\n[Scheduler] Analiza Fezabilitate (Real-Time):\n");
    if(misses > 0)
        printf("   [FAIL] %d procese au depasit deadline-ul! Sistemul este supraincarcat.\n", misses);
    else
        printf("   [OK]   Sistem FEZABIL! Toate procesele au terminat inainte de deadline.\n");

    return 0;
}

int run_scheduler(ProcessQueue *queue, SchedulingAlgorithm algorithm) {
    // 1. Validari de baza (Programare Defensiva)
    if (queue == NULL) {
        fprintf(stderr, "[Eroare][Scheduler] Pointerul catre coada este NULL!\n");
        return -1;
    }
    if (queue->size == 0) {
        printf("[Avertisment][Scheduler] Coada este goala. Nu sunt procese de planificat.\n");
        return 0;
    }
    if (queue->items == NULL) {
        fprintf(stderr, "[Eroare][Scheduler] Structura interna a cozii este corupta.\n");
        return -1;
    }

    printf("\n[Scheduler] Initializare motor simulare...\n");
    printf("[Scheduler] Se analizeaza %d procese.\n", queue->size);

    int result = 0;

    // 2. Selectie Algoritm
    switch (algorithm) {
        case SCHED_PRIORITY:
            result = run_priority_scheduling(queue);
            break;
        
        case SCHED_EDF:
            result = run_edf_scheduling(queue);
            break;
            
        default:
            fprintf(stderr, "[Eroare][Scheduler] Algoritm necunoscut (Cod: %d)!\n", algorithm);
            return -1; 
    }

    // 3. Verificare Rezultat
    if (result != 0) {
        fprintf(stderr, "[Eroare][Scheduler] Simulare oprita prematur din cauza unei erori interne.\n");
        return -1;
    }

    printf("\n[Scheduler] Simulare finalizata cu succes.\n");
    return 0;
}