#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

typedef struct Mail{
    char* user[1024];
    int inbox[1024];
    int inbox_pointer[1024];
    int flag[1024];
    int pointer;
    int currentuser;
}Mail;

Mail M;
char* root;

char* listusers(){
    char* ret;
    if(M.pointer == 0){
        ret = "No users currently";
        return ret;
    }

    int i;
    char users[1024];
    strcpy(users,M.user[0]);
    for(i=1;i<M.pointer;i++){
        strcat(users," ");
        strcat(users,M.user[i]);
    }
    ret = users;
    // printf("---> %s\n", ret );
    return ret;
}

char* addUser(char hello[]){
    // strcpy(M.user[M.pointer],hello);
    char* ret;
    int i;
    for(i=0;i<M.pointer;i++){
        if(strcmp(M.user[i],hello)==0){
            // strcpy(ret,hello);
            strcat(hello," already exists!");
            return hello;
        }
    }
    M.user[M.pointer] = (char*)malloc(sizeof(char)*1024);
    strcpy(M.user[M.pointer],hello);
    // printf("user %s",M.user[M.pointer]);
    M.inbox[M.pointer] = 0;
    M.inbox_pointer[M.pointer] = 0;
    M.flag[M.currentuser] = 0;
    M.pointer++;

    char fname[100];
    strcpy(fname,root);
    strcat(fname,hello);
    strcat(fname,".txt");
    FILE* fp;
    fp = fopen(fname,"w");
    fclose(fp);

    ret = hello;
    strcat(ret," added Successfully");
    return ret;
}

char* setUser(char user[]){
    int i;
    char* ret;
    
    ret = (char*)malloc(sizeof(char)*500);
    for(i=0;i<M.pointer;i++){
        if(strcmp(user,M.user[i])==0){
            char num[10];
            M.currentuser = i;

            sprintf(num ,"%d" , M.inbox[M.currentuser]); 
            strcpy(ret,"User set to- ");
            strcat(ret,user);
            strcat(ret," inbox: ");
            strcat(ret, num);
            return ret;
        }
    }
    ret = user;
    strcat(user," not found!");
    return ret;
}

char* read(){
    if(M.currentuser==-1){
        char* ret = "INVALID USER!";
        return ret;
    }
    if(M.inbox[M.currentuser]==0){
        char* ret = "No More Mail";
        return ret;
    }

    if(M.flag[M.currentuser]==1){
        M.flag[M.currentuser] = 0;
    }
    else{
        if(M.inbox_pointer[M.currentuser]==M.inbox[M.currentuser])
            M.inbox_pointer[M.currentuser] = 1;
        else
            M.inbox_pointer[M.currentuser]++;
    }

    char user[50];
    int i = 1;
    strcpy(user,root);
    strcat(user,M.user[M.currentuser]);
    strcat(user,".txt");

    char* token = (char*)malloc(sizeof(char)*100);
    char* ret = (char*)malloc(sizeof(char)*100);

    FILE* fp = fopen(user,"a+");

    while(i!=M.inbox_pointer[M.currentuser]){
        // fgets(token,20,fp);
        fscanf(fp,"%s",token);

        while(strcmp(token,"###")!=0)
            // fgets(token,20,fp);
            fscanf(fp,"%s",token);

        i++;
    }

    // fgets(token,20,fp);
    fscanf(fp,"%s",token);
    strcpy(ret,"\n");
    strcat(ret,token);
    strcat(ret, " ");			//FROM: 

    fscanf(fp,"%s",token);
    strcat(ret,token); 
    strcat(ret, "\n"); 			//USER

    fscanf(fp,"%s",token);
    strcat(ret,token);
    strcat(ret, " ");			//TO: 

    fscanf(fp,"%s",token);
    strcat(ret,token); 
    strcat(ret, "\n"); 			//RECEIVER

    fscanf(fp,"%s",token);
    strcat(ret,token);
    strcat(ret, " ");			//DATE: 

    fscanf(fp,"%s",token);
    strcat(ret,token); 
    strcat(ret, " "); 

    fscanf(fp,"%s",token);
    strcat(ret,token); 
    strcat(ret, " "); 

    fscanf(fp,"%s",token);
    strcat(ret,token); 
    strcat(ret, " "); 

    fscanf(fp,"%s",token);
    strcat(ret,token); 
    strcat(ret, " "); 

    fscanf(fp,"%s",token);
    strcat(ret,token); 
    strcat(ret, "\n"); 			//DATE

    while(strcmp(token,"###")!=0){
        fscanf(fp,"%s",token);
        strcat(ret,token);
        strcat(ret, " ");
    }

    // if(M.inbox_pointer[M.currentuser]==M.inbox[M.currentuser])
    //     M.inbox_pointer[M.currentuser] = 1;
    // else
    //     M.inbox_pointer[M.currentuser]++;

    return ret;
}

