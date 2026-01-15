#include <stdio.h>
#include "scheduler.h"
#include <stdlib.h>


//segment pentru diagrama
typedef struct {
    int pid; // este IDLE daca e -1
    int start;
    int end;
 } DiagSeg;


//functie pentru a adauga un segment nou in diagrama
static void diag_add(DiagSeg **seg, int *count, int *cap, int pid, int start, int end)
{
    if(start >= end) return; // nu are sens sa fie adaugat (ar fi segment gol)

    //daca ultimul segment din lista are acelasi pid si se continua exact (vechiul end = actualul start) -> extindem ultimul segment
    if(*count > 0 && (*seg)[(*count) - 1].pid == pid && (*seg)[(*count) - 1].end == start)
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


//functie pentru afisare, prima linie e cu procesele pe segmente, a doua linie cu marcajele de timp
static void diag_print(DiagSeg *seg, int count)
{
    printf("[Scheduler] Diagrama compacta:\n");
    printf("[Scheduler] ");

    for (int i = 0; i < count; i++)
    {
        if (seg[i].pid == -1)
            printf("| %-4s ", "IDLE");
        else
            printf("| P%-3d ", seg[i].pid);
    }


    printf("|\n");

    printf("[Scheduler] ");
    for (int i = 0; i < count; i++)
    {
        printf("| %-4d ", seg[i].start);
    }
    printf("|");

    if (count > 0)
        printf(" %d", seg[count - 1].end);
    printf("\n");


}

//apelam functia de mai jos daca avem 2 procese cu aceeasi prioritate
static int tie(Process *a, Process *b)
{
    if(a->arrival_time != b->arrival_time)
        return a->arrival_time < b->arrival_time; // alegem procesul care a venit mai repede
    return a->pid < b->pid; //daca au acelasi arrival_time, il algem pe cel cu pid mai mic
}


//DE MENTIONAT: 1 = PRIORITATE MAXIMA => cu cat valoarea e mai mica, cu cat este mai prioritara
//in functia de mai josm alegem care e urmatorul proces de pus in diagrama pentru varianta priority scheduling
static int priority(Process **r, int r_size)
{
    int best = 0;
    for(int i = 1; i < r_size; i++)
    {
        if(r[i]->priority < r[best]->priority) best = i; // daca gasim un proces cu o valoare mai mica => prioritate mai mare
        else if (r[i]->priority == r[best]->priority && tie(r[i], r[best])) best = i; 
        // daca gasim un proces cu ac valoare ca best-ul de pana acum, apelam functia de mai sus, ce intorcea 1 doar daca primul argument era cel pe care voiam sa il punem urmatorul
    }

    return best;
}

//DE MENTIONAT SI AICI: aceeasi regula ca mai sus! daca valoarea pt deadline e mai mica => este mai urgent!
//functia de mai jos e aceeasi ca cea de mai sus, doar ca este pentru varianta deadline
static int edf(Process **r, int r_size)
{
    int best = 0;
    for(int i = 1; i < r_size; i++)
    {
        if(r[i]->deadline < r[best]->deadline) best = i; // daca gasim un proces cu o valoare mai mica => deadline mai imp
        else if (r[i]->deadline == r[best]->deadline && tie(r[i], r[best])) best = i; 
        // daca gasim un proces cu ac valoare ca best-ul de pana acum, apelam functia de mai sus, ce intorcea 1 doar daca primul argument era cel pe care voiam sa il punem urmatorul
    }

    return best;
}

//resetam campurile pt fiecare proces
static void init_metrics(ProcessQueue *queue)
{
    for(int i = 0; i < queue->size; i++)
    {
        Process *p = queue->items[i];
        p->remaining_time = p->burst_time;
        p->start_time = -1;
        p->finish_time = -1;
        p->waiting_time = 0;
        p->turnaround_time = 0;
        p->response_time = 0;
    }
}


//afisam in functie de ce varianta a fost aleasa: daca mode va fi 0 -> folosim prioriy, daca va fi 1 -> folosim folosim
static void print_table(ProcessQueue *queue, int mode)
{
    printf("\n[Scheduler] Tabel statistici finale:\n");
    //varianta EDF
    if (mode)
    {
        printf("[Scheduler] PID  Arr  Burst  Pri  Dead  Start  Finish  WT   TAT  RT   Miss\n");
        printf("[Scheduler] -----------------------------------------------------------------\n");
        for (int i = 0; i < queue->size; i++)
        {
            Process *p = queue->items[i];
            //miss = deadline ratat
            int miss;
            if(p->finish_time > p->deadline)
                miss = 1;
            else
                miss = 0;

            printf("[Scheduler] P%-3d %-4d %-5d %-4d %-5d %-6d %-7d %-4d %-4d %-4d %-4s\n",
                   p->pid, p->arrival_time, p->burst_time, p->priority, p->deadline,
                   p->start_time, p->finish_time,
                   p->waiting_time, p->turnaround_time, p->response_time,
                   miss ? "YES" : "NO");
        }
    }
    //varianta PRIORITY 
    else
    {
        printf("[Scheduler] PID  Arr  Burst  Pri  Start  Finish  WT   TAT  RT\n");
        printf("[Scheduler] -------------------------------------------------\n");
        for (int i = 0; i < queue->size; i++) 
        {
            Process *p = queue->items[i];
            printf("[Scheduler] P%-3d %-4d %-5d %-4d %-6d %-7d %-4d %-4d %-4d\n",
                   p->pid, p->arrival_time, p->burst_time, p->priority,
                   p->start_time, p->finish_time,
                   p->waiting_time, p->turnaround_time, p->response_time);
        }
    }
}




//functie pentru a afisa rezumatul
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
    double avg1, avg2, avg3;
    if(queue->size > 0)
    {
        avg1 = (double) sum1 / queue->size;
        avg2 = (double) sum2 / queue->size;
        avg3 = (double) sum3 / queue->size;
    }
    else
    {
        avg1 = 0.0;
        avg2 = 0.0;
        avg3 = 0.0;
    }

    //calculam cpu util pt a vedea performanta
    double cpu_util;
    if(total_time > 0)
        cpu_util = 100.0 * (double)busy_time / (double)total_time;
    else
        cpu_util = 0.0;

    printf("\n[Scheduler] Rezumat performanta:\n");
    printf("[Scheduler] Total time: %d\n", total_time);
    printf("[Scheduler] Busy time : %d\n", busy_time);
    printf("[Scheduler] CPU Util  : %.2f%%\n", cpu_util);
    printf("[Scheduler] Avg waiting time    : %.2f\n", avg1);
    printf("[Scheduler] Avg turnaround time   : %.2f\n", avg2);
    printf("[Scheduler] Avg response time    : %.2f\n", avg3);


}

