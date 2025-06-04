#include "scheduler.h"
#include <stdlib.h>
#include <limits.h>
#include <string.h>

static Process *rr_p_global;

// Comparator for sorting Process array by arrivalTime (FCFS)
static int cmp_arrival_proc(const void *a, const void *b) {
    const Process *pa = (const Process *)a;
    const Process *pb = (const Process *)b;
    if (pa->arrivalTime < pb->arrivalTime) return -1;
    if (pa->arrivalTime > pb->arrivalTime) return 1;
    return 0;
}

// Comparator for sorting an array of indices by p[idx].arrivalTime (RR)
static int cmp_arrival_idx(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    if (rr_p_global[ia].arrivalTime < rr_p_global[ib].arrivalTime) return -1;
    if (rr_p_global[ia].arrivalTime > rr_p_global[ib].arrivalTime) return 1;
    return 0;
}

// ---------------- Scheduling Algorithms ----------------

// FCFS Scheduling
Metrics fcfs_metrics(Process proc[], int n) {
    // copying all processes into a local array p[]
    Process *p = (Process *) malloc(n * sizeof(Process));
    for (int i = 0;i < n;i ++) {
        p[i] = proc[i];
        p[i].remainingTime = p[i].burstTime;
        p[i].startTime = -1;
        p[i].completionTime = 0;
    }

    // Sorting by arrival time
    qsort(p, n, sizeof(Process), cmp_arrival_proc);
    
    int currentTime = 0;

    // Simulating FCFS
    for (int i = 0;i < n;i ++) {
        // If CPU is idle, jump to arrival time
        if (currentTime < p[i].arrivalTime) {
            currentTime = p[i].arrivalTime;
        }
        p[i].startTime = currentTime;
        p[i].completionTime = p[i].burstTime + p[i].startTime;
        currentTime = p[i].completionTime;
    }

    // compute sums of metrics
    float sumTurnaround = 0.0f;
    float sumWainting = 0.0f;
    float sumResponse = 0.0f;

    for (int i = 0;i < n;i ++) {
        int turnaround = p[i].completionTime - p[i].arrivalTime;
        int waiting = turnaround - p[i].burstTime;
        int response = p[i].startTime - p[i].arrivalTime;
        sumTurnaround += (float) turnaround;
        sumWainting += (float) waiting;
        sumResponse += (float) response;
    }

    free(p);
    Metrics m = {
        .avgTurnaround = sumTurnaround / n,
        .avgWaiting = sumWainting / n,
        .avgResponse = sumResponse / n
    };
    return m;
}

// SJF Scheduling (Non-preemptive)
Metrics sjf_metrics(Process proc[], int n) {
    // copying all processes into a local array p[]
    Process *p = (Process *) malloc(n * sizeof(Process));
    for (int i = 0;i < n;i ++) {
        p[i] = proc[i];
        p[i].remainingTime = p[i].burstTime;
        p[i].startTime = -1;
        p[i].completionTime = 0;
    }

    // done[i] = false means process i is not yet scheduled
    int *done = (int *)calloc(n, sizeof(int));
    int completed = 0, currentTime = 0;

    while (completed < n) {
        // Find shortest‐job among arrived & not done
        int idx = -1, minBurst = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (!done[i] && p[i].arrivalTime <= currentTime) {
                if (p[i].burstTime < minBurst) {
                    minBurst = p[i].burstTime;
                    idx = i;
                }
            }
        }


        // If none found, jump to the next arrivalTime among undone
        if (idx == -1) {
            int earliestArrival = INT_MAX;
            for (int i = 0; i < n; i++) {
                if (!done[i] && p[i].arrivalTime < earliestArrival) {
                    earliestArrival = p[i].arrivalTime;
                }
            }
            currentTime = earliestArrival;
            continue;
        }

        // Schedule process idx
        p[idx].startTime = currentTime;
        p[idx].completionTime = currentTime + p[idx].burstTime;
        currentTime = p[idx].completionTime;
        done[idx] = 1;
        completed ++;
    }

    // Compute sums of metrics
    float sumTurnaround = 0.0f;
    float sumWaiting = 0.0f;
    float sumResponse = 0.0f;

    for (int i = 0; i < n; i++) {
        int turnaround = p[i].completionTime - p[i].arrivalTime;
        int waiting    = turnaround - p[i].burstTime;
        int response   = p[i].startTime - p[i].arrivalTime;
        sumTurnaround += (float)turnaround;
        sumWaiting    += (float)waiting;
        sumResponse   += (float)response;
    }

    free(p), free(done);
    Metrics m = {
        .avgTurnaround = sumTurnaround / n,
        .avgWaiting    = sumWaiting / n,
        .avgResponse   = sumResponse / n
    };
    
    return m;
}

