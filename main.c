#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>

void *t_routine (void * k);
int n_get(char **argv);

void **thread_results;

struct thread_params {
	double start;
	double end;
	double result;
	double dx;
};

int main(int argc, char **argv) {
	if (argv == NULL || argc != 2) {
        printf("Ayayay!\n");
        return 1;
    }

    int n = n_get(argv);
    if (n == -1) {
    	return 1;
    }

	pthread_t *ts = (pthread_t*) calloc(n, sizeof(pthread_t));
	if (ts == NULL) {
		return 1;
	}
	struct thread_params *params = (struct thread_params *) calloc(n, sizeof(struct thread_params));
	if (params == NULL) {
		free(ts);
		return 1;
	}
	double fraq = 10.0 / n;
	params[0].start = 0;
	params[0].end 	= fraq;
	params[0].dx 	= fraq / 10000.0;
	printf("fraq: %lg dx: %lg\n", params[0].end, params[0].dx);
	for (int i = 1; i < n; i++) {
		params[i].start = params[i-1].end;
		params[i].end 	= params[i].start + fraq;
		params[i].dx 	= fraq / 10000.0;
	}

	cpu_set_t cpu_set;
	CPU_ZERO(&cpu_set);

	time_t start = time(NULL);
	//start calculations
    for (int i = 0; i < n; i++) {
    	pthread_create(ts + i, NULL, t_routine, (void *) (&params[i]));

    	CPU_SET(i, &cpu_set);
    	pthread_setaffinity_np(*(ts + i), 1024, &cpu_set);
    	pthread_getaffinity_np(*(ts + i), 1024, &cpu_set);
    	printf("should be %d core: %d\n", i, CPU_ISSET(i, &cpu_set));
    	CPU_ZERO(&cpu_set);
    }

    for (int i = 0; i < n; i++) {
    	pthread_join(*(ts + i), NULL);
    } 

    double sum = 0;
    printf("Result from every thread:\n");
    for (int i = 0; i < n; i++) {
    	printf("%lg ", params[i].result);
    	sum += params[i].result;
    }
    time_t end = time(NULL);

    printf("\nAnd sum: %lg\n", sum);
    printf("hey\n");
    printf("In time: %lg\n", end - start);

    free(ts);
    free(params);
    //CPU_FREE(&cpu_set);

	return 0;
}

void *t_routine (void * k) {
	struct thread_params *these_params = (struct thread_params *) k;

	for (double x = these_params->start; x < these_params->end; x += these_params->dx) {
		these_params->result += x * these_params->dx;
	}

	pthread_exit(0);
}

int n_get(char **argv) {
	char *end_ptr = NULL;
    long int n = strtol(argv[1], &end_ptr, 10);
    if(end_ptr != NULL && *end_ptr != '\0') {
        printf("It is not a number\n");
        return -1;
    }
    if (n == LONG_MAX || n == LONG_MIN || n < 1) {
        printf("Your number is not apropriate\n");
        return -1;
    }

    return n;
}