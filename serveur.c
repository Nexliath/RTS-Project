#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>

#define PORT 8080

int main()
{

  printf("Listening......\n");
  char server_message[256] = "Welcome to the calculator !\n";

  
  //server socket
  int server_socket;
  server_socket = socket(AF_INET, SOCK_STREAM, 0);


  //struct of the socket
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(PORT);
  server_address.sin_addr.s_addr = INADDR_ANY;


  //bind the socket to our specified IP and POR
  if(bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
  {
    perror("Error: binding\n");
    exit(1);
  }
  
  //backlog : how many connections can be waited
  listen(server_socket, 4); 

  //accepting the connexion
  int client_socket;
  if ((client_socket = accept(server_socket, NULL, NULL)) < 0)
  { 
    perror("Error : accepting\n");
    exit(2);
  }

  send(client_socket, server_message, sizeof(server_message), 0);

  int n1, n2, comp, res, end;
start:
  write(client_socket, "<Please enter a number>", strlen("<Please enter a number>"));
  read(client_socket, &n1, sizeof(n1));
  printf("First number : %d\n", n1);

  write(client_socket, "<Please enter another number>", strlen("<Please enter another number>"));
  read(client_socket, &n2, sizeof(n2));
  printf("Second number : %d\n", n2);

  write(client_socket, "<Pick an operation>\n 1. +\n 2. -\n 3. *\n 4. /\n : ", strlen("Pick an operation\n 1. +\n 2. -\n 3. *\n 4. / \n"));
  read(client_socket, &comp, sizeof(comp));
  printf("Operation choice : %d\n", comp);

  switch (comp)
  {
    case 1:
      res = n1 + n2;
      break;
    case 2:
      res = n1 - n2;
      break;
    case 3:
      res = n1 * n2;
      break;
    case 4:
      res = n1 / n2;
      break;
    default:
      perror("Error : wrong selection");
      exit(3);
  }
  
  write(client_socket, "<Result> \n", strlen("<Result>"));
  write(client_socket, &res, sizeof(res));

   write(client_socket, "<Would you like to continue?> \n 0. Yes\n 1. No\n", strlen("<Would you like to continue?> \n 0. Yes\n 1. No\n"));
  read(client_socket, &end, sizeof(end));
  printf("End ? : %d\n", end);
  
  if(end == 0)
  {
    goto start; 
  }
  else
  {
    goto end;
  }

end:
  write(client_socket, "<Bye!>", strlen("<Bye!>"));
  close(server_socket);
  return 0;
}

