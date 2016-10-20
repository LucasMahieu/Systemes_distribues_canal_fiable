#include "structure.h"

/*
Find the message to write the data in. If the message does not exist yes, create it.
Usually returns the address of the structure where the message is to store.
Else, return null.
*/
IDMessage* findMessage(int RequestNumMessage, int RequestMax) {
	int i;
	IDMessage* freeElement;	// pointer to a void element in case the message does not exist yet
	int freeIsFound = 0;	// bool to know if we have already found a void element
	for (i=0; i<MAXMESSAGE; i++) {
		// Trying to find the message entry
		if (Tab[i].numMessage == RequestNumMessage){
			return &Tab[i];
		}

		// Trying to find a void entry
		if (!freeIsFound){
			if (Tab[i].numMessage == 0){
				freeElement = &Tab[i];
				freeIsFound = 1;
			}
		}
	}
	// printf("freeIsFound %d \n", freeIsFound);
	if (freeIsFound) {
		freeElement->numMessage = RequestNumMessage;
		freeElement->maxSequence = RequestMax;
		freeElement->buffer = (char*) calloc(RequestMax * MAXMESSAGE, sizeof(char));
		// printf("freeElement : %p, buffer  EN %p\n", freeElement, freeElement->buffer);
		if (freeElement->buffer == NULL) {
			perror("calloc invalided");
		}
		return freeElement;
	}
	return NULL;
}


// Just init the Tab with default values. 0 is not an element, it is a default value.
void InitTab(){
	int i;
	for (i=1; i<MAXMESSAGE; i++){
		Tab[i].numMessage = 0;
		Tab[i].maxSequence = 0;
		Tab[i].list.num = 0;
		Tab[i].list.next = NULL;
		Tab[i].buffer = NULL;
	}
}

// Add a value to the listeChaine
void addValue(listeChaine* l, int i){
	
	listeChaine* newElement = malloc(sizeof(listeChaine));	//------------------------------------------------------------------------ attention null
	newElement->num = i;
	listeChaine* L = l;

	while (L->num < i && L->next != NULL){
		// printf("ONESTRENTRE\n");
		L = L->next;
	}
	newElement->next = L->next;
	L->next = newElement;
	// printf("POINT : %p, %p, %p, %p, %p", l->next, L, L->next, newElement, newElement->next);
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

// Print all the elements in a typical list
void printList(listeChaine* l){
	printf("Dans la liste chainee : ");
	while (l != NULL) {
		printf("%d,", l->num);
		l = l->next;
	}
	printf("\n");
}

// Print all the strig stored in a buffer of an IDMessage
void printBuffer(char* buf, int max){
	int i = 0;
	printf("Contenu du buffer en differentes positions : \n");
	for (i=0; i<max; i++) {
		printf("i= %d : %s\n", i, buf+i*MAX_BUFLEN);
	}
}


// Print all the values in the Tab
void printTab() {
	int i;
	IDMessage* IDMes;
	for (i=0; i<MAXMESSAGE; i++) {
		IDMes = &Tab[i];
		printf("numMessage : %d\nmaxSequence : %d\n", IDMes->numMessage, IDMes->maxSequence);
		printList(&(IDMes->list));
		printBuffer(IDMes->buffer, IDMes->maxSequence);
		printf("\n");
	}
}





// Clean an element of Tab
void freeTabElement(IDMessage* messageAddress){
	messageAddress->numMessage = 0;
	messageAddress->maxSequence = 0;
	free(messageAddress->buffer);
	messageAddress->buffer = NULL;
}
