#include "config.h"
#include <glib.h>
#include <gfbgraph/gfbgraph.h>

#include "gfbgraph-simple-authorizer.h"

int
main (int argc, char **argv)
{
        GFBGraphSimpleAuthorizer *authorizer;

        g_type_init ();
        g_test_init (&argc, &argv, NULL);

        authorizer = gfbgraph_simple_authorizer_new ();

        //g_test_add_data_func ("/GInstapaper/Bookmarks/List", proxy, (GTestDataFunc) ginstapaper_test_bookmarks_list);

        return g_test_run ();
}
