#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<strings.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<signal.h>
#include<sys/wait.h>

#define SERVER_PORT 12345 
#define LISTEN_QUEUE 100

/* A simple TCP Echo server*/
void str_echo(int);
void sig_chld(int);
int main() 
{
    int listen_fd = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in server_socket;
    bzero(&server_socket, sizeof(server_socket));
    server_socket.sin_family = AF_INET;
    server_socket.sin_addr.s_addr = htonl(INADDR_ANY);
    server_socket.sin_port = htons(SERVER_PORT);
    signal(SIGCHLD,sig_chld);

    bind(listen_fd, (struct sockaddr*)&server_socket, sizeof(server_socket));

    listen(listen_fd, LISTEN_QUEUE);
  
    while(1)
    {
        struct sockaddr_in client_socket;   
        bzero(&client_socket,sizeof(client_socket));
        int pid;
        int sockaddr_len = sizeof(client_socket);
        int connection_fd = accept(listen_fd, (struct sockaddr*)&client_socket, &sockaddr_len);
        if(connection_fd < 0)
        {
            /* A signal was caught by the process while it was blocked in accept.
             * Continue after waking up!
             */
            if(errno == EINTR)
                continue;
            else
            {
               printf("accept error");
               exit(1);
            }
        }
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

/* Everytime a child process exits, a SIGCHLD is delivered to the parent. This
 * signal can be used to wait() on a child, to clean it up from the kernel process
 * tables, to avoid zombies. The wait() call blocks until one of the child processes
 * exit. If while handling a SIGCHLD, multiple SIGCHLD are received, they will be
 * reduced to a single - UNIX does not queue signals. It is hence important to somehow
 * wait() on each of the child processes that have exitted. 
 *
 * Note that we prefer using waitpid(-1 ...) instead of wait() because waitpid() allows
 * the use of the WNOHANG option.
 */
void sig_chld(int signo)
{
    pid_t pid;
    int stat;
    /* Do not block if there are no terminated children */
    while((pid = waitpid(-1,&stat, WNOHANG)) > 0)
        printf("child %d exit\n", pid);
   
    return;
}
