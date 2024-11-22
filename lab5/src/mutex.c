#include <errno.h>     
#include <pthread.h>     
#include <stdio.h>       
#include <stdlib.h>      

// функции, которые будут использоваться в потоках
void do_one_thing(int *); 
void do_another_thing(int *);
void do_wrap_up(int);

// глобальная переменная, которую будут изменять потоки
int common = 0; // общий счетчик, который будет изменяться потоками
int r1 = 0, r2 = 0, r3 = 0; // дополнительные глобальные переменные  

// инициализация мьютекса для синхронизации доступа к общему ресурсу
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

int main() {
  pthread_t thread1, thread2; // объявляем переменные для потоков

  // создаем первый поток, который выполняет функцию do_one_thing
  if (pthread_create(&thread1, NULL, (void *)do_one_thing, (void *)&common) != 0) {
    perror("pthread_create"); // выводим ошибку, если создание потока не удалось
    exit(1);  
  }

  // Создаем второй поток, который выполняет функцию do_another_thing
  if (pthread_create(&thread2, NULL, (void *)do_another_thing, (void *)&common) != 0) {
    perror("pthread_create");  
    exit(1);  
  }

  // ожидаем завершения первого потока
  if (pthread_join(thread1, NULL) != 0) {
    perror("pthread_join"); // выводим ошибку, если ожидание завершения потока не удалось
    exit(1);  
  }

  // ожидаем завершения второго потока
  if (pthread_join(thread2, NULL) != 0) {
    perror("pthread_join");  
    exit(1);  
  }

  // вызываем функцию для завершения работы и вывода результата
  do_wrap_up(common);

  return 0;  
}

// функция, выполняемая первым потоком
void do_one_thing(int *pnum_times) {
  int i; // счетчик для внешнего цикла
  unsigned long k; // счетчик для внутреннего цикла
  int work; // переменная для хранения значения счетчика

  // цикл, выполняющий 50 итераций
  for (i = 0; i < 50; i++) {
    pthread_mutex_lock(&mut); // захватываем мьютекс для синхронизации доступа к общему ресурсу
    printf("doing one thing\n"); // сообщение о текущем действии
    work = *pnum_times; // читаем текущее значение счетчика
    printf("counter = %d\n", work); // выводим текущее значение счетчика
    work++; // увеличиваем значение счетчика
    for (k = 0; k < 500000; k++) // имитация работы (задержка)
      ;                
    *pnum_times = work; // записываем новое значение счетчика
    pthread_mutex_unlock(&mut); // освобождаем мьютекс
  }
}

// функция, выполняемая вторым потоком
void do_another_thing(int *pnum_times) {
  int i;  
  unsigned long k; 
  int work;  

  for (i = 0; i < 50; i++) {
    pthread_mutex_lock(&mut); 
    printf("doing another thing\n");  
    work = *pnum_times;  
    printf("counter = %d\n", work);  
    work++;  
    for (k = 0; k < 500000; k++)  
      ;                  
    *pnum_times = work;  
    pthread_mutex_unlock(&mut);  
  }
}

// функция для завершения работы и вывода результата
void do_wrap_up(int counter) {
  int total; // переменная для хранения общего результата  
  printf("All done, counter = %d\n", counter); // выводим итоговое значение счетчика
}