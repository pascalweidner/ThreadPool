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

#define MAX_FILE_NAME_SIZE 256

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

void read_directory(const char *path)
{
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

            read_directory(full_path);
        }
        else
        {
            printf("File: %s\n", full_path);
        }
    }

    closedir(directory);
}

int main()
{
    // initialize threadpool
    // tpool_t *tm = tpool_create(num_threads);

    read_directory("/mnt/d/Development/Languages/C/FileCompression");

    /*
        for (i = 0; i < num_items; i++)
        {
            vals[i] = i;
            tpool_add_work(tm, worker, vals + i);
        }

        tpool_wait(tm);

        for (i = 0; i < num_items; i++)
        {
            printf("%d ", vals[i]);
        }
        printf("\n");

        free(vals);

        */
    // tpool_destroy(tm);
    return 0;
}
