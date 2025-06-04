# CPU Scheduling Algorithms

This project implements three fundamental CPU scheduling algorithms in C as part of an Operating Systems course project.

## Algorithms Implemented

1. **FCFS (First Come First Serve)**
2. **SJF (Shortest Job First - Non-Preemptive)**
3. **Round Robin (RR)**

Each algorithm computes the following performance metrics:
- Average Turnaround Time
- Average Waiting Time
- Average Response Time


## Example Output 

```
=== FCFS ===
Average Turnaround Time = 8.00
Average Waiting Time    = 4.00
Average Response Time   = 4.00

=== SJF (Non-preemptive) ===
Average Turnaround Time = 7.00
Average Waiting Time    = 3.00
Average Response Time   = 3.00

=== Round Robin (Quantum = 2) ===
Average Turnaround Time = 7.33
Average Waiting Time    = 3.33
Average Response Time   = 0.67

```