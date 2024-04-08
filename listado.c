#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

typedef struct {
    char name[256];
    int pid;
    long int size;
    long int age;
} ProcessInfo;

long int get_process_size(int pid) {
    char path[1024];
    FILE *file;
    long int size = 0;

    snprintf(path, sizeof(path), "/proc/%d/statm", pid);
    file = fopen(path, "r");
    if (file == NULL) {
        return -1;
    }

    if (fscanf(file, "%*s%ld", &size) != 1) {
        size = -1;
    }

    fclose(file);
    return size;
}

long int get_process_age(int pid) {
    char command[1024];
    FILE *pipe;
    char output[256];
    long int age = -1;

    snprintf(command, sizeof(command), "ps -o etimes= -p %d", pid);
    pipe = popen(command, "r");
    if (pipe == NULL) {
        return -1;
    }

    if (fgets(output, sizeof(output), pipe) != NULL) {
        sscanf(output, "%ld", &age);
    }

    pclose(pipe);
    return age;
}

int main() {
    DIR *dir;
    struct dirent *entry;
    ProcessInfo *processes = NULL;
    int num_processes = 0;

    dir = opendir("/proc");
    if (dir == NULL) {
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        int pid = atoi(entry->d_name);
        if (pid != 0) {
            long int size = get_process_size(pid);
            long int age = get_process_age(pid);
            if (size != -1 && age != -1) {
                processes = realloc(processes, (num_processes + 1) * sizeof(ProcessInfo));
                if (processes == NULL) {
                    return 1;
                }
                strcpy(processes[num_processes].name, entry->d_name);
                processes[num_processes].pid = pid;
                processes[num_processes].size = size;
                processes[num_processes].age = age;
                num_processes++;
            }
        }
    }

    closedir(dir);

    printf("Procesos ordenados por tamaño:\n");
    printf("PID\tTamaño (páginas)\n");
    for (int i = 0; i < num_processes; i++) {
        printf("%d\t%ld\n", processes[i].pid, processes[i].size);
    }

    printf("\nProcesos ordenados por antigüedad:\n");
    printf("PID\tAntigüedad (segundos)\n");
    for (int i = 0; i < num_processes; i++) {
        printf("%d\t%ld\n", processes[i].pid, processes[i].age);
    }

    free(processes);

    return 0;
}
