# SchedSim - Simulator de Planificare a Proceselor

**Curs:** Sisteme de Operare  
**Echipa:** Răzvan & Maia

## 📋 Tema Proiectului

Scopul acestui proiect este scrierea unui **simulator de procese** care să fie folosit pentru a evalua performanța algoritmilor de scheduling. Aplicația trebuie să suporte:
1.  **Priority Scheduling** (Planificare pe bază de prioritate - Preemptiv).
2.  **Real-Time Scheduling** (Earliest Deadline First - EDF).

Simulatorul va oferi utilizatorilor indicatori standard de performanță, precum:
* Utilizarea procesorului (CPU Utilization).
* Timpul de așteptare (Waiting Time).
* Timpul de răspuns (Turnaround Time).

## 📚 Surse și Bibliografie

* [GeeksForGeeks: CPU Scheduling in Operating Systems](https://www.geeksforgeeks.org/operating-systems/cpu-scheduling-in-operating-systems/)
* [GeeksForGeeks: Difference between Arrival Time and Burst Time](https://www.geeksforgeeks.org/operating-systems/difference-between-arrival-time-and-burst-time-in-cpu-scheduling/)
* [GeeksForGeeks: Earliest Deadline First (EDF)](https://www.geeksforgeeks.org/operating-systems/earliest-deadline-first-edf-cpu-scheduling-algorithm/)
* [TutorialsPoint: C function strtol](https://www.tutorialspoint.com/c_standard_library/c_function_strtol.htm)

## 📝 Convenții de Codare

Pentru a facilita debugging-ul și urmărirea fluxului de execuție, toate mesajele afișate la consolă (`printf`) trebuie să includă tag-ul modulului din care sunt apelate.

**Format:** `[Modul] Mesaj`

**Exemplu în `main.c`:**
```c
printf("[Main] SchedSim: Simulator Planificare Procese\n\n");
printf("[Scheduler] Analiza proceselor...\n");
```

## Ghid de Utilizare (Compilare și Rulare)

Proiectul dispune de un sistem automatizat de compilare (`Makefile`) pentru a ușura gestionarea dependențelor.

### 1. Compilare

Deschideți terminalul în folderul proiectului și rulați:

```bash
make
```

Această comandă va compila toate modulele (`main`, `input`, `generator`, `scheduler`) și va crea executabilul `schedsim`.

Pentru a șterge fișierele compilate (curățenie):

```bash
make clean
```

### 2. Rulare

Sintaxa generală este:
`./schedsim <algoritm> [optiuni]`

**Argumente:**
* `<algoritm>`: Poate fi `priority` sau `edf`.
* `[optiuni]`:
    * (Lipsă): Rulează în **Mod Manual** (citește din `input.txt`).
    * `-g <nr>`: Rulează în **Mod Automat** (generează `<nr>` procese în `random_input.txt` și le simulează).

### 3. Exemple Concrete

**Cazul A: Testare Manuală (Priority Scheduling)**
1.  Editați fișierul `input.txt` cu datele dorite.
2.  Rulați:
    ```bash
    ./schedsim priority
    ```

**Cazul B: Stress Test (Real-Time EDF)**
Doriți să testați algoritmul EDF cu 50 de procese generate aleator:
```bash
./schedsim edf -g 50
```

**Cazul C: Testare Manuală (EDF)**
1.  Editați fișierul `input.txt` cu datele dorite.
2.  Rulați:
    ```bash
    ./schedsim edf
    ```
    
## Detalii Implementare - Infrastructură (Partea lui Răzvan)

Această secțiune detaliază arhitectura proiectului, organizarea fișierelor și logica de gestionare a datelor.

### 1. Structura Folderelor

```text
ProiectSO
|-- README.md           # Locul unde sunt notati pasii de elaborare a proiectului
|-- Makefile            # Automatizarea compilarii
|-- main.c              # Programul principal 
|-- process_defs.h      # Definitiile structurilor de date comune
|-- input.txt           # Fisierul cu date pentru simulare (manual)
|-- random_input.txt    # Fisierul cu date pentru simulare (generat automat)
|-- generator.h         # Header pentru generator
|-- generator.c         # Logica de generare random a proceselor
|-- input.c             # Implementarea citirii din fisier
|-- input.h             # Header pentru input
|-- scheduler.c         # Logica simularii
\-- scheduler.h         # Header pentru scheduler
```

### 2. Structuri de Date (`process_defs.h`)

Structura `Process` a fost proiectată să suporte ambele tipuri de algoritmi simultan:
* Pentru **Priority Scheduling**: se folosește variabila `priority`.
* Pentru **Real-Time**: se folosește variabila `deadline`.

**Definirea structurii `Process`:**

| Tip Dată | Câmp | Descriere |
| :--- | :--- | :--- |
| **Static** | `pid` | ID unic (Calculat automat la citire) |
| **Static** | `arrival_time` | Momentul sosirii în sistem |
| **Static** | `burst_time` | Timpul total de execuție necesar |
| **Static** | `priority` | Prioritatea procesului |
| **Static** | `deadline` | Termenul limită (pentru EDF) |
| **Dinamic** | `remaining_time` | Timpul rămas de executat (pentru preempțiune) |
| **Dinamic** | `start_time` | Momentul primei accesări a CPU |
| **Dinamic** | `finish_time` | Momentul finalizării execuției |
| **Metrică** | `waiting_time` | Timpul total de așteptare în coadă |
| **Metrică** | `turnaround_time` | Timpul total petrecut în sistem |
| **Metrică** | `response_time` | Timpul de la sosire până la prima execuție |

**Gestionarea Cozilor (`ProcessQueue`):**
Simulatorul utilizează doi vectori gestionați prin structura `ProcessQueue`:
1.  **`all_processes`**: Variabila care conține toate procesele încărcate din fișier (folosită pentru evidență și curățare).
2.  **`ready_queue`**: Coada activă care conține doar procesele ajunse în sistem și pregătite de execuție.

Structura `ProcessQueue` conține:
* Un vector de pointeri către procese (`Process* items[]`).
* Dimensiunea curentă a vectorului (`size`).

### 3. Formatul Datelor de Intrare (`input.txt`)

Fișierul de intrare **nu conține header**, iar PID-ul este calculat automat (incremental) în momentul citirii pentru a simplifica formatul.

Structura coloanelor este:

| Arrival Time | Burst Time | Priority | Deadline |
| :--- | :--- | :--- | :--- |
| `int` | `int` | `int` | `int` |

**Exemplu linie:**
```text
0 10 3 100
```

### 4. Logica Generală a Aplicației (`main.c`)

Fișierul `main.c` acționează ca un orchestrator ("Dispatcher") și implementează concepte de programare defensivă:

* **Validarea Argumentelor:**
    * Selectarea algoritmului: `./schedsim priority` SAU `./schedsim edf`.
    * Modul de rulare: Manual (implicit) SAU Automat (`-g <nr_procese>`).
* **Generare Automată:** Apelează generatorul de date (`generator.c`) dacă este activat flag-ul `-g`.
* **Intrare Date:** Apelează funcția de citire (`input.c`) și tratează erorile de I/O (fișiere lipsă, permisiuni, format invalid).
* **Execuție:** Predă controlul către motorul de simulare (`run_scheduler`).
* **Management Memorie:** Asigură eliberarea memoriei alocate dinamic (Garbage Collection) la finalul execuției, indiferent de statusul de ieșire (Succes/Eroare).

### 5. Modulul de Generare Automată (`generator.c`)

Acest modul este responsabil pentru crearea seturilor de date de test (Stress Testing). Este apelat automat din `main.c` atunci când se utilizează flag-ul `-g`.

**Funcționalitate:**
Funcția `generate_file` creează un număr specificat de procese cu parametri aleatori, dar validati logic pentru a asigura o simulare coerentă.

**Algoritmul de Generare:**

1.  **Timpul de Sosire (Arrival Time):**
    Nu este generat pur aleator (pentru a evita haosul cronologic). Se folosește un acumulator `current_time` la care se adaugă o pauză aleatoare (`gap`) între procese.
    * *Formula:* `Arrival(n) = Arrival(n-1) + Random(0..MAX_GAP)`

2.  **Timpul de Execuție și Prioritatea:**
    Sunt generate aleator în intervalele [1, MAX_BURST] și [1, MAX_PRIORITY].

3.  **Termenul Limită (Deadline - Critic pentru EDF):**
    Pentru a evita generarea unor procese "imposibile" (care expiră înainte să poată fi executate fizic), deadline-ul este calculat pe baza unei marje de timp (`slack`).
    * *Formula:* `Deadline = Arrival + Burst + Slack`
    * Unde `Slack` este un timp aleator (o marjă de eroare), garantând că procesul are o șansă teoretică să fie finalizat.

**Siguranță și Erori:**
Modulul implementează verificări stricte:
* Validarea numărului de procese (trebuie să fie pozitiv).
* Verificarea erorilor la crearea fișierului (permisiuni).
* Verificarea integrității scrierii pe disc (prin `ferror`), pentru a preveni fișierele corupte în cazul lipsei de spațiu.

### 6. Modulul de Intrare (`input.c`)

Acest modul gestionează interacțiunea cu fișierele de date și transformarea textului în structuri de date utilizabile. Implementează o politică de **"Strict Data Validation"** (Totul sau Nimic).

**Fluxul de Execuție:**

1.  **Deschidere & Validare Preliminară:**
    Verifică existența fișierului și permisiunile de citire.

2.  **Citire și Validare Linie cu Linie (Faza 1):**
    * Citește `Arrival`, `Burst`, `Priority`, `Deadline`.
    * Verifică validitatea logică (ex: `Burst > 0`, `Arrival >= 0`).
    * **Fail-Fast:** La prima eroare detectată (valoare invalidă sau format greșit), funcția închide fișierul, returnează `-1` și oprește întregul proces. Nu se acceptă fișiere parțial valide.
    * Generează automat **PID-ul** (incremental, bazat pe ordinea citirii).
    * Alocă memorie dinamică pentru fiecare proces folosind `create_process`.

3.  **Verificare Integritate (Faza 2):**
    După terminarea buclei de citire, verifică dacă s-a ajuns la finalul fișierului (`EOF`) în mod natural. Dacă fișierul conține caractere invalide la final, este considerat corupt.

4.  **Afișare (Faza 3):**
    Doar dacă toate validările au trecut, afișează tabelul cu procesele încărcate. Această separare asigură că utilizatorul nu primește informații vizuale eronate în caz de eșec.

## Implementare - Partea Maiei (Logică & Scheduling)

Această secțiune descrie implementarea efectivă a algoritmilor de planificare, mecanismul de simulare preemptivă și calculul metricilor de performanță. Modulul este centralizat în fișierul `scheduler.c` și reprezintă „inteligența” simulatorului.

### 1. Diagrama Gantt (Reprezentare Vizuală)

Pentru a evidenția comportamentul algoritmilor și momentele de preempțiune, simulatorul construiește o **diagramă Gantt text-based**, într-o formă compactă.

**Structura unui segment:**
* **PID:** Identificatorul procesului activ.
* **Interval:** `[start, end)`
* **Optimizare:** Procesele consecutive identice sunt comasate într-un singur segment vizual.
* **IDLE:** Perioadele de inactivitate ale procesorului sunt marcate explicit.

Această diagramă este esențială pentru verificarea corectitudinii algoritmilor și analiza vizuală a succesiunii execuției.

### 2. Logica de Selecție a Proceselor

Motorul de decizie evaluează coada de procese (`Ready Queue`) la fiecare unitate de timp pentru a asigura comportamentul preemptiv.

**A. Priority Scheduling (Preemptiv)**
* **Criteriu:** Valoare mai mică a câmpului `priority` = Prioritate mai mare (ex: 1 > 5).
* **Tie-Breaking (Egalitate):**
    1.  Procesul cu `arrival_time` mai mic (FIFO).
    2.  Procesul cu `PID` mai mic.

**B. Earliest Deadline First (EDF – Real-Time)**
* **Criteriu:** Procesul cu `deadline`-ul cel mai apropiat este ales pentru execuție.
* **Tie-Breaking:** Identic cu Priority Scheduling.
* **Comportament:** Algoritm complet dinamic; prioritățile se pot schimba pe măsură ce timpul avansează și apar procese noi cu deadline-uri mai strânse.

### 3. Motorul de Simulare Preemptivă

Simularea funcționează pe baza unui ceas discret ("Tick-Based"):

1.  **Tick Start:** Se incrementează timpul curent.
2.  **New Arrivals:** Procesele care au sosit la acest moment sunt mutate din lista de așteptare în `Ready Queue`.
3.  **Selection:** Se apelează funcția de selecție (Priority sau EDF) pentru a găsi candidatul optim.
4.  **Execution:** Procesul ales rulează exact **o unitate de timp**.
    * Dacă procesul ales diferă de cel anterior -> **Context Switch** (Preempțiune).
5.  **Completion:** Dacă `remaining_time` ajunge la 0, procesul este marcat ca `FINISHED`.

### 4. Calculul Metricilor de Performanță

Pentru fiecare proces individual se calculează următorii indicatori:

| Metrică | Formulă | Descriere |
| :--- | :--- | :--- |
| **Response Time** | `Start Time - Arrival Time` | Timpul până la prima reacție a sistemului. |
| **Turnaround Time** | `Finish Time - Arrival Time` | Timpul total petrecut în sistem (Așteptare + Execuție). |
| **Waiting Time** | `Turnaround - Burst Time` | Timpul total petrecut stând în coada Ready. |
| **Deadline Miss** | `Finish Time > Deadline` | (Doar EDF) Indicator critic pentru sistemele Real-Time. |

### 5. Metrici Globale și Raportare

La finalul simulării, modulul generează un raport complet care include:

* **Medii Statistice:**
    * *Average Waiting Time*
    * *Average Turnaround Time*
    * *Average Response Time*
* **Utilizarea Procesorului (CPU Utilization):**
    ```text
    CPU Utilization = (Busy Time / Total Time) × 100
    ```
    Unde `Busy Time` este timpul efectiv de calcul, iar `Total Time` include și perioadele IDLE.

**Afișare:**
Toate rezultatele sunt afișate la consolă respectând convenția de tagging `[Scheduler]`, oferind utilizatorului o imagine clară asupra performanței algoritmului ales.

---

### Concluzie

Proiectul **SchedSim** implementează cu succes un simulator robust și modular pentru algoritmi de planificare. Prin separarea clară a logicii de intrare/ieșire (Răzvan) de logica algoritmică (Maia), aplicația permite:
1.  Compararea directă între Priority Scheduling și EDF.
2.  Analiza detaliată a impactului preempțiunii.
3.  Evaluarea fezabilității sistemelor Real-Time.

**Vă mulțumim!** Echipa: **Răzvan & Maia**
