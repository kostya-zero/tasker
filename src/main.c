#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TASKS_FILE "tasks.bin"
#define TEMP_FILE "tasks.tmp"
#define DESC_SIZE 256

typedef struct {
    int id;
    char desc[DESC_SIZE];
    bool done;
} Task;

void println(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    putchar('\n');
}

int next_id() {
    FILE *f = fopen(TASKS_FILE, "rb");
    if (!f) return 1;
    Task t;
    int last = 0;
    while (fread(&t, sizeof(Task), 1, f)) last = t.id;
    fclose(f);
    return last + 1;
}

void add_task(const char *desc) {
    Task t;
    t.id = next_id();
    strncpy(t.desc, desc, DESC_SIZE - 1);
    t.desc[DESC_SIZE - 1] = '\0';
    t.done = false;
    FILE *f = fopen(TASKS_FILE, "ab");
    if (!f) {
        perror("Failed to open file");
        exit(1);
    }
    fwrite(&t, sizeof(Task), 1, f);
    fclose(f);
    println("Added to list.");
}

void list_tasks() {
    FILE *f = fopen(TASKS_FILE, "rb");
    if (!f) {
        println("No tasks found.");
        return;
    }
    Task t;
    while (fread(&t, sizeof(Task), 1, f)) {
        println("[%s] %d: %s", t.done == true ? " " : "+", t.id, t.desc);
    }
    fclose(f);
}

void remove_task(const int id) {
    FILE *f = fopen(TASKS_FILE, "rb");
    if (!f) {
        perror("Failed to open tasks file");
        exit(1);
    }
    FILE *tf = fopen(TEMP_FILE, "wb");
    if (!tf) {
        perror("Cannot open temp file");
        fclose(f);
        exit(1);
    }
    Task t;
    bool found = false;
    while (fread(&t, sizeof(Task), 1, f)) {
        if (t.id == id) {
            found = true;
            continue;
        }
        fwrite(&t, sizeof(Task), 1, tf);
    }
    fclose(f);
    fclose(tf);

    if (!found) {
        println("Task not found");
        remove(TEMP_FILE);
        exit(1);
    }

    if (remove(TASKS_FILE) != 0 || rename(TEMP_FILE, TASKS_FILE) != 0) {
        perror("Failed to update tasks file");
        exit(1);
    }

    println("Task has been removed");
}

void check_task(const int id) {
    FILE *f = fopen(TASKS_FILE, "rb");
    if (!f) {
        perror("Failed to open tasks file");
        exit(1);
    }

    FILE *tf = fopen(TEMP_FILE, "wb");
    if (!tf) {
        perror("Failed to prepare temporary file");
        fclose(f);
        exit(1);
    }
    Task t;
    bool found = false;
    while (fread(&t, sizeof(Task), 1, f)) {
        if (t.id == id) {
            println("debug: Task found");
            found = true;
            if (t.done) {
                t.done = false;
                println("Task marked as unfinished.");
            } else {
                t.done = true;
                println("Task marked as finished.");
            }
        }
        fwrite(&t, sizeof(Task), 1, tf);
    }
    fclose(f);
    fclose(tf);

    if (!found) {
        println("Task not found");
        exit(1);
    }

    if (remove(TASKS_FILE) || rename(TEMP_FILE, TASKS_FILE)) {
        perror("Failed to update tasks file");
        exit(1);
    }
}

void print_help() {
    println("Tasker - Minimalistic Tasks Manager.");
    println("Usage: tasker [command] [argument]\n");
    println("Commands:");
    println("   add <task_description>     Add new task.");
    println("   remove <task_id>           Remove task by ID.");
    println("   check <task_id>            Check task as done/unfinished.");
    println("   list                       Print list of all tasks.");
    println("   help                       Print help message.");
}

int main(const int argc, char *argv[]) {
    if (argc < 2) {
        print_help();
    }

    if (strcmp(argv[1], "add") == 0) {
        if (argc != 3) {
            println("Provide a description for new task. Only one argument allowed.");
            return 1;
        }
        add_task(argv[2]);
    } else if (strcmp(argv[1], "list") == 0) {
        list_tasks();
    } else if (strcmp(argv[1], "help") == 0) {
        print_help();
    } else if (strcmp(argv[1], "remove") == 0) {
        if (argc != 3) {
            println("Provide an ID of task. Only one argument allowed.");
            return 1;
        }
        remove_task(atoi(argv[2]));
    } else if (strcmp(argv[1], "check") == 0) {
        if (argc != 3) {
            println("Provide an ID of task. Only one argument allowed.");
            return 1;
        }
        check_task(atoi(argv[2]));
    } else {
        println("Unknown command: %s", argv[1]);
        return 1;
    }

    return 0;
}
