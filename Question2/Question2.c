
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdbool.h>

#define MEMORY 1024


typedef struct {
    char name[256];
    int priority;
    int pid;
    int address;
    int memory;
    int runtime;
    bool suspended;
} proc;

typedef struct {
    proc data;
    struct node* next;
} node;

typedef struct {
    node* front;
    node* rear;
} queue;


void push(queue* q, proc p) {
    node* newNode = (node*)malloc(sizeof(node));
    newNode->data = p;
    newNode->next = NULL;
    if (q->rear == NULL) {
        q->front = q->rear = newNode;
        return;
    }
    q->rear->next = newNode;
    q->rear = newNode;
}

proc pop(queue* q) {
    if (q->front == NULL) {
        proc emptyProc;
        memset(&emptyProc, 0, sizeof(proc));
        return emptyProc; // Return an empty process if the queue is empty
    }
    node* temp = q->front;
    proc p = temp->data;
    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    return p;
}

bool isEmpty(queue* q) {
    return q->front == NULL;
}

int allocateMemory(int memory[], int size) {
    for (int i = 0; i < size; i++) {
        if (memory[i] == 0) {
            memory[i] = 1; // Mark as allocated
            return i; // Return the allocated memory address
        }
    }
    return -1; // No memory available
}

void freeMemory(int memory[], int address) {
    if (address >= 0 && address < MEMORY) {
        memory[address] = 0; // Mark as free
    }
}

int main() {
    queue priority = {NULL, NULL};
    queue secondary = {NULL, NULL};
    int memory[MEMORY] = {0}; // Memory management array

    FILE* file = fopen("processes_q2.txt", "r");
    if (file == NULL) {
        perror("Failed to open processes_q2.txt");
        return EXIT_FAILURE;
    }
    while(!feof(file)) {
        proc p;
        fscanf(file, "%s %d %d %d", p.name, &p.priority, &p.memory, &p.runtime);
        p.pid = 0;
        p.address = -1;
        p.suspended = false;

        if (p.priority == 0) {
            push(&priority, p);
        } else {
            push(&secondary, p);
        }
    }
    fclose(file);


    // Process priority queue first
    while(!isEmpty(&priority)) {

        proc p = pop(&priority);
        p.address = allocateMemory(memory, p.memory);
        if (p.address == -1) {
            printf("Memory allocation failed for process %s\n", p.name);
            continue;
        }

        p.pid = fork();
        if (p.pid == 0) {
            // Child process
            execl("./process", "process", NULL);
        } 
        else {
            // Parent process
            printf("Process %s is running with PID %d\n", p.name, p.pid);
            sleep(p.runtime); // Simulate process execution
            kill(p.pid, SIGTSTP); // Suspend the process after execution
            waitpid(p.pid, NULL, 0); // Wait for the process to finish
            freeMemory(memory, p.address); // Free memory after execution
        }
    }


    // Process secondary queue next
    while(!isEmpty(&secondary)) {
        proc p = pop(&secondary);
        p.address = allocateMemory(memory, p.memory);
        if (p.address == -1) {
            printf("Memory allocation failed for process %s\n", p.name);
            continue;
        }

        if (!p.suspended) {
            p.pid = fork();
            if (p.pid == 0) {
                // Child process
                execl("./process", "process", NULL);
            } 
        }
        else {
            // Parent process
            kill(p.pid, SIGCONT); // Resume the suspended process
        }
        printf("Process %s is running with PID %d with runtime %d\n", p.name, p.pid, p.runtime);
        sleep(1); // Simulate process execution

        if(p.runtime == 1) {
            kill(p.pid, SIGINT); // Kill the process after execution
            waitpid(p.pid, NULL, 0); // Wait for the process to finish
            freeMemory(memory, p.address); // Free memory after execution
        }
         else {
            kill(p.pid, SIGTSTP); // Suspend the process after execution
            p.runtime--; // Decrease runtime for the next iteration
            p.suspended = true; // Mark the process as suspended
            push(&secondary, p); // Push back to the secondary queue if it still has runtime left
        }
    }

    return 0;

}

