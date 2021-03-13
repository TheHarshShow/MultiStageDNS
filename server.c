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

int main(int argc, char *argv[])
{
	int sockfd, port_number;					//Socket descriptor and port number
	socklen_t clilen;							//Length of cli_addr
	char buffer[256];							//used for IO between proxy server and server
	struct sockaddr_in serv_addr, cli_addr;		//Specifies transport address and port for AD_INET Address (both server and client, IP address and Port number)

	//Input format verification
	if (argc < 2) {
		printf("usage: %s <port_number>\n", argv[0]);
		exit(0);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 	//AF_INET is an address family for IPv4 protocol
												//SOCK_STREAM means that a TCP socket is being declared
												//0 corresponds to TCP protocol

	if (sockfd < 0){
		//Error handling
		printf("Error opening socket\n");
		close(sockfd);
		exit(0);
	}

	//If app closes and reopens, allow reuse of IP/Port combo
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
	{
		printf("setsockopt(SO_REUSEADDR) failed\n");
		close(sockfd);
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));	//Clear serv_addr
	port_number = atoi(argv[1]);					//Get port number of application on proxy server
	serv_addr.sin_family = AF_INET;					//Setup server address
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port_number);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {	//Bind socket with address
		//Error handling
		printf("ERROR on binding");
		close(sockfd);
		exit(0);
	}
	printf("DNS Server set up!\n");

	FILE* fp;							//File pointer
	fp=fopen("database.txt","r");		//Open file for read

	while(1)
	{
		//Listen for connection request
		listen(sockfd,5);

		clilen = sizeof(cli_addr);
		int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);	//Accept connection request
		if (newsockfd < 0){
			//Error handling
			printf("Error on accept!\n");
			close(newsockfd);
			close(sockfd);
			exit(0);
		}
		printf("Request Acknowledged\n");


		rewind(fp);					//Bring to beginning
		bzero(buffer,255);

		if (read(newsockfd,buffer,255) < 0){			//Read from input stream into buffer
			printf("Error in reading from socket!\n");
			close(newsockfd);
			close(sockfd);
			exit(0);	
		}

		printf("Client: %s\n",buffer);

		int flag=0;
		//If clients asks to close connection using password
		if(!strncmp(CLOSE_PASSWORD , buffer+2, strlen(CLOSE_PASSWORD))){
			bzero(buffer, 256);
			printf("Closing DNS and Proxy Server\n");
			buffer[0]='B';
			buffer[1]='y';
			buffer[2]='e';
			if(write(newsockfd,buffer,strlen(buffer))<0){
				printf("Error in writing!\n");
				close(newsockfd);
				close(sockfd);
				exit(0);
			}

			close(newsockfd);
			close(sockfd);
			exit(1);
		}

		if(buffer[0]=='1'){

			char line[256];
			int flag=0;
			//Read the file line by line to find domain name
			while(fgets(line, 255, fp)) {
				char DN[255], IP[255];
				int siz=0;
				for(siz=0;siz<strlen(line);siz++){
					if(line[siz]==' ')
						break;
				}
				//Split line into Domain name and IP
				strncpy(DN,line,siz);
				strcpy(IP,line+siz+1);
				DN[siz]='\0';
				for(int i=0;i<strlen(IP);i++){
					if(IP[i]=='\n'){
						IP[i]='\0';
						break;
					}
				}
				//If domain name matches, write it to output stream (send it to proxy server)
				if(!strncmp(DN,buffer+2,strlen(DN))){
					printf("IP Address for the Domain Name found!\n");
					flag=1;
					strcpy(buffer+2,IP);
					buffer[0]='3';
					buffer[1]=' ';
					if(write(newsockfd, buffer, strlen(buffer))<0){
						printf("Error in writing!\n");
						close(newsockfd);
						close(sockfd);
						break;
					}
					close(newsockfd);

				}

			}
			if(!flag){
				printf("IP address for Domain name not found!\n");
				bzero(buffer,256);
				//Not fount in file. Send not found error to Proxy Server.
				buffer[0]='4';
				buffer[1]=' ';
				strcpy(buffer+2, "entry not found in the database");
				if(write(newsockfd, buffer, strlen(buffer))<0){
					printf("Error in writing!\n");
					//Close connection. LISTENING SOCKET REMAINS OPEN FOR FUTURE CONNECTIONS
					close(newsockfd);
					close(sockfd);
				} else {
					close(newsockfd);
				}

			}

		} else {
			
			char line[256];
			int flag=0;
			//Read the file line by line to find IP Address
			while(fgets(line, 255, fp)) {
				char DN[255], IP[255];
				int siz=0;
				for(siz=0;siz<strlen(line);siz++){
					if(line[siz]==' ')
						break;
				}
				//Split line into Domain name and IP, write it to output stream (send it to proxy server)
				strncpy(DN,line,siz);
				strcpy(IP,line+siz+1);
				DN[siz]='\0';
				for(int i=0;i<strlen(IP);i++){
					if(IP[i]=='\n'){
						IP[i]='\0';
						break;
					}
				}
				//If IP Address matches
				if(!strncmp(IP,buffer+2,strlen(IP))){
					printf("Domain name for IP Address found!\n");
					flag=1;
					strcpy(buffer+2,DN);

					buffer[0]='3';
					buffer[1]=' ';
					if(write(newsockfd, buffer, strlen(buffer))<0){
						printf("Error in writing!\n");
						close(newsockfd);
						close(sockfd);
						break;
					}
					close(newsockfd);

				}

			}
			if(!flag){
				printf("Domain Name for IP Address not found!\n");

				bzero(buffer,256);
				//Not fount in file. Send not found error to Proxy Server.
				buffer[0]='4';
				buffer[1]=' ';
				strcpy(buffer+2, "entry not found in the database");
				if(write(newsockfd, buffer, strlen(buffer))<0){
					printf("Error in writing!\n");
					close(newsockfd);
					close(sockfd);
					exit(0);
				} else {
					//Close connection. LISTENING SOCKET REMAINS OPEN FOR FUTURE CONNECTIONS
					close(newsockfd);
				}

			}
		}

	}
    //Close connection and listening socket
	close(sockfd);
	fclose(fp);
	return 0; 
}
