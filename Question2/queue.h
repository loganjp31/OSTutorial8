#include <stdbool.h>

// ---------------------------
// STRUCTS
// ---------------------------
typedef struct proc {
    char name[256];
    int priority;
    int pid;
    int address;
    int memory;
    int runtime;
    bool suspended;
} proc;

typedef struct queue {
    proc process;
    struct queue *next;
} queue;

// ---------------------------
// FUNCTIONS
// ---------------------------
void trim(char *s);

void push(queue **head, proc p);
proc *pop(queue **head);

proc *delete_name(queue **head, char *name);
proc *delete_pid(queue **head, int pid);
