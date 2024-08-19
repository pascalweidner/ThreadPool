#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <regex.h>

#include <dirent.h>

#include "threadpool.h"

tpool_t *tm;
static const size_t num_threads = 20;

void worker(void *arg)
{
    int *val = arg;
    int old = *val;

    *val += 1000;
    printf("tid=%p, old=%d, val=%d\n", (void *)pthread_self(), old, *val);

    if (*val % 2)
        usleep(100000);
}

// arg needs to be a heap allocated variable as it will be freed
void read_directory(void *arg)
{
    char *path = (char *)malloc(strlen((char *)arg) * sizeof(char));
    strcpy(path, (char *)arg);
    free(arg);

    DIR *directory = opendir(path);
    if (directory == NULL)
    {
        perror("Error message");
        exit(-errno);
    }

    struct dirent *entry = NULL;
    struct stat file_stat;
    char full_path[1024];

    regex_t reegex;

    while ((entry = readdir(directory)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".git") == 0 || strcmp(entry->d_name, ".vscode") == 0)
        {
            continue;
        }

        // Build the full path to the entry
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        if (stat(full_path, &file_stat) == -1)
        {
            perror("stat");
            continue;
        }

        if (S_ISDIR(file_stat.st_mode))
        {
            printf("Directory: %s\n", full_path);

            char *p = (char *)malloc(1024 * sizeof(char));
            strcpy(p, full_path);
            tpool_add_work(tm, read_directory, p);
        }
        else
        {
            printf("File: %s\n", full_path);
        }
    }

    free(path);
    closedir(directory);
}

int main()
{
    // initialize threadpool
    tm = tpool_create(num_threads);

    char *path = (char *)malloc(1024 * sizeof(char));
    strcpy(path, "/mnt/d/Development/Languages/C/FileCompression");

    tpool_add_work(tm, read_directory, path);

    tpool_wait(tm);

    tpool_destroy(tm);
    return 0;
}
