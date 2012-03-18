/* Group:
 * 	msivane Manikandan Sivanesan
 * 	anatara Arun Natarajan
 * 	bsundar Balaji Sundaravel
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <math.h>
#include <errno.h>

#define BUFLEN 512

#define MSGSIZE 30

//#define DEBUG

#ifdef DEBUG
#define Printf(...) printf(__VA_ARGS__);
#else
#define Printf(...) 
#endif


int TOTAL_NODES;		// Total Number of nodes present in the chord
int START_MSG_RCVD = 0; // START MSG RECEIVED FLAG
int PUT_MSG_RCVD = 0;	// PUT MSG RECEIVED FLAG
int GET_MSG_RCVD = 0;	// GET MSG RECEIVED FLAG
int FINGER_FOUND = 0;	// FLAG TO CHECK WHETHER ENTRY EXISTS IN FINGER TABLE
int KEY_FOUND = 0;		// FLAG TO CHECK FOR KEY EXIST 
int END_COUNT;			
int NODE_LIST_REMOVAL = 0;	//FLAG TO CHECK WHETHER NODELISTFILE REMOVED OR NOT
int portnum;			//PORT NUMBER
int NUM_LINES;			//NUMBER OF LINES IN THE NODE LIST
int ENTRY_NUM;			//NUMBER OF ENTRIES IN THE FINGER TABLE
char *successor, *predecessor, **s;
int *port_no,p_port,s_port; //Ports of predecessor ,successor


typedef struct finger_table{
	char *node;
	int port;
}ftab_t;

ftab_t *ft;

// random port number generator
void ran_gen_pno(){
	portnum = (rand()%(65535-1024)+1024);
	Printf("%d\n",portnum);
}

// Function to print the hash
void printhash(unsigned char h[16])
{	 int i;
	 for(i=0;i<16;i++)
		 printf("%02x",h[i]);
}

/*
 * hex_compare - Compares the two hash value in hex
 * Returns  1 if str1 is greater than str2 else returns 0
 * Author : bsundar Balaji Sundaravel
 */ 
int hex_compare(char str1[33],char str2[33]){
	int i = 0;
	while((str1[i]!='\0')&&(str2[i]!='\0')){
		unsigned int integer1, integer2;
		char s1[2],s2[2];
		s1[0] = str1[0];
		s2[0] = str2[0];
		s1[1] = '\0';
		s2[1] = '\0';
		sscanf(s1,"%X",&integer1);
		sscanf(s2,"%X",&integer2);
		if(integer1>integer2){
			return 1;
		}
		else if(integer1 < integer2){
			return 0;
		}
		i++;
	}
	return 0;
}
/*
 * hex_compare_eqg - Compares the two hash value in hex
 * Returns  1 if str1 is greater than str2  -1 or if str1 is lesser than str 2  and 0 if str1 is equal to str2
 */

int hex_compare_eqg(char str1[33],char str2[33]){
	int i = 0;
	while((str1[i]!='\0')&&(str2[i]!='\0')){
		unsigned int integer1, integer2;
		char s1[2],s2[2];
		s1[0] = str1[0];
		s2[0] = str2[0];
		s1[1] = '\0';
		s2[1] = '\0';
		sscanf(s1,"%X",&integer1);
		sscanf(s2,"%X",&integer2);
		if(integer1>integer2){
			return 1;
		}
		else if(integer1 < integer2){
			return -1;
		}
		i++;
	}
	return 0;
}

/*
 * hashinhex - Function to convert the hash value into hex format and stores in hex_hash
 * Author : bsundar Balaji Sundaravel
 */

