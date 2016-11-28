/**
 * Processus de test qui créer une instance de canal
 * et écoute sur le canal
 * Il ira écrire tout se qu'il a lu dans une fichier 
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

#include "bug.h"
#include "perf.h"

#define MAX_RECEIVED_BUFFER 1500
#define OUTPUT_FILE "procTestB/data/receive.txt"

// Décommenter cette ligne pour activer l'affichage de trace dans stderr
// #define DEBUG

int main(int argc, char **argv)
{
	// Variable utilisées pour les mesures de performances
	uint8_t perf_debit = 0;
	uint8_t perf_latence = 0;
	uint32_t cpt_msg = 0;
	double perf_duration = 0; 
	struct timeval perf_start_time;
	struct timeval perf_end_time;

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
			fprintf(stderr, "-------------------------------------\n");
			fprintf(stderr, "MESURE DE PERFORMANCE DE LATENCE ACTIVE\n");
			fprintf(stderr, "-------------------------------------\n");
		}
	}

	pid_t canal_pid;
	// Tube de communication entre les 2 processus
	// Il faut deux tube pour communiquer dans les 2 sens
	int tube_BtoCanal[2];
	int tube_CanaltoB[2];

	/* pipe 1*/
	if (pipe(tube_BtoCanal) != 0)
		bug("Erreur dans pipe 1 \n");
	/* pipe 2*/
	if (pipe(tube_CanaltoB) != 0)
		bug("Erreur dans pipe 2 \n");

	canal_pid = fork();    
	if (canal_pid == -1)
		bug("Erreur dans fork \n");

	/**
	* Processus fils: lance le canal pour pouvoir recevoir des données
	*/
	if (canal_pid == 0){
		// On remplace le stdout du proc par le pipe avec B
		dup2(tube_CanaltoB[1],1);

		// On ferme les 3 fd dont on a pas besoin
		close(tube_CanaltoB[0]);
		close(tube_BtoCanal[1]);
		close(tube_BtoCanal[0]);
		
		// liste qui servira au execvp
		char* arg_list[] = {"./myCanal", "0", NULL};

		// On lance le myCanal
		execv("myCanal", arg_list);
		
		bug("Erreur de execvp myCanal\n");

	/** 
	 * processus père
	 * reçoie des données du canal et les écrit dans un fichier
	 */
	} else {
		char receiveBuffer[MAX_RECEIVED_BUFFER];

		FILE* fOUT;
		if ((fOUT = fopen(OUTPUT_FILE,"w")) == NULL)
			bug("Erreur dans fopen fOUT\n");

		// Ferme les pipes inutiles
		close(tube_CanaltoB[1]);
		close(tube_BtoCanal[0]);
		close(tube_BtoCanal[1]);

		fprintf(stderr, "--------------------------------------------------\n");
		fprintf(stderr, "Le processus B est en marche\n");
		fprintf(stderr, "Écriture des messages reçus  dans %s\n", OUTPUT_FILE);
		fprintf(stderr, "Pressez Ctrl+C pour quitter\n");
		fprintf(stderr, "--------------------------------------------------\n");

#ifdef DEBUG
		fprintf(stderr,"### PROC B: pid: %d)\n\n", getpid());
#endif
		while(1){
			if (perf_debit == 1) {
				if (cpt_msg < NB_MSG_1KB) {
					// le canal va faire un déliver et on recoie les données avec read
					read(tube_CanaltoB[0], receiveBuffer, LINE_SIZE);
					if (cpt_msg == 0) {
						gettimeofday(&perf_start_time, NULL);
					} 
				} else {
					gettimeofday(&perf_end_time, NULL);
					perf_duration = compute_perf(&perf_start_time, &perf_end_time);
					fprintf(stderr, "---------------------------------------------\n");
					fprintf(stderr, "---- Durée test débit = %.4f sec       ----\n", perf_duration);
					fprintf(stderr, "---- Débit max        =  125.0 MB/s      ----\n");
					fprintf(stderr, "---- Débit mesuré     = %.3f MB/s       ----\n",250.0/perf_duration);
					fprintf(stderr, "---- Ratio            = %.3f %%         ----\n",(250.0*100.0)/(perf_duration*125.0));
					fprintf(stderr, "---------------------------------------------\n");
					fflush(stderr);
					break;
				}
				cpt_msg ++;
				//fprintf(stderr, "%d message délivré\n", cpt_msg);
			} else { 
				// le canal va faire un déliver et on recoie les données avec read
				read(tube_CanaltoB[0], receiveBuffer, MAX_RECEIVED_BUFFER);
			}
#ifdef DEBUG
			// fprintf(stderr, "#### B à reçu : %s",receiveBuffer);
			// fprintf(stderr, "-------------------------------------------\n");
			// fflush(stderr);
#endif
			if (perf_debit == 0) {
				fwrite(receiveBuffer, sizeof(*receiveBuffer), 
						strlen(receiveBuffer), fOUT);
				fflush(fOUT);
				memset(receiveBuffer, '\0', MAX_RECEIVED_BUFFER);
			}
			
		}
		fclose(fOUT);
		wait(NULL);
	}
	return 0;
}
