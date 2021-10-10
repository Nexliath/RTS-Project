all: 
	gcc -o server server.c -Wall -lpthread
	gcc -o -W -Wall -g `pkg-config --cflags gtk+-3.0` client.c -o client `pkg-config --libs gtk+3.0` -lpthread
#	gcc -o client client_3.c -Wall -lpthread


clean:
	rm -f client
	rm -f server
	rm -f client.o
	rm -f server.o



