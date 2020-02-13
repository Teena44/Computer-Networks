#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>
#include <unistd.h>
#define MAXCHAR 1024
#define IP_Address "127.0.0.1"

int HELLO_INTERVAL, LSA_INTERVAL, SPF_INTERVAL;
pthread_t threads[100000];
int input[1024],thread,t,N;
int **Graph;
int NODE;
FILE *output;
char fname[20];
	
typedef struct Map{
	int Id;
	int seqnum;
	int LSAPacket[50];
}Map;

typedef struct OSPF_Router{
	int iden;
	uint16_t PORT;
	int* neighbour; 
	int *cost;
	uint16_t* ports;
	int neigh;
	struct sockaddr_in si_me;
	struct sockaddr_in si_other[50];
	int soc_me;
	int soc_other[50];
	int seqnum;
	Map LSAMap[1000];
	int mapPointer;
}OSPF_Router;

OSPF_Router Router;
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

void die(char *s)
{
    perror(s);
    exit(1);
}

void * sendHELLO(void* id){
	int slen;
	int iden = *((int*) id);
	char *buff,srcId[50];
	sprintf(srcId,"%d",iden);
	buff = (char*)malloc(sizeof(char)*1024);
	((int*) buff)[0] = 0;
	((int*) buff)[1] = iden;
	// strcpy(buff,"HELLO");
	// strcat(buff,"-");
	// strcat(buff,srcId);
	int s;

	while(1){
		sleep(HELLO_INTERVAL);
		int i;
		for(i=0;i<Router.neigh;i++){
		    
			if(sendto(Router.soc_other[i], buff, 8 , 0 , (struct sockaddr *) &Router.si_other[i], sizeof(Router.si_other[i]))==-1)
		    {
		    	// printf("%s sin_port: %hu\n",buff,Router.si_other.sin_port );
		    	printf("1\n");
		        die("sendto()");
		    }
		    // printf("%d\n",((int*)buff)[1] );

		}
	}
}

void * ClientHello(void *id){
	int iden = *((int*) id);
	char *buff;
	int i=0,slen;
	buff = (char*)malloc(sizeof(char)*16);
	while(1){
		for(i=0;i<Router.neigh;i++)
		{
			memset(buff,0,sizeof(buff));
			slen = sizeof(Router.si_other[i]);
		    if(recvfrom(Router.soc_other[i], buff,64, 0, (struct sockaddr *) &Router.si_other[i], &slen) == -1)
		    {
		        die("recvfrom()");
		    }

			if(((int*)buff)[0]==1){
				int j = ((int*) buff)[1];
				int cost = ((int*) buff)[3];
				for(int k = 0;k<Router.neigh;k++){
					if(Router.neighbour[k] == j){
						Router.cost[k] = cost;
						// printf("%d\n",Router.cost[k] );
						break;
					}
				}
				// printf("%d %d %d %d\n",((int*)buff)[0],((int*)buff)[1],((int*)buff)[2],((int*)buff)[3] );
			}
		}
	}
}

