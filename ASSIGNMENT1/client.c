#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

  
int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Client connected";
    char buffer[1024] = {0};
    char command[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    // #define PORT 8080
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));
      
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    send(sock , hello , strlen(hello) , 0 );
    recv( sock , buffer, sizeof(buffer),0);
    printf("%s\n",buffer );
    char prompt[100];
    char* curr_user = (char*)malloc(sizeof(char)*10);
    int flag = 0;
    strcpy(prompt,"MAIN PROMPT> ");

    do {
        
        memset(command, 0, sizeof(command));
        printf("%s",prompt);
        scanf("%[^\n]s",command);

        if(strcmp(command,"Listusers")==0){
            
            hello = "LSTU";
            send(sock , hello, strlen(hello) , 0 );
            
            memset(buffer, 0, sizeof(buffer));
            recv( sock , buffer, sizeof(buffer), 0);
            printf("Server sent: %s\n",buffer );
            getchar();
            strcpy(prompt,"MAIN PROMPT> ");
        }
        else if(strncmp(command,"Adduser",7)==0){
            
            hello = "ADDU";
            char pcomm[100];
            char userId[10];

            memset(userId, 0, sizeof(userId));
            memset(pcomm, 0, sizeof(pcomm));

            strcpy(userId,strtok(command," "));
            strcpy(userId,strtok(NULL, " "));

            strcpy(pcomm,hello);
            strcat(pcomm," ");
            strcat(pcomm,userId);
            send(sock , pcomm, strlen(pcomm) , 0 );
            
            memset(buffer, 0, sizeof(buffer));
            recv( sock , buffer, sizeof(buffer), 0);
            printf("Server sent: %s\n",buffer );
            getchar();

            strcpy(prompt,"MAIN PROMPT> ");
        }
        else if(strncmp(command,"Setuser",7)==0){
            
            hello = "USER";
            char ppcomm[100];
            char uuserId[10];

            memset(uuserId, 0, sizeof(uuserId));
            memset(ppcomm, 0, sizeof(ppcomm));

            strcpy(uuserId,strtok(command," "));
            strcpy(uuserId,strtok(NULL, " "));

            strcpy(ppcomm,hello);
            strcat(ppcomm," ");
            strcat(ppcomm,uuserId);
            send(sock , ppcomm, strlen(ppcomm) , 0 );
            
            memset(buffer, 0, sizeof(buffer));
            recv( sock , buffer, sizeof(buffer), 0);
            printf("Server sent: %s\n",buffer );
            getchar();

            flag = 1;
            strcpy(curr_user,uuserId);
            strcpy(prompt,"SUB-PROMPT-");
            strcat(prompt,curr_user);
            strcat(prompt,"> ");
        }
        else if(strcmp(command,"Read")==0){
            
            hello = "READM";
            send(sock , hello, strlen(hello) , 0 );
            
            memset(buffer, 0, sizeof(buffer));
            recv( sock , buffer, sizeof(buffer), 0);
            printf("Server sent: %s\n",buffer );
            getchar();

            if(flag == 1){
                strcpy(prompt,"SUB-PROMPT-");
                strcat(prompt,curr_user);
                strcat(prompt,"> ");
            }
        }
        else if(strcmp(command,"Delete")==0){
            
            hello = "DELM";
            send(sock , hello, strlen(hello) , 0 );
            
            memset(buffer, 0, sizeof(buffer));
            recv( sock , buffer, sizeof(buffer), 0);
            printf("Server sent: %s\n",buffer );
            getchar();

            if(flag == 1){
                strcpy(prompt,"SUB-PROMPT-");
                strcat(prompt,curr_user);
                strcat(prompt,"> ");
            }
        }
        else if(strncmp(command,"Send",4)==0){
            
            hello = "SEND";
            send(sock , hello, strlen(hello) , 0 );
            
            memset(buffer, 0, sizeof(buffer));
            recv( sock , buffer, sizeof(buffer), 0);
            // printf("Server sent: %s\n",buffer );
            getchar();

            char message[1024] = {0};
            char ser_mess[1024] = {0};
            char recId[10];

            strcpy(recId,strtok(command," "));
            strcpy(recId,strtok(NULL, " "));

            strcpy(message,"SEND");
            strcat(message," ");
            strcat(message,recId);
            strcat(message," ");

            printf("TYPE MESSAGE: ");
            scanf("%[^\n]s",ser_mess);

            strcat(message,ser_mess);

            send(sock , message , strlen(message) , 0 );
            memset(message, 0, sizeof(message));

            memset(buffer, 0, sizeof(buffer));
            recv( sock , buffer, sizeof(buffer), 0);
            printf("Server sent: %s\n",buffer );
            getchar();

            if(flag == 1){
                strcpy(prompt,"SUB-PROMPT-");
                strcat(prompt,curr_user);
                strcat(prompt,"> ");
            }
        }
        else if(strcmp(command,"Done")==0){
            
            hello = "DONEU";
            send(sock , hello, strlen(hello) , 0 );
            
            memset(buffer, 0, sizeof(buffer));
            recv( sock , buffer, sizeof(buffer), 0);
            printf("Server sent: %s\n",buffer );
            getchar();

            flag = 0;
            strcpy(prompt,"MAIN PROMPT> ");
        }
        else if(strcmp(command,"Quit")==0){
            hello = "QUIT";
            send(sock , hello, strlen(hello) , 0 );
            
            memset(buffer, 0, sizeof(buffer));
            printf("Server Closed");
            getchar();
            break;
        }
        else{
            send(sock , command, strlen(command) , 0 );
            
            memset(buffer, 0, sizeof(buffer));
            recv( sock , buffer, sizeof(buffer), 0);
            printf("Server sent: %s\n",buffer );
            getchar();

            flag = 0;
            strcpy(prompt,"MAIN PROMPT> ");
        }
        // memset(command, 0, sizeof(command));
        // printf("MAIN PROMPT> ");
        // scanf("%[^\n]s",command);
        // hello = command;
        
        // send(sock , hello , strlen(hello) , 0 );
        
        // memset(buffer, 0, sizeof(buffer));
        // recv( sock , buffer, sizeof(buffer), 0);
        // printf("Server sent: %s\n",buffer );
        // getchar();

    }while(strcmp(hello,"Quit")!=0);

    return 0;
}