//Handshake functions
#ifndef STRUCTURE
#include "structure.h"
#define STRUCTURE
#endif/*
Handshake function for the server.
Wait for the message "initialization" and reply "initialization".
*/
int handshakeServer(Socket s) {
	Sockaddr_in si_other;
	char* message = "initialization";
	unsigned int recv_len, slen=10;  // Initialization at 10, do not understand why it does not work otherwise
	char buf[MAX_BUFLEN];
	memset(buf,'\0', MAX_BUFLEN);

	while (strcmp(buf, message)) {
		if ((recv_len = recvfrom(s, buf, MAX_BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1) bug("recvfrom()");
	}
	printf("The server received the message : %s\n", buf);
	//printf("oo, %d, %d", slen, sizeof(si_other));
	
	//now reply the client with "initialization"
	if (sendto(s, message, sizeof(message), 0, (struct sockaddr*) &si_other, sizeof(si_other)) == -1) bug("sendto()");
	return 0;
}

/*
Handshake function for the client.
Send the message "initialization" and wait for the answer "initialization".
*/
int handshakeClient(Socket s, Sockaddr_in* si_other_p) {
	Sockaddr_in si_garbage;
	char* message = "initialization";
	unsigned int recv_len, slen;
	char buf[MAX_BUFLEN];
	memset(buf,'\0', MAX_BUFLEN);

	// Set a timeout to wait before resending a connection
	// struct timeval tv;
	// tv.tv_sec = 1;  // 1 s timeout
	// tv.tv_usec = 0;  // Not init'ing this can cause strange errors

	// if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval))) {
	// 	bug("setsockopt()");
	// }
	// signal(EAGAIN, handle_alarm );
	// alarm(1);

	while (strcmp(buf, message)) {
		if (sendto(s, message, strlen(message), 0, (struct sockaddr*) si_other_p, sizeof(*si_other_p)) == -1) bug("sendto()");
		printf("Trying to connect ... \n");

		if ((recv_len = recvfrom(s, buf, MAX_BUFLEN, 0, (struct sockaddr *) &si_garbage, &slen)) == -1) bug("recvfrom()");
	}
	printf("The server received the message : %s \nThe communication can now begin\n", buf);
	//printf("oo, %d, %d", slen, sizeof(si_other));
	
	//now reply the client with "initialization"

	return 0;
}