#include <stdio.h> //printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>
 
// #define PORT 8888   //The port on which to listen for incoming data

struct sockaddr_in si_me, si_other;
int soc,slen;

typedef struct Packet{
    char* data;
    int seqnum;
    bool ack;
    clock_t timer;
    clock_t start;
    clock_t end;
    int attempts;
    struct timeval genTime;
}Packet;

void* SetTimer(void*);

pthread_t threads[100000];
pthread_mutex_t lock1,lock2,lock3,lock4,lock5,lock6,lock7;
int windowHead, windowTail;

Packet senderWindow[100000];

Packet bufferPackets[100000];
char buf[1024];
int bufend;
int bufstart;
int lengthPacket,ratePacketGen,maxPackets,windowSize,max_buffer_size;
int seqnum;
char IP_Address[32];
uint16_t PORT;
clock_t tim,bim;
clock_t RTT;
int Transmission,Acknowledged;
bool debug;
int num,denom;
 
int current_packet;

int thread;
void die(char *s)
{
    perror(s);
    exit(1);
}

void *CalculateRTT(){
    int index = 0;
    clock_t num = 0, denom = 0;
    while(index != maxPackets && index >= 10){
        if(index < windowHead){
            num = num + senderWindow[index].end - senderWindow[index].start;
            denom = denom++;
            RTT = (clock_t) 2*(num/denom) ;
            index++;
        }
    }
    if(index < 10)
        RTT = CLOCKS_PER_SEC/10;
}

void *genPackets(){
    bufstart = 0;
    bufend = 0;
    int numPackets = 0;
    // printf("here here\n");
    while(bufend < maxPackets){//(senderWindow[windowHead].seqnum != maxPackets){
        pthread_mutex_lock(&lock6);
        numPackets = 0;        
        clock_t start = clock();
        while((clock()-start)*1000/CLOCKS_PER_SEC < 1000 && bufend < maxPackets){
            if(bufend-bufstart < max_buffer_size && bufend < maxPackets && numPackets < ratePacketGen){
                bufferPackets[bufend].data = (char*)malloc(lengthPacket*(sizeof(char)));
                sprintf(bufferPackets[bufend].data,"%d",seqnum);
                bufferPackets[bufend].seqnum = seqnum;
                bufferPackets[bufend].ack = false;
                // bufferPackets[bufend].timer = 5000000;
                bufferPackets[bufend].attempts = 0;
                seqnum++;
                bufend++;
                numPackets++;
            }
        }
        // index++;
        pthread_mutex_unlock(&lock6);
        // printf("there\n");
    }
    // printf("OUT OF GEN PACKETS\n");
    // pthread_exit(0);    
}

