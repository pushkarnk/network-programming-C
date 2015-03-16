#include<sys/socket.h>
#include<netinet/in.h>
#include<stdio.h>
#include<strings.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<poll.h>

#define SERVER_PORT 12345
#define MAX_BUFFER_SIZE 1024
#define INFTIM -1
void str_cli(FILE*, int);
void sig_pipe(int signo);

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
   struct pollfd fds[2];

   fds[0].fd = fileno(fp);
   fds[0].events = POLLRDNORM ;

   fds[1].fd = sockfd;
   fds[1].events = POLLRDNORM  ;

   while(1)
   {
       int numfd = 2;
       poll(fds, numfd, INFTIM);

       int nread;
       char buffer[MAX_BUFFER_SIZE];
       bzero(buffer, MAX_BUFFER_SIZE);

       /* If the socket is read ready */
       if(fds[1].revents & (POLLRDNORM | POLLERR))
       {
           if((nread = read(sockfd, buffer, MAX_BUFFER_SIZE)) == 0)
           {
               if(numfd == 1)
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
       if(fds[0].revents & (POLLRDNORM | POLLERR))
       {
           if((nread = read(fileno(fp), buffer, MAX_BUFFER_SIZE)) == 0)
           {
               shutdown(sockfd, SHUT_WR);
               numfd = 1;
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