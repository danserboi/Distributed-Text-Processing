// SERBOI FLOREA-DAN 335CB

#ifndef _UTILS_
#define _UTILS_

#include "mpi.h"
#include <pthread.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "glist.h"

#define DIE(condition, message) \
	do { \
		if ((condition)) { \
			fprintf(stderr, "[%d]: %s\n", __LINE__, (message)); \
			perror(""); \
			exit(1); \
		} \
	} while (0)

#define NUM_THREADS 4

#define MASTER 0
#define HORROR 1
#define COMEDY 2
#define FANTASY 3
#define SCIFI 4

#define RECEIVER 1

#define NO_LINES_PROC 20

#define CHUNK 1030

#define MAX_NO_LINES 2500

#endif
