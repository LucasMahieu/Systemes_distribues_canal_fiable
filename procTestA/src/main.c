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
#include <signal.h> // pour les signaux
#include <sys/wait.h>
// Pour le seed de random
#include <time.h>
// Pour la structure Time
#include <sys/time.h>
typedef struct timeval Time;

#define MAX_TOSEND_BUFFER 4096

#define DEBUG
// #define PERFORMANCE

void bug(char* msg){
	fprintf(stderr, "%s",msg);
	fflush(stderr);
}

int main(int argc, char **argv)
{

	pid_t canal_pid;
	// Tube de communication entre les 2 processus
	// Il faut deux tube pour communiquer dans les 2 sens
	int tube_AtoCanal[2];
	puts("Création d'un tube\n");
	/* pipe 1*/
	if (pipe(tube_AtoCanal) != 0) bug("Erreur dans pipe 1 \n");


	canal_pid = fork();    
	if (canal_pid == -1) bug("Erreur dans fork \n");

	// processus fils : Créer le Canal pour com avec A
	if (canal_pid == 0){
		// On remplace le stdin du proc par le pipe
		dup2(tube_AtoCanal[0],0);

		// on ferme les 3 pipes inutile
		close(tube_AtoCanal[1]);

#ifdef DEBUG
		fprintf(stderr, "### lancer de canal\n");
		fprintf(stderr, "Fermeture de l'entrée du tube A to Canal dans le proc fils (pid = %d)\n", getpid());
#endif
		// liste qui servira au execvp
		char* arg_list[] = {"./myCanal", "1", NULL};
		// On lance le myCanal
		execv("myCanal", arg_list);
		
		bug("Erreur de execvp myCanal\n");

	// processus père : C'est le processus A qui envoie des msg au canal (son fils)
	} else {
		srand(time(NULL));

		char toSendBuffer[MAX_TOSEND_BUFFER];
		FILE* fIN;
		if((fIN = fopen("procTestA/data/toSend.txt","r"))==NULL) bug("Erreur dans fopen fIN\n");

		FILE* fOUT;
		if((fOUT=fdopen(tube_AtoCanal[1],"w"))==NULL) bug("Erreur fOUT prog A");

		volatile uint8_t stop=0;
		
		// Fermeture des fd inutile
		close(tube_AtoCanal[0]);

#ifdef DEBUG
		fprintf(stderr, "### Proc A: pid= %d\n", getpid());
#endif
		// Petit dodo pour être sur que tout le monde soit bien près pour le test
		sleep(1);

#ifdef PERFORMANCE
		FILE* perf;
		if ((perf = fopen("performance.txt", "w")) == NULL) bug("Erreur dans fopen perf");
		Time currentTime;
		gettimeofday(&currentTime, NULL);
#endif

		while(!stop){

			if(fgets(toSendBuffer, MAX_TOSEND_BUFFER, fIN)==NULL){
				bug("## PROC A : No more data, EOF read\n");
				stop=1;
				continue;
			}
#ifdef DEBUG
			bug("### Proc A\n");
			fprintf(stderr, "-----------------------------------------------------------------------------\n");
			fprintf(stderr,"A envoie: %s ", toSendBuffer);
			fprintf(stderr, "-----------------------------------------------------------------------------\n");
			fflush(stderr);
#endif
			// fonction que doit appeler A pour envoyer des données à B par le canal
			fwrite(toSendBuffer, sizeof(char), strlen(toSendBuffer), fOUT);
			//send(toSendBuffer, fOUT);

			memset(toSendBuffer,'\0', MAX_TOSEND_BUFFER);

			// Pour pas que le test se finisse trop vite, que ca soit plus réaliste
			// on pause quelques sec
			//sleep(rand()%2);
		}
		//kill(canal_pid, SIGTERM); // On fait un envoi de signal pour signifier au fils que c'est fini.
		fclose(fIN);
		fclose(fOUT);
		close(tube_AtoCanal[1]);

#ifdef PERFORMANCE
		Time diffTime;
		gettimeofday(&diffTime, NULL);
		diffTime.tv_sec -= currentTime.tv_sec;
		diffTime.tv_usec -= currentTime.tv_usec;
		// fprintf(perf, " ",)
		fprintf(perf, "Time to send messages from A to canal : %ld.%06ld\n", (long int) diffTime.tv_sec, (long int) diffTime.tv_usec);

		fclose(perf);
#endif

		wait(NULL);
		fprintf(stderr, "## PROC A : END\n");
	}
	return 0;
}
