/*
 * ======================================================================
 *         Author:  Lucas MAHIEU (), lucas.mahieu@grenoble-inp.org
 * ======================================================================
 */

#include "bug.h"

void bug(char *s)
{
	fprintf(stderr,"%s",s);
	fflush(stderr);
}
