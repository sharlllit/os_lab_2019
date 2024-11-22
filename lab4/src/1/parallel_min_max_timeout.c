#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int *child_pids; 
int pnum = -1;
bool isTimeout = false;

void handle_sigalrm(int sig) {
    printf("Timeout. Killing the child processes.\n");
    isTimeout = true;
    for (int i = 0; i < pnum; i++) {
        kill(child_pids[i], SIGKILL); 
    }
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  int timeout = 0;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"timeout", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
              printf("Seed should be > 0\n");
              return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              printf("Array size should be > 0\n");
              return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0 || pnum > array_size) {
              printf("pnum <= 0 and pnum > array_size\n");
              return 1;
            }
            break;
          case 3:
              timeout = atoi(optarg);
              if (timeout <= 0) {
                  printf("timeout > 0\n");
                  return 1;
              }
              break;
          case 4:
            with_files = true;
            break;

          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  if (timeout > 0) {
        signal(SIGALRM, handle_sigalrm);
        alarm(timeout);
  }

  int pipefd[2 * pnum];
  if (!with_files) {
    for (int i = 0; i < pnum; i++) {
      pipe(pipefd + 2 * i);
    }
  }

  for (int i = 0; i < pnum; i++) {
    if (!isTimeout) {
      pid_t child_pid = fork();
      if (child_pid >= 0) {
        // successful fork
        active_child_processes += 1;
        if (child_pid == 0) {
          // child process

          // parallel somehow
          struct MinMax local_min_max = GetMinMax(array, i * (array_size / pnum),
            (i == pnum - 1) ? array_size : (i + 1) * (array_size / pnum));

          if (with_files) {
            char filename[256];
            sprintf(filename, "data_%d.txt", i);
            FILE * file = fopen(filename, "w");
            if (file == NULL) {
              printf("Ошибка открытия файла\n");
              return 1;
            }
            fprintf(file, "%d %d\n", local_min_max.min, local_min_max.max);
            fclose(file);
          } else {
            close(pipefd[2 * i]);
            write(pipefd[2 * i + 1], & local_min_max.min, sizeof(int));
            write(pipefd[2 * i + 1], & local_min_max.max, sizeof(int));
            close(pipefd[2 * i + 1]);
          }
          return 0;
        }

      } else {
        printf("Fork failed!\n");
        return 1;
      }
    }
  }

  while (active_child_processes > 0) {
    int status;
    //The waitpid() system call suspends execution of the calling process 
    //until a child specified by pid argument has changed state.
    //WNOHANG - return immediately if no child has exited.
    //is used with signals so the program isn't stuck
    pid_t finished_pid = waitpid(-1, &status, WNOHANG);
    if (finished_pid > 0) {
        active_child_processes--;
    }
  }

  if(isTimeout) return 1;

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      char filename[256];
      sprintf(filename, "data_%d.txt", i);
      FILE *file = fopen(filename, "r");
      if (file == NULL) {
        printf("Ошибка открытия файла\n");
        return 1;
      }
      fscanf(file, "%d %d", &min, &max);
      fclose(file);
    } else {
      close(pipefd[2 * i + 1]); 
      read(pipefd[2 * i], &min, sizeof(int));
      read(pipefd[2 * i], &max, sizeof(int));
      close(pipefd[2 * i]);
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  free(child_pids);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
