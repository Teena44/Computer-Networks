#include <stdio.h> //printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
 
// #define SERVER "127.0.0.1"
#define BUFLEN 512  //Max length of buffer
// #define PORT 8888   //The port on which to send data

typedef struct Packet{
	int seqnum;
	bool ack;
	bool drop;
	bool info;
	struct timeval recTime;
}Packet;

int BUFFER_SIZE;
Packet Buffer[50];
int bufstart,bufend;
Packet receiverWindow[100000];
int recStart,recEnd;
int windowSize, n;

char IP_Address[32];
uint16_t PORT;
double prob;

int max_packets;
int nextExpected;
pthread_t threads[4];
struct sockaddr_in si_other;
int s, slen;
char buf[BUFLEN];
char buff[512];
char message[BUFLEN];
Packet Acknowledge[100000];
int ackHead,ackTail;
bool debug;
pthread_mutex_t lock1,lock2,lock3,lock4;
bool stop;

void die(char *s)
{
    perror(s);
    exit(1);
}

bool Drop(double probability)
{
    int t = rand()%max_packets ;
    // printf("%d---\n",t );
    return (t) <  probability * (max_packets);
}

void * genBuffer(){
	bufstart = 0;
	bufend = BUFFER_SIZE;
	while(!stop){
		int index = 0;
		pthread_mutex_lock(&lock2);
		while((index < BUFFER_SIZE) && (Buffer[index].seqnum < receiverWindow[recStart].seqnum)){
			Buffer[index].seqnum = -1;
			index++;
		}
		pthread_mutex_unlock(&lock2);
	}
	// printf("OUTTA BUFFER\n");
}

void * debugMode(){
	int index = 0;
	for (int i = 0; i < (max_packets); i++)
	{
		while(!receiverWindow[i].info){}
			if(debug)
				printf("%d: Time received: %ld:%ld\n",receiverWindow[i].seqnum,receiverWindow[i].recTime.tv_sec,receiverWindow[i].recTime.tv_usec );
	}
}

void * Receive(){
    while(nextExpected != max_packets)
    {
        // printf("are you there\n");
        pthread_mutex_lock(&lock1);
        int delim;
        struct timeval recTime;
        memset(buf,0,sizeof(buf));
        if(delim = recvfrom(s, buf, BUFLEN, 0, NULL, NULL) == -1)
        {
        	// printf("HERE?\n");
            die("recvfrom()");
        }
        gettimeofday(&recTime,NULL);
        int seqnum = atoi(buf);
        bool flag = Drop(prob);
        int i;
        if(flag){
        	for(i=recStart;i<recEnd;i++)
        	{
        		if(seqnum == receiverWindow[i].seqnum)
        			receiverWindow[i].drop = true;
        	}
        	// printf("Dropped %d\n",i );
        }
        else{
        	if(seqnum == nextExpected){
        		receiverWindow[seqnum].ack = true;
        		Acknowledge[ackTail] = receiverWindow[seqnum];
        		ackTail++;
        		while(receiverWindow[recStart].ack){
        			recStart++;
        			nextExpected++;
        		}
        		// printf("acknow %d\n",seqnum );
        	}
        	else{
        		for(i=recStart;i<recEnd;i++){
        			if(seqnum == receiverWindow[i].seqnum){
        				int j;
        				for(j=bufstart;j<bufend;j++){
        					if(Buffer[j].seqnum == -1){
        						receiverWindow[i].ack = true;
        						Acknowledge[ackTail] = receiverWindow[i];
        						ackTail++;
        						Buffer[j].seqnum = receiverWindow[i].seqnum;
        						Buffer[j].ack = receiverWindow[i].ack;
        						Buffer[j].drop = receiverWindow[i].drop;
        						// printf("acknow %d\n",seqnum );
        						break;
        					}
        				}
        				if((j == bufend) && (Buffer[j].seqnum == -1)){
        					receiverWindow[i].drop = true;
        					// printf("buffer not empty for acknow %d\n",seqnum );
        				}
        				break;
        			}
        		}
    		}
    	}
    	receiverWindow[seqnum].recTime = recTime;
        // if(debug){
        //     if(receiverWindow[seqnum].drop)
        //         printf("%d:  Time Received: %ld:%ld\tPacket Dropped: True\n",seqnum,recTime.tv_sec,recTime.tv_usec );
        //     else
        //         printf("%d:  Time Received: %ld:%ld\tPacket Dropped: False\n",seqnum,recTime.tv_sec,recTime.tv_usec );
        // }
        pthread_mutex_unlock(&lock1);
    }
    // printf("OUTTA RECEIVE ackHead %d ackTail %d\n",ackHead,ackTail );
}

