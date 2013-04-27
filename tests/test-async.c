#include <gfbgraph/gfbgraph.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "gfbgraph-simple-authorizer.h"

static GMainLoop *main_loop;

void
photo_async_cb (GFBGraphNode *album_node, GAsyncResult *res, GFBGraphAuthorizer *authorizer)
{
        GList *photos;
        GError *error = NULL;

        photos = gfbgraph_node_get_connection_nodes_async_finish (GFBGRAPH_NODE (album_node), res, &error);

        if (error != NULL) {
                g_print ("Error: %s\n", error->message);
                g_main_loop_quit (main_loop);
        }

        while (photos) {
                GFBGraphPhoto *photo;
                gchar *name, *source;
                guint width, height;
                GInputStream *stream;
                GdkPixbuf *pixbuf;

                photo = GFBGRAPH_PHOTO (photos->data);
                g_object_get (photo, "name", &name, "width", &width, "height", &height, "source", &source, NULL);
                g_print ("\t\t%s (%dx%d): %s\n", name, width, height, source);
                g_print ("\t\t\tDownloading... ");
                stream = gfbgraph_photo_download_default_size (photo, authorizer);
                pixbuf = gdk_pixbuf_new_from_stream (stream, NULL, NULL);
                if (gdk_pixbuf_save (pixbuf, name, "jpeg", NULL, NULL))
                        g_print ("  OK\n");
                else
                        g_print ("  Failed\n");
                g_free (name);

                photos = g_list_next (photos);
        }

        g_object_unref (album_node);
        g_list_free (photos);
        g_main_loop_quit (main_loop);
}

void
albums_async_cb (GFBGraphNode *me_node, GAsyncResult *res, GFBGraphAuthorizer *authorizer)
{
        GList *albums;
        GFBGraphAlbum *one_album = NULL;
        GError *error = NULL;

        albums = gfbgraph_node_get_connection_nodes_async_finish (GFBGRAPH_NODE (me_node), res, &error);

        if (error != NULL) {
                g_print ("Error: %s\n", error->message);
                g_main_loop_quit (main_loop);
        }

        /* Print all albums names and count */
        while (albums) {
                GFBGraphAlbum *album;
                gchar *album_name;
                guint album_count;

                album = GFBGRAPH_ALBUM (albums->data);
                if (one_album == NULL) {
                        one_album = album;
                        g_object_ref (one_album);
                }
                g_object_get (album, "name", &album_name, "count", &album_count, NULL);
                g_print ("\tAlbum: %s - Photos: %d\n", album_name, album_count);
                g_free (album_name);

                albums = g_list_next (albums);
        }


        /* For one album, get the photos */
        if (one_album != NULL) {
                gfbgraph_node_get_connection_nodes_async (GFBGRAPH_NODE (one_album), GFBGRAPH_TYPE_PHOTO, authorizer,
                                                          NULL, (GAsyncReadyCallback) photo_async_cb, authorizer);
        }

        g_list_free (albums);
}

void
me_async_cb (GFBGraphAuthorizer *authorizer, GAsyncResult *res, gpointer user_data)
{
        GFBGraphUser *me;
        GError *error = NULL;

        me = gfbgraph_user_get_me_async_finish (authorizer, res, &error);
        if (error != NULL) {
                g_print ("Error: %s\n", error->message);
                g_main_loop_quit (main_loop);
        } else {
                gchar *me_name;

                g_object_get (G_OBJECT (me), "name", &me_name, NULL);
                g_print ("User: %s\n", me_name);
                gfbgraph_node_get_connection_nodes_async (GFBGRAPH_NODE (me), GFBGRAPH_TYPE_ALBUM, authorizer,
                                                          NULL, (GAsyncReadyCallback) albums_async_cb, authorizer);
        }
}

int
main (int argc, char **argv)
{
        GFBGraphSimpleAuthorizer *authorizer;

        g_type_init ();
        main_loop = g_main_loop_new (NULL, TRUE);

        authorizer = gfbgraph_simple_authorizer_new ();

        /* Get "me" user */
        gfbgraph_user_get_me_async (GFBGRAPH_AUTHORIZER (authorizer), NULL, (GAsyncReadyCallback) me_async_cb, NULL);

        g_main_loop_run (main_loop);

        g_main_loop_unref (main_loop);
        g_clear_object (&authorizer);

        return 0;
}
