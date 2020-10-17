/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define CLOSE_PASSWORD "close_the_server_1234"

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
       error("ERROR opening socket");

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    {
    	printf("setsockopt(SO_REUSEADDR) failed\n");
    	close(sockfd);
    	exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR on binding");
	     

    FILE* fp;
    fp=fopen("database.txt","r");
      
    while(1)
    {

     	listen(sockfd,5);

	    clilen = sizeof(cli_addr);


	    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

	    if (newsockfd < 0) 
	        error("ERROR on accept");

		rewind(fp);


        bzero(buffer,255);


        n = read(newsockfd,buffer,255);
        if (n < 0) error("ERROR reading from socket");
        


        printf("Client: %s\n",buffer);

        int flag=0;

        int i=strncmp( CLOSE_PASSWORD , buffer+2, strlen(CLOSE_PASSWORD));
        if(i == 0){
          	bzero(buffer, 256);
			buffer[0]='B';
			buffer[1]='y';
			buffer[2]='e';
          	close(newsockfd);
          	if(write(newsockfd,buffer,strlen(buffer))<0){

				printf("error writing\n");

				close(newsockfd);
				close(sockfd);
				exit(0);

			}

			close(newsockfd);
			close(sockfd);
			exit(1);
             
        }


        char pass[sizeof(CLOSE_PASSWORD)];
        strcpy(pass,buffer+2);

        if(buffer[0]=='1'){

        	char inpsite[256];
        	size_t inpsize=0;
        	ssize_t read;

            strcpy(inpsite,buffer+2);

	        char cursite[256];

			char line[256];
			int flag=0;

			while(fgets(line, 255, fp)) {
				char DN[255], IP[255];
				int siz=0;
				for(siz=0;siz<strlen(line);siz++){
					if(line[siz]==' ')
						break;
				}
				strncpy(DN,line,siz);
				strcpy(IP,line+siz+1);
				DN[siz]='\0';
				for(int i=0;i<strlen(IP);i++){
					if(IP[i]=='\n'){
						IP[i]='\0';
						break;
					}
				}
				if(!strncmp(DN,pass,strlen(DN))){
					flag=1;
					strcpy(buffer+2,IP);
					buffer[0]='3';
					if(write(newsockfd, buffer, strlen(buffer))<0){
						printf("error writing\n");
						close(newsockfd);
						close(sockfd);
						break;
					}
					close(newsockfd);

				}

			}
			if(!flag){
				bzero(buffer,256);

				buffer[0]='4';
				if(write(newsockfd, buffer, strlen(buffer))<0){
					printf("error writing\n");
					close(newsockfd);
					close(sockfd);
				} else {
					close(newsockfd);
				}

			}

        } else {
        	char inpsite[256];
        	size_t inpsize=0;
        	ssize_t read;

            strcpy(inpsite,buffer+2);

	        char cursite[256];

			char line[256];
			int flag=0;

			while(fgets(line, 255, fp)) {
				char DN[255], IP[255];
				int siz=0;
				for(siz=0;siz<strlen(line);siz++){
					if(line[siz]==' ')
						break;
				}
				strncpy(DN,line,siz);
				strcpy(IP,line+siz+1);
				DN[siz]='\0';
				for(int i=0;i<strlen(IP);i++){
					if(IP[i]=='\n'){
						IP[i]='\0';
						break;
					}
				}
				if(!strncmp(IP,pass,strlen(IP))){
					flag=1;
					strcpy(buffer+2,DN);
					buffer[0]='3';
					if(write(newsockfd, buffer, strlen(buffer))<0){
						printf("error writing\n");
						close(newsockfd);
						close(sockfd);
						break;
					}
					close(newsockfd);

				}

			}
			if(!flag){
				bzero(buffer,256);

				buffer[0]='4';
				if(write(newsockfd, buffer, strlen(buffer))<0){
					printf("error writing\n");
					close(newsockfd);
					close(sockfd);
				} else {
					close(newsockfd);
				}

			}
        }

     }
     
     close(sockfd);
     fclose(fp);
     return 0; 
}
