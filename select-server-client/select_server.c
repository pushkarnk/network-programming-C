#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<strings.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<signal.h>
#include<sys/wait.h>

#define SERVER_PORT 12345 
#define MAX_BUFFER_SIZE 1024
#define LISTEN_QUEUE 100

/* A simple echo server using the select() system call */
int main() 
{
    /* Create the socket */
    int listenfd = socket(AF_INET,SOCK_STREAM,0);

    /* Create the sockaddr structure */
    struct sockaddr_in server_socket;
    bzero(&server_socket, sizeof(server_socket));
    server_socket.sin_family = AF_INET;
    server_socket.sin_addr.s_addr = htonl(INADDR_ANY);
    server_socket.sin_port = htons(SERVER_PORT);

    /* Bind */
    bind(listenfd, (struct sockaddr*)&server_socket, sizeof(server_socket));

   /* Listen */
   listen(listenfd, LISTEN_QUEUE);
 
   /* Initialize data structures for select */
   int maxi = -1;
   int maxfd = listenfd;
   int client[FD_SETSIZE], i;
   for(i = 0; i < FD_SETSIZE; i++)
       client[i] = -1; 

   fd_set allset, readset; 
   FD_ZERO(&allset);
   FD_ZERO(&readset);

   /* Set the listen fd on the global allset */
   FD_SET(listenfd, &allset);

   /* The main server loop */
   while(1) 
   {
       readset = allset;
       int nready = select(maxfd + 1, &readset, NULL, NULL, NULL);
       /* If the listenfd is read ready, accept the connection */
       if(FD_ISSET(listenfd, &readset))  
       {
           struct sockaddr_in client_socket;
           int client_sock_length = sizeof(client_socket);
           int connectfd = accept(listenfd, (struct sockaddr*)&client_socket, &client_sock_length);

           /* Save the connectfd */
           for(i = 0; i < FD_SETSIZE; i++)
               if(client[i] < 0 )
               {
                   client[i] = connectfd; 
                   break;
               }

           if( i == FD_SETSIZE )
           {
               printf("Too many clients\n");
               exit(1);
           }
 
           /* Set an interest to read on the new socket */
           FD_SET(connectfd, &allset);

           maxi =  i > maxi? i  : maxi;
           maxfd = connectfd > maxfd ? connectfd : maxfd;
    
           if( --nready <= 0 )
               continue;
       }

       /* If other fds are read ready */
       for(i = 0; i <= maxi; i++)
       {
           int sockfd, n;
           char buffer[MAX_BUFFER_SIZE];
            
           if ( (sockfd = client[i]) < 0 )
               continue;
           if(FD_ISSET(sockfd, &readset))
           {
               if ( (n = read(sockfd, buffer, MAX_BUFFER_SIZE)) == 0 )
               { 
                   close(sockfd);
                   FD_CLR(sockfd, &allset);
                   client[i] = -1;
               }
               else
                   write(sockfd, buffer, n);

               if( --nready <= 0 )
                   break;
          }
       }
   }
}

