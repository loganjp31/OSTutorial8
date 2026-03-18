#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

typedef struct proc {
    char parent[256];
    char name[256];
    int priority;
    int memory;
} proc;

typedef struct proc_tree {
    proc data;
    struct proc_tree *left;
    struct proc_tree *right;
} proc_tree;

proc_tree *root = NULL;

// Trim leading/trailing whitespace in-place
void trim(char *s) {
    if (!s) return;

    // Trim leading whitespace
    char *p = s;
    while (*p && isspace((unsigned char)*p))
        p++;
    if (p != s)
        memmove(s, p, strlen(p) + 1);

    // Trim trailing whitespace
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1]))
        s[--len] = '\0';
}

// Create a new tree node
proc_tree *create_node(proc p) {
    proc_tree *node = malloc(sizeof(proc_tree));
    node->data = p;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// Insert child under the correct parent
void insert_child(proc_tree *parent, proc p) {
    if (parent->left == NULL) {
        parent->left = create_node(p);
    } else if (parent->right == NULL) {
        parent->right = create_node(p);
    } else {
        printf("Warning: Parent %s already has two children.\n", parent->data.name);
    }
}

// Recursively search for a parent node
proc_tree *find_parent(proc_tree *node, const char *parent_name) {
    if (node == NULL) return NULL;

    if (strcmp(node->data.name, parent_name) == 0)
        return node;

    proc_tree *left = find_parent(node->left, parent_name);
    if (left) return left;

    return find_parent(node->right, parent_name);
}

// Insert a process into the tree
void insert_process(proc p) {
    if (root == NULL) {
        root = create_node(p);
        return;
    }

    proc_tree *parent = find_parent(root, p.parent);
    if (parent == NULL) {
        printf("Warning: Parent '%s' not found for child '%s'\n", p.parent, p.name);
        return;
    }

    insert_child(parent, p);
}

// Print tree recursively
void print_tree(proc_tree *node, int depth) {
    if (node == NULL) return;

    for (int i = 0; i < depth; i++)
        printf("  ");

    printf("Parent: %s | Name: %s | Priority: %d | Memory: %dMB\n",
           node->data.parent,
           node->data.name,
           node->data.priority,
           node->data.memory);

    print_tree(node->left, depth + 1);
    print_tree(node->right, depth + 1);
}

int main() {
    FILE *file = fopen("processes_tree.txt", "r");  // match your actual filename
    if (!file) {
        printf("Error: Could not open processes_tree.txt\n");
        return 1;
    }

    char line[512];

    while (fgets(line, sizeof(line), file)) {
        proc p;
        char *token;

        // parent
        token = strtok(line, ",");
        if (!token) continue;
        trim(token);
        strcpy(p.parent, token);

        // name
        token = strtok(NULL, ",");
        if (!token) continue;
        trim(token);
        strcpy(p.name, token);

        // priority
        token = strtok(NULL, ",");
        if (!token) continue;
        trim(token);
        p.priority = atoi(token);

        // memory
        token = strtok(NULL, ",");
        if (!token) continue;
        trim(token);
        p.memory = atoi(token);

        insert_process(p);
    }

    fclose(file);

    printf("\n=== PROCESS TREE ===\n");
    print_tree(root, 0);

    return 0;
}