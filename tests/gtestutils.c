#include "config.h"
#include "credentials.h"
#include <glib.h>
#include <gfbgraph/gfbgraph.h>
#include <gfbgraph/gfbgraph-simple-authorizer.h>

static void
gfbgraph_test_me_albums (GFBGraphSimpleAuthorizer *authorizer)
{
        GFBGraphUser *me;
        GList *albums = NULL;
        gint albums_count = 0;
        GFBGraphAlbum *album;
        GError *error = NULL;

        me = gfbgraph_user_get_me (GFBGRAPH_AUTHORIZER (authorizer), &error);
        g_assert_no_error (error);
        g_assert (GFBGRAPH_IS_USER (me));

        albums = gfbgraph_user_get_albums (GFBGRAPH_USER (me), GFBGRAPH_AUTHORIZER (authorizer), &error);
        g_assert_no_error (error);
        /* Just testing one album */
        while (albums) {
                album = GFBGRAPH_ALBUM (albums->data);
                g_assert (GFBGRAPH_IS_ALBUM (album));

                albums = g_list_next (albums);
                albums_count++;
        }

        g_list_free_full (albums, g_object_unref);
        g_object_unref (me);

        if (albums_count == 0) {
                g_test_fail ();
        }
}

static void
gfbgraph_test_me (GFBGraphSimpleAuthorizer *authorizer)
{
        GFBGraphUser *me;
        GError *error = NULL;

        me = gfbgraph_user_get_me (GFBGRAPH_AUTHORIZER (authorizer), &error);
        g_assert_no_error (error);
        g_assert (GFBGRAPH_IS_USER (me));

        g_object_unref (me);
}

int
main (int argc, char **argv)
{
        GFBGraphSimpleAuthorizer *authorizer;

        g_type_init ();
        g_test_init (&argc, &argv, NULL);

        authorizer = gfbgraph_simple_authorizer_new (GFBGRAPH_TEST_ACCESS_TOKEN);

        g_test_add_data_func ("/GFBGraph/Me", authorizer, (GTestDataFunc) gfbgraph_test_me);
        g_test_add_data_func ("/GFBGraph/Me/Albums", authorizer, (GTestDataFunc) gfbgraph_test_me_albums);

        return g_test_run ();
}
