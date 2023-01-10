#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#define NUM_ELVES 2
#define SIMULATION_TIME 120

#define GIFT_TYPE_CHOCOLATE 1
#define GIFT_TYPE_WOODEN_TOY 2
#define GIFT_TYPE_PLASTIC_TOY 3
#define GIFT_TYPE_GAMESTATION_WOODEN_TOY 4
#define GIFT_TYPE_GAMESTATION_PLASTIC_TOY 5

#define TASK_PAINTING 1
#define TASK_ASSEMBLY 2
#define TASK_PACKAGING 3
#define TASK_QA 4
#define TASK_DELIVERY 5

// Structure to represent a task
typedef struct
{
    int task_type;
    int gift_type;
    int task_id;
} Task;

// Queues for each task type
Task painting_queue[100000];
int painting_queue_size = 0;
Task assembly_queue[100000];
int assembly_queue_size = 0;
Task packaging_queue[100000];
int packaging_queue_size = 0;
Task delivery_queue[100000];
int delivery_queue_size = 0;
Task qa_queue[100000];
int qa_queue_size = 0;

int task_id = 0;

// Mutexes for each queue
pthread_mutex_t painting_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t assembly_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t packaging_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t delivery_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t qa_mutex = PTHREAD_MUTEX_INITIALIZER;

int pthread_sleep(int seconds)
{
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    struct timespec timetoexpire;
    if (pthread_mutex_init(&mutex, NULL))
    {
        return -1;
    }
    if (pthread_cond_init(&conditionvar, NULL))
    {
        return -1;
    }
    struct timeval tp;
    // When to expire is an absolute time, so get the current time and add it to our delay time
    gettimeofday(&tp, NULL);
    timetoexpire.tv_sec = tp.tv_sec + seconds;
    timetoexpire.tv_nsec = tp.tv_usec * 1000;

    pthread_mutex_lock(&mutex);
    int res = pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&conditionvar);

    // Upon successful completion, a value of zero shall be returned
    return res;
}

