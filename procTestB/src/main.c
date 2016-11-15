/*
 * Simple udp server
 */
#include <stdio.h> //printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
// Pour le pipe nommé
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
// Pour le seed de random
#include <time.h>

#define MAX_RECEIVED_BUFFER 4096

#define DEBUG

void bug(char* msg){
	fprintf(stderr, "%s",msg);
	exit(1);
}

int main(int argc, char **argv)
{
	pid_t canal_pid;
	// Tube de communication entre les 2 processus
	// Il faut deux tube pour communiquer dans les 2 sens
	int tube_CanaltoB[2];
	puts("Création d'un tube\n");
	/* pipe 1*/
	if (pipe(tube_CanaltoB) != 0) bug("Erreur dans pipe 2 \n");

	canal_pid = fork();    
	if (canal_pid == -1) bug("Erreur dans fork \n");

	// processus fils: lance le canal pour pouvoir recevoir des données
	if (canal_pid == 0){
		// On remplace le stdout du proc par le pipe avec B
		dup2(tube_CanaltoB[1],1);

		// On ferme les 3 fd dont on a pas besoin
		close(tube_CanaltoB[0]);
		
		// liste qui servira au execvp
		char* arg_list[] = {"./myCanal", "0", NULL};
		// On lance le myCanal
		execv("myCanal", arg_list);
		
		bug("Erreur de execvp myCanal\n");

	// processus père : recoie des données du canal et les écrit dans un fichier
	} else {
		char receiveBuffer[MAX_RECEIVED_BUFFER];
		FILE* fOUT;
		if((fOUT = fopen("procTestB/data/receive.txt","w"))==NULL) bug("Erreur dans fopen fOUT\n");

		// Ferme les pipe inutiles
		close(tube_CanaltoB[1]);

#ifdef DEBUG
		fprintf(stderr,"### PROC B: pid: %d)\n\n", getpid());
#endif
		// Petit dodo pour être sur que tout le monde soit bien prêt pour le test
		sleep(1);
		while(1){
			// le canal va faire un déliver et on recoie les données avec read
			read(tube_CanaltoB[0], receiveBuffer, MAX_RECEIVED_BUFFER);
#ifdef DEBUG
			fflush(stderr);
#endif
			fwrite(receiveBuffer,sizeof(*receiveBuffer),strlen(receiveBuffer), fOUT);
			fflush(fOUT);
			memset(receiveBuffer,'\0', MAX_RECEIVED_BUFFER);
			
		}

		fclose(fOUT);
		wait(NULL);
		fprintf(stderr, "## PROC B : END\n");
	}
	return 0;
}
