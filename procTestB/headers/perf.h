/*
 * ======================================================================
 *         Author:  Lucas MAHIEU (), lucas.mahieu@grenoble-inp.org
 * ======================================================================
 */

#ifndef __PERF_H__
#define __PERF_H__

#include <sys/time.h>
#include <sys/types.h> 

#define NB_MSG_1KB 25000
#define LINE_SIZE 9000

double compute_perf(struct timeval *start, struct timeval *end);

#endif

