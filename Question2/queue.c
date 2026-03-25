#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

// Trim newline and carriage return
void trim(char *s) {
    s[strcspn(s, "\r\n")] = '\0';
}

// ---------------------------
// PUSH — add to end of queue
// ---------------------------
void push(queue **head, proc p) {
    queue *newNode = malloc(sizeof(queue));
    newNode->process = p;
    newNode->next = NULL;

    if (*head == NULL) {
        *head = newNode;
        return;
    }

    queue *temp = *head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = newNode;
}

// ---------------------------
// POP — remove from front
// ---------------------------
proc *pop(queue **head) {
    if (*head == NULL) {
        return NULL;
    }

    queue *temp = *head;
    *head = (*head)->next;

    proc *p = malloc(sizeof(proc));
    *p = temp->process;

    free(temp);
    return p;
}

// ---------------------------
// DELETE BY NAME
// ---------------------------
proc *delete_name(queue **head, char *name) {
    if (*head == NULL) return NULL;

    queue *temp = *head;
    queue *prev = NULL;

    while (temp != NULL) {
        if (strcmp(temp->process.name, name) == 0) {
            if (prev == NULL) {
                *head = temp->next;
            } else {
                prev->next = temp->next;
            }

            proc *p = malloc(sizeof(proc));
            *p = temp->process;
            free(temp);
            return p;
        }
        prev = temp;
        temp = temp->next;
    }

    return NULL;
}

// ---------------------------
// DELETE BY PID
// ---------------------------
proc *delete_pid(queue **head, int pid) {
    if (*head == NULL) return NULL;

    queue *temp = *head;
    queue *prev = NULL;

    while (temp != NULL) {
        if (temp->process.pid == pid) {
            if (prev == NULL) {
                *head = temp->next;
            } else {
                prev->next = temp->next;
            }

            proc *p = malloc(sizeof(proc));
            *p = temp->process;
            free(temp);
            return p;
        }
        prev = temp;
        temp = temp->next;
    }

    return NULL;
}