void hashinhex(char *hash, char *hex_hash){
	
	hex_hash[32] = '\0';
	int j;
	
	for(j=0;j<16;j++){
		
		char str[10];
		int k;
		for(k=0;k<10;k++){
			str[k] = '#';
		}
		
		sprintf(str,"%02x",hash[j]);
		for(k=0;k<10;k++){
			if(str[k] == '#'){
				
				char sr[3];	
				sr[2] = '\0';
				sr[0] = str[k-3];
				sr[1] = str[k-2];
				if(j==0){
						hex_hash[0] = sr[0];
						hex_hash[1] = sr[1];
						hex_hash[2] = sr[2];
				}
				else{	
						strcat(hex_hash,sr);
				}
				break;
			}
		}
	}
	hex_hash[32] = '\0';
}
/*
 *  calculates the hash and stores in h
 */
void calculatehash(char *c, int len, char *h)
{	MD5(c,len, h);
}

/*
 * successor_seek - Finds the successor and predessor of a node
 * Author : bsundar Balaji Sundaravel
 */
int successor_seek(){
	int i;
	
	for(i=0;i<TOTAL_NODES;i++){
		s[i][32] = '\0';
	}
	
	//Opens the node list file in read mode
	FILE *file;
	file = fopen("nodelist","r");

	char *line;
	size_t size = 0;
	ssize_t read;
	
	i=0;
	
	//Reads each line in the file and finds the hash value of each entry
	while((read = getline(&line,&size,file)) != -1){
		int temp = atoi(line);
		port_no[i] = temp;
		char nh[20],h[16];
		sprintf(nh,"localhost%d",temp);
		
		//Calculates the hash value
		calculatehash(nh,strlen(nh),h);
		int j;
		for(j=0;j<16;j++){
			char str[10];
			int k;
			for(k=0;k<10;k++){
				str[k] = '#';
			}
			sprintf(str,"%02x",h[j]);
			for(k=0;k<10;k++){
				if(str[k] == '#'){
					char sr[3];	
					sr[2] = '\0';
					sr[0] = str[k-3];
					sr[1] = str[k-2];
					if(j==0){
						s[i][0] = sr[0];
						s[i][1] = sr[1];
						s[i][2] = sr[2];
					}
					else{	
						strcat(s[i],sr);
					}
					break;
				}
			}
		}
		s[i][32] = '\0';
		i++;
	}
	fclose(file);
	int j;
	Printf("The Strings are:\n");
	for(i=0;i<TOTAL_NODES;i++){
		Printf("%s\n",s[i]);
	}

	//sorting the hash values

	for(i=0;i<TOTAL_NODES;i++){
		for(j=i+1;j<TOTAL_NODES;j++){
			if(hex_compare(s[i],s[j])){
				char temp_str[33];
				temp_str[32] = '\0';
				int tem;
				for(tem=0;tem<32;tem++){
					temp_str[tem] =	s[i][tem];
					s[i][tem] = s[j][tem];
					s[j][tem] = temp_str[tem];
				}
				int t;
				t = port_no[i];
				port_no[i] = port_no[j];
				port_no[j] = t;
			}
		}
	}	
	
	Printf("the sorted hash value\n");
	for(i=0;i<TOTAL_NODES;i++){
		Printf("%s\n",s[i]);
		Printf("%d\n",port_no[i]);
	}
 	/*unsigned long int integer[TOTAL_NODES];
	for(i=0;i<TOTAL_NODES;i++){
		sscanf(s[i],"%X",&integer[i]);
		printf("%ld\n",integer[i]);
	}*/

	//determining the predecessor and successor
	for(i=0;i<TOTAL_NODES;i++){
		if(port_no[i] == portnum){
			
			//Assigning the predecessor & successor to refer itself if there is only one node along with the ports
			if(TOTAL_NODES == 1){			
				predecessor = successor = s[i];
				p_port = s_port = port_no[i];
			}
			
			//Asssigning the predecessor & successor to refer the other if there are two nodes
			else if(TOTAL_NODES == 2){
				if(i == 0){
					predecessor = successor = s[i+1];
					p_port = s_port = port_no[i+1];
				}
				else if(i == 1){
					predecessor = successor = s[i-1];
					p_port = s_port = port_no[i-1];
				}
			}
			
			//If there are more than two nodes the successor & predecessor are assigned based on the sorted hash values
			else{
				//The largest node   is assigned as predecessor  for the smallest  node  
				if(i == 0){
					successor = s[i+1];
					s_port = port_no[i+1];
					predecessor = s[TOTAL_NODES-1];
					p_port = port_no[TOTAL_NODES-1];
				}
				//Tha smallest node is assigned as the successor for the largest node
				else if(i == (TOTAL_NODES-1)){
					predecessor = s[i-1];
					p_port = port_no[i-1];
					successor = s[0];
					s_port = port_no[0];
				}
				
				else {
					predecessor = s[i-1];
					p_port = port_no[i-1];
					successor = s[i+1];
					s_port = port_no[i+1];
				}
			}
			break;
		}
	}

	Printf("predecessor: %s\n",predecessor);
	Printf("successor: %s\n",successor);
	

	return 0;
}