void * replyHELLOandLSA(void *id){

	int slen;
	int iden = *((int*) id);
	char *buf;
	buf = (char*)malloc(sizeof(char)*1024);
	while(1){

		memset(buf,0,sizeof(buf));
		slen = sizeof(Router.si_me);
		// printf("-----------%lu\n",sizeof(buf) );
		if(recvfrom(Router.soc_me, buf, 64, 0, (struct sockaddr *) &Router.si_me, &slen) == -1)
	    {
	        die("recvfrom()");
	    }
    	if(((int*)buf)[0]==1){
    		printf("AREYA\n");
    	}
		else if(((int*)buf)[0] == 0){
			char back[16];
			((int*) back)[0] = 1;
			((int*) back)[1] = iden;
			((int*) back)[2] = ((int*)buf)[1];
			for(int y = 2;y<input[1]*4+2;y=y+4){
				if(input[y] == ((int*)buf)[1] && input[y+1] == iden)
					((int*) back)[3] = rand()%(input[y+3]-input[y+2])+input[y+2];
				if(input[y+1] == ((int*)buf)[1] && input[y] == iden)
					((int*) back)[3] = rand()%(input[y+3]-input[y+2])+input[y+2];
			}
			// ((int*) back)[3] = rand()%(input[2+4*iden+3]-input[2+4*iden+2])+input[2+4*iden+3];

			// printf("%d %d\n",((int*)buf)[0],((int*)buf)[1]);
			// printf("%d %d %d %d\n",((int*) back)[0],((int*) back)[1],((int*) back)[2],((int*) back)[3]);

			if (sendto(Router.soc_me, back, 16, 0, (struct sockaddr*) &Router.si_me, slen) == -1)
	        {
	        	printf("2\n");
	            die("sendto()");
	        }
		}
		else if(((int*)buf)[0] == 2){
			bool flag = false,gen = false;
			int tempId, tempSeqnum;
			tempId = ((int*)buf)[1];
			tempSeqnum = ((int*)buf)[2];
			int newSeqnum = tempSeqnum+1;

			// if(((int*)buf)[1] == 4){
			// 	for(int y = 0;y< ((int*)buf)[3]+4;y++)
			// 		printf("%d ",((int*)buf)[y]);
			// 	printf("\n");
			// }
			for(int y = 0;y<Router.neigh;y++)
				if(Router.neighbour[y] == ((int*)buf)[1])
					gen = true;

			// printf("%d\n",Router.mapPointer );
			for(int k = 0;k<Router.mapPointer;k++)
				if(Router.LSAMap[k].Id == tempId){
					if(Router.LSAMap[k].seqnum < tempSeqnum){
						Router.LSAMap[k].LSAPacket[0] = ((int*)buf)[0];
						Router.LSAMap[k].LSAPacket[1] = ((int*)buf)[1];
						Router.LSAMap[k].LSAPacket[2] = ((int*)buf)[2];
						Router.LSAMap[k].LSAPacket[3] = ((int*)buf)[3];
						for(int q = 4;q < 4+((int*)buf)[3]; q++)
							Router.LSAMap[k].LSAPacket[q] = ((int*)buf)[q];

						Router.LSAMap[k].seqnum = tempSeqnum;
						// ((int*)buf)[2] = newSeqnum;

						// printf("%d %d %d %d %d %d %d %d\n",((int*) buf)[0],((int*) buf)[1],((int*) buf)[2],((int*) buf)[3],((int*) buf)[4],((int*) buf)[5],((int*) buf)[6],((int*) buf)[7]);
						for(int j = 0; j<Router.neigh;j++){
							if(Router.neighbour[j] != tempId){
							    
								if(sendto(Router.soc_other[j], buf, 64 , 0 , (struct sockaddr *) &Router.si_other[j], slen)==-1)
							    {
							    	// printf("%s sin_port: %hu\n",buf,Router.si_other.sin_port );
							    	printf("3\n");
							        die("sendto()");
							    }

							}
						}
					}
					flag = true;
					break;
				}
			
			if(!flag){
				Router.LSAMap[Router.mapPointer].Id = tempId;
				Router.LSAMap[Router.mapPointer].seqnum = tempSeqnum;
				Router.LSAMap[Router.mapPointer].LSAPacket[0] = ((int*)buf)[0];
				Router.LSAMap[Router.mapPointer].LSAPacket[1] = ((int*)buf)[1];
				Router.LSAMap[Router.mapPointer].LSAPacket[2] = ((int*)buf)[2];
				Router.LSAMap[Router.mapPointer].LSAPacket[3] = ((int*)buf)[3];
				for(int q = 4;q < 4+((int*)buf)[3]; q++)
					Router.LSAMap[Router.mapPointer].LSAPacket[q] = ((int*)buf)[q];

								//store the entry only if seqnum id greater then to the old one if present
				Router.mapPointer++;
				flag = false;
				// ((int*)buf)[2] = newSeqnum;206
				// for(int y = 0;y< ((int*)Router.LSAMap[Router.mapPointer-1].LSAPacket)[3]+4;y++)
				// 	printf("%d ",((int*)Router.LSAMap[Router.mapPointer-1].LSAPacket)[y]);
				// printf("\n");

				for(int k = 0; k<Router.neigh;k++){
					if(Router.neighbour[k] != tempId){
					    
						if(sendto(Router.soc_other[k], buf, 64 , 0 , (struct sockaddr *) &Router.si_other[k], slen)==-1)
					    {
					    	printf("4\n");
					        die("sendto()");
					    }
					}
				}
			}
			flag = false;

		}

	}
}

