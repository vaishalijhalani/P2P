#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <pthread.h>

#define SEED_PORT 10000 
#define SEED_IP "127.0.0.1"
#define GLOBAL_PORT 20000

using namespace std;

vector<char *> peers;

void * seed_function(void* )
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
    seed_addr.sin_addr.s_addr = inet_addr(SEED_IP); 
    seed_addr.sin_port = htons( SEED_PORT ); 
    // printf("SEED_PORT: %d\n", SEED_PORT);
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

    peers.push_back(SEED_IP);
    while(1)
    {
        if ((new_socket = accept(seed_fd, (struct sockaddr *)&seed_addr,  
                        (socklen_t*)&addrlen))<0) 
        { 
            perror("accept"); 
            exit(EXIT_FAILURE); 
        } 

        socklen_t addr_size = sizeof(struct sockaddr_in);
        int res = getpeername(new_socket, (struct sockaddr *)&client_addr, &addr_size);
        char *clientip = new char[16];

        /********************************************************************
        //if final system crashes, then change strcpy() with normal for() loop
        ********************************************************************/

        long int num_peers = peers.size();
        write(new_socket, &num_peers , sizeof(num_peers));

        for(int j=0; j<num_peers; j++)
        {
            strcpy(buffer, peers[j]);
            int retval = write(new_socket , buffer , strlen(buffer));
            read(new_socket, dummy_buffer, sizeof(dummy_buffer));
            //cout << retval << endl;
            if(retval <= 0)

            	
                cout << retval << " " << "write failed\n";
        }

        strcpy(clientip, inet_ntoa(client_addr.sin_addr));
        printf("New ClientIP: %s\n", inet_ntoa(client_addr.sin_addr));
        peers.push_back(clientip);
        i++;

   
        close(new_socket);
        //close(seed_fd);
    }


    return NULL;
}

void * client_function( void *)
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

    // Forcefully attaching socket to the port 8080 
    if (setsockopt(seed_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }
    seed_addr.sin_family = AF_INET; 
    seed_addr.sin_addr.s_addr = inet_addr(SEED_IP); 
    seed_addr.sin_port = htons(GLOBAL_PORT); 
    printf("Global_PORT: %d\n", GLOBAL_PORT);
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
    //return NULL;
}


int main(int argc, char const *argv[]) 
{ 
    pthread_t thread_seed_function, thread_client_function;

    pthread_create(&thread_seed_function, NULL, seed_function, NULL);
    pthread_create(&thread_client_function, NULL, client_function, NULL);

    pthread_join(thread_seed_function, NULL);
    pthread_join(thread_client_function, NULL);

    return 0; 
} 