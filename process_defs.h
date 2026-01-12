#ifndef PROCESS_DEFS_H
#define PROCESS_DEFS_H

#define MAX_PROCESSES 100

// Enumerare pentru lizibilitate
typedef enum {
    STATE_NEW,      // Tocmai citit
    STATE_READY,    // Asteapta in coada (Ready Queue)
    STATE_RUNNING,  // Este pe CPU
    STATE_FINISHED  // S-a terminat
} ProcessState;

typedef struct {
    // Date citite din fisier (Nu se modifica)
    int pid;            // ID-ul procesului
    int arrival_time;   // Momentul sosirii
    int burst_time;     // Cat timp de CPU are nevoie total
    int priority;       // Prioritate (pentru Priority Scheduling) - 1 prioritate maxima
    int deadline;       // Pentru Real-Time (Earliest Deadline First)

    // Date dinamice (Se modifica in timpul simularii)
    int remaining_time; // Necesar pentru priority scheduling! (Initial = burst_time)
    
    // Statistici temporale
    int start_time;     // Momentul primei executii. Initializati cu -1! (Daca e 0, inseamna ca incepe la 0, nu ca nu a inceput)
    int finish_time;    // Momentul cand remaining_time ajunge la 0
    
    // Statistici calculate
    int waiting_time;   // Cat a stat in READY (Finish - Arrival - Burst)
    int turnaround_time;// Timpul total in sistem (Finish - Arrival)
    int response_time;  // Timpul pana la prima reactie (Start - Arrival)
    
    ProcessState state; 
} Process;

typedef struct {
    Process* items[MAX_PROCESSES]; 
    int size; 
} ProcessQueue;

#endif