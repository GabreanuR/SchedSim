#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <errno.h> 
#include "process_defs.h"
#include "generator.h" 
#include "input.h"
#include "scheduler.h"

void print_usage() {
    fprintf(stderr, "[Eroare][Main] Utilizare gresita!\n");
    fprintf(stderr, "Sintaxa: ./schedsim <algoritm> [-g <nr_procese>]\n");
    fprintf(stderr, "Algoritmi suportati:\n");
    fprintf(stderr, "  priority  : Priority Scheduling (Preemptive)\n");
    fprintf(stderr, "  edf       : Earliest Deadline First (Real-Time)\n\n");
    fprintf(stderr, "Exemple:\n");
    fprintf(stderr, "  ./schedsim priority          (Citeste din input.txt)\n");
    fprintf(stderr, "  ./schedsim edf -g 50         (Genereaza 50 procese si ruleaza EDF)\n");
}

int main(int argc, char *argv[]) {
    // In caz de erori de buffer la printf
    setbuf(stdout, NULL);
    // Variabila pentru codul de iesire
    int exit_status = 0;

    printf("[Main] SchedSim: Simulator Planificare Procese\n\n");

    // Verificare minima argumente 
    if (argc < 2) {
        print_usage();
        return 1;
    }

    // Determinare Algoritm
    SchedulingAlgorithm selected_algo;
    
    if (strcmp(argv[1], "priority") == 0) {
        selected_algo = SCHED_PRIORITY;
    } else if (strcmp(argv[1], "edf") == 0) {
        selected_algo = SCHED_EDF;
    } else {
        fprintf(stderr, "[Eroare][Main] Algoritm necunoscut: '%s'\n", argv[1]);
        print_usage();
        return 1;
    }

    // Fisierul de input implicit
    const char *input_filename = "input.txt";
    // Flag ca sa stim daca generam random sau daca folosim fisierul implicit
    int mod_generare = 0;

    // Validare argumente linie comanda
    if (argc == 4){
        // Cazul 1: Avem 4 argumente. Verificam strict daca al treilea este "-g"
        if (strcmp(argv[2], "-g") == 0) {
            // Verificam daca al patrulea argument este un numar valid
            char *endptr;
            // Resetam errno inainte de conversie
            errno = 0;
            long val = strtol(argv[3], &endptr, 10);
            // Validam conversia
            if (*endptr != '\0' || errno == ERANGE || val <= 0 || val > MAX_PROCESSES) {
                fprintf(stderr, "[Eroare][Main] Numar invalid (Max: %d, Pozitiv, Numeric).\n", MAX_PROCESSES);
                return 1;
            }
            int count = (int)val;
            // Daca totul e ok, configuram generarea
            input_filename = "random_input.txt";
            mod_generare = 1;
            // Verificam daca generarea a reusit
            if (generate_file(count, input_filename) == -1) {
                fprintf(stderr, "[Eroare] Generarea a esuat.\n");
                exit_status = 1; 
                goto cleanup; 
            }
        } else {
            // Caz 2: Avem 4 argumente, dar al treilea argument nu e "-g"
            fprintf(stderr, "[Eroare][Main] Argument necunoscut '%s'. Foloseste '-g'.\n", argv[2]);
            return 1;
        }
    } else if (argc > 2) {
        // Caz 3: Avem numar de argumente gresite (ex: ./schedsim edf salut sau ./schedsim edf -g)
        print_usage();
        return 1;
    }

    // Sa fim anuntati modul de functionare
    printf("--------------------------------------------------\n");
    printf("[Info][Main] Algoritm: %s\n", (selected_algo == SCHED_PRIORITY) ? "Priority" : "EDF");
    printf("[Info][Main] Mod: %s\n", mod_generare ? "AUTOMAT (Generat)" : "MANUAL (Fisier existent)");
    printf("[Info][Main] Input: %s\n", input_filename);
    printf("--------------------------------------------------\n\n");

    // Initializam si citim procesele din fisier
    ProcessQueue all_processes = {0};
    
    printf("[Main] Citire procese din fisier, intram in read_input...\n\n");
    if (read_input(input_filename, &all_processes) == -1) {
        fprintf(stderr, "[Eroare][Main] Citirea a esuat.\n");
        exit_status = 1;
        goto cleanup;
    }
    printf("[Main] Revenim in main dupa read_input.\n");

    if (all_processes.size == 0) {
        fprintf(stderr, "[Atentie][Main] Nu au fost incarcate procese. Oprire.\n");
        goto cleanup; 
    }

    // Executam scheduler-ul
    printf("[Main] Predare control catre Scheduler...\n");
    if (run_scheduler(&all_processes, selected_algo) == -1) {
        fprintf(stderr, "[Eroare][Main] Simularea a esuat.\n");
        exit_status = 1;
        goto cleanup;
    }
    printf("\n[Main] Revenim in main dupa Scheduler.\n");

cleanup:
    // Curatam memoria alocata pentru procese
    printf("\n[Main] Eliberare memorie...\n");
    for (int i = 0; i < all_processes.size; i++) {
        if (all_processes.items[i] != NULL) {
            free(all_processes.items[i]);
        }
    }
    printf("[Main] Memorie eliberata cu succes.\n");

    if (exit_status == 0) {
        printf("\n[Main] SchedSim: Final Simulare cu SUCCES\n");
    } else {
        printf("\n[Main] SchedSim: Final Simulare cu ERORI\n");
    }

    return exit_status;
}