#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char * argv[]){

	if(argc<3){
		printf("usage: %s <server_IP_Address> <server_port_number>\n", argv[0]);
		exit(0);
	}

	int sockfd;
	int port_number;

	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[256];

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd<0){
		printf("Error opening socket!\n");
		exit(0);
	}

	port_number = atoi(argv[2]);
	server = gethostbyname(argv[1]);

	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(port_number);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
    	printf("Error connecting!\n");
    	exit(0);
    }

    short type;

    enterLabel:

    printf("Enter Type (1/2): ");

    scanf("%hd", &type);
    if(type==1){

    	printf("Enter Domain Name: ");
	    bzero(buffer,256);

	    buffer[0]='1';
	    buffer[1]=' ';

	    scanf("%s",buffer+2);

	    if (write(sockfd,buffer,strlen(buffer)) < 0) {

	    	printf("Error writing to socket!\n");
	    	exit(0);

	    }
	    bzero(buffer,256);
	    
	    if (read(sockfd,buffer,255) < 0){

	    	printf("Error reading from socket!\n");
	    	exit(0);

	    } 
	    printf("Response: %s\n", buffer);

    } else if(type==2){

    	printf("Enter IP Address: ");
	    bzero(buffer,256);

	    buffer[0]='2';
	    buffer[1]=' ';

	    scanf("%s",buffer+2);

	    if (write(sockfd,buffer,strlen(buffer)) < 0) {

	    	printf("Error writing to socket!\n");
	    	exit(0);

	    }
	    bzero(buffer,256);
	    if (read(sockfd,buffer,255) < 0){
	    	printf("Error reading from socket!\n");
	    	exit(0);
	    } 
	    printf("Response: %s\n", buffer);

    } else {
    	printf("Type must be 1 or 2!\nEnter 1 if you want the IP Address corresponding to a Domain Name.\nEnter 2 if you want the Domain name corresponding to an IP Address.\n");
    	goto enterLabel;
    }

    close(sockfd);


}