//definim un pointer la functie pt selectie
typedef int(*SelectFunc)(Process **r, int r_size);


static int run(ProcessQueue *queue, SelectFunc select_func, int mode)
{
    init_metrics(queue);
    int n = queue->size; //nr de procese
    int completed = 0; //cate procese s-au terminat
    int time = 0; //de cat timp in simulare
    int busy_time = 0; //cat CPU a fost ocupat

    Process **r = (Process **)malloc(n * sizeof(Process *)); //vector de procese
    int r_size = 0;

    //ne asiguram ca nu adaugam acelasi produs de mai mutle ori
    int *added = (int*)calloc(n, sizeof(int)); // adica marcam aici daca am introdus deja un proces

    if(!r || !added)
    {
        fprintf(stderr, "[Scheduler] Eroare de alocare memorie\n");
        free(r);
        free(added);
        return -1;
    }
    DiagSeg *seg = NULL;
    int seg_count = 0;
    int seg_cap = 0;

    int current_pid = -1;
    int seg_start = 0;


    while(completed < n)
    {
        //aici adaugam procese sosite
        for(int i = 0; i < n; i++)
        {
            Process *p = queue->items[i];
            if(!added[i] && p->arrival_time <= time) // verificam daca poate fi adaugat si daca nu a mai fost adaugat pana acum
            {
                r[r_size++] = p;
                added[i] = 1;
            }
        }

        if(r_size == 0)
        {
            //lucram cu CPU - daca r e goala -> avem idle cpu
            if(current_pid != -1)
            {
                diag_add(&seg, &seg_count, &seg_cap, current_pid, seg_start, time);
                current_pid = -1;
                seg_start = time;
            }
            time++;
            continue;
        }

        //alegem procesul
        int id = select_func(r, r_size);
        Process *p = r[id];

        //schimbam procesele si inchidem procesul anterior
        if(current_pid != p->pid)
        {
            diag_add(&seg, &seg_count, &seg_cap, current_pid, seg_start, time);
            current_pid = p->pid;
            seg_start = time;
        }

        if(p->start_time == -1)
        {
            p->start_time = time;
            p->response_time = p->start_time - p->arrival_time;
        }

        //ruleaza 
        p->remaining_time--;
        busy_time++;
        time++;

        //procesul a terminat
        if(p->remaining_time == 0)
        {
            p->finish_time = time;
            p->turnaround_time = p->finish_time - p->arrival_time;
            p->waiting_time = p->turnaround_time - p->burst_time;

            //scoatem din queue
            for(int j = id; j < r_size - 1; j++)
                r[j] = r[j + 1];
            r_size--;

            completed++; // maracam ca am incheiat
        }
    }

    //inchidem si ultimul segment
    diag_add(&seg, &seg_count, &seg_cap, current_pid, seg_start, time);


    //facem acum si afisarile
    diag_print(seg, seg_count);
    print_table(queue, mode);
    print_summary(queue, time, busy_time);


    free(seg);
    free(r);
    free(added);
    return 0;


}




// Funcții interne (schelet)
// Le facem să returneze int ca să poată semnala erori pe viitor
int run_priority_scheduling(ProcessQueue *queue) {
    printf("[Scheduler] Rulare algoritm: PRIORITY SCHEDULING\n");
    

    if(run(queue, priority, 0) == -1)
        return -1;

    return 0; // Succes
}

int run_edf_scheduling(ProcessQueue *queue) 
{
    printf("[Scheduler] Rulare algoritm: EARLIEST DEADLINE FIRST\n");
    
    if(run(queue, edf, 1) == -1)
        return -1;


    //
    int misses = 0;
    for(int i = 0; i < queue->size; i++)
    {
         Process *p = queue->items[i];
        if(p->finish_time > p->deadline) misses++;
    }

    if(misses > 0)
        printf("[Scheduler] %d procese au depasit deadline-ul!!\n", misses);
    else
        printf("[Scheduler] Rezultat bun si posibil! Niciun proces nu a depasit deadline-ul!\n");

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

