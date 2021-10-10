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

static int count = 0;
static int check = 0;

char server_message[256] = "Status : connected || !help for the commands ~~\n";
static int id = 0;

  //Information needed from the client side
  struct client
  {
    int socket; //current file descriptor
    struct sockaddr_in address; //current address 
    char nickname[16]; //current name 
    struct client *next; //linked list of the clients
    int id; //id client

  }; 

  struct client *header = NULL;

  sem_t semData;//semaphore use for the critical race
  
  void client_add(struct client *c1)
  {
    sem_wait(&semData);
    
    count++;

    if (header == NULL)
    {
      header = c1;
      (*c1).next = NULL;
    }
    else 
    {
      (*c1).next = header;
      header = c1;
    }
    printf("Number of clients connected : %d\n", count);
    sem_post(&semData);

    return;
  }

	
  //Print the LL
  void client_print()
  {
    struct client *c1 = header;
    while (c1 != NULL)
    {
      printf("%d\n", (*c1).id);
      c1 = (*c1).next;
    }
  }
  
  //Removing client from the LL
  void client_remove(int sock)//removing a client form the list with its id
  {
    sem_wait(&semData);

    struct client *c1 = header;
    struct client *prev = NULL;

    while ((*c1).socket != sock)
    {
      prev = c1;
      c1 = (*c1).next;
    }

    if (prev == NULL)
    {
      header = (*c1).next;
    }

    else
    {
      (*prev).next = (*c1).next;
    }
    count--;
    free(c1);
    printf("Number of clients connected : %d\n", count);
    sem_post(&semData);
    return;

  }


/* send_message : sending the str from the buffer to clients except the sender 
IN : char * buff --> buffer stream
IN : int id --> client id to check if sender
OUT : NULL
DETAILS : utilization of mutex to manage the critical race (race condition) between client processes
*/
void send_message(char *buff, int sock)
{
  sem_wait(&semData);   
    
  struct client *c1 = header;
  
  while(c1 != NULL)
  {
    if((*c1).socket != sock)
    {   
     
      if(write((*c1).socket, buff, strlen(buff)) < 0)
      {
        perror("Writing failed\n");
        exit(8);
      }
    }
    
    c1 = (*c1).next;
     
  }
  
  sem_post(&semData);
  
  return;
}

/* dispatcher : managing threads concurrency */
void *dispatcher(void *cli)
{
  struct client *c1 = (struct client *) cli;
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
    printf("--------> IN : %s\n", client_message);
    send_message(buffer, (*c1).socket);
  }
  
  bzero(buffer, 4096);
  
  while(!check)
  {    
    int recc = recv((*c1).socket, buffer, 4096, 0);
    
    if (recc < 0)
    {
      perror("Receiving failed\n");
      exit(8);
    }
    else if(recc == 0)//shutdown of the client
    {
      check = 1;
      printf("<------ OUT : %s\n", (*c1).nickname);
      sprintf(buffer, "%s left the chat\n", ((*c1).nickname));
      send_message(buffer, (*c1).socket);
      check = 1;
    }
    /*else if(!strcmp(buffer, "!help"))
    {
      sprintf(send_res, "!online : get the number of clients connected\n");
      sprintf(send_res, "%s!exit : leave the chat\n", send_res);
      send((*c1).socket, send_res, sizeof(send_res), 0);

    }*/
    else
    {
      if (strlen(buffer) > 0)//checking if any char in the buffer stream 
      {
        send_message(buffer, (*c1).socket);
        buffer[recc - 1] = '\0';
        printf("%s\n", buffer);
      }
    }

    bzero(buffer, 4096); //reset buffer 
  }
   
  client_remove((*c1).socket);//removing client after closing the client
  close((*c1).socket); //closing socket
  pthread_detach(pthread_self());
  check = 0;//reset check 

  return NULL;
}


  int main()
  {
    
    char* localhost = "127.0.0.1";//local address
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
    printf("Initialisation du chat\n");
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
      
      struct client *c1 = malloc(sizeof(struct client));//allocating memory for our pointer to client
      (*c1).socket = client_socket;
      (*c1).address = client_address;
      (*c1).id = id++;

      client_add(c1);
      //client_print();

      //handling the process via thread
      if (pthread_create(&tid, NULL, &dispatcher, (void *)c1) == -1)
      {
        perror("Error creating thread\n");
        exit(5);
      }

    //pthread_join(tid, NULL);//waiting for thread to terminate

    
    }

    return 0;

}



