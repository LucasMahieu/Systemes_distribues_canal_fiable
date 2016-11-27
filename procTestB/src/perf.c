/*
 * ======================================================================
 *         Author:  Lucas MAHIEU (), lucas.mahieu@grenoble-inp.org
 * ======================================================================
 */

#include "perf.h"
#include "stdio.h"

/**
 * Cette fonction retourne le temps écoulé entre start et end en milli seconde
 */
double compute_perf(struct timeval *start, struct timeval *end)
{
	double elapsed_time = 0.0;

	//fprintf(stderr,"--- start %ldsec %ld usec\n", start->tv_sec, start->tv_usec);
	//fprintf(stderr,"--- end %ldsec %ld usec\n", end->tv_sec, end->tv_usec);
	elapsed_time = (end->tv_sec - start->tv_sec);
	elapsed_time += (end->tv_usec - start->tv_usec) / 1000000.0;

	return elapsed_time;
}
