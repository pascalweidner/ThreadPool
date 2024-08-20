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
pthread_mutex_t mutex_lock;

struct tuple
{
    char *path;
    int *lineCount;
};
typedef struct tuple tuple_t;

// arg needs to be a heap allocated variable as it will be freed
void count_lines(void *arg)
{
    tuple_t *data = (tuple_t *)arg;
    char *path = data->path;

    int count = 1;

    FILE *fptr = fopen(path, "r");
    while (!feof(fptr))
    {
        int val = fgetc(fptr);
        if (val == '\n')
        {
            count++;
        }
    }

    printf("File: %s: %d\n", path, count);

    int *lineCount = data->lineCount;

    pthread_mutex_lock(&(mutex_lock));
    *lineCount += count;
    pthread_mutex_unlock(&(mutex_lock));

    free(path);
    free(arg);
    fclose(fptr);
}

// arg needs to be a heap allocated variable as it will be freed
void read_directory(void *arg)
{
    tuple_t *data = (tuple_t *)arg;
    char *path = data->path;

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
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".git") == 0 || strcmp(entry->d_name, ".vscode") == 0 || strcmp(entry->d_name, "files") == 0)
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

            tuple_t *d = (tuple_t *)malloc(sizeof(tuple_t));

            char *p = (char *)malloc(1024 * sizeof(char));
            strcpy(p, full_path);

            d->path = p;
            d->lineCount = data->lineCount;

            tpool_add_work(tm, read_directory, d);
        }
        else
        {
            tuple_t *d = (tuple_t *)malloc(sizeof(tuple_t));

            char *p = (char *)malloc(1024 * sizeof(char));
            strcpy(p, full_path);

            d->path = p;
            d->lineCount = data->lineCount;

            tpool_add_work(tm, count_lines, d);
        }
    }

    free(path);
    free(arg);
    closedir(directory);
}

int main()
{
    // initialize threadpool
    tm = tpool_create(num_threads);

    pthread_mutex_init(&(mutex_lock), NULL);

    tuple_t *data = (tuple_t *)malloc(sizeof(tuple_t));

    char *path = (char *)malloc(1024 * sizeof(char));
    strcpy(path, "/mnt/d/Development/Languages/C/FileCompression");
    data->path = path;

    int *lineCount = (int *)malloc(sizeof(int));
    data->lineCount = lineCount;

    tpool_add_work(tm, read_directory, data);

    tpool_wait(tm);

    printf("Lines: %d\n", *lineCount);

    free(lineCount);

    tpool_destroy(tm);
    pthread_mutex_destroy(&(mutex_lock));
    return 0;
}