// Function to generate new tasks at a given probability
void *generate_tasks_work()
{
    while(1) {
        // Use a random number generator to determine the probability of a gift request arriving
        int probability = rand() % 20 + 1; // Generates a random number between 1 and 20

        if (probability <= 18 && 
        (sizeof(painting_queue)/sizeof(Task) > painting_queue_size) &&
        (sizeof(packaging_queue)/sizeof(Task) > packaging_queue_size) &&
        (sizeof(assembly_queue)/sizeof(Task) > assembly_queue_size) &&
        (sizeof(delivery_queue)/sizeof(Task) > delivery_queue_size) &&
        (sizeof(qa_queue)/sizeof(Task) > qa_queue_size))
        { // 90% probability of a gift request arriving
            if (probability <= 8)
            { // 40% chance of a gift containing only a chocolate
                // No tasks required, add the gift directly to the packaging queue
                Task task;
                task.task_type = TASK_PACKAGING;
                task.gift_type = GIFT_TYPE_CHOCOLATE;
                task.task_id = task_id;
                task_id++;
                pthread_mutex_lock(&packaging_mutex);
                packaging_queue[packaging_queue_size++] = task;
                pthread_mutex_unlock(&packaging_mutex);
            }
            if (probability <= 12)
            { // 20 chance of a gift containing a chocolate and a wooden toy
                Task task1;
                task1.task_type = TASK_PAINTING;
                task1.gift_type = GIFT_TYPE_WOODEN_TOY;
                task1.task_id = task_id;
                task_id++;
                pthread_mutex_lock(&painting_mutex);
                painting_queue[painting_queue_size++] = task1;
                pthread_mutex_unlock(&painting_mutex);

                Task task2;
                task2.task_type = TASK_PACKAGING;
                task2.gift_type = GIFT_TYPE_WOODEN_TOY;
                task2.task_id = task_id;
                task_id++;
                pthread_mutex_lock(&packaging_mutex);
                packaging_queue[packaging_queue_size++] = task2;
                pthread_mutex_unlock(&packaging_mutex);
            }
            if (probability <= 16)
            { // 20 chance of a gift containing a chocolate and a plastic toy
                Task task1;
                task1.task_type = TASK_ASSEMBLY;
                task1.gift_type = GIFT_TYPE_PLASTIC_TOY;
                task1.task_id = task_id;
                task_id++;
                pthread_mutex_lock(&assembly_mutex);
                assembly_queue[assembly_queue_size++] = task1;
                pthread_mutex_unlock(&assembly_mutex);

                Task task2;
                task2.task_type = TASK_PACKAGING;
                task2.gift_type = GIFT_TYPE_PLASTIC_TOY;
                task2.task_id = task_id;
                task_id++;
                pthread_mutex_lock(&packaging_mutex);
                packaging_queue[packaging_queue_size++] = task2;
                pthread_mutex_unlock(&packaging_mutex);
            }
            if (probability <= 17)
            { // 5 chance of a gift containing a chocolate, a wooden toy and a GS5
                Task task1;
                task1.task_type = TASK_PAINTING;
                task1.gift_type = GIFT_TYPE_WOODEN_TOY;
                task1.task_id = task_id;
                task_id++;
                pthread_mutex_lock(&painting_mutex);
                painting_queue[painting_queue_size++] = task1;
                pthread_mutex_unlock(&painting_mutex);

                Task task2;
                task2.task_type = TASK_QA;
                task2.gift_type = GIFT_TYPE_GAMESTATION_WOODEN_TOY;
                task2.task_id = task_id;
                task_id++;
                pthread_mutex_lock(&qa_mutex);
                qa_queue[qa_queue_size++] = task2;
                pthread_mutex_unlock(&qa_mutex);

                Task task;
                task.task_type = TASK_PACKAGING;
                task.gift_type = GIFT_TYPE_GAMESTATION_WOODEN_TOY;
                task.task_id = task_id;
                task_id++;
                pthread_mutex_lock(&packaging_mutex);
                packaging_queue[packaging_queue_size++] = task;
                pthread_mutex_unlock(&packaging_mutex);
            }
            if (probability <= 18)
            { // 5 chance of a gift containing a chocolate, a plastic toy and a GS5
                Task task1;
                task1.task_type = TASK_ASSEMBLY;
                task1.gift_type = GIFT_TYPE_PLASTIC_TOY;
                task1.task_id = task_id;
                task_id++;
                pthread_mutex_lock(&assembly_mutex);
                assembly_queue[assembly_queue_size++] = task1;
                pthread_mutex_unlock(&assembly_mutex);

                Task task2;
                task2.task_type = TASK_QA;
                task2.gift_type = GIFT_TYPE_GAMESTATION_PLASTIC_TOY;
                task2.task_id = task_id;
                task_id++;
                pthread_mutex_lock(&qa_mutex);
                qa_queue[qa_queue_size++] = task2;
                pthread_mutex_unlock(&qa_mutex);

                Task task;
                task.task_type = TASK_PACKAGING;
                task.gift_type = GIFT_TYPE_GAMESTATION_PLASTIC_TOY;
                task.task_id = task_id;
                task_id++;
                pthread_mutex_lock(&packaging_mutex);
                packaging_queue[packaging_queue_size++] = task;
                pthread_mutex_unlock(&packaging_mutex);
            }
        }
        pthread_sleep(1);
    }
    return NULL;    
}
// Function for the Elf A worker thread
void *elf_a_work(void *arg)
{
    while (1)
    {
        // Check the painting queue for tasks
        pthread_mutex_lock(&painting_mutex);
        if (painting_queue_size > 0)
        {
            Task task = painting_queue[0];
            // Remove the task from the queue
            for (int i = 0; i < painting_queue_size - 1; i++)
            {
                painting_queue[i] = painting_queue[i + 1];
            }
            painting_queue_size--;
            pthread_mutex_unlock(&painting_mutex);

            // Perform the task (sleep for the appropriate amount of time to simulate the task)
            pthread_sleep(3);

            // Add the follow-up tasks to the appropriate queues
            pthread_mutex_lock(&packaging_mutex);
            packaging_queue[packaging_queue_size++] = task;
            pthread_mutex_unlock(&packaging_mutex);
        }
        else
        {
            pthread_mutex_unlock(&painting_mutex);
        }

        // Check the packaging queue for tasks
        pthread_mutex_lock(&packaging_mutex);
        if (packaging_queue_size > 0)
        {
            Task task = packaging_queue[0];
            // Remove the task from the queue
            for (int i = 0; i < packaging_queue_size - 1; i++)
            {
                packaging_queue[i] = packaging_queue[i + 1];
            }
            packaging_queue_size--;
            pthread_mutex_unlock(&packaging_mutex);
            // Perform the task (sleep for the appropriate amount of time to simulate the task)
            pthread_sleep(1);

            // Add the follow-up tasks to the appropriate queues
            pthread_mutex_lock(&delivery_mutex);
            delivery_queue[delivery_queue_size++] = task;
            pthread_mutex_unlock(&delivery_mutex);
        }
        else
        {
            pthread_mutex_unlock(&packaging_mutex);
        }
    }
    return NULL;
}
void *elf_b_work(void *arg)
{
    while (1)
    {
        // Check the assembly queue for tasks
        pthread_mutex_lock(&assembly_mutex);
        if (assembly_queue_size > 0)
        {
            Task task = assembly_queue[0];
            // Remove the task from the queue
            for (int i = 0; i < assembly_queue_size - 1; i++)
            {
                assembly_queue[i] = assembly_queue[i + 1];
            }
            assembly_queue_size--;
            pthread_mutex_unlock(&assembly_mutex);
            // Perform the task (sleep for the appropriate amount of time to simulate the task)
            pthread_sleep(2);

            // Add the follow-up tasks to the appropriate queues
            pthread_mutex_lock(&packaging_mutex);
            packaging_queue[packaging_queue_size++] = task;
            pthread_mutex_unlock(&packaging_mutex);
        }
        else
        {
            pthread_mutex_unlock(&assembly_mutex);
        }

        // Check the packaging queue for tasks
        pthread_mutex_lock(&packaging_mutex);
        if (packaging_queue_size > 0)
        {
            Task task = packaging_queue[0];
            // Remove the task from the queue
            for (int i = 0; i < packaging_queue_size - 1; i++)
            {
                packaging_queue[i] = packaging_queue[i + 1];
            }
            packaging_queue_size--;
            pthread_mutex_unlock(&packaging_mutex);

            // Perform the task (sleep for the appropriate amount of time to simulate the task)
            pthread_sleep(1);

            // Add the follow-up tasks to the appropriate queues
            pthread_mutex_lock(&delivery_mutex);
            delivery_queue[delivery_queue_size++] = task;
            pthread_mutex_unlock(&delivery_mutex);
        }
        else
        {
            pthread_mutex_unlock(&packaging_mutex);
        }
    }
    return NULL;
}
// Function for the Santa worker thread
void *santa_work(void *arg)
{
    while (1)
    {
        // Check the QA queue for tasks
        pthread_mutex_lock(&qa_mutex);
        if (qa_queue_size > 0)
        {
            Task task = qa_queue[0];
            // Remove the task from the queue
            for (int i = 0; i < qa_queue_size - 1; i++)
            {
                qa_queue[i] = qa_queue[i + 1];
            }
            qa_queue_size--;
            pthread_mutex_unlock(&qa_mutex);
            // Perform the task (sleep for the appropriate amount of time to simulate the task)
            pthread_sleep(1);

            // Add the follow-up tasks to the appropriate queues
            pthread_mutex_lock(&packaging_mutex);
            packaging_queue[packaging_queue_size++] = task;
            pthread_mutex_unlock(&packaging_mutex);
        }
        else
        {
            pthread_mutex_unlock(&qa_mutex);
        }

        // Check the delivery queue for tasks
        pthread_mutex_lock(&delivery_mutex);
        if (delivery_queue_size > 0)
        {
            Task task = delivery_queue[0];
            // Remove the task from the queue
            for (int i = 0; i < delivery_queue_size - 1; i++)
            {
                delivery_queue[i] = delivery_queue[i + 1];
            }
            delivery_queue_size--;
            pthread_mutex_unlock(&delivery_mutex);

            // Perform the task (sleep for the appropriate amount of time to simulate the task)
            pthread_sleep(1);
        }
        else
        {
            pthread_mutex_unlock(&delivery_mutex);
        }
    }
    return NULL;
}

