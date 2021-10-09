all: 
	gcc -o server server.c -Wall -lpthread
#	gcc -o client client.c 'pkg-config --cflags --libs gtk+-3.0' -Wall -lpthread
	gcc -o client client_3.c -Wall -lpthread


clean:
	rm -f client
	rm -f server
	rm -f client.o
	rm -f server.o