void * setWindow(){
    windowHead = 0;
    windowTail = 0;
    int p = 0;
    while(windowTail < maxPackets){//senderWindow[windowTail].seqnum < maxPackets){
        pthread_mutex_lock(&lock4);
        if((windowTail - windowHead < windowSize) && bufstart !=  bufend){  //fill window unless its not full or there are more packets in the buffer
            senderWindow[windowTail].data = (char*)malloc(lengthPacket*(sizeof(char)));
            strcpy(senderWindow[windowTail].data,bufferPackets[bufstart].data);
            senderWindow[windowTail].seqnum = bufferPackets[bufstart].seqnum;
            senderWindow[windowTail].ack = bufferPackets[bufstart].ack;
            senderWindow[windowTail].timer = bufferPackets[bufstart].timer;  
            senderWindow[windowTail].attempts = bufferPackets[bufstart].attempts;
            gettimeofday(&senderWindow[windowTail].genTime,NULL);
            // printf("window %d \n",windowTail);      
            bufstart++;
            windowTail++;
        }
        pthread_mutex_unlock(&lock4);
        // printf("there\n");
    }
    // printf("OUT OF SET WINDOW\n");
    // pthread_exit(0); 
}
void * Receiving(){
    int index = 0;
    while(!senderWindow[maxPackets-1].ack ){//&& windowTail < maxPackets){
        pthread_mutex_lock(&lock1);
        memset(buf,0,sizeof(buf));
        // if(index != 0){
            if(recvfrom(soc, buf, lengthPacket, 0, (struct sockaddr *) &si_other, &slen) == -1)
            {
                die("recvfrom()");
            }
            int sn = atoi(buf);
            // printf(" Received %s \n",buf);
            senderWindow[sn].end = clock();
            // if(sn == 0){
            //     bim = clock();
            //     printf("time is --->>>>>> %ld and %ld and %ld\n",bim - tim, 1000*(bim-tim)/CLOCKS_PER_SEC, (senderWindow[windowHead].timer)*1000/CLOCKS_PER_SEC );
            // }

            while(senderWindow[windowHead].seqnum != sn)
            {}
                senderWindow[sn].ack = true;
                bufferPackets[sn].ack = true;
                Acknowledged++;
                num = num + 1000*(senderWindow[sn].end-senderWindow[sn].start)/CLOCKS_PER_SEC;
                denom++;
                if(debug)
                    printf("%d:   Time Generated: %lu:%lu\tRTT: %lu\tAverage RTT:   %lu\t# of attempts: %d\n",sn,senderWindow[sn].genTime.tv_sec,senderWindow[sn].genTime.tv_usec, 1000*(senderWindow[sn].end-senderWindow[sn].start)/CLOCKS_PER_SEC,(clock_t)num/denom,senderWindow[sn].attempts );
            // }
        pthread_mutex_unlock(&lock1);
    }
    // printf("OUT OF RECEIVING\n");
    // pthread_exit(0); 
}

void AckReceiver(Packet P){
    pthread_mutex_lock(&lock5);
    if(senderWindow[windowHead].seqnum == P.seqnum){
        windowHead++;
    }
    pthread_mutex_unlock(&lock5);
    // pthread_exit(0); 
}

void NoAckReceiver(Packet P){
    int t, recv_len;
    pthread_mutex_lock(&lock3);
    if (sendto(soc,P.data, lengthPacket, 0, (struct sockaddr*) &si_other, slen) == -1)
    {
    	printf("Heraa2?\n");
        die("sendto()");
    }
    Transmission++;
    senderWindow[P.seqnum].attempts++;
    // printf("Sending again %d at %d\n",P.seqnum,thread );
    // printf("--------------------------------- 11111 Do you come here? %d\n",P.seqnum); 
    
    // printf("--------------------------------- 22222 Do you come here? %d\n",P.seqnum); 
    t = pthread_create(&(threads[thread++]),NULL, SetTimer, &senderWindow[P.seqnum]);
    // printf("---------------------------------Do you come here? %d\n",P.seqnum); 
    pthread_mutex_unlock(&lock3);
    // pthread_exit(0);
}

void * SetTimer(void* p){
    Packet* P = (Packet*) p;
    int sn = P->seqnum;
    pthread_mutex_lock(&lock7);
    clock_t currTime;
    // printf("%lu %ld time\n",clock()-currTime,senderWindow[sn].timer );
    currTime = clock();
    while(clock()-currTime < RTT){
        if(senderWindow[sn].ack == true)
            break;
    }
    clock_t end = clock()-currTime;
    // pthread_mutex_unlock(&lock7);
    // printf("11111111111----- %d and time is %lu\n",sn,end);
    if(senderWindow[sn].ack == true){
        // printf("2222222222 %d and time is: %ld\n",sn,end);
        AckReceiver(senderWindow[sn]);
    }
    else{
        // printf("33333333333 %d time is: %ld\n",sn,end);
        NoAckReceiver(senderWindow[sn]);
    }
    // /////////pthread_exit(0);
    pthread_mutex_unlock(&lock7);
}

