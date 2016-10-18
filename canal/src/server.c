

#include "structure.h"




/*
Find the message to write the data in. If the message does not exist yes, create it.
Usually returns the address of the structure where the message is to store.
Else, return null.
*/
IDMessage* findMessage(int RequestNumMessage, int RequestMax) {
	int i;
	IDMessage* voidElement;	// pointer to a void element in case the message does not exist yet
	int voidIsFound = 0;	// bool to know if we have already found a void element
	for (i=0; i<MAXMESSAGE; i++) {
		// Trying to find the message entry
		if (Tab[i].numMessage == RequestNumMessage){
			return &Tab[i];
		}

		// Trying to find a void entry
		if (!voidIsFound){
			if (Tab[i].numMessage == 0){
				voidElement = &Tab[i];
				voidIsFound = 1;
			}
		}
	}
	printf("WHAT %d \n", voidIsFound);
	if (voidIsFound) {
		voidElement->numMessage = RequestNumMessage;
		voidElement->maxSequence = RequestMax;
		voidElement->buffer = (char*) calloc(RequestMax * MAXMESSAGE, sizeof(char));
		printf("AALOCATION EN %x\n", voidElement->buffer);
		if (voidElement->buffer == NULL) {
			perror("calloc invalided");
		}
		return &Tab[i];
	}
	return NULL;
}


// Just init the Tab with default values. 0 is not an element, it is a default value.
void InitTab(){
	int i;
	for (i=1; i<MAXMESSAGE; i++){
		Tab[i].numMessage = -1;
		Tab[i].maxSequence = -1;
		listeChaine l = {-1, NULL};
		Tab[i].list = l;
		Tab[i].buffer = NULL;
	}
}

// Add a value to the listeChaine
void addValue(listeChaine* l, int i){
	listeChaine newElement;
	listeChaine* L = l;
	listeChaine* M = l;
	newElement.num = i;

	while (M->num < i && M->next != NULL){
		L = M;
		M = M->next;
	}
	newElement.next = L->next;
	L->next = &newElement;
}

void printList(listeChaine* l){
	listeChaine* L = l;
	printf("Dans la liste chainee : ");
	while (L != NULL) {
		printf("%d", L->num);
		L = L->next;
	}
	printf("\n");
}

void printBuffer(char* buf, int max){
	int i = 0;
	for (i=0; i<max; i++) {
		printf("Contenu du buffer en %d : %s\n", i, buf+i*MAX_BUFLEN);
	}
}

// Check that all sequences of the message have been received.
// return 0 if true, 1 if some misses.
int checkCompletionMessage(listeChaine* l, int maxSequence) {
	listeChaine* L = l;
	int cnt = 0;
	while (L->next != NULL) {
		if (cnt != L->num) {
			return 1;
		}
		cnt ++;
		L = L->next;
	}
	if (cnt == maxSequence) {
		return 0;
	} else {
		return 1;
	}
}

void printTab() {
	int i;
	IDMessage* IDMes;
	for (i=0; i<MAXMESSAGE; i++) {
		IDMes = &Tab[i];
		printf("numMessage : %d\nmaxSequence : %d\n", IDMes->numMessage, IDMes->maxSequence);
		printList(&(IDMes->list));
		printBuffer(IDMes->buffer, IDMes->maxSequence);
		printf("dans le buffer %s", IDMes->buffer);
	}
}



// Clean an element of Tab
void freeTabElement(IDMessage* messageAddress){
	messageAddress->numMessage = 0;
	messageAddress->maxSequence = 0;
	free(messageAddress->buffer);
	messageAddress->buffer = NULL;
}