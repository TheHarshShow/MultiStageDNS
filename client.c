#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char * argv[]){

	// Need Proxy server IP and port number as arguments.
	if(argc<3){
		printf("usage: %s <server_IP_Address> <server_port_number>\n", argv[0]);
		exit(0);
	}

	int sockfd;				//Socket file descriptor to connect with DNS Server
	int port_number;		//Port number of application in DNS Server

	struct sockaddr_in serv_addr;	//Specifies transport address and port for AD_INET Address (both server and client, IP address and Port number)
	struct hostent *server;			//Structure hostent describes host

	char buffer[256];				//Buffer used for input and output of proxy server in this connection

	sockfd = socket(AF_INET, SOCK_STREAM, 0);	//AF_INET is an address family for IPv4 protocol
												//SOCK_STREAM means that a TCP socket is being declared
												//0 corresponds to TCP protocol

	//Error Handling
	if(sockfd<0){
		printf("Error opening socket!\n");
		close(sockfd);
		exit(0);
	}

	port_number = atoi(argv[2]);			//Port number of application on DNS Server
	server = gethostbyname(argv[1]);		//Creates hostent structure from name

	bzero((char *)&serv_addr, sizeof(serv_addr));	//Clears serv_addr
	serv_addr.sin_family = AF_INET;					//Specifies Address family
    bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);	//copies h_addr value from server (hostent) to serv_addr
    serv_addr.sin_port = htons(port_number);		//Setup port number
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){ //Send connectionn request and error handling
    	printf("Error connecting!\n");
    	close(sockfd);
    	exit(0);
    }

    printf("Connection Established!\n");

    short type;

    //Prompt user for Type of request

    printf("Enter Type (1/2): ");

    scanf("%hd", &type);
    if(type==1){
    	//Prompt user to enter data
    	printf("Enter Domain Name: ");
	    bzero(buffer,256);

	    buffer[0]='1';
	    buffer[1]=' ';
	    //Read user input
	    scanf("%s",buffer+2);

	    if (write(sockfd,buffer,strlen(buffer)) < 0) { //Write buffer to output stream of socket
	    	//Error handling
	    	printf("Error writing to socket!\n");
	    	close(sockfd);
	    	exit(0);

	    }
	    bzero(buffer,256); //Clear buffer
	    
	    if (read(sockfd,buffer,255) < 0){	//Read from socket descriptor to buffer
	    	//error handling
	    	printf("Error reading from socket!\n");
	    	close(sockfd);
	    	exit(0);

	    }
	    printf("Response: %s\n", buffer);	//Output response received to user

    } else if(type==2){
    	//Prompt user to enter data
		printf("Enter IP Address: ");
		bzero(buffer,256);

	    buffer[0]='2';
	    buffer[1]=' ';
	    //Read user input
	    scanf("%s",buffer+2);

	    if (write(sockfd,buffer,strlen(buffer)) < 0) { //Write buffer to output stream of socket
	    	//Error handling
	    	printf("Error writing to socket!\n");
	    	close(sockfd);
	    	exit(0);

	    }

	    bzero(buffer,256); //Clear buffer

	    if (read(sockfd,buffer,255) < 0){	//Read from socket descriptor to buffer
	    	//error handling
	    	printf("Error reading from socket!\n");
	    	close(sockfd);
	    	exit(0);
	    }
	    printf("Response: %s\n", buffer);	//Output response received to user

    } else {
    	//Specify the corrrect input format if wrong input is received
    	printf("Type must be 1 or 2!\nEnter 1 if you want the IP Address corresponding to a Domain Name.\nEnter 2 if you want the Domain name corresponding to an IP Address.\n");
    	exit(1);
    }

    //Closing connection with Proxy server
	printf("Closing Connection!\n");

    close(sockfd);


}