#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h> 
#include <netinet/ip.h> 


#define BUFSIZE 2048
#define HISIP "192.168.0.11"	// Ecriture en dur de l'adresse IP pour faciliter les tests

// Structure pour passer des arguments à la fonction appellée lorsque l'on crée le thread qui recoit les messages
typedef struct arguments {
	int myPort;
	int fd;

} arguments;


// Fonction appelée par le thread qui recoit les messages
void* polling(void* arg) {
	int recvlen;                    /* # bytes received */
	unsigned char buf[BUFSIZE];     /* receive buffer */

	while (1) {
		printf("waiting on port %d\n", ((arguments*) arg)->myPort);
		recvlen = recv(((arguments*) arg)->fd, buf, BUFSIZE, 0);
		printf("received %d bytes\n", recvlen);
		if (recvlen > 0) {
			buf[recvlen] = 0;
			printf("received message: \"%s\"\n", buf);
		}
	}
}





/* 
	Arguments dans l'ordre : Port der réception (ex : 1153), port sur lequel il faut envoyer les données,
 	adresse IP sur laquelle il faut envoyer les données
*/
int main(int argc, char* argv[]) {

	/*
	------------------------------------------------------------------------------------
	On crée un file descriptor, on bind les adresses etc. Partie commune à tout le monde
	------------------------------------------------------------------------------------
	*/
	int fd; 						// Descripteur de fichier
	int myPort  = atoi(argv[1]);	// Port de réceptrion des données
	int hisPort = atoi(argv[2]);	// Port sur lequel envoyer les données
 	char* hisIP = HISIP; 			// Adresse ip de l'autre personne au bout du canal
	struct sockaddr_in mySocket;  	// Notre Socket
	struct sockaddr_in hisSocket;   // Socket de l'autre personne au bout du canal


	// On crée une socket UDP
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("cannot create socket\n");
			return 0;
	}

	// On la bind sur notre adresse par défaut et sur le port MYPORT
	memset((char *)&mySocket, 0, sizeof(mySocket));
	mySocket.sin_family = AF_INET;
	mySocket.sin_addr.s_addr = htonl(INADDR_ANY);
	mySocket.sin_port = htons(myPort);

	if (bind(fd, (struct sockaddr *)&mySocket, sizeof(mySocket)) < 0) {
			perror("bind failed");
			return 0;
	}



	/*
	------------------------------------------------------------------------------------
	On crée un thread pour recevoir les messages et on reste dans le main pour les envoyer
	------------------------------------------------------------------------------------
	*/
	pthread_t listener;	// Thread pour l'écoute (la réception)

	// On lance la boucle infinie pour recevoir des messages
	arguments* args = malloc(sizeof(*args));
	args->myPort = myPort;
	args->fd = fd;

    if (pthread_create(&listener, NULL, polling, args)) {
		perror("pthread_create");
		return EXIT_FAILURE;
    }


    // On envoie un message pour test
    sleep(1);
	char* my_message = "this is a test message";  // Message pour les test

	memset((char*)&hisSocket, 0, sizeof(hisSocket));
	hisSocket.sin_family = AF_INET;
	// hisSocket.sin_addr.s_addr = htonl(inet_aton(hisIP));
	hisSocket.sin_port = htons(hisPort);



	// On envoie un message sur au socket qui écoute
	while (1) {
		if (sendto(fd, my_message, strlen(my_message), 0, (struct sockaddr *)&hisSocket, sizeof(hisSocket)) < 0) {
			perror("sendto failed");
			return 0;
		}
	}





    // On attend la fin du thread "listener", je ne pense pas que ce soit utile vu qu'il y aura ausi une boucle infinie
    // pour l'envoie de messages
    if (pthread_join(listener, NULL)) {
		perror("pthread_join");
		return EXIT_FAILURE;
    }


    return 0;
}