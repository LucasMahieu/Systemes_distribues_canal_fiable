/*
 * ======================================================================
 *         Author:  Lucas MAHIEU (), lucas.mahieu@grenoble-inp.org
 * ======================================================================
 */

#include "bug.h"
#include "structure.h"
#include <stdio.h>
int main()
{
	int i = 0;

	Packet p;
	Packet *q;
	p.numPacket = 12345;
	p.ack = 0;
	p.message[0] = 'Z';
	p.message[1] = 'M';
	p.message[2] = 'J';
	p.message[3] = 'R';
	p.message[4] = '\0';

	char buff[MAX_PKT_LEN];
	char duff[MAX_PKT_LEN];
	serialize_packet(buff, &p);

	for(i = 0; i < 10; i++) {
		printf("buff[%d]=%u\n",i,buff[i]);
	}

	q = deserialize_packet(buff, &p);

	printf("p.num=%u, p.ack=%u, p.message=%s",p.numPacket,p.ack,p.message);
	printf("q.num=%u, q.ack=%u, q.message=%s",q->numPacket,q->ack,q->message);

}
