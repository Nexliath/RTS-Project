all: 
	gcc -o server server.c -Wall -lpthread
	gcc -o client client.c -Wall -lpthread


clean:
	rm -f client
	rm -f server
	rm -f client.o
	rm -f server.o



