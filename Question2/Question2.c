#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <errno.h>
#include "queue.h"

#define MEMORY 1024

int avail_mem[MEMORY];

void initialize_memory();
int allocate_memory(int memory_needed);
void free_memory(int address, int memory_needed);
void execute_priority_processes(queue **priority_queue);
void execute_secondary_processes(queue **secondary_queue);
void read_processes_from_file(const char *filename, queue **priority_queue, queue **secondary_queue);

int main() {
    queue *priority_queue = NULL;
    queue *secondary_queue = NULL;

    initialize_memory();

    read_processes_from_file("processes_q2.txt", &priority_queue, &secondary_queue);

    execute_priority_processes(&priority_queue);

    execute_secondary_processes(&secondary_queue);

    printf("All processes have been executed. Program terminating.\n");

    return 0;
}

void initialize_memory() {
    for (int i = 0; i < MEMORY; i++) {
        avail_mem[i] = 0;
    }
}

int allocate_memory(int memory_needed) {
    int start = -1;
    int count = 0;

    for (int i = 0; i < MEMORY; i++) {
        if (avail_mem[i] == 0) {
            if (start == -1) {
                start = i;
            }
            count++;
            if (count == memory_needed) {
                // Mark memory as used
                for (int j = start; j < start + memory_needed; j++) {
                    avail_mem[j] = 1;
                }
                return start;
            }
        } else {
            start = -1;
            count = 0;
        }
    }

    return -1; // No memory available
}

void free_memory(int address, int memory_needed) {
    if (address >= 0 && address < MEMORY) {
        for (int i = address; i < address + memory_needed; i++) {
            if (i < MEMORY) {
                avail_mem[i] = 0;
            }
        }
    }
}

void read_processes_from_file(const char *filename, queue **priority_queue, queue **secondary_queue) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(1);
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        proc p;
        memset(&p, 0, sizeof(proc));

        char name[256];
        int priority, memory, runtime;

        sscanf(line, "%255[^,],%d,%d,%d", name, &priority, &memory, &runtime);

        strcpy(p.name, name);
        p.priority = priority;
        p.memory = memory;
        p.runtime = runtime;
        p.pid = 0;
        p.address = 0;
        p.suspended = false;

        // Add to appropriate queue
        if (priority == 0) {
            push(priority_queue, p);
        } else {
            push(secondary_queue, p);
        }
    }

    fclose(file);
}

void execute_priority_processes(queue **priority_queue) {
    proc *p;

    while ((p = pop(priority_queue)) != NULL) {
        // Allocate memory
        p->address = allocate_memory(p->memory);
        if (p->address == -1) {
            fprintf(stderr, "Error: Not enough memory for process %s\n", p->name);
            free(p);
            continue;
        }

        // Fork and execute
        pid_t pid = fork();

        if (pid == 0) {
            // Child process
            char path[512];
            snprintf(path, sizeof(path), "./%s.exe", p->name);
            execl(path, path, NULL);
            perror("exec failed");
            exit(1);
        } else if (pid > 0) {
            // Parent process
            p->pid = pid;

            // Print process information
            printf("Executing priority process: %s, Priority: %d, PID: %d, Memory: %d, Runtime: %d\n",
                   p->name, p->priority, p->pid, p->memory, p->runtime);

            // Run for specified runtime and then terminate
            sleep(p->runtime);

            kill(pid, SIGINT);

            int status;
            waitpid(pid, &status, 0);

            free_memory(p->address, p->memory);

            printf("Priority process %s completed and terminated.\n", p->name);
        } else {
            perror("fork failed");
        }

        free(p);
    }
}

void execute_secondary_processes(queue **secondary_queue) {
    proc *p;
    bool process_remaining = true;

    while (process_remaining) {
        process_remaining = false;

        // Check if there are any processes left in the queue
        if (*secondary_queue != NULL) {
            process_remaining = true;
        }

        p = pop(secondary_queue);
        if (p == NULL) {
            continue;
        }

        if (p->pid != 0 && p->suspended) {
            // Check if process is still alive
            if (kill(p->pid, 0) == -1 && errno == ESRCH) {
                // Process no longer exists, clean up and continue
                printf("Process %s no longer exists, cleaning up.\n", p->name);
                free_memory(p->address, p->memory);
                free(p);
                continue;
            }

            // Resume suspended process - memory is already allocated
            printf("Resuming secondary process: %s, Priority: %d, PID: %d, Memory: %d, Runtime: %d\n",
                   p->name, p->priority, p->pid, p->memory, p->runtime);

            kill(p->pid, SIGCONT);

            // Run for 1 second and then suspend
            sleep(1);
            kill(p->pid, SIGTSTP);

            // Decrement runtime
            p->runtime--;

            // Check if process is finished
            if (p->runtime <= 0) {
                // Terminate the process with SIGINT
                kill(p->pid, SIGINT);

                int status;
                waitpid(p->pid, &status, 0);

                free_memory(p->address, p->memory);

                printf("Process %s completed and terminated.\n", p->name);
                free(p);
                continue;
            }

            p->suspended = true;

            // Add back to queue
            push(secondary_queue, *p);
            free(p);
        } else if (p->pid == 0) {
            // New process - need to allocate memory
            p->address = allocate_memory(p->memory);

            if (p->address == -1) {
                // Not enough memory, push back to queue
                printf("Not enough memory for process %s, pushing back to queue\n", p->name);
                push(secondary_queue, *p);
                free(p);
                continue;
            }

            if (p->runtime <= 1) {
                pid_t pid = fork();

                if (pid == 0) {
                    // Child process
                    char path[512];
                    snprintf(path, sizeof(path), "./%s.exe", p->name);
                    execl(path, path, NULL);
                    perror("exec failed");
                    exit(1);
                } else if (pid > 0) {
                    // Parent process
                    p->pid = pid;

                    printf("Executing secondary process (final): %s, Priority: %d, PID: %d, Memory: %d, Runtime: %d\n",
                           p->name, p->priority, p->pid, p->memory, p->runtime);

                    sleep(p->runtime);
                    kill(pid, SIGINT);

                    int status;
                    waitpid(pid, &status, 0);

                    free_memory(p->address, p->memory);

                    printf("Process %s completed and terminated.\n", p->name);
                    free(p);
                } else {
                    perror("fork failed");
                    free(p);
                }
            } else {
                pid_t pid = fork();

                if (pid == 0) {
                    // Child process
                    char path[512];
                    snprintf(path, sizeof(path), "./%s.exe", p->name);
                    execl(path, path, NULL);
                    perror("exec failed");
                    exit(1);
                } else if (pid > 0) {
                    // Parent process
                    p->pid = pid;

                    printf("Executing secondary process: %s, Priority: %d, PID: %d, Memory: %d, Runtime: %d\n",
                           p->name, p->priority, p->pid, p->memory, p->runtime);

                    sleep(1);
                    kill(pid, SIGTSTP);

                    p->runtime--;

                    p->suspended = true;

                    push(secondary_queue, *p);
                    free(p);
                } else {
                    perror("fork failed");
                    free(p);
                }
            }
        } else {
            printf("Unexpected state for process %s\n", p->name);
            free(p);
        }
    }
}