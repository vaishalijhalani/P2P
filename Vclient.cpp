#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <sys/time.h>
#include <set>
#include <openssl/sha.h>
#include <chrono> 
#include <fstream> 

using namespace std; 


struct IP_Info
{
	char thread_ip[1024][1024];
	int no_peers;
};
struct IP_SHA1
{
    char sha1[10000][1024];
    int no_entry;
};
typedef struct client_function_data { 
   char *buffer;
   char * ip_to_send;
} client_data_t; 

#define SEED_PORT 10000
#define GLOBAL_PORT 20000
int MY_PORT;
char * MY_IP;
struct IP_Info *peers_ip= (IP_Info *)malloc(sizeof(IP_Info));
struct IP_SHA1 *ip_sha1 = (IP_SHA1 *)malloc(sizeof(IP_SHA1));
//char * clientip = (char *) malloc(1024*sizeof(char));
set<int> random_list;
//map <char*, map<char*, int> > ip_sha1;
//vector<char *> ip_sha1;

void * client_function(void *threadarg)
{
	int sock;
    struct sockaddr_in address; 
    int valread; 
    struct sockaddr_in serv_addr;  
    char dummy_buffer[10]={0};
    client_data_t *connection_data;
    connection_data = (client_data_t *)threadarg;


    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        printf("\n Socket creation error \n"); 

    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, 
    	sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }

    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = inet_addr(MY_IP); 
    address.sin_port = htons(MY_PORT); 
       
    // Forcefully attaching socket to the port 
    if (bind(sock, (struct sockaddr *)&address, sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
     
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(GLOBAL_PORT);
	//cout << "Peer id in client function: "<< peers->thread_ip[i] << endl;
    
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, connection_data->ip_to_send, &serv_addr.sin_addr)<=0)
        printf("\nInvalid address/ Address not supported \n"); 
    

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        //exit(EXIT_FAILURE);
    }
    //cout << "client function " << buffer << endl;
    //write to random IP in random list
    int retvalwr = write(sock, connection_data->buffer, 1024);
    read(sock, dummy_buffer, sizeof(dummy_buffer));
    cout << "Sent\n";
    //close this socket
    close(sock);
    free(threadarg); 
    pthread_exit(NULL); 	
}

