#include <stdio.h>
#include <stdlib.h>
#include "input.h"
#include "process_defs.h"

// Aloca memorie si initializeaza un proces
// Returneaza pointer catre Process sau NULL in caz de eroare
static Process* create_process(int pid, int arrival, int burst, int priority, int deadline) {
    // Alocare memorie
    Process* p = (Process*)malloc(sizeof(Process));
    if (p == NULL) {
        fprintf(stderr, "[Eroare][Input] Nu s-a putut aloca memorie pentru procesul PID %d!\n", pid);
        return NULL; // Propagam eroarea, nu dam exit(1)
    }

    // 1. Setam datele statice (Input)
    p->pid = pid;
    p->arrival_time = arrival;
    p->burst_time = burst;
    p->priority = priority;
    p->deadline = deadline;

    // 2. Initializam datele dinamice
    p->state = STATE_NEW;
    p->remaining_time = burst; // La inceput, trebuie sa execute tot timpul (burst)
    
    // 3. Initializam statisticile cu valori "nule" sau sigure
    p->start_time = -1;  // -1 inseamna ca inca nu a inceput sa ruleze
    p->finish_time = 0;
    p->waiting_time = 0;
    p->turnaround_time = 0;
    p->response_time = 0;

    return p;
}

int read_input(const char* filename, ProcessQueue* queue) {
    printf("[Input] Am intrat in functia read_input.\n");
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        // perror va afisa automat cauza (ex: No such file, Permission denied)
        perror("[Eroare][Input] Deschidere fisier"); 
        return -1; 
    }
    printf("[Input] Am deschis fisierul '%s'.\n", filename);

    int arrival, burst, priority, deadline;
    int count = 0;

    printf("[Input] Se citeste din fisierul: %s, in forma asta:\n\n", filename);
    printf("--------------------------------------------------\n");
    printf("| PID | Arrival | Burst | Priority | Deadline |\n");
    printf("--------------------------------------------------\n\n");

    // Citim cat timp gasim 4 numere pe linie
    while (fscanf(file, "%d %d %d %d", &arrival, &burst, &priority, &deadline) == 4) {
        // Verificare limite memorie
        if (queue->size >= MAX_PROCESSES) {
            printf("[Avertisment][Input] S-a atins limita de %d procese. Ignoram restul.\n", MAX_PROCESSES);
            break; 
        }
        // Validare logica a datelor citite
        if (burst <= 0) {
            fprintf(stderr, "[Eroare][Input] Linie invalida la procesul #%d (Burst <= 0).\n", count + 1);
            fclose(file);
            return -1; 
        }
        if (arrival < 0) {
            fprintf(stderr, "[Eroare][Input] Linie invalida la procesul #%d (Arrival < 0).\n", count + 1);
            fclose(file);
            return -1;
        }
        if (priority < 0) {
            fprintf(stderr, "[Eroare][Input] Linie invalida la procesul #%d (Priority < 0).\n", count + 1);
            fclose(file);
            return -1;
        }

        // 3. Generare PID Automat
        int current_pid = count + 1;

        // 4. Creare Proces
        Process* new_proc = create_process(current_pid, arrival, burst, priority, deadline);
        if (new_proc == NULL) {
            // Daca malloc a esuat, oprim citirea si returnam eroare
            fclose(file);
            return -1;
        }
        
        // 5. Adaugare in coada
        queue->items[queue->size] = new_proc;
        queue->size++;
        count++;
    }

    printf("--------------------------------------------------\n");
    
    // Verificam daca citirea s-a oprit prematur (din cauza formatului gresit)
    // Daca nu am ajuns la EOF (End Of File), inseamna ca fscanf s-a blocat in ceva (ex: litere)
    if (!feof(file)) {
        fprintf(stderr, "[Eroare][Input] Fisier corupt! S-au gasit date invalide/format gresit.\n");
        fprintf(stderr, "[Eroare][Input] Citirea a fost oprita la linia %d.\n", count + 1);
        fclose(file);
        return -1; 
    }
    // Verificam daca am citit macar un proces
    if (count == 0) {
        fprintf(stderr, "[Eroare][Input] Fisierul este gol sau nu contine nicio linie valida.\n");
        fclose(file);
        return -1; 
    }

    printf("[Input] Am citit cu succes si inchidem fisierul '%s'.\n\n", filename);
    fclose(file);

    printf("[Input] Procesele incarcate sunt:\n\n");

    printf("--------------------------------------------------\n");
    printf("| PID | Arrival | Burst | Priority | Deadline |\n");
    printf("--------------------------------------------------\n");

    for (int i = 0; i < queue->size; i++) {
        Process* p = queue->items[i];
        printf("| %3d | %7d | %5d | %8d | %8d |\n", 
               p->pid, p->arrival_time, p->burst_time, p->priority, p->deadline);
    }

    printf("--------------------------------------------------\n");
    printf("[Input] S-au incarcat cu succes %d procese.\n\n", count);

    return 0; 
}