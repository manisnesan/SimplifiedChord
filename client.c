/*
 *  *   CSC 501 - HW5 sample code
 *   */
/* Group:
 * 	msivane Manikandan Sivanesan
 * 	anatara Arun Natarajan
 * 	bsundar Balaji Sundaravel
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>

#define BUFLEN 512

//#define DEBUG
#ifdef DEBUG
#define Printf(...) printf(__VA_ARGS__);
#else
#define Printf(...) 
#endif

/*
 *  * Example:
 *   *
 *    * client 50000 "PUT:key1:value1"
 *     *
 *      * valid commands:
 *       * PUT, GET, END, PRINT
 *        *
 *         *  Syntax: ./client portnumber command
 *          */

// random port number generator
int ran_gen_pno(){
	int prtno = (rand()%(65535-1024)+1024);
	Printf("%d\n",prtno);
	return prtno;
}

/*
 * client_server_listen - server running on the client side when GET command is issued in order to retrieve the value from the server
 * Author : msivane Manikandan Sivanesan
 */
void client_server_listen(int port) {
	
	struct sockaddr_in sock_server, sock_client;
	char msg[BUFLEN];

	srand(time(NULL));
	int portnum = port;//generating random port number
	
	int s, slen = sizeof(sock_client);
	char *command,*key,*value;
	
	int client;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("error in socket creation");
		exit(1);
	}

	memset((char *) &sock_server, 0, sizeof(sock_server));
	sock_server.sin_family = AF_INET;
	sock_server.sin_port = htons(portnum);
	sock_server.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (bind(s, (struct sockaddr *) &sock_server, sizeof(sock_server)) == -1) {
		printf("error in binding socket");
		exit(1);
	}

	if (listen(s, 10) == -1) {
		printf("listen error");
		exit(1);
	}
	
	printf("Listening on port no %d\n",portnum);
	while(1){
		
		if ((client = accept(s, (struct sockaddr *) &sock_client, &slen)) == -1) {
			printf("accept error");
			exit(1);
		}
		
		if (recv(client, msg, BUFLEN, 0) == -1) {
			printf("recv error");
			exit(1);
		}
		
		//Parse the message into tokens and look for command "FOUND" or "NOT FOUND"
		command = strtok(msg, ":");
		if (strcmp(command, "FOUND") == 0) {
				
				key = strtok(NULL,":");
				value = strtok(NULL,":");
				printf("\nfound %s:%s",key,value);
				break;

		}
		else if(strcmp(command, "NOTFOUND") == 0){
							
				printf("The user provided key was not found\n ");	
				break;
		}
		
		else{
			
			printf("Unknown command to the client server");}
			break;
		}
	close(s);
}

/*
 * Main Function
 * Author : msivane Manikandan Sivanesan
 */
int main(int argc, char *argv[]) {
	struct sockaddr_in sr;
	int s, i, slen = sizeof(sr);
	char buf[BUFLEN];
	struct hostent *hent;
	char *command;
	int portnum;
	char msg[BUFLEN];
	
	if (argc != 3) {
		printf("Correct syntax: %s <portnum> <message>", argv[0]);
		return 1;
	}

	
	portnum = atoi(argv[1]);
	strcpy(buf, argv[2]);
	strcpy(msg,buf);
	
	hent = gethostbyname("localhost");
	if(hent == NULL)
	{	printf("gethostbyname failed ");
		exit(1);
	}
	
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {}

	memset((char *) &sr, 0, sizeof(sr));
	sr.sin_family = AF_INET;
	sr.sin_port = htons(portnum);
	sr.sin_addr = *(struct in_addr*)(hent ->h_addr_list[0]);

	
	if (connect(s, (struct sockaddr *) &sr, sizeof(sr)) == -1) {
		printf("connect error");
		exit(1);
	}
	
	//RANDOM port number generation
	int port = ran_gen_pno();
	char port_str[10];
	
	command = strtok(msg,":");
		
	if (strcmp(command, "GET") == 0){
		sprintf(port_str,":%d",port);
		strcat(buf,port_str);
		Printf("Get Message: %s\n",buf);
	}

	/* send a message to the server PORT on machine HOST */
	if (send(s, buf, BUFLEN, 0) == -1) {
		printf("send error");
		exit(1);
	}
	
	//Server up at the client side once the GET COMMAND is issued
	Printf("Sending message %s\n",buf);	
	if (strcmp(command, "GET") == 0){
		client_server_listen(port);
	}
	close(s);
	return 0;
}

