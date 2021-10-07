client: client.o
	gcc -o client client.o -Wall -O -lpthread
serveur: server.o
	gcc -o serveur serveur.o -Wall -O -lpthread

clean:
	rm -f client
	rm -f serveur
	rm -f client.o
	rm -f serveur.o



