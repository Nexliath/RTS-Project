#include <sys/socket.h> /* basic socket definitions */
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>

#define SERVER_PORT 8989
#define BUFFER_SIZE 4096

char server_message[256] = "Connected\n";

  //Information needed from the client side
  typedef struct 
  {
    int socket; //current file descriptor
    struct sockaddr_in address; //current address 
    char nickname[16]; //current name 
  } client_t;

  sem_t semData;//semaphore use for the critical race

 
/* send_message : sending the str from the buffer to clients except the sender 
IN : char * buff --> buffer stream
IN : int id --> client id to check if sender
OUT : NULL
DETAILS : utilization of mutex to manage the critical race (race condition) between client processes
*/
void send_message(char *buff, int sock)
{
  printf("J'attends ?\n");
  sem_wait(&semData);   

  if(write(sock, buff, strlen(buff)) < 0)
  {
    perror("Writing failed\n");
    exit(8);
  }
  
  sem_post(&semData);
  
  return;
}

void *dispatcher(void *cli)
{
  printf("ENTER DISPATCHER \n");
  client_t *c1 = (client_t *) cli;
  char client_message[256];
  
  send((*c1).socket, server_message, sizeof(server_message), 0);

  char buffer[4096];

  bzero(buffer, 4096);
  bzero(client_message, 256);

 //checking client entry --> nickname ?
  int rec = recv((*c1).socket, client_message, sizeof(client_message), 0);
  if (rec < 0)
  {
    perror("Receiving failed\n");
    exit(6);
  }
  else if (rec == 0)
  {
    printf("Stream empty\n");
    exit(7);
  }
  else 
  {
    strcpy((*c1).nickname,  client_message);
    sprintf(buffer, "New user : %s !\n", (*c1).nickname);
    printf("--------> IN : %s", client_message);
    send_message(buffer, (*c1).socket);
    printf("HOP\n");
  }
  
  bzero(buffer, 4096);

  while(1)
  {    
    printf("ENTERING LOOP\n");
    int recc = recv((*c1).socket, buffer, 4096, 0);
    if (recc < 0)
    {
      perror("Receiving failed\n");
      exit(8);
    }
    else if(recc == 0)//shutdown of the client
    {   
      send_message(buffer, (*c1).socket);
      printf("------> OUT : %s", buffer);
      bzero(buffer, 4096);
      break;
    }
    else
    {
      if (strlen(buffer) > 0)//checking if any char in the buffer stream 
      {
        send_message(buffer, (*c1).socket);
      }
    }

    bzero(buffer, 4096); 
  }

  close((*c1).socket);
  free(c1);
  sem_destroy(&semData);

  return;
}


  int main()
  {
    
    char* localhost = "127.0.0.1";
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    
    pthread_t tid;
    if(sem_init(&semData, 0, 1) < 0)//initialisation of the semaphore
    {
      perror("Sem non init");
      exit(10);
    }
  
    int server_socket, client_socket;

    //socket configuration
    server_socket = socket(AF_INET, SOCK_STREAM, 0); 
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = inet_addr(localhost);

    int enable = 1;

    //check port available - setsockopt()
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
      perror("setsockopt(SO_REUSEADDR) failed");
      exit(1);
    }

    //Associating the socket with a port on the local machine - bind()
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    {
      perror("Binding failed\n");
      exit(2);
    }

    //Listening to remote connections - listen()
    if (listen(server_socket, 4) < 0)
    {
      perror("Listening failed\n");
      exit(3);
   }

    printf("IP : 127.0.0.1 || Port : 8989\n");
    printf("Chat initialization");
    sleep(1);
    printf(".");
    sleep(1);
    printf(".\n");
    sleep(1);
    printf("Listening : OK\n");

    
    while(1)
    {
      socklen_t address_size = sizeof(client_address); 
      client_socket = accept(server_socket, (struct sockaddr *) &client_address, &address_size);

      if (client_socket < 0)
      {
        perror("Accepting failed\n");
        exit(4);
      }
      
      client_t *c1 = (client_t *)malloc(sizeof(client_t));//allocating memory for our pointer to client
      (*c1).socket = client_socket;
      (*c1).address = client_address;
      
      //handling the process via thread
      if (pthread_create(&tid, NULL, &dispatcher, (void *)c1) == -1)
      {
        perror("Error creating thread\n");
        exit(5);
      }

     // pthread_join(tid, NULL);//waiting for thread to terminate

    
    }

    return 0;

}