// Round Robin Scheduling (Revised)
//
// We implement a FIFO queue using a simple linkedlist of indices
typedef struct Node {
    int pidIndex;
    struct Node *next;
} Node;

static Node *make_node(int idx) {
    Node *n = (Node *) malloc(sizeof(Node));
    n->pidIndex = idx;
    n->next = NULL;
    return n;
}

static void enqueue(Node **head, Node **tail, int idx) {
    Node* n = make_node(idx);
    if (*tail) {
        (*tail)->next = n;
        *tail = n;
    } else {
        *head = *tail = n;
    }

}

static int dequeue(Node **head, Node **tail) {
    if (!*head) return -1;
    Node *n = *head;
    int idx = n->pidIndex;
    *head = n->next;
    if (!(*head)) *tail = NULL;
    free(n);
    return idx;
}

Metrics rr_metrics(Process proc[], int n, int timeQuantum) {
    // copying all processes into a local array p[]
    Process *p = (Process *) malloc(n * sizeof(Process));
    for (int i = 0;i < n;i ++) {
        p[i] = proc[i];
        p[i].remainingTime = p[i].burstTime;
        p[i].startTime = -1;
        p[i].completionTime = 0;
    }

    // Create an array of indices sorted by arrivalTime
    int *order = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        order[i] = i;
    }
    rr_p_global = p;  
    qsort(order, n, sizeof(int), cmp_arrival_idx);

    // Initializing queue pointers
    Node *head = NULL, *tail = NULL;
    int currentTime = 0;
    int completed = 0;
    int nextArrIdx = 0; // index in order[] for the next process to enqueue

    while (completed < n) {
        // a) If queue is empty, fast‐forward to next arrival if any
        if (head == NULL) {
            if (nextArrIdx < n) {
                currentTime = p[ order[nextArrIdx] ].arrivalTime;
                enqueue(&head, &tail, order[nextArrIdx]);
                nextArrIdx++;
            }
        }

        // Dequeue a process
        int idx = dequeue(&head, &tail);
        if (idx < 0) continue;

        // If this is first time on CPU, record startTime
        if (p[idx].startTime < 0) {
            p[idx].startTime = currentTime;
        }

        // Execute for timeQuantum or the remaining time
        int slice = (p[idx].remainingTime < timeQuantum) ? p[idx].remainingTime : timeQuantum;
        p[idx].remainingTime -= slice;
        currentTime += slice;

        // Enqueue any processes that have arrived by currentTime
        while (nextArrIdx < n && p[ order[nextArrIdx] ].arrivalTime <= currentTime) {
            enqueue(&head, &tail, order[nextArrIdx]);
            nextArrIdx++;
        }

        // If this process is not finished, re‐enqueue; else mark completion
        if (p[idx].remainingTime > 0) {
            enqueue(&head, &tail, idx);
        } else {
            p[idx].completionTime = currentTime;
            completed++;
        }
    }

    // Compute sums of metrics
    float sumTurnaround = 0.0f;
    float sumWaiting = 0.0f;
    float sumResponse = 0.0f;
    for (int i = 0; i < n; i++) {
        int turnaround = p[i].completionTime - p[i].arrivalTime;
        int waiting    = turnaround - p[i].burstTime;
        int response   = p[i].startTime - p[i].arrivalTime;
        sumTurnaround += (float)turnaround;
        sumWaiting    += (float)waiting;
        sumResponse   += (float)response;
    }

    free(p), free(order);
    while (head) {
        Node *tmp = head;
        head = head->next;
        free(tmp);
    }

    Metrics m = {
        .avgTurnaround  = sumTurnaround / n,
        .avgResponse    = sumResponse / n,
        .avgWaiting     = sumWaiting / n
    };
    return m;
}
