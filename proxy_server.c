#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

//SERVER INFORMATION AND CLOSE PASSWORD FOR SERVER AND PROXY SERVER
#define CLOSE_PASSWORD "close_the_server_1234"
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9100

//Declaring circular linked list node for cache

struct node {

	struct node *next;
	struct node *prev;
	char domainName[256];
	char IPAddress[256];

};

//Allocate memory and assign Domain name and IP Address values;

struct node *createNode(char *DN, char *IP){
	struct node *ans = (struct node *)malloc(sizeof(struct node));
	bzero(ans->domainName, 256);
	bzero(ans->IPAddress, 256);

	//Points to self
	ans->next=ans;
	ans->prev=ans;

	//Assign Domain name and IPAddress attributes
	strcpy(ans->domainName, DN);
	strcpy(ans->IPAddress, IP);

	return ans;
}


//Delete node from cache (linked list)
void removeNode(struct node **x){

	if(x==NULL)
		return;

	//If alone in list
	if((*x)->next == *x){
		free(*x);
		return;
	}

	//Modify pointers 
	(*x)->prev->next=(*x)->next;
	(*x)->next->prev=(*x)->prev;
	free(*x);

}

//Get number of entries in cache. (number of nodes in linked list)
int getSize(struct node *start){
	int ans=0;

	//If no node is in linked list
	if(start==NULL)
		return 0;
	//Start node
	ans++;
	struct node *temp=start->next;

	//Count all other nodes
	while(temp!=start){
		temp=temp->next;
		ans++;
	}
	return ans;
}

//Insert node into cache
void insert(struct node **start, struct node *x){

	//If cache is empty
	if(getSize(*start)==0){
		*start = x;
		return;
	}

	insert_label:
	if(getSize(*start)<3){

		//If cache is not full, insert at end (before beginning since linked list is circular)
		(*start)->prev->next = x;
		x->prev=(*start)->prev;
		x->next=(*start);
		(*start)->prev=x;
		x->next = *start;
	} else {

		//If cache is full, FIFO entry and removal
		struct node *temp = (*start);
		(*start) = (*start)->next;
		removeNode(&temp);
		goto insert_label;
	}
}

//Find IP Address in Cache. DOES NOT QUERY DNS SERVER HERE.
char* getIPforDN(char * DN, struct node *start){

	if(start == NULL)
		return "NOT FOUND";

	//If in first node
	if(!strncmp(start->domainName,DN,strlen(DN)))
		return start->IPAddress;

	struct node*temp=start->next;

	//Look in every other node
	while(temp!=start){
		if(!strncmp(temp->domainName,DN,strlen(DN)))
			return temp->IPAddress;
		temp=temp->next;
	}
	return "NOT FOUND";

}

//Find Domain name in Cache. DOES NOT QUERY DNS SERVER HERE.
char *getDNforIP(char *IP, struct node *start){
	if(start==NULL)
		return "NOT FOUND";

	//If in first node
	if(!strncmp(start->IPAddress, IP,strlen(IP)))
		return start->domainName;
	struct node *temp = start->next;

	//Look in every other node.
	while(temp!=start){
		if(!strncmp(temp->IPAddress, IP, sizeof(IP)))
			return temp->domainName;
		temp=temp->next;
	}
	return "NOT FOUND";
}


//Query DNS Server with identical request to get a response copied to buff.
void getDataFromServer(char *req,char *buff){

	int sockfd;				//Socket file descriptor to connect with DNS Server
	int port_number;		//Port number of application in DNS Server

	struct sockaddr_in serv_addr;	//Specifies transport address and port for AD_INET Address (both server and client, IP address and Port number)
	struct hostent *server;			//Structure hostent describes host

	char buffer[256];				//Buffer used for input and output of proxy server in this connection
	strcpy(buffer, req);			//Copy client request to buffer

	sockfd = socket(AF_INET, SOCK_STREAM, 0);	//AF_INET is an address family for IPv4 protocol
												//SOCK_STREAM means that a TCP socket is being declared
												//0 corresponds to TCP protocol
	//Error handling
	if(sockfd<0){
		printf("Error opening socket!\n");
		exit(0);
	}
	port_number = SERVER_PORT;				//Port number of application on DNS Server
	server = gethostbyname(SERVER_IP);		//Creates hostent structure from name
	
	bzero((char *)&serv_addr, sizeof(serv_addr));	//Clears serv_addr
	serv_addr.sin_family = AF_INET;					//Specifies Address family
    bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length); //copies h_addr value from server (hostent) to serv_addr
    serv_addr.sin_port = htons(port_number);		//Setup port number
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){ //Send connectionn request and error handling
    	printf("Error connecting!\n");
    	exit(0);
    }

    
    if (write(sockfd,buffer,strlen(buffer)) < 0) { //Write buffer to output stream of socket
    	//Error handling
    	printf("Error writing to socket!\n");
    	exit(0);

    }
    bzero(buffer,256); //Clear buffer
    if (read(sockfd,buffer,255) < 0){	//Read from socket descriptor to buffer
    	//error handling
    	printf("Error reading from socket!\n");
    	exit(0);

    } 
    //copy from buffer to buff so it can be used outside the function.
    strcpy(buff, buffer);
}

