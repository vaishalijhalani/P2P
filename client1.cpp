#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <sys/time.h>
#include <chrono>  

pthread_mutex_t mutex;

struct IP_Info
{
	char thread_ip[1024][1024];
	int no_peers;
};

#define SEED_PORT 10000
#define GLOBAL_PORT 20000
int MY_PORT;
char * MY_IP;

using namespace std;

void * clients_server_function( void *)
{
    int seed_fd, new_socket; 
    struct sockaddr_in seed_addr, client_addr; 
    int opt = 1, i=0; 
    int addrlen = sizeof(seed_addr); 
    char buffer[1024] = {0}; 
    char dummy_buffer[10]= {0};
    //char *hello = "Hello from server";

    // Creating socket file descriptor 
    if ((seed_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    }

    // Forcefully attaching socket to the port 8080 
    if (setsockopt(seed_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }
    seed_addr.sin_family = AF_INET; 
    seed_addr.sin_addr.s_addr = inet_addr(MY_IP); 
    seed_addr.sin_port = htons(GLOBAL_PORT); 
    //printf("Global_PORT: %d\n", GLOBAL_PORT);
    // fflush(stdout) ;

    // Forcefully attaching socket to the port 8080 
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
    }
    return NULL;
}

void * client_function(void * peer_ip)
{


    struct IP_Info *peers  = (IP_Info *)malloc(sizeof(IP_Info));
    peers = (IP_Info *)peer_ip;
    char buffer[1024] = {0};
    set<int> random_list;
    int sock[peers->no_peers];

    //cout << "Peer id in client function: "<< peers->no_peers << endl;

    //choose random ips from the ip list***************************************************
    if(peers->no_peers <= 4)
    {
    	for(int i=0;i<peers->no_peers;i++)
    		random_list.insert(i);
    }

    else
    {

		while(random_list.size()!=4)
		{
    		int index = rand()%(peers->no_peers);
    		random_list.insert(index);
    	}

    }

    for (std::set<int>::iterator it = random_list.begin(); it != random_list.end(); ++it)
    {

		    struct sockaddr_in address; 
		    int valread; 
		    struct sockaddr_in serv_addr;  
		    char dummy_buffer[10]={0};
		    int no_sock = 0;

		    if ((sock[no_sock] = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
		    {
		        printf("\n Socket creation error \n"); 
		    }

		    int opt = 1;

		    if (setsockopt(sock[no_sock], SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, 
		    	sizeof(opt))) 
		    { 
		        perror("setsockopt"); 
		        exit(EXIT_FAILURE); 
		    }

		    address.sin_family = AF_INET; 
		    address.sin_addr.s_addr = inet_addr(MY_IP); 
		    address.sin_port = htons(MY_PORT); 
		       
		    // Forcefully attaching socket to the port 8080 
		    if (bind(sock[no_sock], (struct sockaddr *)&address, sizeof(address))<0) 
		    { 
		        perror("bind failed"); 
		        exit(EXIT_FAILURE); 
		    } 
		     
		    memset(&serv_addr, '0', sizeof(serv_addr)); 
		   
		    serv_addr.sin_family = AF_INET; 
		    serv_addr.sin_port = htons(GLOBAL_PORT);
			//cout << "Peer id in client function: "<< peers->thread_ip[i] << endl;
		    
		    // Convert IPv4 and IPv6 addresses from text to binary form 
		    if(inet_pton(AF_INET, peers->thread_ip[*it], &serv_addr.sin_addr)<=0)  
		    { 
		        printf("\nInvalid address/ Address not supported \n"); 
		    } 

		    if (connect(sock[no_sock], (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
		    { 
		        printf("\nConnection Failed \n"); 
		        //exit(EXIT_FAILURE);
		    }

		    no_sock++;

	}

    while(true)
    {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		unsigned long long secondsSinceEpoch = (unsigned long long)(tv.tv_sec);
		snprintf(buffer, sizeof(buffer), "%lld", secondsSinceEpoch);
		strcat(buffer,":");
		strcat(buffer, MY_IP);
		strcat(buffer, " pays <N> BTC to ");
		int i = 0;

		for (std::set<int>::iterator it = random_list.begin(); it != random_list.end(); ++it)
    	{
			char buffer1[1024];
			strcat(buffer1, peers->thread_ip[*it]);
			write(sock[i++], buffer1, sizeof(buffer1));
		}
		
		std::this_thread::sleep_for (std::chrono::seconds(5));

    }


}





   
int main(int argc, char const *argv[]) 
{ 
    struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    list< char *> peers_list;
    char buffer[1024] = {0}; 
    char dummy_buffer[10]={0};
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
       
    // Forcefully attaching socket to the port 8080 
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
    struct IP_Info *peers_ip = (IP_Info *)malloc(sizeof(IP_Info));
    valread = read(sock , &num_peers, sizeof(num_peers)); 
    if(valread<=0)
    {
        printf("READ failed1\n");
    }
    printf("%ld\n", num_peers);

    peers_ip->no_peers = num_peers;
    //char thread_args[num_peers][1024];
    for(int j=0; j<num_peers; j++)
    {
    	int i = 0;
        bzero(buffer, sizeof(buffer));
        valread = read(sock , buffer, sizeof(buffer));
        write(sock, dummy_buffer, sizeof(dummy_buffer));
        if(valread<=0)
        {
            printf("READ failed2\n");
        }
        //cout <<"Peer "<<j<<": "<< buffer << endl;
        for(i = 0;i<strlen(buffer);i++)
        	peers_ip->thread_ip[j][i] = buffer[i];
        peers_ip->thread_ip[j][i] = '\0';
        //cout <<"Peer "<<j<<": "<< peers_ip->thread_ip[j] << endl;

    }
    close(sock);

    pthread_t client_function_thread, clients_server_function_thread;
    pthread_create(&clients_server_function_thread, NULL, clients_server_function, NULL);

    //cout <<"Before thread: "<< thread_args[j] << endl;
    pthread_create(&client_function_thread, NULL, client_function, (void *)peers_ip);
    
    pthread_join(clients_server_function_thread, NULL);
    pthread_join(client_function_thread, NULL);
    return 0; 
} 