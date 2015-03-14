#include<sys/socket.h>
#include<netinet/in.h>
#include<stdio.h>
#include<strings.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>

#define SERVER_PORT 12345 
#define MAX_BUFFER_SIZE 1024

void str_cli(FILE*, int);
void sig_pipe(int signo);

int max( int a, int b)
{
    return ( a > b ? a : b );
}

int main( int argc, char * argv[] )
{
 
    if(argc < 2 )
    {
        printf("IP address of server not passed\n");
        exit(0);
    }

    int socket_fd = socket(AF_INET, SOCK_STREAM,0);

    signal(SIGPIPE,sig_pipe);
 
    struct sockaddr_in server_socket;    
    bzero(&server_socket, sizeof(server_socket)); 
    server_socket.sin_family = AF_INET;
    server_socket.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET,argv[1], &server_socket.sin_addr);
  
    connect(socket_fd, (struct sockaddr*)&server_socket, sizeof(server_socket));

    str_cli(stdin, socket_fd); 
    
    return 0;
}

#define MAXLINE 1024

void str_cli(FILE * fp, int sockfd)
{
   fd_set rset;
   FD_ZERO(&rset); 
   int shutdownFlag = 0;
   while(1)
   {
       if(!shutdownFlag)
           FD_SET(fileno(fp), &rset);
       FD_SET(sockfd, &rset);
       int maxfdp1 = max(fileno(fp), sockfd) + 1;
       select(maxfdp1, &rset, NULL, NULL, NULL);
       
       int nread;
       char buffer[MAX_BUFFER_SIZE];
       bzero(buffer, MAX_BUFFER_SIZE); 
       /* If the socket is read ready */
       if(FD_ISSET(sockfd, &rset))
       {
           if((nread = read(sockfd, buffer, MAX_BUFFER_SIZE)) == 0)
           {
               if(shutdownFlag == 1)
                   return;
               else
               {
                   printf("Server terminated the connection\n");
                   exit(1);
               }   
           }
           write(fileno(stdout), buffer, nread);
       } 
      
       bzero(buffer, MAX_BUFFER_SIZE);

       /* If stdin is read read */
       if(FD_ISSET(fileno(fp), &rset))
       {
           if((nread = read(fileno(fp), buffer, MAX_BUFFER_SIZE)) == 0) 
           {
               shutdownFlag = 1;
               shutdown(sockfd, SHUT_WR);
               FD_CLR(fileno(fp), &rset);
               continue;
           }
           write(sockfd, buffer, nread);
       }
       
   }
}

void sig_pipe(int signo)
{
    printf("Broken pipe\n");
    exit(0);
}
