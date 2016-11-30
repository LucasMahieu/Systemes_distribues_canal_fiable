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
// to poll the pipe
#include <poll.h>
// Pour sig kill
#include <signal.h>

#define MAX_RECEIVED_BUFFER 8000

// #define DEBUG

void bug(char* msg){
	fprintf(stderr, "%s",msg);
	exit(1);
}

int main(int argc, char **argv)
{
	pid_t canal_pid;
	// Tube de communication entre les 2 processus
	// Il faut deux tube pour communiquer dans les 2 sens
	int tube_DtoCanal[2];
	int tube_CanaltoD[2];
	/* pipe 1*/
	if (pipe(tube_DtoCanal) != 0)
		bug("Erreur dans pipe 1 \n");
	/* pipe 2*/
	if (pipe(tube_CanaltoD) != 0)
		bug("Erreur dans pipe 2 \n");

	canal_pid = fork();    
	if (canal_pid == -1)
		bug("Erreur dans fork \n");

	// processus fils: lance le canal pour pouvoir recevoir des données
	if (canal_pid == 0){
		// On remplace le stdout du proc par le pipe avec B
		dup2(tube_CanaltoD[1],1);
		dup2(tube_DtoCanal[0],0);

		// On ferme les 2 fd dont on n'a pas besoin
		close(tube_CanaltoD[0]);
		close(tube_DtoCanal[1]);

		// liste qui servira au execvp
		char* arg_list[] = {"./myCanal", "0", "-f", NULL};
		// On lance le myCanal
		execv("myCanal", arg_list);
		
		bug("Erreur de execvp myCanal\n");

	// processus père : recoie des données du canal et les écrit dans un fichier
	} else {
		char receiveBuffer[MAX_RECEIVED_BUFFER];
		char message[MAX_RECEIVED_BUFFER];
		memcpy(message, "Yes I am!", strlen("Yes I am!"));

		dup2(tube_DtoCanal[1],1);
		dup2(tube_CanaltoD[0],0);

		// Ferme les pipes inutiles
		close(tube_CanaltoD[1]);
		close(tube_DtoCanal[0]);
#ifdef DEBUG
		fprintf(stderr,"### PROC D: pid: %d)\n\n", getpid());
#endif
		int cpt_life = 10;
		// Petit dodo pour être sur que tout le monde soit bien prêt pour le test
		sleep(1);
		while(1){
			// le canal va faire un déliver et on recoit les données avec read
			memset(receiveBuffer,'\0', MAX_RECEIVED_BUFFER);
			read(tube_CanaltoD[0], receiveBuffer, MAX_RECEIVED_BUFFER);
			fprintf(stderr, "#### D à reçu un message\n");
			// fprintf(stderr, "#### D à reçu : %s", receiveBuffer);

#ifdef DEBUG
			bug("### Proc D\n");
			fprintf(stderr, "-----------------------------------------------------------------------------\n");
			fprintf(stderr, "#### D à reçu : %s", receiveBuffer);
			fprintf(stderr, "-----------------------------------------------------------------------------\n");
#endif
			
			fprintf(stderr, "On fait un signe de vie\n");
			fprintf(stderr, "\n");
			fprintf(stdout, "%s\n", message);
			fflush(stdout);
			fprintf(stderr, "%s pas bon\n", message);
			//cpt_life --;
			
		}
		close(tube_DtoCanal[1]);
		close(tube_CanaltoD[0]);
		kill(canal_pid, SIGINT);
		wait(NULL);
	}
	return 0;
}
