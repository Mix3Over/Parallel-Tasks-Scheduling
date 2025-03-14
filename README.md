# Parallel Task Scheduling using Hill Climbing in C++

## Overview

This project implements a parallel task scheduling algorithm using the **Hill Climbing** heuristic to minimize the average flow time. The implementation processes workload data from **Standard Workload Format (SWF)** files and produces an optimal schedule that minimizes the sum of completion times.

## Problem Statement

Given a set of parallel tasks characterized by:

- `p_j`: execution time (column 4 in SWF)
- `r_j`: ready/arrival time (column 2 in SWF)
- `size_j`: number of requested/allocated processors (column 5 in SWF)

We aim to determine a schedule that minimizes the average **flow time**, which is equivalent to minimizing the sum of **completion times**.

## Input

- **A SWF file** containing a workload trace.
- **An integer N**, specifying the maximum number of tasks to be scheduled.
- The program filters out tasks that:
  - Have incomplete data.
  - Have a zero execution time (`p_j = 0`).

## Output Format

The program produces a schedule in the format:

```plaintext
job_id start_time end_time assigned_processors
```

Each line represents a scheduled task, where:

- `job_id`: The original job ID from the SWF file (without renumbering).
- `start_time`: The computed start time of the task.
- `end_time`: The computed completion time.
- `assigned_processors`: A list of allocated processor indices.

Each field is separated by a **space**, and no additional text is included.

## Methodology

1. **Parsing SWF Data**: Extract relevant fields while handling missing or invalid entries.
2. **Initial Greedy Schedule**: Construct an initial schedule using a heuristic (e.g., **Earliest Release Time First**).
3. **Hill Climbing Optimization**:
   - Generate neighboring schedules by swapping task order or shifting start times.
   - Evaluate and select the schedule with the lowest sum of completion times.
   - Repeat until no improvement is found or the time cap is reached.
4. **Output the final optimized schedule**.

## Assumptions

- The system has **MaxNodes = MaxProcs**.
- The system can handle **MaxJobs = MaxRecords**.
- Tasks are **non-preemptive** and must be executed in one continuous block.

## Time Cap

To limit the execution time of the algorithm, a **time cap** is enforced:

```cpp
int timeLimit = 170; // Time limit in seconds
```

Changing this value allows adjusting the maximum duration of the optimization process.

## Installation & Usage

### Prerequisites

- C++17 or later
- CMake (for build automation)
- A standard C++ compiler (e.g., g++, clang++)

### Compilation

```sh
mkdir build && cd build
cmake ..
make
```

### Running the Program

```sh
./scheduler input.swf N
```

Where:

- `input.swf` is the workload trace file.
- `N` is the number of tasks to schedule.

### Example

```sh
./scheduler workload.swf 100 schedule.txt
```

This schedules the first **100** tasks from `workload.swf` and writes the output to `schedule.txt`.

## Handling Special Cases

- Tasks with **missing** or **invalid** data are ignored.
- Tasks with **zero duration** (`p_j = 0`) are skipped.
- The input task order **does not guarantee** sorted arrival times (`r_j`).

## License

This project is open-source under the **MIT License**.

## Author

[Your Name]

