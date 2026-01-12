#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process_defs.h"

// Definim un tip nou de dată pentru a alege algoritmul
typedef enum {
    SCHED_PRIORITY, // 0
    SCHED_EDF       // 1 (Earliest Deadline First)
} SchedulingAlgorithm;

// Returneaza 0 (Succes) sau -1 (Eroare)
int run_scheduler(ProcessQueue *queue, SchedulingAlgorithm algorithm);

#endif