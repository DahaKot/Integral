#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>

       int pthread_setaffinity_np(pthread_t thread, size_t cpusetsize,
                                  const cpu_set_t *cpuset);
       int pthread_getaffinity_np(pthread_t thread, size_t cpusetsize,
                                  cpu_set_t *cpuset);

void *routine (void *p);

void **thread_results;

struct thread_params {
	double start;
	double end;
	double result;
};

int main(int argc, char **argv) {
	if (argv == NULL || argc != 2) {
        printf("Ayayay!\n");
        return 0;
    }

    char *end_ptr = NULL;
    long int n = strtol(argv[1], &end_ptr, 10);
    if(end_ptr != NULL && *end_ptr != '\0') {
        printf("It is not a number\n");
        return 0;
    }
    if (n == LONG_MAX || n == LONG_MIN || n < 1) {
        printf("Your number is not apropriate\n");
        return 0;
    }

    pthread_attr_t threadAttributes;
	int initRet = pthread_attr_init (&threadAttributes);

    struct thread_params *params = (struct thread_params *) calloc(n, sizeof(struct thread_params));
    assert(params);
    for (int i = 0; i < n; i++) {
    	params[i].result = 0.0;
    }

    pthread_t *threads = (pthread_t*) calloc(n, sizeof(pthread_t));
    assert(threads);

    for (int i = 0; i < n; i++) {
    	pthread_create(&threads[i], &threadAttributes, routine, (void *) (&params[i]));
    }

    for (int i = 0; i < n; i++) {
    	pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < n; i++) {
    	printf("i = %d result=%lg pointer=%p\n", i, params[i].result, &params[i]);
    	printf("result; %lg\n", params[i].result);
    }

    initRet = pthread_attr_destroy (&threadAttributes);
    free(threads);
    free(params);

    return 0;
}

void *routine (void *p) {
	struct thread_params *params = (struct thread_params*) p;
	params->result = 1.9;

	pthread_exit(0);
}