/*
 * initialize_ft - Initializing the finger table with the first entry as successor , the second entry as successor of successor and the other entries are based on  ith entry containing ith power of 2
 * Author : anatara Arun Natarajan
 */
int initialize_ft(){
	
	//Initialising the successor and predecessor of the each node
	if(successor_seek()!=0){
		printf("something wrong with successor_seek()\n");
	}
	
	
	int i=0;
	for(i=0;i<TOTAL_NODES;i++){
		if(port_no[i] == portnum){
			int j,count=0;
			
			if(i==(TOTAL_NODES-1)){
				i=-1;
			}
			for(j=i+1;count<ENTRY_NUM;j=j*2){
				//if (j==0) { j=1;}
				Printf("cnt %d  ent num %d j %d\n",count,ENTRY_NUM,j);
				if(j>=TOTAL_NODES){
					j = j/2;
					int k=1,l=j;
					while(k<=l){
						j = j+k;
						if(j>=TOTAL_NODES){
							j=0;
						}
						k++;
					}
				}
				
				//Finger entry containing the hash value of the node and the port no
				ft[count].node = s[j];
				ft[count].port = port_no[j];
				count++;
			}
			break;
		}
	}

	for(i=0;i<ENTRY_NUM;i++){
		Printf("node: %s\n",ft[i].node);
		Printf("port: %d\n",ft[i].port);
	}
		
	return 0;
}
/*
 * forwards message m to port
 */
