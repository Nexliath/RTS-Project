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
#include <gtk/gtk.h>

#define MAX_CLIENTS 100 // nombre de client max supserver_portés par le serveur
#define LENGTH 2048     // taille maximal du text du client
#define PSEUDO_LEN 32
static char banner[] =
    "\n\n\
/*************************************************************************/\n\
/*    Chat Client/Serveur - Programmation Multitâche et Temps Réel       */\n\
/*                                                                       */\n\
/*         GARNIER Victor | JOLY Antoine | MAGNANI Nicolas               */\n\
/*                Professeur: M.BOUGUEROUA Lamine                        */\n\
/*************************************************************************/\n\
\n\n";

int socket_ret_func = 0;
char *pseudo;
int server_port = 10000;
char *server_ip;
struct sockaddr_in server_adress;

typedef struct
{
    gpointer field_ip;
    GtkWidget *window;
} login_data;

GtkBuilder *p_builder = NULL;
GError *p_err = NULL;
GtkTextView *text_view;
GtkTextBuffer *text_view_buffer;
GtkTextIter iter;
GtkTextMark *mark;

void exit_chat()
{
    close(socket_ret_func);
    exit(1);
}

void *receive_msg()
{
    char client_message[LENGTH] = "";

    while (1)
    {
        int receiving_status = recv(socket_ret_func, client_message, LENGTH, 0);
        if (receiving_status > 0)
        {
            text_view_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
            mark = gtk_text_buffer_get_insert(text_view_buffer);
            gtk_text_buffer_get_iter_at_mark(text_view_buffer, &iter, mark);
            gtk_text_buffer_insert(text_view_buffer, &iter, client_message, -1);
            
        }
        else if (receiving_status == 0)
        {
            break;
        }
        memset(client_message, 0, sizeof(client_message));
    }
}

static void send_msg(GtkWidget *p_wid, gpointer *data)
{
    char buffer[LENGTH] = "";
    const char *client_message = gtk_entry_get_text(GTK_ENTRY(data));
    if (strlen(client_message) > 0)
    {
        strcat(buffer, pseudo);
        strcat(buffer, ": ");
        strcat(buffer, client_message);
        strcat(buffer, "\n");

        send(socket_ret_func, buffer, strlen(buffer), 0);
        text_view_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
        mark = gtk_text_buffer_get_insert(text_view_buffer);
        gtk_text_buffer_get_iter_at_mark(text_view_buffer, &iter, mark);
        gtk_text_buffer_insert(text_view_buffer, &iter, buffer, -1);
        bzero(buffer, LENGTH);
        gtk_entry_set_text(GTK_ENTRY(data), "");
    }
}

static void win_chat()
{
    GtkBuilder *p_builder = NULL;
    GError *p_err = NULL;

    /* Creation d'un nouveau GtkBuilder */
    p_builder = gtk_builder_new();

    if (p_builder != NULL)
    {
        /* Chargement du XML dans p_builder */
        gtk_builder_add_from_file(p_builder, "xml/window_chat.xml", &p_err);

        if (p_err == NULL)
        {
            /* Recuparation d'un pointeur sur la fenetre. */
            GtkWidget *p_win = (GtkWidget *)gtk_builder_get_object(
                p_builder, "chat_window");

            /* Signal d'envoi du message */
            g_signal_connect(
                gtk_builder_get_object(p_builder, "field_message"),
                "activate", G_CALLBACK(send_msg), (gpointer)GTK_ENTRY(gtk_builder_get_object(p_builder, "field_message")));

            g_signal_connect(G_OBJECT(p_win), "delete-event", G_CALLBACK(exit_chat), NULL);

            text_view = (gpointer)GTK_TEXT_VIEW(gtk_builder_get_object(p_builder, "list_messages"));

            text_view_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
            mark = gtk_text_buffer_get_insert(text_view_buffer);
            gtk_text_buffer_get_iter_at_mark(text_view_buffer, &iter, mark);
            gtk_text_buffer_insert(text_view_buffer, &iter, banner, -1);
            gtk_widget_show_all(p_win);
            gtk_main();
        }
        else
        {
            /* Affichage du message d'erreur de GTK+ */
            g_error("%s", p_err->message);
            g_error_free(p_err);
        }
    }
}

static void login_func(GtkWidget *p_wid, login_data *data)
{
    GtkWidget *p_dialog = NULL;
    char *server_ip = data->field_ip;
    GtkWidget *window = data->window;

    if (strlen(pseudo) > 32 || strlen(pseudo) < 2)
    { // vérification de l'input pseudo
        p_dialog = gtk_message_dialog_new(
            NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "Votre pseudo doit faire 2 à 32 caractères");

        gtk_dialog_run(GTK_DIALOG(p_dialog));
        gtk_widget_destroy(p_dialog);
    }
    else
    {

        socket_ret_func = socket(AF_INET, SOCK_STREAM, 0); // Configuration du socket
        server_adress.sin_family = AF_INET;
        server_adress.sin_addr.s_addr = inet_addr(server_ip);
        server_adress.sin_port = htons(server_port);

        int err = connect(socket_ret_func, (struct sockaddr *)&server_adress, sizeof(server_adress)); // Connexion au serveur

        if (err == -1)
        {
            p_dialog = gtk_message_dialog_new(
                NULL,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_INFO,
                GTK_BUTTONS_OK,
                "Erreur de connexion au serveur");

            gtk_dialog_run(GTK_DIALOG(p_dialog));
            gtk_widget_destroy(p_dialog);
        }
        else
        {
            send(socket_ret_func, pseudo, strlen(pseudo), 0); // Envoie du pseudo de l'utilisateur chat
            pthread_t receive_msg_t;
            if (pthread_create(&receive_msg_t, NULL, &receive_msg, NULL) != 0)
            { // reception d'un message du serveur via la variable sending_message et de la fonction de réception
                printf("Erreur du pthread_create()");
                exit(EXIT_FAILURE);
            }

            gtk_widget_hide(window);
            win_chat();
        }
    }
}

void main(int argc, char **argv)
{

    /* Initialisation de GTK+ */
    gtk_init(&argc, &argv);

    /* Creation d'un nouveau GtkBuilder */
    p_builder = gtk_builder_new();

    if (p_builder != NULL)
    {
        /* Chargement du XML dans p_builder */
        gtk_builder_add_from_file(p_builder, "xml/window_login.xml", &p_err);

        if (p_err == NULL)
        {
            /* Recuparation d'un pointeur sur la fenetre. */
            GtkWidget *p_win = (GtkWidget *)gtk_builder_get_object(
                p_builder, "login_window");

            login_data data;
            pseudo = (gpointer)gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(p_builder, "field_pseudonyme")));
            data.field_ip = (gpointer)gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(p_builder, "field_ip")));
            data.window = p_win;

            /* Signal du bouton de connexion */
            g_signal_connect(
                gtk_builder_get_object(p_builder, "btn_login"),
                "clicked", G_CALLBACK(login_func), &data);

            gtk_widget_show_all(p_win);
            gtk_main();
        }
        else
        {
            /* Affichage du message d'erreur de GTK+ */
            g_error("%s", p_err->message);
            g_error_free(p_err);
        }
    }
}