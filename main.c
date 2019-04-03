#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/sysinfo.h>

#pragma option -O3

#define min(a, b) a > b ? b : a

struct thread_params {
	double start;
	double end;
	double result;
	double dx;
};

void *usefull_routine (void * k);
void *useless_routine (void * k);
int n_get(char **argv);
int *get_info(int *n_proc);
int in_array(int *arr, int j, int size);
int init(struct thread_params *params, int n);

int main(int argc, char **argv) {
	if (argv == NULL || argc != 2) {
        printf("Ayayay!\n");
        return 1;
    }

    //get n_threads
    int n = n_get(argv);
    if (n == -1) {
    	return 1;
    }

    //get number of available cpu-s and their list
    int n_proc = 0;
    int *proc = get_info(&n_proc);

    for (int i = 0; i < n_proc; i++) {
		printf("u_proc: %d\n", proc[i]);
	}

	//init threads structures
	pthread_t *ts = (pthread_t*) calloc(n, sizeof(pthread_t));
	if (ts == NULL) {
		return 1;
	}
	struct thread_params *params = (struct thread_params *) calloc(n, sizeof(struct thread_params));
	pthread_attr_t *pthread_attributes = (pthread_attr_t *) calloc(n, sizeof(pthread_attr_t));
	if (params == NULL) {
		free(ts);
		free(proc);
		return 1;
	}

	int err = init(params, min(n_proc, n));
	if (err != 0) {
		printf("error\n");
		free(ts);
    	free(params);
    	free(proc);
		return 1;
	}

	cpu_set_t cpu_set;
	CPU_ZERO(&cpu_set);

	pthread_attr_t attr;
	err = pthread_attr_init(&attr);
	if (err != 0) {
		printf("error\n");
		free(ts);
    	free(params);
    	free(proc);
		return 1;
	}

	void * (*routine) (void *) = usefull_routine;

	//start calculations
    for (int i = 0, proc = 0; i < n; i++, proc++) {
    	if (i >= n_proc && proc != 0) {
    		routine = useless_routine;
    		proc = 0;
    	}
    	err = pthread_attr_init(pthread_attributes + i);

    	CPU_ZERO(&cpu_set);
    	CPU_SET(proc, &cpu_set);
    	//pthread_attr_setaffinity_np(pthread_attributes + i, sizeof(cpu_set_t), &cpu_set);

    	pthread_create(ts + i, pthread_attributes + i, routine, (void *) (&params[i]));
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
    
    printf("\nAnd sum: %lg\n", sum);

    err = pthread_attr_destroy(&attr);

    free(ts);
    free(params);
    free(proc);

	return 0;
}

void *useless_routine (void * k) {
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

int *get_info(int *n_proc) {
	*n_proc = get_nprocs();
	
	if (*n_proc == 0 || *n_proc > 10) {
		printf("No processeror O_o_O!\n");
		assert(0);
	}

	int unique_proc_n = 0;
	int *unique_proc = calloc(*n_proc, sizeof(int));
	char *path = calloc(56, sizeof(char));
	char *path_c = "/sys/bus/cpu/devices/cpu0/topology/thread_siblings";

	for (int i = 0; i < 56; i++) {
		path[i] = path_c[i];
	}

	for (int i = 0; i < *n_proc; i++) {
		path[24] = i + '0';
		FILE *fd = fopen(path, "r");
		if (fd == NULL) {
			break;
		}

		char *str_mask = calloc(4, sizeof(char));
		int j = 0;
		fscanf(fd, "%s", str_mask);
		
		int mask = strtol(str_mask, NULL, 16);

		while(mask != 0) {
			if (mask % 2 == 0) {
				mask = mask >> 1;
				j++;
				continue;
			}

			if (in_array(unique_proc, j, unique_proc_n)) {
				break;
			}

			if (j == i) {
				unique_proc[unique_proc_n] = j;
				unique_proc_n++;
			}

			mask = mask >> 1;
			j++;
		}

		fclose(fd);
		free(str_mask);
	}

	free(path);
	*n_proc = unique_proc_n;
	return unique_proc;
}

int in_array(int *arr, int j, int size) {
	for (int i = 0; i < size; i++) {
		if (arr[i] == j) {
			return 1;
		}
	}

	return 0;
}

int init(struct thread_params *params, int n) {
	double fraq = 10.0 / (double) n;
	params[0].start = 0;
	params[0].end 	= fraq;
	params[0].dx 	= 0.00000001;
	printf("fraq: %lg dx: %lg\n", params[0].end, params[0].dx);
	for (int i = 1; i < n; i++) {
		params[i].start = params[i-1].end;
		params[i].end 	= params[i].start + fraq;
		params[i].dx 	= 0.00000001;
	}

	return 0;
}

void *usefull_routine (void * k) {
	struct thread_params *these_params = (struct thread_params *) k;
	double a = these_params->start;
	double b = these_params->end;
	double dx = these_params->dx;

	for (double x = a; x < b; x += dx) {
		these_params->result += x * dx;
	}

	pthread_exit(0);
}