void * clients_server_function( void *)
{
    int seed_fd, new_socket; 
    struct sockaddr_in seed_addr, client_addr; 
    int opt = 1, i=0; 
    int addrlen = sizeof(seed_addr); 
    char buffer[1024] = {0}; 
    char dummy_buffer[10]= {0};

    // Creating socket file descriptor 
    if ((seed_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    }

    // Forcefully attaching socket to the port 
    if (setsockopt(seed_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }
    seed_addr.sin_family = AF_INET; 
    seed_addr.sin_addr.s_addr = inet_addr(MY_IP); 
    seed_addr.sin_port = htons(GLOBAL_PORT); 

    // Forcefully attaching socket to the port 
    if (bind(seed_fd, (struct sockaddr *)&seed_addr, sizeof(seed_addr))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(seed_fd, 1024) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    }

    while(1)
    {
        if ((new_socket = accept(seed_fd, (struct sockaddr *)&seed_addr,  
                        (socklen_t*)&addrlen))<0) 
        { 
            perror("accept"); 
            exit(EXIT_FAILURE); 
        } 

        cout << "New client connected: " << new_socket << endl;
        int retval = read(new_socket, buffer, sizeof(buffer));
        if(retval < 0)
        	cout << "Read failed\n";
        write(new_socket, dummy_buffer, sizeof(dummy_buffer));
        

        cout << "In client_server function read: " << buffer << endl;
        socklen_t addr_size = sizeof(struct sockaddr_in);
        int res = getpeername(new_socket, (struct sockaddr *)&client_addr, &addr_size);
		char *clientip = new char[16];
        strcpy(clientip, inet_ntoa(client_addr.sin_addr));
        close(new_socket);

        size_t length = strlen(buffer);
        unsigned char result[SHA_DIGEST_LENGTH*2];
        //char* buffer1 = new char[SHA_DIGEST_LENGTH];
        char buffer1[SHA_DIGEST_LENGTH*2];
        SHA1((unsigned char*)buffer, length, result);
        for(i = 0; i < SHA_DIGEST_LENGTH; i++)
            snprintf(buffer1+i*2, 3, "%02x", result[i]);

        cout << "SHA1: " << buffer1 << endl; 
        
        int found=0;
        for(int i=0; i<ip_sha1->no_entry; i++)
            if(strcmp(ip_sha1->sha1[i],buffer1)==0)
                found=1;

        printf("ip_sha1->sha1[i]: %s\n", ip_sha1->sha1[--i]);
        printf("buffer1: %s\n", buffer1);


        if(found==0)
        {
            strcpy(ip_sha1->sha1[ip_sha1->no_entry], buffer1);
            //printf("What's stored?: %s\n", ip_sha1->sha1[ip_sha1->no_entry]);
            ++(ip_sha1->no_entry);

            char msg_store[1024];
            strcpy(msg_store, buffer);
            strcat(msg_store, " : RECEIVED FROM ");
            strcat(msg_store, clientip);

            cout << msg_store << endl;


        	for (std::set<int>::iterator it = random_list.begin(); it != random_list.end(); ++it)
    		{
			    printf("In client_server_function: peers[*it]: %s\n", peers_ip->thread_ip[*it]);
			    printf("In client_server_function: clientip: %s\n", clientip);

			    if(strcmp(peers_ip->thread_ip[*it], clientip))
				{
                    pthread_t thread_id;
                    client_data_t *cdata = (client_data_t *) malloc(sizeof(client_data_t));
                    cdata->buffer = buffer;
                    cdata->ip_to_send = peers_ip->thread_ip[*it];
                    int rc = pthread_create(&thread_id, NULL, client_function, (void *) cdata); 
                    if (rc) 
                    { 
                        printf("ERR; pthread_create() ret = %d\n", rc); 
                        exit(-1); 
                    } 
                    pthread_join(thread_id, NULL);
                    //client_function(buffer, peers_ip->thread_ip[*it]);
				}
			}
		}
	}
    //return NULL;
}
   
int main(int argc, char const *argv[]) 
{ 
    struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    list< char *> peers_list;
    char buffer[1024] = {0}; 
    char dummy_buffer[10]={0};
    ip_sha1->no_entry = 0;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    }

    int opt = 1;

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 

    MY_IP = (char *)argv[1];
    MY_PORT = atoi(argv[2]);

    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = inet_addr(MY_IP); 
    address.sin_port = htons(MY_PORT); 
       
    // Forcefully attaching socket to the port 
    if (bind(sock, (struct sockaddr *)&address, sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(SEED_PORT); 
       
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
 
    long int num_peers;
    valread = read(sock , &num_peers, sizeof(num_peers)); 
    if(valread<=0)
        printf("READ failed1\n");
    printf("Number of peers: %ld\n", num_peers);

    peers_ip->no_peers = num_peers;
    for(int j=0; j<num_peers; j++)
    {
    	int i = 0;
        valread = read(sock , buffer, sizeof(buffer));
        cout << "valread " << valread << endl;
        write(sock, dummy_buffer, sizeof(dummy_buffer));
        if(valread<=0)
            printf("READ failed2\n");
        cout <<"Peer "<<j<<": "<< buffer << endl;
        for(i = 0;i<strlen(buffer);i++)
        	peers_ip->thread_ip[j][i] = buffer[i];
        peers_ip->thread_ip[j][i] = '\0';
        //cout <<"Peer "<<j<<": "<< peers_ip->thread_ip[j] << endl;
    }
    close(sock);

    //choose random ips from the ip list
    if(peers_ip->no_peers <= 4)
    {
    	for(int i=0;i<peers_ip->no_peers;i++)
    		{
    			random_list.insert(i);
    			//cout << "Kuch bhi "<<i << endl;
    		}
    }
    else
    {
		while(random_list.size()<=4)
		{
    		int index = rand()%(peers_ip->no_peers);
    		random_list.insert(index);
    	}
    }

    pthread_t client_function_thread, clients_server_function_thread;
    pthread_create(&clients_server_function_thread, NULL, clients_server_function, NULL);
   
    while(true)
    {
        char random_num[10];
    	bzero(buffer, sizeof(buffer));
		struct timeval tv;
		gettimeofday(&tv, NULL);
		unsigned long long secondsSinceEpoch = (unsigned long long)(tv.tv_sec);
		snprintf(buffer, sizeof(buffer), "%lld", secondsSinceEpoch);
		strcat(buffer,":");
		strcat(buffer, MY_IP);
		strcat(buffer, " pays ");
        snprintf(random_num, sizeof(random_num), "%d", (rand()%100));
        strcat(buffer, random_num);
        strcat(buffer, " BTC to ");
		int index = rand()%(peers_ip->no_peers);
		strcat(buffer, peers_ip->thread_ip[index]);

        cout << "Before SHA1: " << buffer << endl;

        size_t length = strlen(buffer);
        unsigned char result[SHA_DIGEST_LENGTH*2];
        char buffer1[SHA_DIGEST_LENGTH];
        SHA1((unsigned char*)buffer, length, result);
        for(int i = 0; i < SHA_DIGEST_LENGTH; i++)
            snprintf(buffer1+i*2, 3, "%02x", result[i]);
        
        cout << "SHA1: " << buffer1 << endl;
        cout << "Before sending: " << buffer << endl;
        strcpy(ip_sha1->sha1[ip_sha1->no_entry], buffer1);
        ++(ip_sha1->no_entry);

        for (std::set<int>::iterator it = random_list.begin(); it != random_list.end(); ++it)
        {
            pthread_t thread_id;
            client_data_t *cdata = (client_data_t *) malloc(sizeof(client_data_t));
            cdata->buffer = buffer;
            cdata->ip_to_send = peers_ip->thread_ip[*it];
            int rc = pthread_create(&thread_id, NULL, client_function, (void *) cdata); 
            if (rc) 
            { 
                printf("ERR; pthread_create() ret = %d\n", rc); 
                exit(-1); 
            } 
            pthread_join(thread_id, NULL);
            //client_function(buffer, peers_ip->thread_ip[*it]);
        }
		sleep(5);
    }

    pthread_join(clients_server_function_thread, NULL);
    return 0; 
}