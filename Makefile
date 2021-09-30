c_client: client.o serveur.o
	gcc -o client client.o -Wall -O 
	gcc -o serveur serveur.o -Wall -O

clean:
	rm -f client
	rm -f serveur
	rm -f client.o
	rm -f serveur.o



