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
#include <sys/time.h>
// to poll the pipe
#include <poll.h>


// Pour le seed de random
#include	<time.h>

#define BUFFER_SIZE 256
#define INPUT_FILE "procTestA/data/toSend_2.txt"
//time in seconds
#define TIME_TO_WAIT 2
#define DEBUG

void bug(char* msg){
	fprintf(stderr, "%s",msg);
	fflush(stderr);
}

int main(int argc, char **argv)
{

	pid_t canal_pid;
	// Tube de communication entre les 2 processus
	// Il faut deux tube pour communiquer dans les 2 sens
	int tube_CtoCanal[2];
	int tube_CanaltoC[2];
	puts("Création d'un tube\n");
	/* pipe 1*/
	if (pipe(tube_CtoCanal) != 0) bug("Erreur dans pipe 1 \n");
	/* pipe 2*/
	if (pipe(tube_CanaltoC) != 0) bug("Erreur dans pipe 2 \n");

	canal_pid = fork();    
	if (canal_pid == -1) bug("Erreur dans fork \n");

	// processus fils : Créer le Canal pour com avec A
	if (canal_pid == 0){
		// On remplace le stdin du proc par le pipe
		dup2(tube_CtoCanal[0],0);
		// On remplace le stdout du proc par le pipe
		dup2(tube_CanaltoC[1],1);

		// on ferme les 2 pipes inutile
		close(tube_CtoCanal[1]);
		close(tube_CanaltoC[0]);
#ifdef DEBUG
		fprintf(stderr, "### lancer de canal\n");
		fprintf(stderr, "Fermeture de l'entrée du tube A to Canal dans le proc fils (pid = %d)\n", getpid());
		fprintf(stderr, "Fermeture de la sortie du tube Canal to A dans le proc fils (pid = %d)\n", getpid());
#endif
		// liste qui servira au execvp
		char* arg_list[] = {"./myCanal", "1", NULL};
		// On lance le myCanal
		execv("myCanal", arg_list);
		
		bug("Erreur de execvp myCanal\n");

	// processus père : C'est le processus A qui envoie des msg au canal (son fils)
	} else {
		srand(time(NULL));

		char sendBuffer[BUFFER_SIZE];
		memcpy(sendBuffer, "Are you alive?", strlen("Are you alive?"));



		// On remplace le stdin du proc par le pipe
		dup2(tube_CanaltoC[0],0);
		// On remplace le stdout du proc par le pipe
		dup2(tube_CtoCanal[1],1);

		// on ferme les 2 pipes inutiles
		close(tube_CtoCanal[0]);
		close(tube_CanaltoC[1]);

		// structures temporelles
		struct timeval timeToPrint;

		// structure à donner à poll() pour savoir si il y a des data à lire 
		struct pollfd pfd[1];
		pfd[0].fd = fileno(stdin);
		pfd[0].events = POLLIN | POLLPRI;



#ifdef DEBUG
		fprintf(stderr, "### Proc C: pid= %d\n", getpid());
#endif
		
		// Petit dodo pour être sur que tout le monde soit bien près pour le test
		sleep(1);
		while(1){

#ifdef DEBUG
			gettimeofday(&timeToPrint, NULL);
			bug("### Proc C\n");
			fprintf(stderr, "-----------------------------------------------------------------------------\n");
			fprintf(stderr,"C teste la vivacité de D à t = : %llu, %llu \n", (uint64_t) timeToPrint.tv_sec, (uint64_t) timeToPrint.tv_usec);
			fprintf(stderr, "-----------------------------------------------------------------------------\n");
			fflush(stderr);
#endif
			// fonction que doit appeler C pour envoyer des données à D par le canal (simple printf dans stdout)
			fprintf(stdout, "%s\n", sendBuffer);

			// réception de signe de vie de D
			if (poll(pfd,1,TIME_TO_WAIT*1000) > 0) {
				fgets(sendBuffer, BUFFER_SIZE, stdin);
				bug("### Proc C\n");
				fprintf(stderr, "-----------------------------------------------------------------------------\n");
				fprintf(stderr, "D est en vie %s\n", sendBuffer);
				fprintf(stderr, "-----------------------------------------------------------------------------\n");
			}
		}
		fprintf(stderr, "### PROC C : TEST FINISHED\n");
		close(tube_CanaltoC[0]);
		close(tube_CtoCanal[1]);
		wait(NULL);
	}
	return 0;
}