char* delete(){
    if(M.currentuser==-1)
        return "INVALID USER!";
    if(M.inbox[M.currentuser]==0)
        return "No More Mail";

    int pt,mptr;

    if(M.inbox_pointer[M.currentuser] == 0 && M.inbox[M.currentuser]>0)
        mptr = 1;
    else
        mptr = M.inbox_pointer[M.currentuser];

    char* usr = (char*)malloc(sizeof(char)*100);
    char* dup = (char*)malloc(sizeof(char)*100);

    char* token = (char*)malloc(sizeof(char)*100);

    strcpy(usr,root);
    strcat(usr,M.user[M.currentuser]);
    strcat(usr,".txt");

    strcpy(dup,root);
    strcat(dup,"copy.txt");

    FILE* dupl = fopen(dup,"a+");
    FILE* fp = fopen(usr,"a+");

    pt = 1;

    while(pt!=mptr){
        fscanf(fp,"%s",token);
        fprintf(dupl, "%s ",token );	//from: 

        fscanf(fp,"%s",token);
        fprintf(dupl, "%s\n",token );	//user

        fscanf(fp,"%s",token);
        fprintf(dupl, "%s ",token );	//to

        fscanf(fp,"%s",token);
        fprintf(dupl, "%s\n",token );	//rec

        fscanf(fp,"%s",token);
        fprintf(dupl, "%s ",token );	//date

        fscanf(fp,"%s",token);
        fprintf(dupl, "%s ",token );
        fscanf(fp,"%s",token);
        fprintf(dupl, "%s ",token );
        fscanf(fp,"%s",token);
        fprintf(dupl, "%s ",token );
        fscanf(fp,"%s",token);
        fprintf(dupl, "%s ",token );
        fscanf(fp,"%s",token);
        fprintf(dupl, "%s\n",token );	//date

        while(strcmp(token,"###")!=0){
            fscanf(fp,"%s",token);
            fprintf(dupl, "%s ",token );
        }

        fprintf(dupl, "\n" );
        pt++;
    }

    fscanf(fp,"%s",token);
    // fprintf(dupl, "%s\n",token );

    while(strcmp(token,"###")!=0){
        fscanf(fp,"%s",token);
        // fprintf(dupl, "%s\n",token );
    }

    while(pt!=M.inbox[M.currentuser]){
        fscanf(fp,"%s",token);
        fprintf(dupl, "%s ",token );	//from: 

        fscanf(fp,"%s",token);
        fprintf(dupl, "%s\n",token );	//user

        fscanf(fp,"%s",token);
        fprintf(dupl, "%s ",token );	//to

        fscanf(fp,"%s",token);
        fprintf(dupl, "%s\n",token );	//rec

        fscanf(fp,"%s",token);
        fprintf(dupl, "%s ",token );	//date

        fscanf(fp,"%s",token);
        fprintf(dupl, "%s ",token );
        fscanf(fp,"%s",token);
        fprintf(dupl, "%s ",token );
        fscanf(fp,"%s",token);
        fprintf(dupl, "%s ",token );
        fscanf(fp,"%s",token);
        fprintf(dupl, "%s ",token );
        fscanf(fp,"%s",token);
        fprintf(dupl, "%s\n",token );	//date

        while(strcmp(token,"###")!=0){
            fscanf(fp,"%s",token);
            fprintf(dupl, "%s ",token );
        }
        fprintf(dupl, "\n" );
        pt++;
    }

    if(M.inbox[M.currentuser]==1){
        M.inbox[M.currentuser] = 0;
        M.inbox_pointer[M.currentuser] = 0;
    }
    else{
        if(M.inbox_pointer[M.currentuser] == 1){
            M.inbox[M.currentuser]--;
            M.inbox_pointer[M.currentuser] = 1;
        }
        else if(M.inbox_pointer[M.currentuser] == M.inbox[M.currentuser]){
            M.inbox_pointer[M.currentuser] = 1;
            M.inbox[M.currentuser]--;
        }
        else{
            M.inbox[M.currentuser]--;
        }
    }


    fclose(fp);
    fclose(dupl);
    remove(usr);
    rename("MAILSERVER/copy.txt", usr);

    M.flag[M.currentuser] = 1;

    return "MESSAGE DELETED";
}

