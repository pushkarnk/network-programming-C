#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<strings.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<signal.h>
#include<sys/wait.h>
#include<linux/limits.h>
#include<poll.h>

#define SERVER_PORT 12345
#define MAX_BUFFER_SIZE 1024
#define LISTEN_QUEUE 100

//newer systems don't define this constant
#define OPEN_MAX 2048

/* A simple echo server using the select() system call */
int main()
{
    /* Create the socket */
    int listenfd = socket(AF_INET,SOCK_STREAM,0);

    /* Create the sockaddr structure */
    struct sockaddr_in server_socket, connecting_socket;
    bzero(&server_socket, sizeof(server_socket));
    server_socket.sin_family = AF_INET;
    server_socket.sin_addr.s_addr = htonl(INADDR_ANY);
    server_socket.sin_port = htons(SERVER_PORT);

    /* Bind */
    bind(listenfd, (struct sockaddr*)&server_socket, sizeof(server_socket));

   /* Listen */
   listen(listenfd, LISTEN_QUEUE);

   /* Initialize data structures for poll operation*/
   int maxfd = listenfd;

   struct pollfd client [OPEN_MAX];
   client[0].fd = listenfd;
   client[0].events = POLLRDNORM;

   int i;
   for(i = 1; i < OPEN_MAX; i++)
      client[i].fd = -1;

   int maxi = 0;  //first entry already occupied in client[]

   char buffer[MAX_BUFFER_SIZE];
   while(1)
   {
      int nready = poll(client, maxi+1, -1);
      int i;
      /* Just like select, poll will return for two cases here.
       * 1) new connection
       * 2) an fd that is read ready
       */

      if(client[0].revents & POLLRDNORM)
      {
          int client_length = sizeof(connecting_socket);
          int connectfd = accept(listenfd, (struct sockaddr*)&connecting_socket, &client_length);
          printf("accepted\n");
          for(i=1; i<OPEN_MAX;i++)
              if(client[i].fd < 0)
              {
                  client[i].fd = connectfd;
                  break;
              }

          if(i == OPEN_MAX)
          {
              printf("Too many clients\n");
              exit(1);
          }

          client[i].events = POLLRDNORM;
          maxi = (i > maxi)?i : maxi;

          if(--nready <= 0)
            continue;
      }

      for(i=1; i<maxi; i++)
      {
          int sockfd;
          if((sockfd = client[i].fd) < 0)
            continue;

          int n;
          if(client[i].revents & (POLLRDNORM | POLLERR))
          {
            if((n=read(client[i].fd, buffer, MAX_BUFFER_SIZE)) < 0)
            {
                if(errno == ECONNRESET)
                {
                    close(sockfd);
                    client[i].fd = -1;
                }
                else
                {
                    printf("Read error\n");
                    exit(1);
                }
            }
            else if(n == 0)
            {
                close(sockfd);
                client[i].fd = -1;
            }
            else
              write(sockfd, buffer, n);

            if(--nready <= 0)
              break;
          }
      }
   }

}
