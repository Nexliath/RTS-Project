#include <stdlib.h>
#include <gtk/gtk.h>

typedef struct
{
    gpointer field_pseudonyme;
    gpointer field_ip;
    GtkWidget *window;
} login_data;

GtkBuilder *p_builder = NULL;
GError *p_err = NULL;

// void listen() {

// }

static void send(GtkWidget *p_wid, gpointer field_message) {
    // p_builder = gtk_builder_new();
    // gtk_builder_add_from_file(p_builder, "window_chat.xml", &p_err);
    // gpointer messsages_list = (gpointer)gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(p_builder, "field_ip")));
    // GtkWidget   * p_dialog  = NULL;
    // char *message = field_message;

    // gtk_text_view_set_buffer(messsages_list, message);
}

int win_chat() {
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

            /* Signal du bouton d'envoi  */
            // g_signal_connect(
            //     gtk_builder_get_object(p_builder, "btn_envoyer"),
            //     "clicked", G_CALLBACK(send), (gpointer) gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(p_builder, "field_message")))
            // );

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

    return EXIT_SUCCESS;
}

static void login_func(GtkWidget *p_wid, login_data *data)
{
    GtkWidget *p_dialog = NULL;
    char *pseudo = data->field_pseudonyme;
    char *ip = data->field_ip;
    GtkWidget *window = data->window;
    //Test de connexion
    // if ()
    // {
    //     p_dialog = gtk_message_dialog_new(
    //         NULL,
    //         GTK_DIALOG_MODAL,
    //         GTK_MESSAGE_INFO,
    //         GTK_BUTTONS_OK,
    //         "Connexion au serveur impossible");

    //     gtk_dialog_run(GTK_DIALOG(p_dialog));
    //     gtk_widget_destroy(p_dialog);
    // } else {
        gtk_widget_destroy(window);
        win_chat();
    //}
}

int main(int argc, char **argv)
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
            data.field_pseudonyme = (gpointer)gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(p_builder, "field_pseudonyme")));
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

    return EXIT_SUCCESS;
}