
#ifndef STRUCTURE
#include "structure.h"
#define STRUCTURE
#endif

IDMessage* findMessage(int RequestNumMessage, int RequestMax);
void InitTab();
void addValue(listeChaine* l, int i);
int checkCompletionMessage(listeChaine* l, int maxSequence);
void printList(listeChaine* l);
void printBuffer(char* buf, int max);
void printTab();
void freeTabElement(IDMessage* messageAddress);