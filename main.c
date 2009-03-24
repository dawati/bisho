#include <gtk/gtk.h>
#include <mojito-client/mojito-client.h>

static MojitoClient *client;

static GtkWidget *window, *master_box;

typedef enum {
  AUTH_USERNAME,
  AUTH_USERNAME_PASSWORD
} ServiceAuthType;

typedef struct {
  char *name;
  char *icon_name;
  char *description;
  char *link;
  ServiceAuthType auth;
} ServiceInfo;

#define GROUP "MojitoService"

static ServiceInfo *
get_info_for_service (const char *name)
{
  char *filename, *path, *authstring;
  GKeyFile *keys;
  ServiceInfo *info;

  g_assert (name);

  filename = g_strconcat (name, ".keys", NULL);
  path = g_build_filename ("mojito", "services", filename, NULL);
  g_free (filename);

  keys = g_key_file_new ();

  if (g_key_file_load_from_data_dirs (keys, path, NULL, G_KEY_FILE_NONE, NULL)) {
    g_free (path);
  } else {
    g_free (path);
    g_key_file_free (keys);
    return NULL;
  }

  /* Sanity check for required keys */
  if (!g_key_file_has_key (keys, GROUP, "Name", NULL) ||
      !g_key_file_has_key (keys, GROUP, "AuthType", NULL)) {
    g_key_file_free (keys);
    return NULL;
  }

  info = g_slice_new0 (ServiceInfo);
  info->name = g_key_file_get_string (keys, GROUP, "Name", NULL);
  info->icon_name = g_key_file_get_string (keys, GROUP, "Icon", NULL);
  info->description = g_key_file_get_string (keys, GROUP, "Description", NULL);
  info->link = g_key_file_get_string (keys, GROUP, "Link", NULL);

  authstring = g_key_file_get_string (keys, GROUP, "AuthType", NULL);
  if (g_ascii_strcasecmp (authstring, "username") == 0) {
    info->auth = AUTH_USERNAME;
  } else if (g_ascii_strcasecmp (authstring, "password") == 0) {
    info->auth = AUTH_USERNAME_PASSWORD;
  }

  g_key_file_free (keys);

  return info;
}

static gboolean
on_link_event (GtkTextTag  *tag,
               GObject     *object,
               GdkEvent    *event,
               GtkTextIter *iter,
               gpointer     user_data)
{
  if (event->type == GDK_BUTTON_PRESS && event->button.button == 1) {
    GtkWidget *widget = GTK_WIDGET (object);
    ServiceInfo *info = user_data;

    gtk_show_uri (gtk_widget_get_screen (widget), info->link,
                  event->button.time, NULL);

    return TRUE;
  }
  return FALSE;
}

static void
construct_ui (const char *service_name)
{
  ServiceInfo *info;
  GtkWidget *box, *label, *text;
  GtkTextBuffer *buffer;
  GtkTextIter end;

  g_assert (service_name);

  info= get_info_for_service (service_name);
  if (info == NULL)
    return;

  box = gtk_vbox_new (FALSE, 4);

  label = gtk_label_new (info->name);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  text = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (text), FALSE);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (text), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text), GTK_WRAP_WORD);
  /* TODO: something like this */
  gtk_widget_modify_base (text, GTK_STATE_NORMAL, &text->style->bg[GTK_STATE_NORMAL]);
  gtk_widget_show (text);
  gtk_box_pack_start (GTK_BOX (box), text, FALSE, FALSE, 0);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));

  if (info->description) {
    gtk_text_buffer_get_end_iter (buffer, &end);
    gtk_text_buffer_insert (buffer, &end, info->description, -1);
  }

  if (info->link) {
    GtkTextTag *tag;

    gtk_text_buffer_get_end_iter (buffer, &end);

    tag = gtk_text_buffer_create_tag (buffer, NULL,
                                      "foreground", "blue",
                                      "underline", PANGO_UNDERLINE_SINGLE,
                                      NULL);
    g_signal_connect (tag, "event", G_CALLBACK (on_link_event), info);

    gtk_text_buffer_insert (buffer, &end, "  ", -1);
    gtk_text_buffer_insert_with_tags (buffer, &end,
                                      "Launch site for more information", -1,
                                      tag, NULL);
    gtk_text_buffer_insert (buffer, &end, ".", -1);
  }

  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (master_box), box);
}

static void
client_get_services_cb (MojitoClient *client,
                        const GList        *services,
                        gpointer      userdata)
{
  const GList *l;

  for (l = services; l; l = l->next) {
    construct_ui (l->data);
  }
}

int
main (int argc, char **argv)
{
  gtk_init (&argc, &argv);

  client = mojito_client_new ();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  master_box = gtk_vbox_new (TRUE, 4);
  gtk_widget_show (master_box);
  gtk_container_add (GTK_CONTAINER (window), master_box);

  mojito_client_get_services (client, client_get_services_cb, NULL);

  gtk_widget_show (window);

  gtk_main ();

  return 0;
}