void * Transmit(){
    while(nextExpected != max_packets)
    {
        pthread_mutex_lock(&lock4);
        if(ackHead != ackTail){
            memset(buff,0,sizeof(buff));
            sprintf(buff,"%d",Acknowledge[ackHead].seqnum);
            if(sendto(s, buff, sizeof(buff) , 0 , (struct sockaddr *) &si_other, slen)==-1)
            // if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
            {
                die("sendto()");
            }
            // printf("Acknowledgement sent for %d \n",Acknowledge[ackHead].seqnum);
            receiverWindow[Acknowledge[ackHead].seqnum].info = true;
            ackHead++;
        }
        pthread_mutex_unlock(&lock4);
    }
    while(ackHead != ackTail){
        pthread_mutex_lock(&lock3);
    	memset(buff,0,sizeof(buff));
    	sprintf(buff,"%d",Acknowledge[ackHead].seqnum);
    	if(sendto(s, buff, sizeof(buff) , 0 , (struct sockaddr *) &si_other, slen)==-1)
        // if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
        {
            die("sendto()");
        }
        // printf("Acknowledgement sent for %d \n",Acknowledge[ackHead].seqnum);
        receiverWindow[Acknowledge[ackHead].seqnum].info = true;
        ackHead++;
        
        pthread_mutex_unlock(&lock3);
    }
    // printf("OUTTA TRANSMIT\n");
}

bool str_to_uint16(const char *str, uint16_t *res) {
    char *end;
    errno = 0;
    long val = strtol(str, &end, 10);
    if (errno || end == str || *end != '\0' || val < 0 || val >= 0x10000) {
        return false;
    }
    *res = (uint16_t)val;
    return true;
}

int main(int argc, char const *argv[])
{
    if(argc == 8)
        debug = true;
    else
        debug = false;

    strcpy(IP_Address,"127.0.0.1");
    char const* p = argv[1];
    str_to_uint16(p, &PORT);
    max_packets = atoi(argv[2]);
    n = atoi(argv[3]);
    windowSize = atoi(argv[4]);
    BUFFER_SIZE = atoi(argv[5]);
    prob = atof(argv[6]);

    // debug = false;
    // strcpy(IP_Address,"127.0.0.1");
    // char *p = "8888";
    // str_to_uint16(p,&PORT);
    // max_packets = 10;
    // BUFFER_SIZE = 10;
    // windowSize = 6;
    // prob = 0.0000001;
    // n = 10;

    slen=sizeof(si_other);
    int t,i;
 
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
 
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    si_other.sin_addr.s_addr = inet_addr(IP_Address);
     
    if (inet_aton(IP_Address , &si_other.sin_addr) == 0) 
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    recStart = 0;
    recEnd = windowSize;
    for(int j = 0;j<max_packets;j++){
    	receiverWindow[j].seqnum = j;
    	receiverWindow[j].ack = false;
    	receiverWindow[j].drop = false;
    	receiverWindow[j].info = false;
    }

    nextExpected = 0;
    // printf("%d %s\n", max_packets,argv[1]);

    strcpy(buff,"Connection inititated");
    if(sendto(s, buff, sizeof(buff) , 0 , (struct sockaddr *) &si_other, slen)==-1)
    {
        die("sendto()");
    }
    // printf("Connection inititated\n");

    t = pthread_create(&(threads[0]),NULL, genBuffer, NULL);
    t = pthread_create(&(threads[1]),NULL, Receive, NULL);
    t = pthread_create(&(threads[2]),NULL, Transmit, NULL);
    t = pthread_create(&(threads[3]),NULL, debugMode, NULL);
    ackHead = 0, ackTail = 0;

    // while(nextExpected != max_packets);
    while(ackHead != max_packets);
	stop = true;
	int index = 0;
	while((index < BUFFER_SIZE) && (Buffer[index].seqnum < receiverWindow[recStart].seqnum)){
		Buffer[index].seqnum = -1;
		index++;
	}
    for(i=0;i<3;i++)
        pthread_join(threads[i],NULL);
    // printf("HEREREEEEEEEEEEEEEEEEEEE\n");
    memset(buf,0,sizeof(buf));
        if(recvfrom(s, buf, BUFLEN, 0, NULL, NULL) == -1)
        {
            die("recvfrom()");
        }
        // printf("Still receiving %s\n",buf );
        // if(strcmp(buf,"DONE")==0)
        //     break;

 printf("<--------%s-------->\n","DONE");
    // close(s);
    return 0;
}