void forward_message(int port, char *m)
{
		struct sockaddr_in sock_client;
		struct hostent *hent;
		int sc, i, slen = sizeof(sock_client);

		if ((sc = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			printf("socket creation failed ");
			exit(1);
		}

		hent = gethostbyname("localhost");
		if(hent == NULL)
		{	printf("gethostbyname failed ");
			exit(1);
		}

		memset((char *) &sock_client, 0, sizeof(sock_client));

		sock_client.sin_family = AF_INET;
		sock_client.sin_port = htons(port);
		sock_client.sin_addr = *(struct in_addr*)(hent ->h_addr_list[0]);

		if (connect(sc, (struct sockaddr *) &sock_client, slen) == -1) {
			
			printf("connect failed with error no %s, May be I am the last node existing.\n",strerror(errno));
			exit(1);
		}

		if (send(sc, m, BUFLEN, 0) == -1) {
			printf("send failed ");
			exit(1);
		}
		close(sc);
}

/*
 * nodelist_write - Creating a node list containing the port number of each node and forward the start message once it reaches the last node
 * Author : anatara Arun Natarajan
 * */
void nodelist_write(){
		
		FILE *file;
        char *line;
        size_t size = 0;
        ssize_t read;
	
		//If there is a no such file create a new node list file
        if((access("nodelist",F_OK))==-1){
                Printf("Creating file nodelist\n");
                file = fopen("nodelist","w");
                fprintf(file,"%d\n",portnum);
                fclose(file);

        }
        else{
                NUM_LINES = 0;
				
				
                file = fopen("nodelist","r");
                while((read = getline(&line,&size,file))!=-1){
                        //printf("%s\n",line);
                        NUM_LINES++;
                }
                
                if(line){
                        free(line);
                }
                Printf("The number of lines in the file %d\n",NUM_LINES);
                //Open the file and append the port number at the end of file
                if(NUM_LINES < TOTAL_NODES){
                        fclose(file);
                        file = fopen("nodelist","a");
                        fprintf(file,"%d\n",portnum);
                }
				fclose(file);
				
				//Once the total number of line reaches the total number of nodes then initialise the finger table of the last table
				if((NUM_LINES+1) == TOTAL_NODES){
			
					if(!initialize_ft()){
						Printf("Finger table successfully initialised\n");
						
						//Forward the start message to its successor
						forward_message(s_port,"START");
						Printf("Forwarding START msg to port %d\n",s_port);
						START_MSG_RCVD = 1;
					}
					else
					Printf("Finger table not initialised\n");
				}
		}
}

/*
 * keyvaluelist_write - Create a database file at each node to store the key value pair given by the PUT command
 * Author : msivane Manikandan Sivanesan
 */
void keyvaluelist_write(char *key_value){
	
		FILE *file;
        char *line;
        size_t size = 0;
        ssize_t read;
        
        char keylist_name[15];
        
        sprintf(keylist_name,"keyvaluelist_%d",portnum);
		
		//If there is a no such file create a new keyvalue list file
        if((access(keylist_name,F_OK))==-1){
                Printf("Creating file keyvaluelist_%d\n",portnum);
                file = fopen(keylist_name,"w");
                fprintf(file,"%s\n",key_value);
                fclose(file);
        }
        //Append the key value pair at the end of file
        else{
                file = fopen(keylist_name,"a");
                fprintf(file,"%s\n",key_value);
                fclose(file);
        }
		
}

// Reads the keyvalue file to retrieve the keys and values stored in a given node for the QUERY command
// Author : anatara Arun Natarajan
void keyvaluelist_read(){
	
	FILE *file;
	char keylist_name[15];
	char *line;
	size_t size = 0;
	ssize_t read;
	char *temp;
		
	sprintf(keylist_name,"keyvaluelist_%d",portnum);
	//If there is no such file then no key is inserted in the given node
	if((access(keylist_name,F_OK))==-1){
		
		printf("No key inserted in the node\n");
	}
	//else open the file and read each entry
	else{
		file = fopen(keylist_name,"r");
	
		printf("Key \t\t\t\t Value\n");
		printf("-----------------------------------------\n");
		while((read = getline(&line,&size,file)) != -1){
		
			temp = strtok(line,":");
			printf("%s \t\t\t\t ",temp);
			temp = strtok(NULL,":");
			printf("%s\n",temp);
		}
		printf("-----------------------------------------\n");
		fclose(file);
	}
}

//Get the value for the given key
//Author : msivane Manikandan Sivanesan
int getvalue(char *key,char *value){
	
	FILE *file;
	char keylist_name[15];
	char *line;
	size_t size = 0;
	ssize_t read;
	char *temp;
	char *key_val;
	
		
	sprintf(keylist_name,"keyvaluelist_%d",portnum);
	
	//If there is no such file then no key is inserted in the given node
	if((access(keylist_name,F_OK))==-1){
		printf("No key inserted in the node\n");
	}

	else{
		file = fopen(keylist_name,"r");
		
		//reads each line and compares the given key with each key 
		while((read = getline(&line,&size,file)) != -1){
			temp = strtok(line,":");
			
			//Once the key found the copy its value in the provided value variable 
			if (strcmp(temp, key) == 0){
				Printf("Key value pair:%s\n",temp);
				key_val = strtok(NULL,":");
				//*len = strlen(key_val);
				printf("%s\n",temp);
				Printf("Value found %s",key_val);
				
				memcpy((char *)value,(char *)key_val,sizeof(key_val));
				KEY_FOUND = 1;
				break;
			}
		}
		
		fclose(file);
		// If key is not found then returns -1
		if(KEY_FOUND == 0){
			KEY_FOUND = 0;
			return -1;
		}
		//else returns 1
		else{
			KEY_FOUND = 0;
			return 1;
		}
		
	}
}

/* server_listen - server waits to execute the commands given by the server
 * Authors : bsundar Balaji Sundaravel anatara Arun Natarajan msivane Manikandan sivanesan
 */

void server_listen() {
	struct sockaddr_in sock_server, sock_client;

	srand(time(NULL));
	ran_gen_pno();//generating random port number
	
	int s, slen = sizeof(sock_client);
	char *command;
	char buf[BUFLEN];
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
	
	char nodehash_string[20],node_hash[16],node_hexhash[33];
	
	sprintf(nodehash_string,"localhost%d",portnum);
	
	//calculates the hash and stores it in node_hash
	calculatehash(nodehash_string, strlen(nodehash_string), node_hash);
	
	//converts the hash into hex format and stores it in the second arg
	hashinhex(node_hash,node_hexhash);
	
	// writing file nodelist
	nodelist_write();	
	
	printf("DHT node (");
	//printhash(node_hash);
	printf("): Listening on port number %d . . . \n", portnum);

	while (1) { /* quit only on END message */
		if ((client = accept(s, (struct sockaddr *) &sock_client, &slen)) == -1) {
			printf("accept error");
			exit(1);
		}

		if (recv(client, buf, BUFLEN, 0) == -1) {
			printf("recv error");
			exit(1);
		}
		
		//Parse the message into tokens
		command = strtok(buf, ":");
		if (strcmp(command, "END") == 0) {
				
				char keylistname[20];
				printf("END message received \n");
				sprintf(keylistname,"keyvaluelist_%d",portnum);
				 
				 //Remove the keyvaluelist vale if END message is received
				if( remove(keylistname) != 0 )
					perror( "Error deleting \'keylistfile\' file\n" );
				if( remove("nodelist") != 0 )
					perror( "Error deleting \'nodelist\' file, May be already deleted\n" );
				printf( "Tmp Files successfully deleted\n" );
				
				//Forwards the end message to its successor except the last node
				Printf("end count is %d \t s port %d\n",END_COUNT,s_port);
				if( END_COUNT > 0){
					forward_message(s_port,"END");
					END_COUNT--;
					exit(1);
		
				}
				//If it is last node in the system then close the connecton with the client
				else{     
					  //exit(1);                                      
                      close(client);
                      break;
				}
		
		}
		//Query the node for the list of key value pairs stored
		else if(strcmp(command, "QUERY") == 0){

				keyvaluelist_read();
		}
		
		//PUT command stores the key value pair if the node is responsible to store the key value pair else forward to the approriate node using the finger table
		 
		else if (strcmp(command, "PUT") == 0) {
			
			Printf("%s\n",buf);
			char *key,*value;
			char key_hash[16];
            char key_hexhash[33];
            char keyvalue_buf[MSGSIZE];
            char keyvalue_msg[MSGSIZE];

			key = strtok(NULL,":");
			value = strtok(NULL,":");
			Printf("Key : %s \t Value :%s\n",key,value);
			
			//Calculates the hash value of key
			calculatehash(key,strlen(key),key_hash);
			//Converts the key into hex format and stores in the second argument
            hashinhex(key_hash,key_hexhash);

			sprintf(keyvalue_buf,"%s:%s",key,value);
			sprintf(keyvalue_msg,"PUT:%s:%s",key,value);
			printhash(key_hash);
						
			//Store the key value pair if there is only one node
			if(TOTAL_NODES == 1){
				keyvaluelist_write(keyvalue_buf);
				printf("(key:value) =(%s[ %s ] : %s) inserted\n",key,key_hexhash,value);
			}
			
			//Stores the key value pair if the node hash is greater than or equal to the key hash else forward it to the other node
			else if(TOTAL_NODES == 2){
				
				/*Comparing with node in the chord and if the nodehash is equal to the keyhash then insert the key in the node*/
				if(hex_compare_eqg(node_hexhash,key_hexhash) == 0){
						
						keyvaluelist_write(keyvalue_buf);
						PUT_MSG_RCVD = 0;
						printf("(key:value) =(%s[ %s ] : %s) inserted\n",key,key_hexhash,value);
						
				}
					
				else if( ((hex_compare_eqg(node_hexhash,key_hexhash)) == 1 && (hex_compare_eqg(successor,key_hexhash) == 1)) || (PUT_MSG_RCVD == 1) ){
					keyvaluelist_write(keyvalue_buf);
					PUT_MSG_RCVD = 0;
					printf("(key:value) =(%s[ %s ] : %s) inserted\n",key,key_hexhash,value);
				}
				else{
									
					forward_message(s_port,keyvalue_msg);
					PUT_MSG_RCVD = 1;
					printf("PUT for %s [%s] was forwarded to localhost : %d\n",key,key_hexhash,s_port);
				}
			}
			else{
					/*Comparing with node in the chord and if the nodehash is equal to the keyhash then insert the key in the node*/
					if(hex_compare_eqg(node_hexhash,key_hexhash) == 0){
						
						keyvaluelist_write(keyvalue_buf);
						PUT_MSG_RCVD = 0;
						printf("(key:value) =(%s[ %s ] : %s) inserted\n",key,key_hexhash,value);
						
					}
							
					/*Comparing with node in the chord and if the nodehash is greater than keyahash and the key hash greater than the predecessor
					This also captures special case if the nodehash is the smallest node in the chord*/
					
					else if( ((hex_compare_eqg(node_hexhash,key_hexhash) == 1) ) && 
					  ( ( hex_compare_eqg(predecessor,key_hexhash) == -1) || (hex_compare_eqg(predecessor,node_hexhash) == 1) )) {
						  
						keyvaluelist_write(keyvalue_buf);
						PUT_MSG_RCVD = 0;
						printf("(key:value) =(%s[ %s ] : %s) inserted\n",key,key_hexhash,value);
					}
								
					//Comparing with node in the chord and if the keyhash is greater than the nodehash and smaller than the successor then forward to the successor
					else if( (hex_compare_eqg(node_hexhash,key_hexhash) == -1) && 
						   ( (hex_compare_eqg(successor,key_hexhash) == 1) || (hex_compare(node_hexhash,successor) == 1)) ){
						printf("PUT for %s [%s] was forwarded to localhost : %d\n",key,key_hexhash,s_port);
						forward_message(s_port,keyvalue_msg);
					}									
					
					//scanning the finger table
					else{
			
						int i;
						for(i=0;i<ENTRY_NUM;i++){
							//Compares the key hash with the node hash in the finger table and forward to the predecessor of the first node whose hash is greater than the key hash
							if( (i>=1) && (hex_compare(ft[i].node,key_hexhash) == 1) ){
								FINGER_FOUND = 1;
								forward_message(ft[i-1].port,keyvalue_msg);
								printf("PUT for %s [%s] was forwarded to localhost : %d\n",key,key_hexhash,ft[i-1].port);
								break;
							}
							//Compares the key hash with the node hash in the finger table and if they are equal then forward to the node in the finger entry
							else if((i>=1) && (hex_compare(ft[i].node,key_hexhash) == 0)){
								
								FINGER_FOUND = 1;
								forward_message(ft[i].port,keyvalue_msg);
								printf("PUT for %s [%s] was forwarded to localhost : %d\n",key,key_hexhash,ft[i].port);
								break;
							}
						}
						
						//If you dont find any entry greater than key hash then forward to the successor
						if(FINGER_FOUND == 0){
							forward_message(s_port,keyvalue_msg);
							printf("PUT for %s [%s] was forwarded to localhost : %d\n",key,key_hexhash,s_port);
						}
						FINGER_FOUND = 0;
					}
			}	
		}
		
		//GET command finds the node responsible for the key and retrieves the value
		else if(strcmp(command, "GET") == 0){
			
			char *key;
			char key_hash[16];
            char key_hexhash[33];
            char *value = (char *)malloc(50);
            char keyvalue_msg[MSGSIZE];
            char *port_str;
            int client_port;
            int len=0;
            
            
            key = strtok(NULL,":");
            calculatehash(key,strlen(key),key_hash);
            hashinhex(key_hash,key_hexhash);
            port_str = strtok(NULL,":");
            client_port = atoi(port_str);
            sprintf(keyvalue_msg,"GET:%s:%d",key,client_port);
            
            //Retrieves the value and forward it to the client if FOUND else forward NOT FOUND message to the client
            if(TOTAL_NODES == 1){
			
					if(getvalue(key,value) == 1){
						//value[len] = '\0';
						sprintf(keyvalue_msg,"FOUND:%s:%s:",key,value);
						printf("Found key value pair %s:%s\n",key,value);
						forward_message(client_port,keyvalue_msg);
					}
					else{
						forward_message(client_port,"NOTFOUND");
						Printf("Value for key %s was not found\n",key);
					}
			}
			
			
			else if(TOTAL_NODES == 2){
				
				/*Comparing with node in the chord and if the nodehash is equal to the keyhash then insert the key in the node*/
				if(hex_compare_eqg(node_hexhash,key_hexhash) == 0){
						
					//Retrieves the value and forward it to the client if FOUND else forward NOT FOUND message to the client
					if(getvalue(key,value) == 1){
						sprintf(keyvalue_msg,"FOUND:%s:%s:",key,value);
						printf("Found key value pair %s:%s\n",key,value);
						forward_message(client_port,keyvalue_msg);
						GET_MSG_RCVD = 0;
					}
					else{
						forward_message(client_port,"NOTFOUND");
						Printf("Value for key %s was not found\n",key);
					}
						
				}
			
				//checks if the given node is responsible for the key hash
				else if( ((hex_compare_eqg(node_hexhash,key_hexhash) == 1) && (hex_compare_eqg(successor,key_hexhash) == 1)) || (GET_MSG_RCVD == 1) ){
					
					//Retrieves the value and forward it to the client if FOUND else forward NOT FOUND message to the client
					if(getvalue(key,value) == 1){
						sprintf(keyvalue_msg,"FOUND:%s:%s:",key,value);
						printf("Found key value pair %s:%s\n",key,value);
						forward_message(client_port,keyvalue_msg);
						GET_MSG_RCVD = 0;
					}
					else{
						forward_message(client_port,"NOTFOUND");
						Printf("Value for key %s was not found\n",key);
					}
				}
				
				//forward the GET message to the successor if node is reponsible for the key hash
				else{
									
					forward_message(s_port,keyvalue_msg);
					GET_MSG_RCVD = 1;
					printf("GET for %s [%s] was forwarded to localhost : %d\n",key,key_hexhash,s_port);
				}
			}
			else{
					/*Comparing with node in the chord and if the nodehash is equal to the keyhash then insert the key in the node*/
					if(hex_compare_eqg(node_hexhash,key_hexhash) == 0){
						
						//Retrieves the value and forward it to the client if FOUND else forward NOT FOUND message to the client
						if(getvalue(key,value) == 1){
							sprintf(keyvalue_msg,"FOUND:%s:%s:",key,value);
							printf("Found key value pair %s:%s\n",key,value);
							forward_message(client_port,keyvalue_msg);
							GET_MSG_RCVD = 0;
						}
						else{
							forward_message(client_port,"NOTFOUND");
							Printf("Value for key %s was not found\n",key);
						}
						
					}
					
					/*Comparing with node in the chord and if the nodehash is greater than keyahash and the key hash greater than the predecessor
					This also captures special case if the nodehash is the smallest node in the chord*/
					
					else if( ((hex_compare_eqg(node_hexhash,key_hexhash) == 1) ) && 
					  ( ( hex_compare_eqg(predecessor,key_hexhash) == -1) || (hex_compare_eqg(predecessor,node_hexhash) == 1) )) {
							
							if(getvalue(key,value) == 1){
								sprintf(keyvalue_msg,"FOUND:%s:%s:",key,value);
								printf("Found key value pair %s:%s\n",key,value);
								forward_message(client_port,keyvalue_msg);
								
							}
							else{
								forward_message(client_port,"NOTFOUND");
								Printf("Value for key %s was not found\n",key);
							}
					
					}
								
					//Comparing with node in the chord and if the keyhash is greater than the nodehash and smaller than the successor then forward to the successor
					else if( (hex_compare_eqg(node_hexhash,key_hexhash) == -1) && 
						   ( (hex_compare_eqg(successor,key_hexhash) == 1) || (hex_compare(node_hexhash,successor) == 1)) ){
						printf("GET for %s [%s] was forwarded to localhost : %d\n",key,key_hexhash,s_port);
						forward_message(s_port,keyvalue_msg);
					}									
					
					//scanning the finger table
					else{
						
						Printf("Inside finger table scanning\n");
						int i;
						for(i=0;i<ENTRY_NUM;i++){
							
							if( (i>=1) && (hex_compare(ft[i].node,key_hexhash) == 1) ){
								FINGER_FOUND = 1;
								forward_message(ft[i-1].port,keyvalue_msg);
								printf("GET for %s [%s] was forwarded to localhost : %d\n",key,key_hexhash,ft[i-1].port);
								break;
							}
							
							//Compares the key hash with the node hash in the finger table and if they are equal then forward to the node in the finger entry
							else if((i>=1) && (hex_compare(ft[i].node,key_hexhash) == 0)){
								
								FINGER_FOUND = 1;
								forward_message(ft[i].port,keyvalue_msg);
								printf("GET for %s [%s] was forwarded to localhost : %d\n",key,key_hexhash,ft[i].port);
								break;
							}
						}
						if(FINGER_FOUND == 0){
							forward_message(s_port,keyvalue_msg);
							Printf("GET for %s [%s] was forwarded to localhost : %d\n",key,key_hexhash,s_port);
						}
						FINGER_FOUND = 0;
					}
				
			}
			free(value);				
		}
		
		//If START message is received only once then initialise the finger table and forward the START message to its successor
		else if (strcmp(command, "START") == 0){
			
			if(START_MSG_RCVD == 0){
				Printf("\nSTART MESSAGE RECEIVED\n");
				Printf("Finger table successfully initialised\n");
				if(!initialize_ft()){
					forward_message(s_port,"START");
					Printf("START message forwarded to port %d\n",s_port);
					START_MSG_RCVD = 1;
				}
			}
		}
	close(client);
	}

	close(s);
}

/* Main function
 * Author : bsundar Balaji Sundaravel
 */
int main(int argc, char *argv[]) {

	if (argc != 2) {
		printf("wrong number of arguments");
		return;
	}
	
	int i=0;
	TOTAL_NODES = atoi(argv[1]);
	END_COUNT = TOTAL_NODES-1;
	
	//Number of entries in the finger table
	ENTRY_NUM = (int)( (log((double)TOTAL_NODES))/(log(2)));
	Printf("Number of entries %d\n",ENTRY_NUM);
	
	port_no = malloc(TOTAL_NODES * sizeof(int));
	s = malloc(TOTAL_NODES * sizeof(char *));
	
	for(i=0;i<TOTAL_NODES;i++){
		s[i] = malloc(32 * sizeof(char));
	}
	
	ft = (ftab_t *)malloc(ENTRY_NUM * sizeof(ftab_t));
	
	//Start the server
	server_listen();
	
	return 0;
}

