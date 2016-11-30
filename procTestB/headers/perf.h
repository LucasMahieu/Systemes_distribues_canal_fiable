/*
 * ======================================================================
 *         Author:  Lucas MAHIEU (), lucas.mahieu@grenoble-inp.org
 * ======================================================================
 */

#ifndef __PERF_H__
#define __PERF_H__

#include <sys/time.h>
#include <sys/types.h> 

#define NB_MSG_KB 25000
#define LINE_SIZE 8000

#define NB_MSG_LATENCE 1

double compute_duration(struct timeval *start, struct timeval *end);

#endif

