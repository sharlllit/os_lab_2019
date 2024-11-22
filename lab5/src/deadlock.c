#include <stdio.h>       
#include <pthread.h>    
#include <stdlib.h>     
#include <unistd.h>    

// объявление мьютексов для синхронизации доступа к ресурсам
pthread_mutex_t mutex1; 
pthread_mutex_t mutex2;

// функция, выполняемая в первом потоке
void* thread_func1(void* arg) {
    pthread_mutex_lock(&mutex1); // захват первого мьютекса
    printf("Thread 1 used mutex1\n");  
    sleep(1); // имитация работы с ресурсом
    printf("Thread 1 wants to use mutex2\n");  
    pthread_mutex_lock(&mutex2); // попытка захвата второго мьютекса
    printf("Thread 1 used mutex2\n");
    // освобождение мьютексов  
    pthread_mutex_unlock(&mutex2);  
    pthread_mutex_unlock(&mutex1); 
    return NULL;  
}

// функция, выполняемая во втором потоке
void* thread_func2(void* arg) {
    pthread_mutex_lock(&mutex2);  
    printf("Thread 2 used mutex2\n"); 
    sleep(1);  
    printf("Thread 2 wants to use mutex1\n");  
    pthread_mutex_lock(&mutex1);  
    printf("Thread 2 used mutex1\n");  
    pthread_mutex_unlock(&mutex1);  
    pthread_mutex_unlock(&mutex2);  
    return NULL;  
}

int main() {
    pthread_t thread1, thread2; // объявление переменных для потоков

    // инициализация мьютексов
    pthread_mutex_init(&mutex1, NULL); 
    pthread_mutex_init(&mutex2, NULL); 

    // создание потоков
    pthread_create(&thread1, NULL, thread_func1, NULL);  
    pthread_create(&thread2, NULL, thread_func2, NULL);  

    // ожидание завершения потоков
    pthread_join(thread1, NULL);  
    pthread_join(thread2, NULL);  

    // уничтожение мьютексов
    pthread_mutex_destroy(&mutex1); 
    pthread_mutex_destroy(&mutex2); 

    return 0; 
}