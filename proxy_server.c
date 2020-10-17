#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define CLOSE_PASSWORD "close_the_proxy_server_1234"
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9100

struct node {

	struct node *next;
	struct node *prev;
	char domainName[256];
	char IPAddress[256];

};

struct node *createNode(char *DN, char *IP){
	struct node *ans = (struct node *)malloc(sizeof(struct node));
	bzero(ans->domainName, 256);
	bzero(ans->IPAddress, 256);
	ans->next=ans;
	ans->prev=ans;

	strcpy(ans->domainName, DN);
	strcpy(ans->IPAddress, IP);

	return ans;
}

void removeNode(struct node **x){

	if(x==NULL)
		return;

	if((*x)->next == *x){
		free(*x);
		return;
	}
	(*x)->prev->next=(*x)->next;
	(*x)->next->prev=(*x)->prev;

}

int getSize(struct node *x){
	int ans=0;
	if(x==NULL)
		return 0;
	ans++;
	struct node *temp=x->next;
	while(temp!=x){
		temp=temp->next;
		ans++;
	}
	return ans;
}

void insert(struct node **start, struct node *x){
	if(getSize(*start)==0){
		*start = x;
		return;
	}

	insert_label:

	if(getSize(*start)<3){
		(*start)->prev->next = x;
		x->prev=(*start)->prev;
		x->next=(*start);
		(*start)->prev=x;
		x->next = *start;
	} else {
		struct node *temp = (*start);
		(*start) = (*start)->next;
		removeNode(&temp);

		goto insert_label;

	}
}

char* getIPforDN(char * DN, struct node *start){

	if(start == NULL)
		return "NOT FOUND";

	if(!strncmp(start->domainName,DN,strlen(DN)))
		return start->IPAddress;

	struct node*temp=start->next;
	while(temp!=start){
		if(!strncmp(temp->domainName,DN,strlen(DN)))
			return temp->IPAddress;
		temp=temp->next;
	}
	return "NOT FOUND";

}

char *getDNforIP(char *IP, struct node *start){
	if(start==NULL)
		return "NOT FOUND";
	if(!strncmp(start->IPAddress, IP,strlen(IP)))
		return start->domainName;
	struct node *temp = start->next;
	while(temp!=start){
		if(!strncmp(temp->IPAddress, IP, sizeof(IP)))
			return temp->domainName;
		temp=temp->next;
	}
	return "NOT FOUND";
}

char* getDataFromServer(char *req,char *buff){

	int sockfd;
	int port_number;

	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[256];
	strcpy(buffer, req);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd<0){
		printf("Error opening socket!\n");
		exit(0);
	}
	port_number = SERVER_PORT;
	server = gethostbyname(SERVER_IP);
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(port_number);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
    	printf("Error connecting!\n");
    	exit(0);
    }

    int n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) {

    	printf("Error writing to socket!\n");
    	exit(0);

    }
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0){

    	printf("Error reading from socket!\n");
    	exit(0);

    } 
    strcpy(buff, buffer);
}

int main(int argc, char *argv[])
{

	struct node *cache = NULL;

	if(argc < 2)
	{
		printf("usage: %s port_number>\n", argv[0]);
		exit(0);
	}

	int sockfd, port_number;

    char buffer[256];

    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0){

    	printf("Error opening socket!\n");
    	close(sockfd);
    	exit(0);

    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    {
    	printf("setsockopt(SO_REUSEADDR) failed!\n");
    	close(sockfd);
    	exit(0);
    }

    port_number = atoi(argv[1]);

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_number);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {

		printf("Error on binding!\n");
		close(sockfd);
		exit(0);

	}

	printf("Send CLOSE_PASSWORD (from server.c file) with any type (1 or 2) from client to safely close the server!\n");


	while(1){

		listen(sockfd,5);

	    socklen_t clilen = sizeof(cli_addr);
	    int newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);

	    if (newsockfd < 0){
	    	printf("Error on accept!\n");
	    	exit(0);
	    }


		bzero(buffer, 256);
		if(read(newsockfd,buffer,255)<0){

			printf("Error reading!\n");

			close(newsockfd);
			close(sockfd);
			exit(0);

		}

		if(!strncmp(buffer+2, CLOSE_PASSWORD, strlen(CLOSE_PASSWORD))){
			bzero(buffer, 255);
			buffer[0]='B';
			buffer[1]='y';
			buffer[2]='e';

			if(write(newsockfd,buffer,strlen(buffer))<0){

				printf("Error writing!\n");

				close(newsockfd);
				close(sockfd);
				exit(0);

			}

			close(newsockfd);
			close(sockfd);
			exit(1);
		}

		// printf("Request: %s\n", buffer);
		if(buffer[0]=='1'){

			printf("Request: %s\n", buffer+2);

			char *temp = (char *)malloc(sizeof(getIPforDN(buffer+2, cache)));
			char req[256];
			strcpy(req,buffer);
			bzero(temp,sizeof(temp));
			strcpy(temp,getIPforDN(buffer+2, cache));

			bzero(buffer, 256);
			strcpy(buffer+2, temp);
			buffer[0]='3';
			buffer[1]=' ';

			if(!strcmp(buffer+2, "NOT FOUND")){
				bzero(buffer,256);
				getDataFromServer(req, buffer);
				if(buffer[0]!='4'){
					struct node* newNode = createNode(req+2,buffer+2);

					insert(&cache,newNode);
				}
			} else printf("Found in cache!\n");

			if(write(newsockfd,buffer,strlen(buffer))<0){

				printf("Error writing!\n");

				close(newsockfd);
				close(sockfd);
				exit(0);

			}

		} else {

			printf("Request: %s\n", buffer+2);

			char *temp = (char *)malloc(sizeof(getDNforIP(buffer+2, cache)));
			char req[256];
			strcpy(req, buffer);

			bzero(temp,sizeof(temp));
			strcpy(temp,getDNforIP(buffer+2, cache));

			bzero(buffer, 256);
			strcpy(buffer+2, temp);
			buffer[0]='3';
			buffer[1]=' ';


			if(!strcmp(buffer+2,"NOT FOUND")){
				bzero(buffer, 256);
				getDataFromServer(req,buffer);
				if(buffer[0]!='4'){
					struct node* newNode = createNode(req+2,buffer+2);

					insert(&cache,newNode);
				}

			}

			if(buffer[0]!='4'){
				struct node*newNode = createNode(buffer+2,req+2);
				insert(&cache,newNode);
			}

			if(write(newsockfd,buffer,strlen(buffer))<0){

				printf("Error writing!\n");

				close(newsockfd);
				close(sockfd);
				exit(0);
			}

		}

		close(newsockfd);
	}
	
    close(sockfd);


}