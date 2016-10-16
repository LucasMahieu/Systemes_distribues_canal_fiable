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
// Pour le seed de random
#include	<time.h>

#define MAX_TOSEND_BUFFER 2048

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
	int tube_AtoCanal[2];
	int tube_CanaltoA[2];
	puts("Création d'un tube\n");
	/* pipe 1*/
	if (pipe(tube_AtoCanal) != 0) bug("Erreur dans pipe 1 \n");
	/* pipe 2*/
	if (pipe(tube_CanaltoA) != 0) bug("Erreur dans pipe 2 \n");

	canal_pid = fork();    
	if (canal_pid == -1) bug("Erreur dans fork \n");

	// processus fils : Créer le Canal de com avec A
	if (canal_pid == 0){
		// On remplace le stdin du proc par le pipe
		dup2(tube_AtoCanal[0],0);

		// on ferme les 3 pipes inutile
		close(tube_AtoCanal[1]);
		close(tube_CanaltoA[0]);
		close(tube_CanaltoA[1]);
#ifdef DEBUG
		printf("Fermeture de l'entrée du tube A to Canal dans le proc fils (pid = %d)\n", getpid());
		printf("Fermeture de la sortie du tube Canal to A dans le proc fils (pid = %d)\n", getpid());
#endif
		// liste qui servira au execvp
		char* arg_list[] = {"./myCanal", "1", NULL};
		// On lance le myCanal
		execv("myCanal", arg_list);
		
		bug("Erreur de execvp myCanal\n");

		//read(tube_AtoCanal[0], bufferR, BUFFER_SIZE);
		//printf("Le fils (%d) a lu : %s \n", getpid(), bufferR);

		//sprintf(bufferW, "Message du fils (%d) au père: Coucou papounet", getpid());
		//write(tube_CanaltoA[1], bufferW, BUFFER_SIZE);
	// processus père : C'est le processus A qui envoie des msg au canal (son fils)
	} else {
		char toSendBuffer[MAX_TOSEND_BUFFER];
		srand(time(NULL));
		FILE* fIN;
		if((fIN = fopen("procTestA/data/toSend.txt","r"))==NULL) bug("Erreur dans fopen fIN\n");

		FILE* fOUT;
		if((fOUT=fdopen(tube_AtoCanal[1],"w"))==NULL) bug("Erreur fOUT prog A");

		// Fermeture des fd inutile
		close(tube_AtoCanal[0]);
		close(tube_CanaltoA[1]);
		close(tube_CanaltoA[0]);
#ifdef DEBUG
		printf("Fermeture de la sortie du tube A to Canal dans le proc pere (pid = %d)\n", getpid());
		printf("Fermeture de l'entrée du tube Canal to A dans le proc pere (pid = %d)\n", getpid());
#endif
		// Petit dodo pour être sur que tout le monde soit bien près pour le test
		sleep(5);
		while( fgets(toSendBuffer, MAX_TOSEND_BUFFER, fIN) ){
#ifdef DEBUG
			printf("A envoie le msg suivant à B: %s\n",toSendBuffer);
#endif
			// fonction que doit appeler A pour envoyer des données à B par le canal
			fwrite(toSendBuffer, 1, strlen(toSendBuffer), fOUT);

			// Pour pas que le test se finisse trop vite, que ca soit plus réaliste
			// on pause quelques sec
			sleep(rand()%3);
		}
		fclose(fIN);
		fclose(fOUT);
		wait(NULL);
	}
	return 0;
}