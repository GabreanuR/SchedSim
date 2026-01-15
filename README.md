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

## Partea Maiei:

1. Modulul de Scheduling și Analiză (Partea Maiei)
Această secțiune descrie implementarea efectivă a algoritmilor de planificare a proceselor, mecanismul de simulare preemptivă și calculul metricilor de performanță. Modulul este implementat în fișierul scheduler.c și reprezintă „inteligența” simulatorului.

2. Diagrama Gantt (Reprezentare Vizuală)
Pentru a evidenția comportamentul algoritmilor de scheduling și preempțiunea, simulatorul construiește o diagramă Gantt text-based, într-o formă compactă.
Fiecare segment din diagramă conține:
PID-ul procesului care rulează
intervalul de timp [start, end)
Procesele consecutive identice sunt comasate într-un singur segment, pentru lizibilitate.
Intervalele în care CPU-ul nu execută niciun proces sunt marcate ca IDLE.
Această diagramă este utilă pentru:
verificarea corectitudinii algoritmilor,
evidențierea preempțiunilor,
analiza vizuală a execuției proceselor.

3. Logica de Selecție a Proceselor
Selecția procesului care va rula pe CPU se face la fiecare unitate de timp, asigurând un comportament preemptiv.
Priority Scheduling (Preemptiv)
Se consideră că o valoare mai mică a câmpului priority înseamnă o prioritate mai mare (ex: priority = 1 este cea mai mare).
La fiecare pas de timp, este ales procesul cu prioritatea cea mai mare din ready_queue.
Dacă două procese au aceeași prioritate:
se alege procesul cu arrival_time mai mic,
în caz de egalitate, procesul cu PID mai mic.
Această strategie garantează selecție deterministă și corectă.
Earliest Deadline First (EDF – Real-Time)
Se alege procesul cu deadline-ul cel mai apropiat.
O valoare mai mică a câmpului deadline indică un proces mai urgent.
Tie-break-ul se face identic cu Priority Scheduling (arrival time, apoi PID).
Algoritmul este complet preemptiv, fiind reevaluat la fiecare unitate de timp.

4. Simularea Preemptivă
Motorul de simulare funcționează pe unități discrete de timp:
La fiecare pas:
sunt adăugate în ready_queue procesele care au sosit,
se selectează procesul optim conform algoritmului ales,
procesul rulează exact o unitate de timp.
Dacă apare un proces mai prioritar sau cu deadline mai urgent, CPU-ul este preemptat.
Execuția continuă până când toate procesele sunt finalizate.
Această abordare reflectă fidel comportamentul unui scheduler real dintr-un sistem de operare.

5. Calculul Metricilor de Performanță
Pentru fiecare proces sunt calculate următoarele valori:
Start Time – momentul primei execuții pe CPU.
Finish Time – momentul finalizării execuției.
Response Time
Response Time = Start Time – Arrival Time
Turnaround Time
Turnaround Time = Finish Time – Arrival Time
Waiting Time
Waiting Time = Turnaround Time – Burst Time
Pentru algoritmul EDF, se verifică suplimentar:
dacă Finish Time > Deadline, procesul este marcat ca deadline miss.

6. Metrici Globale și Performanță
După finalizarea simulării, sunt calculate metrici globale:
Average Waiting Time
Average Turnaround Time
Average Response Time
Acestea sunt obținute prin însumarea valorilor individuale și împărțirea la numărul total de procese.
CPU Utilization
Utilizarea procesorului este calculată astfel:
CPU Utilization = (Busy Time / Total Time) × 100
Busy Time reprezintă timpul total în care CPU-ul a executat procese.
Total Time include și perioadele de inactivitate (IDLE).

7. Afișarea Rezultatelor
La finalul simulării, modulul afișează:
Diagrama Gantt compactă
Tabelul complet cu statistici pentru fiecare proces
Rezumatul performanței globale
Pentru EDF, un mesaj explicit care indică dacă au existat procese ce au depășit deadline-ul
Toate mesajele sunt afișate respectând convenția de tagging:
[Scheduler] Mesaj

8. Concluzie
Modulul de scheduling implementează complet și corect:
Priority Scheduling preemptiv
Earliest Deadline First (EDF)
Simulare realistă a execuției proceselor
Analiză detaliată a performanței
Acesta permite compararea directă a algoritmilor și oferă atât rezultate numerice, cât și o reprezentare vizuală clară a execuției.

## Va multumim!
