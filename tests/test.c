#include <gfbgraph/gfbgraph.h>
#include "gfbgraph-simple-authorizer.h"

int
main (int argc, char **argv)
{
        GFBGraphSimpleAuthorizer *authorizer;
        GFBGraphUser *me;
        gchar *me_name;
        GError *error = NULL;
        GList *albums;

        g_type_init ();

        authorizer = gfbgraph_simple_authorizer_new ();

        /* Get "me" user */
        me = gfbgraph_user_get_me (GFBGRAPH_AUTHORIZER (authorizer), &error);
        if (error != NULL) {
                g_print ("Error getting \"me\" user\n");
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

        g_clear_object (&authorizer);

        return 0;
}
