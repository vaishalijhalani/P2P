#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <bits/stdc++.h>

pthread_mutex_t mutex;

#define SEED_PORT 10000
#define GLOBAL_PORT 20000
int MY_PORT;
char * MY_IP;

using namespace std;

vector<char *> peers_list;

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

    //peers.push_back(SEED_IP);
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

void * client_function(void *)
{

	for(int i = 0; i < peers_list.size(); i++)  
        // Convert IPv4 and IPv6 addresses from text to binary form 
        cout << peers_list[i] << endl;
    struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    char buffer[1024] = {0}; 
    char dummy_buffer[10]={0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        printf("\n Socket creation error \n"); 
    }
    int opt = 1;

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }

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
    serv_addr.sin_port = htons(GLOBAL_PORT);

    pthread_mutex_lock(&mutex);
    for(int i = 0; i < peers_list.size(); i++)
    {    
        // Convert IPv4 and IPv6 addresses from text to binary form 
        if(inet_pton(AF_INET, peers_list[i], &serv_addr.sin_addr)<=0)  
        { 
            printf("\nInvalid address/ Address not supported \n"); 
        } 

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        { 
            printf("\nConnection Failed \n"); 
            //exit(EXIT_FAILURE);
        }

    }
    pthread_mutex_unlock(&mutex);

}


   
int main(int argc, char const *argv[]) 
{ 
    struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    //char *hello = "Hello from client"; 
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
    // write(sock , hello , strlen(hello)); 
    // printf("Hello message sent\n"); 
    long int num_peers;
    valread = read( sock , &num_peers, sizeof(num_peers)); 
    if(valread<=0)
    {
        printf("READ failed1\n");
    }
    printf("%ld\n", num_peers);
    // fflush(stdout);
    for(int j=0; j<num_peers; j++)
    {
        //bzero(buffer, sizeof(buffer));
        char *clientip = new char[16];
        valread = read(sock ,buffer, 1024);
        write(sock, dummy_buffer, sizeof(dummy_buffer));
        if(valread<=0)
        {
            printf("READ failed2\n");
        }
        cout <<"Peer "<<j<<": "<< buffer << endl;
        strcpy(clientip, buffer);
        peers_list.push_back(clientip);
        for(int i=0; i<num_peers;i++)
        {
        	cout << "peers_list[j]: "<< peers_list[i]<< endl;
        }

    }
    close(sock);

    pthread_t client_function_thread, clients_server_function_thread;

    pthread_create(&client_function_thread, NULL, client_function, NULL);
    pthread_create(&clients_server_function_thread, NULL, clients_server_function, NULL);
    pthread_join(clients_server_function_thread, NULL);
    pthread_join(client_function_thread, NULL);
    return 0; 
} 