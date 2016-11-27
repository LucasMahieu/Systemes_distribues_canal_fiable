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
	fflush(stdout);
}
