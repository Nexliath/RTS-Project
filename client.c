#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
     
#define PORT 8080

int main(int argc, char const *argv[])
{
  int server_fd, new_socket, valread;
  struct sockaddr_in server_address;
  int addrlen = sizeof(server_address);
  char buffer[1024] = {0};

  //Creating socket file descriptor

  if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) //socket(family, type, protocol)
  {
     perror("Error : Socket failed");
     exit(EXIT_FAILURE);
  }

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(PORT); //conversion function int PORT --> DATA format different
  server_address.sin_addr.s_addr = INADDR_ANY; //0.0.0.0 sin_addr --> struct // data of the adress we trying to connect to

  if(connect(server_fd, (struct sockaddr *) &server_address, addrlen) == -1)
  {
      perror("Error : Connection failed\n");
      exit(2);
  }

  //Recieving data from the server
  char server_response[256];
  recv(server_fd, &server_response, sizeof(server_response), 0);

  //Server response
  
  printf("%s\n", server_response);

  //calculator part
  int n1, n2, comp, res, end;

start:
  bzero(buffer, 1024); //cleaning the buffer
  read(server_fd, buffer, 255);
  printf("%s\n", buffer);
  scanf("%d", &n1);
  write(server_fd, &n1, sizeof(n1));
  
  bzero(buffer, 1024);
  read(server_fd, buffer, 255);
  printf("%s\n", buffer);
  scanf("%d", &n2);
  write(server_fd, &n2, sizeof(n2));
 
  bzero(buffer, 1024);
  read(server_fd, buffer, 255);
  printf("%s\n", buffer);
  scanf("%d", &comp);
  write(server_fd, &comp, sizeof(comp));
  
  bzero(buffer, 1024);
  read(server_fd, buffer, 255);
  printf("%s\n", buffer);
  
  read(server_fd, &res, sizeof(res));
  printf("%d\n", res);

  bzero(buffer, 1024);
  read(server_fd, buffer, 255);
  printf("%s\n", buffer);
  scanf("%d", &end);
  write(server_fd, &end, sizeof(end));

  if (end == 0)
  {
    goto start;
  }
  else 
  { 
    goto end;
  }
  
end:
  bzero(buffer, 1024);
  read(server_fd, buffer, sizeof(buffer));

  close(server_fd); //always close the socket
  return 0;
}
