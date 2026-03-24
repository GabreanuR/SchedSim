# SchedSim - CPU Scheduling Simulator

> A preemptive CPU scheduling simulator built in C to evaluate the performance of Priority and Real-Time (EDF) algorithms. 

**Course:** Operating Systems  
**Authors:** Răzvan & Maia 

## Overview

**SchedSim** is a lightweight, command-line based process simulator designed to analyze and compare scheduling algorithms. It provides a tick-based simulation environment, calculating standard operating system performance metrics such as CPU Utilization, Waiting Time, and Turnaround Time.

### Supported Algorithms:
1. **Priority Scheduling** (Preemptive)
2. **Real-Time Scheduling** (Earliest Deadline First - EDF)

## Tech Stack & Features
* **Language:** C 
* **Build System:** Makefile
* **Key Features:**
  * Custom Automated Test Generator (Stress Testing).
  * Strict I/O Data Validation (Fail-Fast architecture).
  * Dynamic Memory Management (No memory leaks).
  * Text-based Gantt Chart generation for visual execution tracking.

## Getting Started

The project uses a standard `Makefile` for dependency management and compilation.

### Prerequisites
* GCC Compiler
* Make utility

### Compilation
Open your terminal in the project directory and run:
```bash
make
```
This will compile all modules (`main`, `input`, `generator`, `scheduler`) and generate the `schedsim` executable.

To clean up compiled objects and executable files:
```bash
make clean
```

## Usage

The general syntax for running the simulator is:
```bash
./schedsim <algorithm> [options]
```

**Arguments:**
* `<algorithm>`: Choose either `priority` or `edf`.
* `[options]`:
  * *(None)*: Runs in **Manual Mode** (reads process data from `input.txt`).
  * `-g <number>`: Runs in **Automatic Mode** (generates `<number>` of random processes into `random_input.txt` and simulates them).

### Examples

**1. Manual Testing (Priority Scheduling)**
Edit the `input.txt` file with your custom data, then run:
```bash
./schedsim priority
```

**2. Stress Testing (Earliest Deadline First)**
Simulate the EDF algorithm using 50 randomly generated processes:
```bash
./schedsim edf -g 50
```

## Architecture & Implementation Details

To ensure modularity and maintainability, the project is divided into distinct subsystems, handled collaboratively by the team.

### Infrastructure, I/O & Data Generation (Răzvan)
* **Orchestrator (`main.c`):** Acts as the central dispatcher. Handles CLI argument validation, routes execution flow, and guarantees dynamic memory cleanup upon exit.
* **Strict Data Validation (`input.c`):** Implements a "All-or-Nothing" parsing policy. If `input.txt` contains invalid logical values (e.g., negative burst times) or formatting errors, the system fails-fast, safely halting execution before simulation.
* **Automated Generator (`generator.c`):** Creates mathematically viable stress-test datasets. To prevent "impossible" EDF scenarios, deadlines are calculated using a custom formula: `Deadline = Arrival + Burst + Slack`, ensuring theoretical schedulability.

### Core Scheduling Logic & Metrics (Maia)
* **Simulation Engine (`scheduler.c`):** Operates on a discrete tick-based clock. It evaluates the Ready Queue at every time unit, handling Context Switches and Preemption dynamically.
* **Tie-Breaking Mechanisms:** In case of priority collisions, the engine falls back to FIFO (Arrival Time), and subsequently to PID comparison.
* **Telemetry & Analytics:** Calculates per-process metrics (`Response Time`, `Waiting Time`, `Turnaround Time`, and `Deadline Misses` for EDF) and aggregates them into global statistical averages, alongside Total CPU Utilization.
* **Gantt Chart Visualization:** Dynamically generates a compact, text-based timeline of CPU allocation, efficiently merging consecutive identical process states and explicitly marking `IDLE` times.

## Input File Format

If running in Manual Mode, `input.txt` must not contain a header. PIDs are auto-generated sequentially during parsing.

**Format (Space-separated integers):**
`[Arrival Time] [Burst Time] [Priority] [Deadline]`

**Example:**
```text
0 10 3 100
2 5 1 50
```

## 📖 Logging Conventions
For debugging and execution tracing, all console outputs include their source module tag:
`[Module] Message` (e.g., `[Scheduler] Analyzing processes...`)

## References
* [GeeksForGeeks: CPU Scheduling in Operating Systems](https://www.geeksforgeeks.org/operating-systems/cpu-scheduling-in-operating-systems/)
* [GeeksForGeeks: Difference between Arrival Time and Burst Time](https://www.geeksforgeeks.org/operating-systems/difference-between-arrival-time-and-burst-time-in-cpu-scheduling/)
* [GeeksForGeeks: Earliest Deadline First (EDF)](https://www.geeksforgeeks.org/operating-systems/earliest-deadline-first-edf-cpu-scheduling-algorithm/)
* [TutorialsPoint: C function strtol](https://www.tutorialspoint.com/c_standard_library/c_function_strtol.htm)
