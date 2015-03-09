#include<sys/socket.h>
#include<netinet/in.h>
#include<stdio.h>
#include<strings.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>

#define SERVER_PORT 12345 

void str_cli(FILE*, int);
void sig_pipe(int signo);
int main( int argc, char * argv[] )
{
 
    if(argc < 2 )
    {
        printf("IP address of server not passed");
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
    char sendline[MAXLINE], recvline[MAXLINE];
    
    while(fgets(sendline,MAXLINE,fp))
    {
        write(sockfd,sendline,strlen(sendline));
        if(read(sockfd, recvline,MAXLINE) == 0) 
        {
            printf("Server terminated pre-maturely\n");
            exit(1);
        }
        fputs(recvline,stdout);
        bzero(recvline,MAXLINE);
    }
}
void sig_pipe(int signo)
{
    printf("Broken pipe\n");
    exit(1);
}
