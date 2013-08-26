#include <gfbgraph/gfbgraph.h>
#include <gfbgraph/gfbgraph-simple-authorizer.h>
#include "credentials.h"

int
main (int argc, char **argv)
{
        GFBGraphSimpleAuthorizer *authorizer;
        GFBGraphUser *me;
        gchar *me_name;
        GError *error = NULL;
        GList *albums;
        GFBGraphPhoto *photo;
        GInputStream *in_stream;
        GOutputStream *out_stream;
        GFile *out_file;
        GFBGraphPhotoImage *smaller;

        g_type_init ();

        authorizer = gfbgraph_simple_authorizer_new (GFBGRAPH_TEST_ACCESS_TOKEN);

        /* Get "me" user */
        me = gfbgraph_user_get_me (GFBGRAPH_AUTHORIZER (authorizer), &error);
        if (error != NULL) {
                g_print ("Error getting \"me\" user: %s\n", error->message);
                return -1;
        }
        g_object_get (G_OBJECT (me), "name", &me_name, NULL);
        g_print ("User: %s\n", me_name);

        /* Get my albums */
        albums = gfbgraph_node_get_connection_nodes (GFBGRAPH_NODE (me), GFBGRAPH_TYPE_ALBUM, GFBGRAPH_AUTHORIZER (authorizer), &error);
        if (error != NULL) {
                g_print ("Error get connected nodes\n");
                return -1;
        }
        if (albums == NULL) {
                g_print ("Es nulo\n");
        }
        while (albums) {
                GFBGraphAlbum *album;
                gchar *album_name;
                guint album_count;

                album = GFBGRAPH_ALBUM (albums->data);
                g_object_get (album, "name", &album_name, "count", &album_count, NULL);
                g_print ("\tAlbum: %s - Photos: %d\n", album_name, album_count);
                g_free (album_name);

                albums = g_list_next (albums);
        }

        photo = gfbgraph_photo_new_from_id (GFBGRAPH_AUTHORIZER (authorizer), "553619791342827", &error);
        if (error != NULL) {
                g_print ("Error getting photo\n");
                return -1;
        }

        smaller = gfbgraph_photo_get_image_near_width (photo, 1);
        if (smaller == NULL)
                g_error ("Can't get the smaller image\n");
        else
                g_print ("%dx%d %s", smaller->width, smaller->height, smaller->source);

        in_stream = gfbgraph_photo_download_default_size (photo, GFBGRAPH_AUTHORIZER (authorizer), NULL);
        out_file = g_file_new_for_path ("/tmp/facebook.jpeg");
        out_stream = G_OUTPUT_STREAM (g_file_create (out_file, G_FILE_CREATE_PRIVATE, NULL, &error));
        if (error != NULL) {
                g_print ("Error creating temp file\n");
                return -1;
        }

        g_output_stream_splice (G_OUTPUT_STREAM (out_stream), in_stream,
                              G_OUTPUT_STREAM_SPLICE_CLOSE_SOURCE | G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET,
                              NULL, &error);
        if (error != NULL) {
                g_print ("Error splicing streams\n");
                return -1;
        }

        g_list_free_full (albums, g_object_unref);
        g_clear_object (&me);
        g_clear_object (&authorizer);

        return 0;
}
