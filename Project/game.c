/*************************************/
/* Linux System Programming Project  */
/* Project name : Cell Matrix Game   */
/* Creator      : 컴퓨터학부 윤세연   */
/* Professor    : 김철홍 교수님       */
/*************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <ctype.h>
#include <pthread.h>

#define BUFFER_SIZE 30000   // 열 크기 이상
#define PATH_SIZE 100

struct thread_data {
    int order;
    int thread_location;
    int thread_count;
};

void SequentialProcessing();
void ParallelProcessingProcess();
void ParallelProcessingThread();
void *ThreadProcessing(void *);

struct thread_data **thread_data_array;
struct timeval start_time, end_time;
double delay_time;
char input_file[PATH_SIZE];
char fname[PATH_SIZE];
char buf[BUFFER_SIZE];
char character;
char **matrix;
int position;
int m, n;
FILE *fp;
FILE **fpp;

int main(int argc, char *argv[])
{
    struct rlimit rlim;
    int menu;

    if (argc != 2) {
        fprintf(stderr, "usage: <%s> <matrix_file>\n", argv[0]);
        exit(1);
    }
    strcpy(input_file, argv[1]);

    getrlimit(RLIMIT_NOFILE, &rlim);
    rlim.rlim_cur = 20000L;
    setrlimit(RLIMIT_NOFILE, &rlim);

    while(1) {
        printf("┌─────────────────────────────────────┐\n");
        printf("│>> Cell Matrix Game <<               │\n");
        printf("│-------------------------------------│\n");
        printf("│(1) Exit Program                     │\n");
        printf("│(2) Sequential Processing            │\n");
        printf("│(3) Parallel Processing with Process │\n");
        printf("│(4) Parallel Processing with Thread  │\n");
        printf("└─────────────────────────────────────┘\n");
        printf(">> Enter the number : ");
        scanf("%d", &menu);

        switch (menu)
        {
            case 1:
                exit(0);
            case 2:
                SequentialProcessing();
                break;
            case 3:
                ParallelProcessingProcess();
                break;
            case 4:
                ParallelProcessingThread();
                break;
            default:                        // 이상한 값 에러처리
                while (getchar() != '\n');  // 입력 버퍼 비워주기
                system("clear");
                continue;
        }

        delay_time = (((double)end_time.tv_sec - (double)start_time.tv_sec) * 1000) + (((double)end_time.tv_usec - (double)start_time.tv_usec) / 1000);
        printf("# Execution Time : %lf ms\n\n", delay_time);
    }
    exit(0);
}

void SequentialProcessing() {
    int num_of_generation;
    int alive_cell;
    int i, j;

    strcpy(fname, input_file);

    printf(">> Enter the number of generation : ");
    scanf("%d", &num_of_generation);

    if ((fp = fopen(fname, "r")) == NULL) {
        fprintf(stderr, "open error for %s\n", fname);
        exit(1);
    }

    m = 0;
    n = 0;

    while ((character = fgetc(fp)) != '\n') {
        if (isdigit(character))
            n++;
    }
    rewind(fp);

    while (fgets(buf, BUFFER_SIZE, fp) != NULL) m++;
    fclose(fp);

    matrix = (char **)malloc(sizeof(char *) * (m + 2));
    for (int i = 0; i < m + 2; i++) {
        matrix[i] = (char *)malloc(sizeof(char) * (n + 2));
    }

    gettimeofday(&start_time, NULL);
    for (int gen = 0; gen < num_of_generation; gen++) {
        i = 1;
        j = 1;

        for (int i = 0; i < m + 2; i++) {
            memset(matrix[i], '0', n + 2);
        }   
        if ((fp = fopen(fname, "r")) == NULL) {
            fprintf(stderr, "open error for %s\n", fname);
            exit(1);
        }

        while (!feof(fp)) {
            character = getc(fp);

            if (character == '1') {
                matrix[i][j] = '1';
                j++;
            }
            else if (character == '0') {
                j++;
            }
            else if (character == '\n') {
                i++;
                j = 1;
            }
        }

        if (gen < num_of_generation - 1) {
            sprintf(fname, "gen_%d.matrix", gen + 1);
        }
        else {
            sprintf(fname, "output.matrix");
        }

        if ((fp = freopen(fname, "w", fp)) == NULL) {
            fprintf(stderr, "open error for %s\n", fname);
            exit(1);
        }

        for (int i = 1; i <= m; i++) {
            for (int j = 1; j <= n; j++) {
                alive_cell = 0;

                for (int k = -1; k <= 1; k++) {
                    for (int l = -1; l <= 1; l++) {
                        if (k == 0 && l == 0)
                            continue;

                        if (matrix[i + k][j + l] == '1')
                            alive_cell++;
                    }
                }

                if (matrix[i][j] == '1') {
                    if (alive_cell < 3 || alive_cell > 6) {
                        putc('0', fp);
                    }
                    else if (alive_cell >= 3 && alive_cell <= 6) {
                        putc('1', fp);
                    }
                }
                else if (matrix[i][j] == '0') {
                    if (alive_cell == 4) {
                        putc('1', fp);
                    }
                    else {
                        putc('0', fp);
                    }
                }
                putc(' ', fp);
            }
            putc('\n', fp);
        }
        fclose(fp);
    }
    gettimeofday(&end_time, NULL);

    for (int i = 0; i < m + 2; i++) {
        free(matrix[i]);
    }
    free(matrix);
    printf("\n");
}

void ParallelProcessingProcess() {
    int num_of_generation;
    int num_of_child;
    int alive_cell;
    int i, j;
    int quotient, remainder;
    int *process_count;
    int *process_location;
    int status;
    int pn;
    pid_t **process_id;
    pid_t pid, ppid;

    strcpy(fname, input_file);

    printf(">> Enter the number of child processes to generate : ");
    scanf("%d", &num_of_child);

    printf(">> Enter the number of generation : ");
    scanf("%d", &num_of_generation);

    process_id = (pid_t **)malloc(sizeof(pid_t *) * num_of_generation);
    for (int i = 0; i < num_of_generation; i++) {
        process_id[i] = (pid_t *)malloc(sizeof(pid_t) * num_of_child);
    }

    fpp = (FILE **)malloc(sizeof(FILE *) * num_of_child);

    if ((fp = fopen(fname, "r")) == NULL) {
        fprintf(stderr, "open error for %s\n", fname);
        exit(1);
    }

    m = 0;
    n = 0;

    while ((character = fgetc(fp)) != '\n') {
        if (isdigit(character))
            n++;
    }
    rewind(fp);
    while (fgets(buf, BUFFER_SIZE, fp) != NULL) m++;
    fclose(fp);

    matrix = (char **)malloc(sizeof(char *) * (m + 2));
    for (int i = 0; i < m + 2; i++) {
        matrix[i] = (char *)malloc(sizeof(char) * (n + 2));
    }

    position = n * 2 + 1;
    quotient = m / num_of_child;
    remainder = m % num_of_child;
    ppid = getpid();

    // process_location : 각 프로세스의 매트릭스에서의 위치 정보
    process_location = (int *)malloc(sizeof(int) * num_of_child);
    process_location[0] = 0;
    for (int i = 1; i < num_of_child; i++) {
        if (i <= remainder)
            process_location[i] = process_location[i - 1] + quotient + 1;
        else
            process_location[i] = process_location[i - 1] + quotient;
    }

    // process_count : 각 프로세스의 작업할 행의 수
    process_count = (int *)malloc(sizeof(int) * num_of_child);
    for (int i = 0; i < num_of_child; i++) {
        if (i < remainder)
            process_count[i] = quotient + 1;
        else
            process_count[i] = quotient;
    }

    gettimeofday(&start_time, NULL);
    for (int gen = 0; gen < num_of_generation; gen++) {
        i = 1;
        j = 1;

        for (int i = 0; i < m + 2; i++) {   // matrix 초기화
            memset(matrix[i], '0', n + 2);
        }

        if ((fp = fopen(fname, "r")) == NULL) {
            fprintf(stderr, "open error for %s\n", fname);
            exit(1);
        }
        
        while (!feof(fp)) {
            character = getc(fp);

            if (character == '1') {
                matrix[i][j] = '1';
                j++;
            }
            else if (character == '0') {
                j++;
            }
            else if (character == '\n') {
                i++;
                j = 1;
            }
        }
        fclose(fp);
        
        if (gen < num_of_generation - 1) {
            sprintf(fname, "gen_%d.matrix", gen + 1);
        }
        else {
            sprintf(fname, "output.matrix");
        }

        for (int i = 0; i < num_of_child; i++) {
            if ((fpp[i] = fopen(fname, "w")) == NULL) {
                fprintf(stderr, "open error for %d, %s\n", i, fname);
                exit(1);
            }
        }

        for (int p = 0; p < num_of_child; p++) {
            pid = fork();
            if (pid < 0) {
                fprintf(stderr, "fork error\n");
                exit(1);
            }
            else if (pid == 0) {
                process_id[gen][p] = getpid();
                pn = p;
                break;
            }
            else {
                process_id[gen][p] = pid;               
            }
        }

        if (process_id[gen][pn] == getpid()) {
            fseek(fpp[pn], process_location[pn] * position, SEEK_SET);
            for (int i = 1; i <= process_count[pn]; i++) {
                for (int j = 1; j <= n; j++) {
                    alive_cell = 0;

                    for (int k = -1; k <= 1; k++) {
                        for (int l = -1; l <= 1; l++) {
                            if (k == 0 && l == 0)
                                continue;

                            if (matrix[process_location[pn] + i + k][j + l] == '1')
                                alive_cell++;
                        }
                    }
                    if (matrix[process_location[pn] + i][j] == '1') {
                        if (alive_cell < 3 || alive_cell > 6) {
                            putc('0', fpp[pn]);
                        }
                        else if (alive_cell >= 3 && alive_cell <= 6) {
                            putc('1', fpp[pn]);
                        }
                    }
                    else if (matrix[process_location[pn] + i][j] == '0') {
                        if (alive_cell == 4) {
                            putc('1', fpp[pn]);
                        }
                        else {
                            putc('0', fpp[pn]);
                        }
                    }
                    putc(' ', fpp[pn]);
                }
                putc('\n', fpp[pn]);
            }
        }
        if (getpid() != ppid)
            exit(0);

        while ((pid = wait((int *)0)) != -1);

        for (int i = 0; i < num_of_child; i++) {
            fclose(fpp[i]);
        }
    }
    gettimeofday(&end_time, NULL);

    for (int i = 0; i < num_of_generation; i++) {
        printf("[%d generation]\n", i + 1);
        for (int j = 0; j < num_of_child; j++)
            printf("Process ID : %d\n", process_id[i][j]);
        printf("\n");
    } 

    free(process_count);
    free(process_location);

    for (int i = 0; i < m + 2; i++) {
        free(matrix[i]);
    }
    free(matrix);

    free(fpp);
    for (int i = 0; i < num_of_generation; i++) {
        free(process_id[i]);
    }
    free(process_id);
}

void ParallelProcessingThread() {
    int num_of_generation;
    int num_of_thread;
    int i, j;
    int quotient, remainder;
    int *thread_count;
    int *thread_location;
    char character;
    pthread_t **thread_id;

    strcpy(fname, input_file);

    printf(">> Enter the number of thread to generate : ");
    scanf("%d", &num_of_thread);

    printf(">> Enter the number of generation : ");
    scanf("%d", &num_of_generation);

    thread_id = (pthread_t **)malloc(sizeof(pthread_t *) * num_of_generation);
    for (int i = 0; i < num_of_generation; i++) {
        thread_id[i] = (pthread_t *)malloc(sizeof(pthread_t) * num_of_thread);
    }

    thread_data_array = (struct thread_data **)malloc(sizeof(struct thread_data *) * num_of_generation);
    for (int i = 0; i < num_of_generation; i++) {
        thread_data_array[i] = (struct thread_data *)malloc(sizeof(struct thread_data) * num_of_thread);
    }

    fpp = (FILE **)malloc(sizeof(FILE *) * num_of_thread);

    if ((fp = fopen(fname, "r")) == NULL) {
        fprintf(stderr, "open error for %s\n", fname);
        exit(1);
    }

    m = 0;
    n = 0;

    while ((character = fgetc(fp)) != '\n') {
        if (isdigit(character))
            n++;
    }
    rewind(fp);
    while (fgets(buf, BUFFER_SIZE, fp) != NULL) m++;
    fclose(fp);

    matrix = (char **)malloc(sizeof(char *) * (m + 2));
    for (int i = 0; i < m + 2; i++) {
        matrix[i] = (char *)malloc(sizeof(char) * (n + 2));
    }

    position = n * 2 + 1;
    quotient = m / num_of_thread;
    remainder = m % num_of_thread;

    // thread_location : 각 스레드의 매트릭스에서의 위치 정보
    thread_location = (int *)malloc(sizeof(int) * num_of_thread);
    thread_location[0] = 0;
    for (int i = 1; i < num_of_thread; i++) {
        if (i <= remainder)
            thread_location[i] = thread_location[i - 1] + quotient + 1;
        else
            thread_location[i] = thread_location[i - 1] + quotient;
    }

    // thread_count : 각 스레드의 작업할 행의 수
    thread_count = (int *)malloc(sizeof(int) * num_of_thread);
    for (int i = 0; i < num_of_thread; i++) {
        if (i < remainder)
            thread_count[i] = quotient + 1;
        else
            thread_count[i] = quotient;
    }

    gettimeofday(&start_time, NULL);
    for (int gen = 0; gen < num_of_generation; gen++) {
        i = 1;
        j = 1;

        for (int i = 0; i < m + 2; i++) {   // matrix 초기화
            memset(matrix[i], '0', n + 2);
        }

        if ((fp = fopen(fname, "r")) == NULL) {
            fprintf(stderr, "open error for %s\n", fname);
            exit(1);
        }
        
        while (!feof(fp)) {
            character = getc(fp);

            if (character == '1') {
                matrix[i][j] = '1';
                j++;
            }
            else if (character == '0') {
                j++;
            }
            else if (character == '\n') {
                i++;
                j = 1;
            }
        }
        
        fclose(fp);
        if (gen < num_of_generation - 1) {
            sprintf(fname, "gen_%d.matrix", gen + 1);
        }
        else {
            sprintf(fname, "output.matrix");
        }

        for (int i = 0; i < num_of_thread; i++) {
            if ((fpp[i] = fopen(fname, "w")) == NULL) {
                fprintf(stderr, "open error for %d, %s\n", i, fname);
                exit(1);
            }
        }

        for (int i = 0; i < num_of_thread; i++) {
            thread_data_array[gen][i].order = i;
            thread_data_array[gen][i].thread_location = thread_location[i];
            thread_data_array[gen][i].thread_count = thread_count[i];
            if (pthread_create(&thread_id[gen][i], NULL, ThreadProcessing, (void *)&thread_data_array[gen][i]) != 0) {
                fprintf(stderr, "pthread_create error\n");
                exit(1);
            }
        }
        
        for (i = 0; i < num_of_thread; i++) {
            pthread_join(thread_id[gen][i], NULL);
        }

        for (int i = 0; i < num_of_thread; i++) {
            fclose(fpp[i]);
        }
    }
    gettimeofday(&end_time, NULL);

    for (int i = 0; i < num_of_generation; i++) {
        printf("[%d generation]\n", i + 1);
        for (int j = 0; j < num_of_thread; j++)
            printf("Thread ID : %ld\n", thread_id[i][j]);
        printf("\n");
    } 

    for (int i = 0; i < m + 2; i++) {
        free(matrix[i]);
    }
    free(matrix);

    free(fpp);

    for (int i = 0; i < num_of_generation; i++) {
        free(thread_data_array[i]);
    }
    free(thread_data_array);

    for (int i = 0; i < num_of_generation; i++) {
        free(thread_id[i]);
    }
    free(thread_id);
}

void *ThreadProcessing(void *arg) {
    struct thread_data *data;
    int location;
    int count;
    int alive_cell;
    int t;
    
    data = (struct thread_data *)arg;
    location = data->thread_location;
    count = data->thread_count;
    t = data->order;
    fseek(fpp[t], location * position, SEEK_SET);

    for (int i = 1; i <= count; i++) {
        for (int j = 1; j <= n; j++) {
            alive_cell = 0;

            for (int k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    if (k == 0 && l == 0)
                        continue;

                    if (matrix[location + i + k][j + l] == '1')
                        alive_cell++;
    
                }
            }

            if (matrix[location + i][j] == '1') {
                if (alive_cell < 3 || alive_cell > 6) {
                    putc('0', fpp[t]);
                }
                else if (alive_cell >= 3 && alive_cell <= 6) {
                    putc('1', fpp[t]);
                }
            }
            else if (matrix[location + i][j] == '0') {
                if (alive_cell == 4) {
                    putc('1', fpp[t]);
                }
                else {
                    putc('0', fpp[t]);
                }
            }
            putc(' ', fpp[t]);
        }
        putc('\n', fpp[t]);
    }

    pthread_exit(NULL);
    return NULL;
}
