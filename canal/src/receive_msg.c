/*
 * ======================================================================
 *         Author:  Lucas MAHIEU (), lucas.mahieu@grenoble-inp.org
 * ======================================================================
 */
#include "receive_msg.h" 
#include "stdio.h" 

void deliver(char* s) 
{
	fprintf(stdout, "%s", s);
	fprintf(stderr, "Le message écrit dans le fd : %s", s);
	fflush(stdout);
}
