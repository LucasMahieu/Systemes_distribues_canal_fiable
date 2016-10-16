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

#define BUFFER_SIZE 256


int main(int argc, char **argv)
{

	pid_t canal_pid;
	// Tube de communication entre les 2 processus
	// Il faut deux tube pour communiquer dans les 2 sens
	int tube_AtoCanal[2];
	int tube_CanaltoA[2];
	char bufferR[256], bufferW[256];
	puts("Création d'un tube\n");
	/* pipe 1*/
	if (pipe(tube_AtoCanal) != 0)
	{
		{
			fprintf(stderr, "Erreur dans pipe 1 \n");
			exit(1);
		}
	}
	/* pipe 2*/
	if (pipe(tube_CanaltoA) != 0)
	{
		{
			fprintf(stderr, "Erreur dans pipe 2 \n");
			exit(1);
		}
	}
	canal_pid = fork();    
	if (canal_pid == -1)
	{
		fprintf(stderr, "Erreur dans fork \n");
		exit(1);
	}
	/* processus fils */
	if (canal_pid == 0){
		printf("Fermeture de l'entrée du tube A to Canal dans le proc fils (pid = %d)\n", getpid());
		close(tube_AtoCanal[1]);
		printf("Fermeture de la sortie du tube Canal to A dans le proc fils (pid = %d)\n", getpid());
		close(tube_CanaltoA[0]);

		read(tube_AtoCanal[0], bufferR, BUFFER_SIZE);
		printf("Le fils (%d) a lu : %s \n", getpid(), bufferR);

		sprintf(bufferW, "Message du fils (%d) au père: Coucou papounet", getpid());
		write(tube_CanaltoA[1], bufferW, BUFFER_SIZE);
	} else {
		printf("Fermeture de la sortie du tube A to Canal dans le proc pere (pid = %d)\n", getpid());
		close(tube_AtoCanal[0]);
		printf("Fermeture de l'entrée du tube Canal to A dans le proc pere (pid = %d)\n", getpid());
		close(tube_CanaltoA[1]);

		sprintf(bufferW, "Message du pére (%d) au fils: Je suis ton pére", getpid());
		write(tube_AtoCanal[1], bufferW, BUFFER_SIZE);


		read(tube_CanaltoA[0], bufferR, BUFFER_SIZE);
		printf("Le père (%d) a lu : %s \n", getpid(), bufferR);

		wait(NULL);
	}

	return 0;
}
