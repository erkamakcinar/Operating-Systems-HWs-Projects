#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define TRUE 1

// the number of philosophers
#define NUMBER 5
 
// the maximum time (in millisecond) to sleep
#define MAX_SLEEP_TIME 5000
 
void *philosopher(void *param);
void *waiter();

void thinking(int sleep_time);
void eating(int sleep_time, int number);

//philosopher functions
int left_neighbor(int number);
int right_neighbor(int number);
void test(int i);
void pickup_forks(int number);
void return_forks(int number);

enum {THINKING, HUNGRY, EATING} state[NUMBER];
 
// the thread id of each philosopher (0 .. NUMBER - 1)
int thread_id[NUMBER];

// condition variables and associated mutex lock
pthread_cond_t cond_vars[NUMBER];
pthread_mutex_t mutex_lock;

int totalPorsion = 0;
int porsion = 5;
int total[NUMBER];

pthread_t tid[NUMBER];
pthread_t waiterTid;
 
void init()
{
    int i;
    for (i = 0; i < NUMBER; i++) {
        state[i] = THINKING;
        thread_id[i] = i;
        pthread_cond_init(&cond_vars[i],NULL);
    }

    pthread_mutex_init(&mutex_lock, NULL);
}
    
void create_philosophers()
{
    int i;
    for (i = 0; i < NUMBER; i++) {
        pthread_create(&tid[i], 0, philosopher, (void *)&thread_id[i]);
    }
}
void create_waiter() {
    pthread_create(&waiterTid, 0, waiter,NULL);
}
 
int main()
{ 
    init();
    create_waiter();
    create_philosophers();
    
    pthread_join(waiterTid,NULL);
    for (int i = 0; i < NUMBER; i++)
        pthread_join(tid[i],NULL);


    return 0;
}

void *waiter(){

    time_t start, end;
    double elapsed;  // seconds
    int terminate;
    int days = 0;
    while(days < 1000) {
        printf("***** Day: %d *****\n", days);
        
        for (int i = 0; i < NUMBER; i++) {
            printf("Total porsion number of philosopher %d  : %d\n", i+1, total[i]);
        }    
        start = time(NULL);
        terminate = 1;
        while (terminate) 
        {
            end = time(NULL);
            elapsed = difftime(end, start);
            if (elapsed >= 0.01 ) // seconds 
                terminate = 0;
            else 
                usleep(100);
        }
        
        while(porsion > 0){
            continue;
        }
        if(porsion <= 0) {
            srand((unsigned)time(NULL));
            pthread_mutex_lock(&mutex_lock);
            totalPorsion = totalPorsion + porsion;
            porsion = (int)(rand() % NUMBER) + 1; 
            for(int i = 0; i < NUMBER; i++) {
                pthread_cond_signal(&cond_vars[i]);
            }
            pthread_mutex_unlock(&mutex_lock); 
        }
        days++;
    }
}
 
void *philosopher(void *param)
{
    int *lnumber = (int *)param;
    int number = *lnumber;
    int sleep_time;

    srand((unsigned)time(NULL));
 
    while (TRUE) {
        sleep_time = (int)((rand() % MAX_SLEEP_TIME) + 1);
        thinking(sleep_time);

        pickup_forks(number);
 
        sleep_time = (int)((random() % MAX_SLEEP_TIME) + 1);
        eating(sleep_time, number);

        return_forks(number);

    }
}

/*simulate thinking*/
void thinking(int sleep_time) 
{
    usleep(sleep_time);
}

/*simulate eating*/
void eating(int sleep_time, int number) 
{
    total[number]++;
    porsion--;
    totalPorsion++;
    usleep(sleep_time);
}

 
/* return the left neighbor */
int left_neighbor(int number)
{
    if (number == 0)
        return NUMBER - 1;
    else
        return number - 1;
}
 
/* return the right neighbor */
int right_neighbor(int number)
{
    if (number == NUMBER - 1)
        return 0;
    else
        return number + 1;
}
 
void test(int i)
{
    if(porsion > 0) {
        if ((state[left_neighbor(i)] != EATING) && (state[i] == HUNGRY) && (state[right_neighbor(i)] != EATING) ) {
            state[i] = EATING;
            pthread_cond_signal(&cond_vars[i]);
        }
    } else {
        pthread_cond_wait(&cond_vars[i], &mutex_lock);
    }

}
 
/* A philosopher calls this when it wants to pick up its forks.*/
 
void pickup_forks(int number)
{
    pthread_mutex_lock(&mutex_lock);
 
    state[number] = HUNGRY;
    test(number);
 
    while (state[number] != EATING) {
        pthread_cond_wait(&cond_vars[number], &mutex_lock);
    }
 
    pthread_mutex_unlock(&mutex_lock);
}
 
/* A philosopher calls this when it wants to return its forks.*/
 
void return_forks(int number)
{
    pthread_mutex_lock(&mutex_lock);
 
    state[number] = THINKING;
    test(left_neighbor(number));
    test(right_neighbor(number));
 
    pthread_mutex_unlock(&mutex_lock);
}