void * sendLSAPackets(void *id){
	int iden = *((int*) id);
	int slen;

	while(1){
		sleep(LSA_INTERVAL);
		int i;

		char *LSAPacket,temp[10];
		LSAPacket = (char*)malloc(sizeof(char)*1024);
		
		((int*)LSAPacket)[0] = 2;
		((int*)LSAPacket)[1] = iden;
		((int*)LSAPacket)[2] = Router.seqnum++;
		((int*)LSAPacket)[3] = 2*Router.neigh;
		
		int g = 4;
		// printf("%d %d %d %d ",((int*)LSAPacket)[0] ,((int*)LSAPacket)[1],((int*)LSAPacket)[2],((int*)LSAPacket)[3]);
		for(int k = 0;k<Router.neigh;k++){
			((int*)LSAPacket)[g++] = Router.neighbour[k]; 
			((int*)LSAPacket)[g++] = Router.cost[k];
			// printf("%d %d\n",((int*)LSAPacket)[g-2],((int*)LSAPacket)[g-1] );
		}

		// for(int y=0;y<((int*)LSAPacket)[3]+4;y++)
		// 	printf("%d ",((int*)LSAPacket)[y] );
		// printf("\n");
		// printf("WBTHIS? %s\n",LSAPacket );
		for(i=0;i<Router.neigh;i++){
			slen = sizeof(Router.si_other[i]);		    
			if(sendto(Router.soc_other[i], LSAPacket, 64 , 0 , (struct sockaddr *) &Router.si_other[i], slen)==-1)
		    {
		    	// printf("%s sin_port: %hu\n",LSAPacket,Router.si_other.sin_port );
		    	printf("5 %lu %s\n",strlen(LSAPacket),LSAPacket);
		        die("sendto()");
		    }
		}
	}
}

void * shortestPath(){

	while(1)
	{
		sleep(SPF_INTERVAL);
		int iden = NODE;
		// printf("-----------------------------------------------%d\n",iden );
		int i,j,x,y,xycost;
		char* token;
		int parent[N],path = 0;
	
		for(int k = 0;k< Router.neigh;k++){
			Graph[iden][Router.neighbour[k]] = Router.cost[k];
			// printf("%d ",Router.neighbour[k] );
		}
		// printf("\n");
	
		for(int k = 0;k< Router.mapPointer;k++){
			// token = strtok(Router.LSAMap[k].LSAPacket, "-");
			// token = strtok(NULL,"-");
			x = Router.LSAMap[k].LSAPacket[1];
			int m = Router.LSAMap[k].LSAPacket[3];
	
			for(i=0;i<m;i=i+2){
				y = Router.LSAMap[k].LSAPacket[i+4];
				xycost = Router.LSAMap[k].LSAPacket[i+5];
				// printf("%d\n",xycost );
				Graph[x][y] = xycost;
			}
		}

		// for(i=0;i<N;i++){
		// 	for(j=0;j<N;j++){
		// 		printf("%d ",Graph[i][j] );
		// 	}
		// 	printf("\n");
		// }
	
		int cost[N];                  		  
	    bool topology[N]; 
	
	    for (int i = 0; i < N; i++){
	    	cost[i] = INT_MAX;
	        topology[i] = false;
	    }
	
	    cost[iden] = 0;
	
		for (i = 0; i < N; i++)
		{
			int min = INT_MAX, minIndex;
	   		for (int v = 0; v < N; v++)
	     		if (topology[v] == false && cost[v] <= min)
	         		min = cost[v], minIndex = v;
	
		   	topology[minIndex] = true;
			int v;
		   	for (v = 0; v < N; v++)
		    	if (!topology[v] && Graph[minIndex][v] != -1 && cost[minIndex] != INT_MAX && cost[minIndex]+Graph[minIndex][v] < cost[v]){
		        	cost[v] = cost[minIndex] + Graph[minIndex][v];
		        	parent[v] = minIndex;
		    	}
		}
	
		parent[NODE] = -1;

		char Data[1000];
		char temp[20];
		strcpy(Data,"Routing Table for Node No. ");
		sprintf(temp,"%d",iden);
		strcat(Data,temp);
		strcat(Data," at Time ");
		time_t clk = time(NULL);
	    strcat(Data,ctime(&clk));
	
		output = fopen(fname,"w+");
	    fprintf(output,"%s\n",Data );

	    fprintf(output,"Dest   cost   Path\n");

	    for(int v = 0; v< N;v++){
	    	fprintf(output,"%d\t%d\t",v, cost[v] );
	    	int w = v;
	    	if(v == NODE || parent[w] < 0){
	    		fprintf(output,"-\n");

	    		continue;
	    	}
	    	fprintf(output,"%d<--",v );

	  		while(parent[w] != NODE){
	  			fprintf(output,"%d<--",parent[w] );
	  			w = parent[w];
	  		}
	    	fprintf(output,"%d\n",NODE);
	    }
	    fclose(output);
	}

	// int x1=1;
	// int y1 = 2;
	// printf("%d\n",x1+y1 );
}