void *printer_work(void *arg)
{
    int *argv = (int *)arg;
    int t = *argv;
    time_t start, end;
    double elapsed; // seconds
    int terminate = 1;
    start = time(NULL);

    while (1)
    {
        while (terminate)
        {
            end = time(NULL);
            elapsed = difftime(end, start);
            if (elapsed >= ((double)t)) // seconds
                terminate = 0;
            else
                usleep(5000);
        }
        end = time(NULL);
        t = (int)difftime(end, start);
        pthread_mutex_lock(&packaging_mutex);
        pthread_mutex_lock(&painting_mutex);
        pthread_mutex_lock(&assembly_mutex);
        pthread_mutex_lock(&qa_mutex);
        pthread_mutex_lock(&delivery_mutex);

        printf("At %d sec packaging: ", t);
        for (int i = 0; i < packaging_queue_size; i++)
        {
            printf("%d ", packaging_queue[i].task_id);
        }
        printf("\n");
        printf("At %d sec painting: ", t);
        for (int i = 0; i < painting_queue_size; i++)
        {
            printf("%d ", painting_queue[i].task_id);
        }
        printf("\n");
        printf("At %d sec assembly: ", t);
        for (int i = 0; i < assembly_queue_size; i++)
        {
            printf("%d ", assembly_queue[i].task_id);
        }
        printf("\n");
        printf("At %d sec QA: ", t);
        for (int i = 0; i < qa_queue_size; i++)
        {
            printf("%d ", qa_queue[i].task_id);
        }
        printf("\n");
        printf("At %d sec delivery: ", t);
        for (int i = 0; i < delivery_queue_size; i++)
        {
            printf("%d ", delivery_queue[i].task_id);
        }
        printf("\n");

        pthread_mutex_unlock(&packaging_mutex);
        pthread_mutex_unlock(&painting_mutex);
        pthread_mutex_unlock(&assembly_mutex);
        pthread_mutex_unlock(&qa_mutex);
        pthread_mutex_unlock(&delivery_mutex);
        pthread_sleep(1);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    // Seed the random number generator
    srand(time(NULL));

    // Create the worker threads
    pthread_t elf_a_thread;
    pthread_t elf_b_thread;
    pthread_t santa_thread;
    pthread_t printer_thread;
    pthread_t generate_tasks_thread;
    
    pthread_create(&elf_a_thread, NULL, elf_a_work, NULL);
    pthread_create(&elf_b_thread, NULL, elf_b_work, NULL);
    pthread_create(&santa_thread, NULL, santa_work, NULL);
    pthread_create(&generate_tasks_thread, NULL, generate_tasks_work, NULL);

    if (argc > 1)
    {
        int param = atoi(argv[1]);
        pthread_create(&printer_thread, NULL, printer_work, &param);
    }

    // Run the simulation for a given amount of time
    pthread_sleep(SIMULATION_TIME);

    // Terminate the worker threads
    pthread_cancel(elf_a_thread);
    pthread_cancel(elf_b_thread);
    pthread_cancel(santa_thread);
    pthread_cancel(printer_thread);
    pthread_cancel(generate_tasks_thread);

    return 0;
}
