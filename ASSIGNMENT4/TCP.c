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
#include <math.h>

double Ki, Km, Kn, Kf, prob, CW, thresh;
int T,cwstart,cwend, MSS, RWS;
int* segments;

bool Drop()
{
    // int t = rand()%T ;
    // // printf("%d---\n",t );
    // return (t) <  prob * (T);
    double t = ((double)(rand()%T)) / T;
    if (t <= prob)
    	return true;
	else return false;
}

double min(double a, double b){
	if(a <= b)
		return a;
	return b;
}

double max(double a, double b){
	if(a >= b)
		return a;
	return b;
}

int main(int argc, char const *argv[]){

	Ki = atof(argv[1]);
	Km = atof(argv[2]);
	Kn = atof(argv[3]);
	Kf = atof(argv[4]);
	prob = atof(argv[5]);
	T = atoi(argv[6]);

	RWS = 1024;
	MSS = 1;
	FILE *output;
	output = fopen("output.dat","w+");

	int round = 0,j = 0;
	segments = (int*)malloc(sizeof(int)*T);
	memset(segments, 0, sizeof(segments));

	CW = Ki*MSS;
	thresh = (0.5)*CW;
	cwstart = 0;
	cwend = cwstart + (int)(ceil(CW));
	while(cwstart < T){
		double cwtemp = CW;	
		int cwseg = cwstart;
		for(int i = cwstart; i< cwend; i++){
			if(!Drop()){
				if(CW < thresh)
					CW = min(CW + Km*MSS, RWS);
				else
					CW = min(CW + Kn*MSS*(MSS/CW), RWS);
				segments[i] = 1;
				fprintf(output,"%d %f\n",j++,CW );
				// printf("%d %f\n",j++,CW );
				// fprintf(output,"%f,%d\n",CW,j++ );
			}
			else{
				thresh = CW/2;
				CW = max(1, Kf*CW);
				
				fprintf(output,"%d %f\n",j++,CW );
				// printf("%d %f\n",j++,CW );
				// fprintf(output,"%f,%d\n",CW,j++ );
			}
		}
		while(segments[cwstart] == 1)
					cwstart++;

		cwend = cwstart + CW;
		
		round++;
	}
	return 0;
}