int main(int argc, char const *argv[])
{
	FILE *fp;
	char inname[20];

	NODE = atoi(argv[1]);
	strcpy(inname, argv[2]);
	strcpy(fname,argv[3]);
	strcat(fname,".txt");
	HELLO_INTERVAL = atoi(argv[4]);
	LSA_INTERVAL = atoi(argv[5]);
	SPF_INTERVAL = atoi(argv[6]);
	
	// printf("%d %d\n",argc,NODE );
	
	char line[MAXCHAR];
	char* token;
	int i = 0;
	thread = 0;
	fp = fopen(inname,"r+");
	while (fgets(line, MAXCHAR, fp) != NULL){
		token = strtok(line, " ");
	   	while( token != NULL ) {
	   		input[i] = atoi(token);
	      	token = strtok(NULL, " ");
	      	i++;
	   	}
	}
	// Routers = (Router*)malloc(sizeof(Router)*input[0]);
	N = input[0];
	i = 0;
	// while(i<input[0]){
		// printf("%d\n",input[i] );
		Router.iden = NODE;
		char port[10];
		sprintf(port,"%d",10000 + NODE);
		str_to_uint16(port, &Router.PORT);
		// printf("%d %hu-----\n",i,Router.PORT );
		Router.neigh = 0;
		Router.seqnum = 0;
		Router.mapPointer = 0;

		//----------setting up server at 10000+NODE------------------ 
		int slen = sizeof(Router.si_other);

		Router.soc_me =socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	    if (Router.soc_me == -1)
	    {
	        die("socket");
	    }
	    memset((char *) &Router.si_me, 0, sizeof(Router.si_me));
	    
	    Router.si_me.sin_family = AF_INET;
	    Router.si_me.sin_port = htons(Router.PORT);
	    Router.si_me.sin_addr.s_addr = inet_addr("127.0.0.1");
	     
	    //bind socket to port
	    if( bind(Router.soc_me , (struct sockaddr*)&Router.si_me, sizeof(Router.si_me) ) == -1)
	    {
	        die("bind");
	    }
	    //----------done-----------    
        
   	Router.neighbour = (int*)malloc(sizeof(int)*input[0]);

	for(i=2;i<input[1]*4;i = i+4){
		if(input[i]==NODE)
			Router.neighbour[Router.neigh++] = input[i+1];
		if(input[i+1]==NODE)
			Router.neighbour[Router.neigh++] = input[i];
	}

	Router.ports = (uint16_t*)malloc(sizeof(uint16_t)*Router.neigh);
	for(int j = 0;j<Router.neigh;j++){
		Router.soc_other[j] =socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if ( Router.soc_other[j] == -1)
	    {
	        die("socket");
	    }
	    memset(port,0,sizeof(port));
		sprintf(port,"%d",10000 + Router.neighbour[j]);
		str_to_uint16(port, &Router.ports[j]);

	    memset((char *) &Router.si_other[j], 0, sizeof(Router.si_other[j]));
	    Router.si_other[j].sin_family = AF_INET;
	    Router.si_other[j].sin_port = htons(Router.ports[j]);
	    Router.si_other[j].sin_addr.s_addr = inet_addr(IP_Address);
	     
	    if (inet_aton(IP_Address , &Router.si_other[j].sin_addr) == 0) 
	    {
	        fprintf(stderr, "inet_aton() failed\n");
	        exit(1);
	    }
	}

	Router.cost = (int*)malloc(Router.neigh*sizeof(int));
	for(i=0;i<Router.neigh;i++)
		Router.cost[i] = INT_MAX;
	
	t = pthread_create(&(threads[thread++]),NULL,&sendHELLO, &NODE);

	t = pthread_create(&(threads[thread++]),NULL,&ClientHello, &NODE);

  	t = pthread_create(&(threads[thread++]),NULL,&replyHELLOandLSA, &NODE);


	t = pthread_create(&(threads[thread++]),NULL,&sendLSAPackets, &NODE);

    clock_t start = clock();
		while((clock()-start)*1000/CLOCKS_PER_SEC < 1000*10){}

	Graph = (int**)malloc(N*sizeof(int*));

	for(i=0;i<N;i++){
		Graph[i] = (int*)malloc(N*sizeof(int));

		for(int j=0;j<N;j++)
			Graph[i][j] = -1;
	}

	// printf("==================================================== %d\n",NODE );

	t = pthread_create(&(threads[thread++]),NULL,&shortestPath, NULL);

	// while(1){
	// 	sleep(35);
	// 	for(int h = 0;h<Router.mapPointer;h++){
	// 		for(int y = 0;y< Router.LSAMap[h].LSAPacket[3]+4;y++)
	// 				printf("%d ",((int*) Router.LSAMap[h].LSAPacket)[y]);
	// 			printf("\n");
	// 	}
	// 	printf("\n\n");
	// }
   

    fclose(fp);

    for(i=0;i<thread;i++)
        pthread_join(threads[i],NULL);
    return 0;
}