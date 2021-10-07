#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 100 // nombre de client max supserver_portés par le serveur
#define LENGTH 2048 // taille maximal du text du client
#define PSEUDO_LEN 32 // taille max du pseudo du client

static char banner[] =
"\n\n\
/*************************************************************************/\n\
/*    Chat Client/Serveur - Programmation Multitâche et Temps Réel       */\n\
/*                                                                       */\n\
/*         GARNIER Victor | JOLY Antoine | MAGNANI Nicolas               */\n\
/*                Professeur: M.BOUGUEROUA Lamine                        */\n\
/*************************************************************************/\n\
\n\n";

volatile sig_atomic_t flag = 0;

int socket_ret_func = 0;
char pseudo[PSEUDO_LEN];
int server_port = 10000;
char *server_ip = "127.0.0.1"; // a modifier en fonction de l'adresse server_ip voulue -> a implémenter dans l'interface graphique
struct sockaddr_in server_adress;

void str_overwrite_stdout(){
    printf("\r%s", "> ");
    fflush(stdout);
}

void str_trim_lf(char *table, int len){
    for (int i = 0; i < len; i++){
        if (table[i] == '\n'){
            table[i] = '\0';
            break;
        }
    }
}

void exit_chat(){
    flag = 1;
}

void* receiving_client_message_controller(){
    char client_message[LENGTH] = "";

    while (1){
        int receiving_status = recv(socket_ret_func, client_message, LENGTH, 0);
        if (receiving_status > 0){
            printf("%s", client_message);
            str_overwrite_stdout();
        }
        else if (receiving_status == 0){
            break;
        }
        memset(client_message, 0, sizeof(client_message));
    }
}

void* sending_client_message_controller(){
    char buffer[LENGTH] = "";
    char client_message[LENGTH+ PSEUDO_LEN] = "";

    while (1){
        str_overwrite_stdout();
        fgets(buffer, LENGTH, stdin);
        str_trim_lf(buffer, LENGTH);

        if (strcmp(buffer, "exit") == 0){
            break;
        }
        else{
            sprintf(client_message, "%s: %s\n", pseudo, buffer);
            send(socket_ret_func, client_message, strlen(client_message), 0);
        }
        
        bzero(buffer, LENGTH);
        bzero(client_message, LENGTH+ PSEUDO_LEN);
    }
    exit_chat();
}

int main(int argc, char** argv){
    printf("%s\n", banner); // affichage de la bannière de présentation du projet

    if(argc > 1){
        server_port = atoi(argv[1]);
    }
    signal(SIGINT, exit_chat);

    printf("Entrez votre pseudo: ");
    fgets(pseudo, PSEUDO_LEN, stdin);
    str_trim_lf(pseudo, strlen(pseudo));

    if(strlen(pseudo)> PSEUDO_LEN-1){ // vérification de l'input pseudo
        printf("Veuillez rentrer un pseudo valide: \n");
        exit(EXIT_FAILURE);
    }

    socket_ret_func = socket(AF_INET, SOCK_STREAM, 0); // Configuration du socket
    server_adress.sin_family = AF_INET;
    server_adress.sin_addr.s_addr = inet_addr(server_ip);
    server_adress.sin_port = htons(server_port);

    int err = connect(socket_ret_func, (struct sockaddr*)&server_adress, sizeof(server_adress)); // Connexion au serveur

    if (err == -1){ 
        printf("Erreur de connexion au serveur  ");
        exit(EXIT_FAILURE);
    }

    send(socket_ret_func, pseudo, strlen(pseudo), 0); // Envoie du pseudo de l'utilisateur chat


    printf("\n----------------- CHAT RTS ---------------\n");
    pthread_t sending_message_thread;
    if (pthread_create(&sending_message_thread, NULL, &sending_client_message_controller, NULL) != 0){ // envoie du message au serveur via la variable sending_message et de la fonction d'envoi
        printf("Erreur du pthread_create()");
        exit(EXIT_FAILURE);
    }

    pthread_t received_message_thread;
    if (pthread_create(&received_message_thread, NULL, &receiving_client_message_controller, NULL) != 0){ // reception d'un message du serveur via la variable sending_message et de la fonction de réception
        printf("Erreur du pthread_create()");
        exit(EXIT_FAILURE);
    }

    while (1){
        if (flag) { // fermeture du chat
            printf("\nMerci d'avoir utilisé notre chat\n");
            break;
        }
    }

    close(socket_ret_func);
    return EXIT_SUCCESS;
}
