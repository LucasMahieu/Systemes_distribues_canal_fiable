/*
 * This is the processus A that send messages to other processus through UDP socket 
 */

#include <stdio.h> //printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
// Pour le pipe
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "bug.h"
#include "perf.h"

#define MAX_TOSEND_BUFFER 9000

#define INPUT_FILE "procTestA/data/toSend.txt"

// Uncomment to be verbose tracing message in stderr
// #define DEBUG


int main(int argc, char **argv)
{
	uint8_t perf_debit = 0;
	uint8_t perf_latence = 0;

	if (argc > 1) {
		if (!strcmp(argv[1], "-d")) {
			// test de perf de débit
			perf_debit = 1;
			fprintf(stderr, "-------------------------------------\n");
			fprintf(stderr, "MESURE DE PERFORMANCE DE DÉBIT ACTIVE\n");
			fprintf(stderr, "-------------------------------------\n");
		} else if (!strcmp(argv[1], "-l")) {
			// test de perf de latence
			perf_latence = 1;
			fprintf(stderr, "---------------------------------------\n");
			fprintf(stderr, "MESURE DE PERFORMANCE DE LATENCE ACTIVE\n");
			fprintf(stderr, "---------------------------------------\n");
		}
	}

	pid_t canal_pid;
	// Tube de communication entre les 2 processus : procA et Canal
	// Il faut deux tube pour communiquer dans les 2 sens
	int tube_AtoCanal[2];
	int tube_CanaltoA[2];

	/* pipe 1*/
	if (pipe(tube_AtoCanal) != 0) bug("Erreur dans pipe 1 \n");
	/* pipe 2*/
	if (pipe(tube_CanaltoA) != 0) bug("Erreur dans pipe 2 \n");

	canal_pid = fork();    
	if (canal_pid == -1)
		bug("Erreur dans fork \n");
	/**
	* processus fils : Créer le Canal pour com avec A
	*/
	if (canal_pid == 0){
		// On remplace le stdin du proc par le pipe
		dup2(tube_AtoCanal[0],0);

		// on ferme les 3 pipes inutiles
		close(tube_AtoCanal[1]);
		close(tube_CanaltoA[0]);
		close(tube_CanaltoA[1]);
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

	/** 
	 * Processus père
	 * C'est le processus A qui envoie des msg au canal (son fils)
	 */
	} else {
		char toSendBuffer[MAX_TOSEND_BUFFER];
		int i;
		for(i = 0; i < MAX_TOSEND_BUFFER - 7; i++) {
			toSendBuffer[i] = 'a';
		}
		toSendBuffer[i] = '\n';
		i++;
		toSendBuffer[i] = '\0';
		
		FILE* fIN;
		if((fIN = fopen(INPUT_FILE,"r"))==NULL) bug("Erreur dans fopen fIN\n");

		FILE* fOUT;
		if((fOUT=fdopen(tube_AtoCanal[1],"w"))==NULL) bug("Erreur fOUT prog A");

		volatile uint8_t stop=0;
		
		// Fermeture des fd inutile
		close(tube_AtoCanal[0]);
		close(tube_CanaltoA[1]);
		close(tube_CanaltoA[0]);
#ifdef DEBUG
		fprintf(stderr, "### Proc A: pid= %d\n", getpid());
#endif
		int cpt = 0;
		// Petit dodo pour être sur que tout le monde soit bien prêt pour le test
		sleep(1);
		while (!stop) {
			if (perf_debit == 1) { 
				if (cpt > NB_MSG_1KB){
					// Le test de perf est fini
					stop = 1;
					continue;
				}
				cpt++;
			} else {
				// Lecture d'une ligne du fichier de test
				if (fgets(toSendBuffer, MAX_TOSEND_BUFFER, fIN) == NULL) {
					bug("## PROC A : No more data, EOF read\n");
					stop = 1;
					continue;
				}
			}
#ifdef DEBUG
			// bug("### Proc A\n");
			// fprintf(stderr, "---------------------------------------------\n");
			// fprintf(stderr,"A envoie: %s ", toSendBuffer);
			// fprintf(stderr, "---------------------------------------------\n");
			// fflush(stderr);
#endif
			// fonction que doit appeler A pour envoyer des données à B 
			// par le canal
			fwrite(toSendBuffer, sizeof(char), strlen(toSendBuffer), fOUT);

			if (perf_debit == 0) {
				memset(toSendBuffer,'\0', MAX_TOSEND_BUFFER);
			}
		}
		fprintf(stderr, "### PROC A : TEST FINISHED\n");
		fclose(fIN);
		fclose(fOUT);
		close(tube_AtoCanal[1]);
		wait(NULL);
	}
	return 0;
}
