#include <ctype.h>      
#include <limits.h>     
#include <stdbool.h>    
#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>    
#include <unistd.h>      
#include <errno.h>      
#include <pthread.h>     
#include <getopt.h>      

long long fact = 1;  
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER; // Инициализация мьютекса 
 
struct FactArgs {
  int begin; // начало диапазона
  int end;   // конец диапазона
};

// функция, выполняемая в потоке для вычисления факториала
void *ThreadFact(void *args) {
    pthread_mutex_lock(&mut); // захват мьютекса для защиты доступа к переменной fact
    struct FactArgs *fact_args = (struct FactArgs *)args; // приведение указателя к нужному типу
    for (int i = fact_args->begin; i < fact_args->end; i++) // цикл по диапазону
        fact *= i; // умножение на текущее значение
    pthread_mutex_unlock(&mut); // освобождение мьютекса
    return NULL;  
}

int main(int argc, char **argv) {
    u_int32_t k = -1;  
    u_int32_t pnum = 0;  
    u_int32_t mod = 0;  

    // обработка аргументов командной строки
    while (true) {
        int current_optind = optind ? optind : 1; // получаем текущий индекс опции

        // определение опций командной строки
        static struct option options[] = {
            {"k", required_argument, 0, 0}, 
            {"pnum", required_argument, 0, 0},  
            {"mod", required_argument, 0, 0}, 
            {0, 0, 0, 0}  
        };

        int option_index = 0; // индекс текущей опции
        int c = getopt_long(argc, argv, "f", options, &option_index); // обработка опций

        if (c == -1) break; 

        switch (c) {
            case 0:  
                switch (option_index) {
                    case 0:  
                        k = atoi(optarg); 
                        if (k < 0) {  
                            printf("k should be >= 0\n");
                            return 1;  
                        }
                        break;
                    case 1:  
                        pnum = atoi(optarg);  
                        if (pnum <= 0 || pnum > k) {  
                            printf("pnum should be > 0 and <= k\n");
                            return 1;  
                        }
                        break;
                    case 2:  
                        mod = atoi(optarg);  
                        if (mod <= 0) {  
                            printf("mod should be > 0\n");
                            return 1;  
                        }
                        break;
                    default:  
                        printf("Index %d is out of options\n", option_index);
                }
                break;
            case '?':  
                break;
            default:  
                printf("getopt returned character code 0%o ?\n", c);
        }
    }

    // проверка на корректность введенных значений
    if (k == -1 || pnum == 0 || mod == 0) {
        printf("Usage: %s --k \"num\" --pnum \"num\" --mod \"num\"\n", argv[0]);
        return 1;  
    }

    pthread_t threads[pnum]; // массив потоков
    struct FactArgs args[pnum]; // массив структур для аргументов потоков
    int segment_size = k / pnum; // размер сегмента для каждого потока

    // разделение диапазона на сегменты для каждого потока
    for (u_int32_t i = 0; i < pnum; i++) {
        args[i].begin = i * segment_size + 1; // начало сегмента
        args[i].end = (i == pnum - 1) ? k + 1 : (i + 1) * segment_size + 1; // конец сегмента
    }

    // создание потоков
    for (u_int32_t i = 0; i < pnum; i++) {
        if (pthread_create(&threads[i], NULL, ThreadFact, (void *)&args[i])) { // создание потока
            printf("Error: pthread_create failed!\n");
            return 1;  
        }
    }

    // ожидание завершения всех потоков
    for (u_int32_t i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL); // ожидание завершения потока
    }

    printf("Result: %lld\n", fact % mod);
    return 0;  
}