int main(int argc, char *argv[])
{

	//Declare start pointer of cache.
	struct node *cache = NULL;

	//Need a port number argument
	if(argc < 2)
	{
		printf("usage: %s <port_number>\n", argv[0]);
		exit(0);
	}

	int sockfd, port_number; //LISTENING SOCKET (for creating connection): Socket descriptor and port number of proxy server 

    char buffer[256];		//Buffer to help with I/O between Client and Poxy server

    struct sockaddr_in serv_addr, cli_addr; //Specifies transport address and port for AD_INET Address

    sockfd = socket(AF_INET, SOCK_STREAM, 0);	//AF_INET is an address family for IPv4 protocol
												//SOCK_STREAM means that a TCP socket is being declared
												//0 corresponds to TCP protocol
    											//This is for listening socket.

    if (sockfd < 0){
    	//Error handling
    	printf("Error opening socket!\n");
    	close(sockfd);
    	exit(0);

    }

    //If app closes and reopens, allow reuse of IP/Port combo
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    {
    	printf("setsockopt(SO_REUSEADDR) failed!\n");
    	close(sockfd);
    	exit(0);
    }

    port_number = atoi(argv[1]);	//Get port number of application on proxy server

    bzero((char *) &serv_addr, sizeof(serv_addr));	//Clear and assign listen connection parameters to serv_addr
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_number);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {	//Binds socket to address and port number 
		//error handling
		printf("Error on binding!\n");
		close(sockfd);
		exit(0);
	}

	printf("Send CLOSE_PASSWORD (from server.c file) with any type (1 or 2) from client to safely close the server!\n");


	while(1){

		//Listen for connection requests on sockfd socket
		listen(sockfd,5);
	    socklen_t clilen = sizeof(cli_addr);
	    //Create socket for comminucation between client and proxy server. Accept connection request
	    int newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
	    if (newsockfd < 0){
	    	//Error handling
	    	printf("Error on accept!\n");
	    	exit(0);
	    }
	    printf("Client Connected\n");

	    //Clear buffer
		bzero(buffer, 256);
		//Read message from client into buffer
		if(read(newsockfd,buffer,255)<0){
			//Error handling
			printf("Error in reading!\n");
			close(newsockfd);
			close(sockfd);
			exit(0);
		}
		//If request is to close proxy server. Password is used by the client
		if(!strncmp(buffer+2, CLOSE_PASSWORD, strlen(CLOSE_PASSWORD))){

			char CloseResponse[255];
			getDataFromServer(buffer,CloseResponse);

			bzero(buffer, 255);
			printf("Closing DNS and Proxy Server!\n");
			buffer[0]='B';
			buffer[1]='y';
			buffer[2]='e';

			if(write(newsockfd,buffer,strlen(buffer))<0){ //Write "Bye" into output buffer.
				//Error handling
				printf("Error in writing!\n");
				close(newsockfd);
				close(sockfd);
				exit(0);

			}

			close(newsockfd);
			close(sockfd);
			exit(1);
		}

		//If DNS is given and IP Address is needed.
		if(buffer[0]=='1'){

			printf("Request: %s\n", buffer);

			char *temp = (char *)malloc(sizeof(getIPforDN(buffer+2, cache)));
			char req[256];
			strcpy(req,buffer);
			bzero(temp,sizeof(temp));
			strcpy(temp,getIPforDN(buffer+2, cache));		//Look in cache for hit

			bzero(buffer, 256);		//clear buffer
			strcpy(buffer+2, temp);	//Copy output of cache lookup into buffer
			buffer[0]='3';	//Assuming found (DON'T WORRY, WILL UNDERSTAND LATER)
			buffer[1]=' ';

			if(!strcmp(buffer+2, "NOT FOUND")){	//If cache miss occured hit DNS Server
				bzero(buffer,256);				//clear buffer
				getDataFromServer(req, buffer);	//Copy output from DNS Server query to buffer
				if(buffer[0]!='4'){
					//Update cache if found
					struct node* newNode = createNode(req+2,buffer+2);
					insert(&cache,newNode);
				}
			} else
				printf("Found in cache!\n");

			//Write to output stream of socket
			if(write(newsockfd,buffer,strlen(buffer))<0){

				//Error handling
				printf("Error in writing!\n");
				close(newsockfd);
				close(sockfd);
				exit(0);

			}

		} else {

			printf("Request: %s\n", buffer);

			char *temp = (char *)malloc(sizeof(getDNforIP(buffer+2, cache)));
			char req[256];
			strcpy(req, buffer);

			bzero(temp,sizeof(temp));
			strcpy(temp,getDNforIP(buffer+2, cache));	//Look in cache for a hit

			bzero(buffer, 256);		//Clear buffer
			strcpy(buffer+2, temp);	//Copy cache lookup output to buffer
			buffer[0]='3';			//Assuming found in cache (YOU WILL UNDERSTAND LATER)
			buffer[1]=' ';


			if(!strcmp(buffer+2,"NOT FOUND")){	//If cache lookup failed
				bzero(buffer, 256);				//Clear Buffer
				getDataFromServer(req,buffer);	//Query DNS Server and copy output into buffer
				if(buffer[0]!='4'){
					//Update cache if found in DNS Server
					struct node* newNode = createNode(req+2,buffer+2);

					insert(&cache,newNode);
				}

			} else
				printf("Found in cache!\n");

			if(write(newsockfd,buffer,strlen(buffer))<0){
				//Error handling
				printf("Error in writing!\n");
				close(newsockfd);
				close(sockfd);
				exit(0);
			}

		}
		//CLOSE CONNECTION BETWEEN CLIENT AND PROXY SERVER

		close(newsockfd);
		printf("Disconnected client!\n");

	}
	
    close(sockfd);

}