#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<strings.h>
#include<sys/socket.h>
#include<netinet/in.h>

#define SERVER_PORT 12345 
#define LISTEN_QUEUE 100

/* A simple TCP Echo server*/
void str_echo(int);

int main() 
{
    int listen_fd = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in server_socket;
    bzero(&server_socket, sizeof(server_socket));
    server_socket.sin_family = AF_INET;
    server_socket.sin_addr.s_addr = htonl(INADDR_ANY);
    server_socket.sin_port = htons(SERVER_PORT);
<<<<<<< HEAD
    signal(SIGCHLD,sig_chld);
=======

>>>>>>> parent of c0ac180... Added code to handle SIGCHLD in parent
    bind(listen_fd, (struct sockaddr*)&server_socket, sizeof(server_socket));

    listen(listen_fd, LISTEN_QUEUE);
  
    while(1)
    {
        struct sockaddr_in client_socket;   
        bzero(&client_socket,sizeof(client_socket));
        int pid;
        int sockaddr_len = sizeof(client_socket);
        int connection_fd = accept(listen_fd, (struct sockaddr*)&client_socket, &sockaddr_len);
        if( (pid = fork()) == 0 ) 
        {
            //child process
            close(listen_fd);
            str_echo(connection_fd);
            close(connection_fd);
            exit(0);
        }
        close(connection_fd);
    }
    return 0;
}

#define MAX_SIZE 1024
void str_echo(int fd)
{
    ssize_t n_bytes;
    char buffer[MAX_SIZE];    

    repeat:
    while( (n_bytes = read(fd, buffer, MAX_SIZE)) > 0 )
        write(fd, buffer, n_bytes); 
 
    if(n_bytes < 0 && errno == EINTR)
        goto repeat;
    
    if(n_bytes < 0)
        printf("read error\n");
}
