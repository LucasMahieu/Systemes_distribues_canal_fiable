typedef struct listeChaine {
	int num;
	struct listeChaine* next;
} listeChaine;

// Structure for the Tab to get informations about the message stored in this element
typedef struct IDMessage {
	int numMessage; 	// Number of the message 
	int maxSequence;	// The message is composed of max messages to assemble
	listeChaine list; 	// List of already received package numbers
	char* buffer;		// Where the message is stored
} IDMessage;

// En tete of a udp package
typedef struct enTete {
	int numMessage;		// Number of the message 
	int numSequence; 	// number of the package in the message
	int maxSequence; 	//	The message is composed of maxSequence messages to assemble
} enTete;