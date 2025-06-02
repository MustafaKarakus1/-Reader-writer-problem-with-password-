#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#define MAX_THREADS 10
#define MAX_OPS 5

int BUFFER = 0;
int read_count = 0;
int thread_no = 0;

pthread_mutex_t read_count_mutex;
pthread_mutex_t password_mutex;
pthread_mutex_t print_mutex;

sem_t resource;
sem_t readTry;

unsigned long password_table[MAX_THREADS];
int password_count = 0;

typedef struct {
    int thread_no;
    unsigned long hash;
    char validity[10];
    char role[10];
    char value[20];
} LogEntry;

LogEntry logs[MAX_THREADS * 2 * MAX_OPS];
int log_index = 0;

unsigned long hash_pthread_t(pthread_t thread_id) {
    return (unsigned long)(uintptr_t)thread_id;
}

int check_password(unsigned long hash) {
    pthread_mutex_lock(&password_mutex);
    for (int i = 0; i < password_count; i++) {
        if (password_table[i] == hash) {
            pthread_mutex_unlock(&password_mutex);
            return 1;
        }
    }
    pthread_mutex_unlock(&password_mutex);
    return 0;
}

void add_password(unsigned long hash) {
    pthread_mutex_lock(&password_mutex);
    password_table[password_count++] = hash;
    pthread_mutex_unlock(&password_mutex);
}

void log_access(int thread_no, unsigned long hash, const char* validity, const char* role, const char* value_str) {
    pthread_mutex_lock(&print_mutex);
    LogEntry entry;
    entry.thread_no = thread_no;
    entry.hash = hash;
    snprintf(entry.validity, sizeof(entry.validity), "%s", validity);
    snprintf(entry.role, sizeof(entry.role), "%s", role);
    snprintf(entry.value, sizeof(entry.value), "%s", value_str);
    logs[log_index++] = entry;
    pthread_mutex_unlock(&print_mutex);
}

void* real_reader(void* arg) {
    int id = __sync_add_and_fetch(&thread_no, 1);
    unsigned long hash = hash_pthread_t(pthread_self());
    add_password(hash);

    for (int i = 0; i < MAX_OPS; i++) {
        sleep(1);

        if (!check_password(hash)) continue;

        sem_wait(&readTry);
        pthread_mutex_lock(&read_count_mutex);
        read_count++;
        if (read_count == 1)
            sem_wait(&resource);
        pthread_mutex_unlock(&read_count_mutex);
        sem_post(&readTry);

        int val = BUFFER;
        char val_str[20];
        sprintf(val_str, "%d", val);
        log_access(id, hash, "real", "reader", val_str);

        pthread_mutex_lock(&read_count_mutex);
        read_count--;
        if (read_count == 0)
            sem_post(&resource);
        pthread_mutex_unlock(&read_count_mutex);
    }

    pthread_exit(NULL);
}

void* dummy_reader(void* arg) {
    int id = __sync_add_and_fetch(&thread_no, 1);
    unsigned long hash = 0;

    if (!check_password(hash)) {
        log_access(id, hash, "dummy", "reader", "ACCESS DENIED");
        pthread_exit(NULL);
    }

    for (int i = 0; i < MAX_OPS; i++) {
        sleep(1);
        int val = BUFFER;
        char val_str[20];
        sprintf(val_str, "%d", val);
        log_access(id, hash, "dummy", "reader", val_str);
    }

    pthread_exit(NULL);
}

void* real_writer(void* arg) {
    int id = __sync_add_and_fetch(&thread_no, 1);
    unsigned long hash = hash_pthread_t(pthread_self());
    add_password(hash);

    for (int i = 0; i < MAX_OPS; i++) {
        sleep(1);

        if (!check_password(hash)) continue;

        sem_wait(&resource);
        int val = rand() % 10000;
        BUFFER = val;
        char val_str[20];
        sprintf(val_str, "%d", val);
        log_access(id, hash, "real", "writer", val_str);
        sem_post(&resource);
    }

    pthread_exit(NULL);
}

void* dummy_writer(void* arg) {
    int id = __sync_add_and_fetch(&thread_no, 1);
    unsigned long hash = 0;

    if (!check_password(hash)) {
        log_access(id, hash, "dummy", "writer", "ACCESS DENIED");
        pthread_exit(NULL);
    }

    for (int i = 0; i < MAX_OPS; i++) {
        sleep(1);
        int val = rand() % 10000;
        BUFFER = val;
        char val_str[20];
        sprintf(val_str, "%d", val);
        log_access(id, hash, "dummy", "writer", val_str);
    }

    pthread_exit(NULL);
}

void print_logs_to_file(int test_case_number) {
    char filename[50];
    sprintf(filename, "test_case_%d_output.txt", test_case_number);
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("File open error");
        return;
    }

    fprintf(f, "%-10s %-15s %-10s %-10s %-10s\n", "Thread_No", "Hash_Value", "Validity", "Role", "Value");
    for (int i = 0; i < log_index; i++) {
        fprintf(f, "%-10d %-15lu %-10s %-10s %-10s\n",
                logs[i].thread_no, logs[i].hash, logs[i].validity, logs[i].role, logs[i].value);
    }

    fclose(f);
}

void run_test_case(int readers, int writers, int test_case_number) {
    if (readers + writers > MAX_THREADS) {
        printf("Total number of real threads cannot exceed %d.\n", MAX_THREADS);
        return;
    }

    log_index = 0;
    thread_no = 0;
    password_count = 0;

    pthread_t threads[(readers + writers) * 2];
    int index = 0;

    // Create real readers
    for (int i = 0; i < readers; i++) pthread_create(&threads[index++], NULL, real_reader, NULL);

    // Create real writers
    for (int i = 0; i < writers; i++) pthread_create(&threads[index++], NULL, real_writer, NULL);

    // Create dummy readers (same number as real readers)
    for (int i = 0; i < readers; i++) pthread_create(&threads[index++], NULL, dummy_reader, NULL);

    // Create dummy writers (same number as real writers)
    for (int i = 0; i < writers; i++) pthread_create(&threads[index++], NULL, dummy_writer, NULL);

    for (int i = 0; i < index; i++) pthread_join(threads[i], NULL);

    print_logs_to_file(test_case_number);
}

int main() {
    srand(time(NULL));
    pthread_mutex_init(&read_count_mutex, NULL);
    pthread_mutex_init(&password_mutex, NULL);
    pthread_mutex_init(&print_mutex, NULL);
    sem_init(&resource, 0, 1);
    sem_init(&readTry, 0, 1);

    printf("Running test case 1...\n");
    run_test_case(2, 3, 1);

    printf("Running test case 2...\n");
    run_test_case(3, 2, 2);

    printf("Running test case 3...\n");
    run_test_case(1, 1, 3);

    printf("Output files generated for each test case.\n");

    pthread_mutex_destroy(&read_count_mutex);
    pthread_mutex_destroy(&password_mutex);
    pthread_mutex_destroy(&print_mutex);
    sem_destroy(&resource);
    sem_destroy(&readTry);

    return 0;
}
