#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <pthread.h>
#include <openssl/sha.h>
#include <fstream> 
#include <iostream>

#define SEED_PORT 10000 
#define SEED_IP "127.0.0.1"
#define GLOBAL_PORT 20000

using namespace std;

vector<char *> peers;
//map <char*, map<char*, int> > ip_sha1;
//vector<char *> ip_sha1;
struct IP_SHA1
{
    char sha1[10000][1024];
    int no_entry;
};
struct IP_SHA1 *ip_sha1 = (IP_SHA1 *)malloc(sizeof(IP_SHA1));

//ofstream myfile;

void * seed_function(void* )
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
    seed_addr.sin_addr.s_addr = inet_addr(SEED_IP); 
    seed_addr.sin_port = htons( SEED_PORT ); 

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
        strcpy(clientip, inet_ntoa(client_addr.sin_addr));

        long int num_peers = peers.size();
        write(new_socket, &num_peers , sizeof(num_peers));

        for(int j=0; j<num_peers; j++)
        {
            strcpy(buffer, peers[j]);
            int retval = write(new_socket , buffer , strlen(buffer));
            read(new_socket, dummy_buffer, sizeof(dummy_buffer));
            cout << retval << endl;
            if(retval <= 0)
                cout << retval << " " << "write failed\n";
        }

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
    set<int> random_list;

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
    seed_addr.sin_addr.s_addr = inet_addr(SEED_IP); 
    seed_addr.sin_port = htons(GLOBAL_PORT); 
    printf("Global_PORT: %d\n", GLOBAL_PORT);

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

        socklen_t addr_size = sizeof(struct sockaddr_in);
        int res = getpeername(new_socket, (struct sockaddr *)&client_addr, &addr_size);
        char *clientip = new char[16];
        strcpy(clientip, inet_ntoa(client_addr.sin_addr));

        cout << "New client connected: " << new_socket << endl;
        int retval = read(new_socket, buffer, sizeof(buffer));
        if(retval < 0)
        	cout << "Read failed\n";
        else 
        	cout << "After accept: " << buffer << endl;
        
        write(new_socket, dummy_buffer, sizeof(dummy_buffer));
        close(new_socket);

        size_t length = strlen(buffer);
        unsigned char result[SHA_DIGEST_LENGTH*2];
        char* buffer1 = new char[SHA_DIGEST_LENGTH];
        //char buffer1[SHA_DIGEST_LENGTH];
        SHA1((unsigned char*)buffer, length, result);
        for(i = 0; i < SHA_DIGEST_LENGTH; i++)
            snprintf(buffer1+i*2, 3, "%02x", result[i]);

        cout << "SHA1: " << buffer1 << endl;
        cout << "Client_IP: " << clientip << endl;

        int found=0;
        for(int i=0; i<ip_sha1->no_entry; i++)
        	if(strcmp(ip_sha1->sha1[i],buffer1)==0)
        		found=1;


        if(found==0)
        {
    		strcpy(ip_sha1->sha1[ip_sha1->no_entry], buffer1);
            ++(ip_sha1->no_entry);
    		
    		char msg_store[1024];
    		strcpy(msg_store, buffer);
    		strcat(msg_store, " : RECEIVED FROM ");
    		strcat(msg_store, clientip);

    		cout << msg_store << endl;

   //  		fstream myfile;
			// myfile.open ("example.txt", ios::out | ios::app);
			// if (myfile.is_open()) 
			// { /* ok, proceed with output */ 
			// 	printf("Opened file\n");
			// 	for(int i=0; i<strlen(buffer); i++)
			// 		myfile << buffer[i];
   //  			myfile << "\n";
   //  			myfile.close();
			// }
			// else
			// 	printf("Unable to open file\n");

			// FILE *fp;
			// fp= fopen("filename.txt", "a");
			// if(fp == NULL)
			// 	printf("Can't open file\n");
			// printf("Opened\n");
			// fputs(buffer1, fp);
			// printf("Before close\n");
			// fclose(fp);


    		printf("Here\n");


		    if(peers.size() <= 4)
		    {
		    	for(int i=0;i<peers.size();i++)
		    		random_list.insert(i);
		    }
		    else
		    {
				while(random_list.size()<=4)
				{
		    		int index = rand()%(peers.size());
		    		random_list.insert(index);
		    	}
		    }

	        for (std::set<int>::iterator it = random_list.begin(); it != random_list.end(); ++it)
	    	{
				int sock;
			    struct sockaddr_in address; 
			    int valread; 
			    struct sockaddr_in serv_addr;  
			    char dummy_buffer[10]={0};

			    if(strcmp(peers[*it], clientip) && strcmp(SEED_IP, peers[*it]))
			    {
				    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
				    {
				        printf("\n Socket creation error \n"); 
				    }

				    int opt = 1;

				    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, 
				    	sizeof(opt))) 
				    { 
				        perror("setsockopt"); 
				        exit(EXIT_FAILURE); 
				    }

				    address.sin_family = AF_INET; 
				    address.sin_addr.s_addr = inet_addr(SEED_IP); 
				    address.sin_port = htons(SEED_PORT); 
				       
				    // Forcefully attaching socket to the port 8080 
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
				    if(inet_pton(AF_INET, peers[*it], &serv_addr.sin_addr)<=0)  
				    { 
				        printf("\nInvalid address/ Address not supported \n"); 
				    } 

				    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
				    { 
				        printf("\nConnection Failed \n"); 
				        //exit(EXIT_FAILURE);
				    }

				    //write to random IP in random list
				    write(sock, buffer, sizeof(buffer));
				    read(sock, dummy_buffer, sizeof(dummy_buffer));
				    //close this socket
				    close(sock);
				}
			}//for ends
		}//if ends
    //return NULL;
	}//while ends

}


int main(int argc, char const *argv[]) 
{ 
	ip_sha1->no_entry=0;
	//myfile.open ("example.txt", ios::out | ios::app);
    pthread_t thread_seed_function, thread_client_function;

    pthread_create(&thread_seed_function, NULL, seed_function, NULL);
    pthread_create(&thread_client_function, NULL, client_function, NULL);

    pthread_join(thread_seed_function, NULL);
    pthread_join(thread_client_function, NULL);

    return 0; 
}