char* outbox(char* message[], int index){
    int search;
    for(search=0;search<M.pointer;search++){
        if(strcmp(M.user[search],message[1])==0){
            char receiver[50];
            int i;

            strcpy(receiver,root);
            strcat(receiver,message[1]);
            strcat(receiver,".txt");

            FILE* fp;   
            fp = fopen(receiver,"a+");
            fprintf(fp, "FROM: %s\n",M.user[M.currentuser] );

            fprintf(fp, "TO: %s\n",message[1] );

            time_t current_time;
    		char* c_time_string;
    		current_time = time(NULL);
    		c_time_string = ctime(&current_time);

            fprintf(fp, "DATE: %s",c_time_string);
            
            for(i=2;i<index;i++)
                fprintf(fp, "%s ",message[i] );

            fprintf(fp, "\n\n" );
            fclose(fp);

            for(i=0;i<M.pointer;i++)
            {
                if(strcmp(M.user[i],message[1])==0){
                    if(M.inbox[i]==0){
                        M.inbox_pointer[i]=0;
                    }
                    M.inbox[i]++;
                    break;
                }
            }
            char* ret;
            ret = "MESSAGE SENT!";

            return ret;
        }
    }
    return "INVALID RECEIVER!";
}

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Server connected";
    char ret[1024] = {0};
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( atoi(argv[1]) );
      
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                       (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    recv( new_socket , buffer, sizeof(buffer),0);
    printf("%s\n",buffer );
    send(new_socket , hello , strlen(hello) , 0 );

    mkdir("MAILSERVER",0777);
    root = "MAILSERVER/";

    M.pointer = 0;
    M.currentuser = -1;

    memset(buffer, 0, sizeof(buffer));
    recv( new_socket , buffer, sizeof(buffer),0);
    while(1){

        if(strcmp(buffer,"LSTU")==0){
            // hello = "You sent the command - LSTU";
            strcpy(ret,listusers());
            // strcat(ret,"\nMAIN PROMPT> ");
            send(new_socket , ret , strlen(ret) , 0 );
            memset(ret, 0, sizeof(ret));

            memset(buffer, 0, sizeof(buffer));
            recv( new_socket , buffer, sizeof(buffer),0);
        }
        if(strcmp(buffer,"PTR")==0){
            // hello = "You sent the command - LSTU";
            int ptr = M.inbox_pointer[M.currentuser];
            sprintf(ret ,"Number is:%d" , ptr);  
            // strcpy(ret,listusers());
            // strcat(ret,"\nMAIN PROMPT> ");
            send(new_socket , ret , strlen(ret) , 0 );
            memset(ret, 0, sizeof(ret));

            memset(buffer, 0, sizeof(buffer));
            recv( new_socket , buffer, sizeof(buffer),0);
        }
        else if(strncmp(buffer,"ADDU",4)==0){
            char usrId[1024];
            strcpy(usrId,strtok(buffer," "));
            strcpy(usrId,strtok(NULL, " "));
            strcpy(ret,addUser(usrId));
            // strcat(ret,"\nCLIENT: ");
            // hello = "You sent the command - ADDU";
            send(new_socket , ret , strlen(ret) , 0 );
            memset(ret, 0, sizeof(ret));

            memset(buffer, 0, sizeof(buffer));
            recv( new_socket , buffer, sizeof(buffer),0);
        }
        else if(strncmp(buffer,"USER",4)==0){
            // hello = "You sent the command - USER";
            if(M.currentuser == -1)
            {         
                char usrId[1024];
                strcpy(usrId,strtok(buffer," "));
                strcpy(usrId,strtok(NULL, " "));
                strcpy(ret,setUser(usrId));
            
                send(new_socket , ret , strlen(ret) , 0 );
            }
            else
            {
                strcpy(ret,"Current User ");
                strcat(ret,M.user[M.currentuser]);
                strcat(ret," not done!");

                send(new_socket , ret , strlen(ret) , 0 );
            }
            memset(ret, 0, sizeof(ret));

            memset(buffer, 0, sizeof(buffer));
            recv( new_socket , buffer, sizeof(buffer),0);
        }
        else if(strcmp(buffer,"READM")==0){
            hello = read();
            send(new_socket , hello , strlen(hello) , 0 );

            memset(buffer, 0, sizeof(buffer));
            recv( new_socket , buffer, sizeof(buffer),0);
        }
        else if(strcmp(buffer,"DELM")==0){
            hello = delete();
            send(new_socket , hello , strlen(hello) , 0 );

            memset(buffer, 0, sizeof(buffer));
            recv( new_socket , buffer, sizeof(buffer),0);
        }
        else if(strcmp(buffer,"SEND")==0){

            if(M.currentuser != -1){
                hello = "TYPE MESSAGE:";
                send(new_socket , hello , strlen(hello) , 0 );

                memset(buffer, 0, sizeof(buffer));
                // char new_m[100] = {0};
                recv( new_socket , buffer, sizeof(buffer),0);

                // printf("look here ---- %s\n", new_m);

                char* token;
                char* message[50];
                int index=0,i;

                token = strtok(buffer," ");
                // printf("1 %s\n",token );
                // message[index] = (char*)malloc(sizeof(char)*100);
                // strcpy(message[index],token);
                // index++;
                while(token!=NULL)
                {
                    message[index] = (char*)malloc(sizeof(char)*100);
                    strcpy(message[index],token);
                    // printf("1 %s\n",token );
                    token = strtok(NULL," ");
                    index++;
                }
                
                // return 0;
                hello = outbox(message, index);
                // hello = "MESSAGE SENT!";
                send(new_socket , hello , strlen(hello) , 0 );

                memset(buffer, 0, sizeof(buffer));
                recv( new_socket , buffer, sizeof(buffer),0);
            }
            else{
                hello = "INVALID USER!";
                send(new_socket , hello , strlen(hello) , 0 );

                memset(buffer, 0, sizeof(buffer));
                recv( new_socket , buffer, sizeof(buffer),0);
            }
        }
        else if(strcmp(buffer,"DONEU")==0){
            M.currentuser = -1;
            hello = "No user set!";
            send(new_socket , hello , strlen(hello) , 0 );

            memset(buffer, 0, sizeof(buffer));
            recv( new_socket , buffer, sizeof(buffer),0);
        }
        else if(strcmp(buffer,"QUIT")==0){
            hello = "You sent the command - QUIT";
            send(new_socket , hello , strlen(hello) , 0 );
            break;
        }
        else{
            hello = (char*)malloc(sizeof(char)*1024);
            strcpy(hello,"Error!");
            printf("%s\n",hello );
            // strcat(hello,"\nMAIN PROMPT> ");
            send(new_socket , hello , strlen(hello) , 0 );

            memset(buffer, 0, sizeof(buffer));
            recv( new_socket , buffer, sizeof(buffer),0);
        }
    }

    return 0;
}