void *Transmit(){
    int recv_len ,t;
    while(current_packet < maxPackets){
        pthread_mutex_lock(&lock2);
        if(current_packet < windowTail){
            char buff[512];
            sprintf(buff,"%d",current_packet);
            // scanf("%s",buff);
            int delim;
            if(current_packet == 0)
                tim = clock();
            if ((delim = sendto(soc,buff, lengthPacket, 0, (struct sockaddr*) &si_other, slen)) == -1)
            {
                die("sendto()");
            }
            Transmission++;
            senderWindow[current_packet].attempts++;
            senderWindow[current_packet].start = clock();
            bufferPackets[windowHead].start = senderWindow[windowHead].start ;
            // if(delim <0){
            //     printf("---> %s %d",strerror(errno),errno);
            // }
            // printf("Sending %d at %d \n", current_packet,thread);
            current_packet++;
            t = pthread_create(&(threads[thread++]),NULL, SetTimer, &senderWindow[current_packet-1]);
        }
        pthread_mutex_unlock(&lock2);
        // printf("here\n");
    }
    // printf("--------------OUT OF TRANSMIT\n");
    // pthread_exit(0);
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
    // struct sockaddr_in si_me, si_other;
    strcpy(IP_Address,argv[1]);
    char const* p = argv[2];
    str_to_uint16(p, &PORT);
    lengthPacket = atoi(argv[3]);
    ratePacketGen = atoi(argv[4]);
    maxPackets = atoi(argv[5]);
    windowSize = atoi(argv[6]);
    max_buffer_size = atoi(argv[7]);

    if(argc == 9)
        debug = true;
    else
        debug = false;

    // strcpy(IP_Address,"127.0.0.1");
    // char const* p = "8888";
    // str_to_uint16(p, &PORT);
    // lengthPacket = 1024;
    // ratePacketGen = 10;
    // maxPackets = 50;
    // windowSize = 3;
    // max_buffer_size = 10;

    Transmission = 0;
    Acknowledged = 0;
    num = 0;

    int i, recv_len;
    slen = sizeof(si_other);
     
    //create a UDP socket
    if ((soc =socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
    memset((char *) &si_other, 0, sizeof(si_other));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = inet_addr(IP_Address);
     
    //bind socket to port
    if( bind(soc , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }
     
    seqnum = 0;
    thread = 0;

    int t;
    windowHead = 0;
    
    t = pthread_create(&(threads[thread++]),NULL,&genPackets, NULL); 
       // create a separate thread to generate new packets and store in the buffer   
    t = pthread_create(&(threads[thread++]),NULL,&setWindow, NULL);    //creating a sender window and allocating the first windowSize windows

    current_packet = windowHead;

    if(recvfrom(soc, buf, lengthPacket, 0, (struct sockaddr *) &si_other, &slen) == -1)
    {
        die("recvfrom()");
    }

    t = pthread_create(&(threads[thread++]),NULL,&CalculateRTT, NULL);

    t = pthread_create(&(threads[thread++]),NULL,&Transmit, NULL);

    t = pthread_create(&(threads[thread++]),NULL,&Receiving, NULL);

    // while(1);
    for(i=0;i<thread;i++)
        pthread_join(threads[i],NULL);

    // memset(buff,0,sizeof(buff));
    // strcpy(buff,"DONE!!");
    if (sendto(soc,"DONE", lengthPacket, 0, (struct sockaddr*) &si_other, slen) == -1)
    {
        die("sendto()");
    }
    // sleep(10);
    // while(bufend < maxPackets){}//senderWindow[windowHead].seqnum != maxPackets){}//printf("NOT YET\n");}
        printf("PACKET_GEN_RATE: %d\nPACKET_LENGTH: %d\nRETRANS RATIO: %d/%d\nAVERAGE RTT: %ld\n",ratePacketGen,lengthPacket,Transmission,Acknowledged,(clock_t)num/denom);
        // exit(1);

    
    return 0;
}

// ./server 127.0.0.1 8888 1024 10 50 3 10

// while(1)
//     {
//         printf("Waiting for data...");
//         fflush(stdout);
         
//         //try to receive some data, this is a blocking call
//         if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
//         {
//             die("recvfrom()");
//         }
         
//         //print details of the client/peer and the data received
//         printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
//         printf("Data: %s\n" , buf);
         
//         //now reply the client with the same data
//         // puts("Type your message ");
//         // gets(buf);
//         // if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
//         // {
//         //     die("sendto()");
//         // }
//     }
 
//     close(s);