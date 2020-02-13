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
#define BUFLEN 1024  //Max length of buffer
// #define PORT 8888   //The port on which to send data
 
char IP_Address[32];
uint16_t PORT;
double prob;

int max_packets;
int nextExpected;
pthread_t threads[2];
struct sockaddr_in si_other;
int s, slen;
char buf[BUFLEN];
char buff[512];
char message[BUFLEN];
int Acknowledge[100000];
bool drop[10000];
int ackHead,ackTail;
bool debug;
pthread_mutex_t lock1,lock2,lock3;

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
            die("recvfrom()");
        }
        // if(delim <0){
        //         printf("---> %s %d",strerror(errno),errno);
        // }
        gettimeofday(&recTime,NULL);
        // printf("I have sent ---> %s %d\n",strerror(errno),errno);
        // printf("Received data %s\n",buf );
        int seqnum = atoi(buf);
        bool flag = Drop(prob);
        if(seqnum == nextExpected){  
            if(!flag){
                Acknowledge[ackTail] = seqnum;
                ackTail++;
                nextExpected++;
                drop[seqnum] = false;
            }
            else{
                // printf("DROPPPPPPPEEEEEEEEEEEEDDDDDDDDD %d\n", seqnum);
                drop[seqnum] = true;
            }
        }
        if(debug){
            if(drop[seqnum])
                printf("%d:   Time Received: %ld:%ld\tPacket Dropped: True\n",seqnum,recTime.tv_sec,recTime.tv_usec );
            else
                printf("%d:   Time Received: %ld:%ld\tPacket Dropped: False\n",seqnum,recTime.tv_sec,recTime.tv_usec );
        }
        pthread_mutex_unlock(&lock1);
    }
}

void * Transmit(){
    while(nextExpected != max_packets)
    {
        pthread_mutex_lock(&lock2);
        if(ackHead != ackTail){
            memset(buff,0,sizeof(buff));
            sprintf(buff,"%d",Acknowledge[ackHead]);
            if(sendto(s, buff, sizeof(buff) , 0 , (struct sockaddr *) &si_other, slen)==-1)
            // if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
            {
                die("sendto()");
            }
            // printf("Acknowledgement sent for %d \n",Acknowledge[ackHead]);
            ackHead++;
        }
        pthread_mutex_unlock(&lock2);
    }
    while(ackHead != ackTail){
        pthread_mutex_lock(&lock3);
        memset(buff,0,sizeof(buff));
        sprintf(buff,"%d",Acknowledge[ackHead]);
        if(sendto(s, buff, sizeof(buff) , 0 , (struct sockaddr *) &si_other, slen)==-1)
        // if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
        {
            die("sendto()");
        }
        // printf("Acknowledgement sent for %d \n",Acknowledge[ackHead]);
        ackHead++;
        pthread_mutex_unlock(&lock3);
    }
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
    if(argc == 5)
        debug = true;
    else
        debug = false;

    strcpy(IP_Address,"127.0.0.1");
    char const* p = argv[1];
    str_to_uint16(p, &PORT);
    max_packets = atoi(argv[2]);
    prob = atof(argv[3]);

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

    nextExpected = 0;
    // printf("%d %s\n", max_packets,argv[1]);

    strcpy(buff,"Connection inititated");
    if(sendto(s, buff, sizeof(buff) , 0 , (struct sockaddr *) &si_other, slen)==-1)
    {
        die("sendto()");
    }
    // printf("Connection inititated\n");

    t = pthread_create(&(threads[0]),NULL, Receive, NULL);
    t = pthread_create(&(threads[1]),NULL, Transmit, NULL);
    ackHead = 0, ackTail = 0;

    // while(nextExpected != max_packets);
    for(i=0;i<2;i++)
        pthread_join(threads[i],NULL);
    while(1){
    memset(buf,0,sizeof(buf));
        if(recvfrom(s, buf, BUFLEN, 0, NULL, NULL) == -1)
        {
            die("recvfrom()");
        }
        // printf("Still receiving %s\n",buf );
        if(strcmp(buf,"DONE")==0)
            break;
    }

 printf("<--------%s-------->\n",buf);
    // close(s);
    return 0;
}