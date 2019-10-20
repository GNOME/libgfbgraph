/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 8; tab-width: 8 -*-  */
/*
 * libgfbgraph - GObject library for Facebook Graph API
 * Copyright (C) 2013-2015 Álvaro Peña <alvaropg@gmail.com>
 *                    2019 Leesoo Ahn <yisooan@fedoraproject.org>
 *
 * GFBGraph is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * GFBGraph is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GFBGraph.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <glib.h>
#include <json-glib/json-glib.h>
#include <rest/rest-proxy.h>
#include <string.h>

#include <gfbgraph/gfbgraph.h>
#include <gfbgraph/gfbgraph-simple-authorizer.h>

/* #include "config.h" */

typedef struct _GFBGraphTestFixture GFBGraphTestFixture;
typedef struct _GFBGraphTestApp     GFBGraphTestApp;

struct _GFBGraphTestFixture
{
        gchar *user_id;
        gchar *user_email;

        GFBGraphSimpleAuthorizer *authorizer;
};

struct _GFBGraphTestApp
{
        gchar *client_id;
        gchar *client_secret;
        gchar *access_token;
};

#define FACEBOOK_ENDPOINT "https://graph.facebook.com/v2.10"

#define FACEBOOK_TEST_USER_PERMISSIONS "email,user_about_me,user_photos,publish_actions"

GFBGraphTestApp*
gfbgraph_test_app_setup (void)
{
        GFBGraphTestApp *app;
        RestProxy *proxy;
        RestProxyCall *rest_call;
        g_autofree gchar *function_path = NULL;
        g_autofree gchar *app_key_filename = NULL;
        g_autoptr(GError) error = NULL;
        g_autoptr(GKeyFile) app_key_file = NULL;
        g_autoptr(JsonParser) jparser = NULL;
        g_autoptr(JsonReader) jreader = NULL;
        JsonNode *jnode;
        const char *payload;

        app_key_filename = g_test_build_filename (G_TEST_BUILT,
                                                  "credentials.ini",
                                                  NULL);
        app_key_file = g_key_file_new ();
        g_key_file_load_from_file (app_key_file,
                                   app_key_filename,
                                   G_KEY_FILE_NONE,
                                   &error);
        g_assert_no_error(error);

        app = g_new0(GFBGraphTestApp, 1);

        app->client_id = g_key_file_get_string (app_key_file,
                                                "Client",
                                                "ClientId",
                                                &error);
        g_assert_no_error(error);
        app->client_secret = g_key_file_get_string (app_key_file,
                                                    "Client",
                                                    "ClientSecret",
                                                    &error);
        g_assert_no_error(error);

        proxy = rest_proxy_new (FACEBOOK_ENDPOINT, FALSE);
        rest_call = rest_proxy_new_call (proxy);

        rest_proxy_call_add_param (rest_call, "client_id", app->client_id);
        rest_proxy_call_add_param (rest_call, "client_secret", app->client_secret);
        rest_proxy_call_add_param (rest_call, "grant_type", "client_credentials");

        rest_proxy_call_set_method (rest_call, "GET");
        function_path = g_strdup ("/oauth/access_token");
        rest_proxy_call_set_function (rest_call, function_path);

        rest_proxy_call_sync (rest_call, &error);
        g_assert_no_error (error);

        payload = rest_proxy_call_get_payload (rest_call);
        jparser = json_parser_new ();
        json_parser_load_from_data (jparser, payload, -1, &error);
        g_assert_no_error (error);
        jnode = json_parser_get_root (jparser);
        jreader = json_reader_new (jnode);
        json_reader_read_element (jreader, 0);
        app->access_token = g_strdup(json_reader_get_string_value (jreader));
        json_reader_end_element (jreader);

        g_clear_object(&rest_call);
        g_clear_object(&proxy);

        return app;
}

void
gfbgraph_test_fixture_setup (GFBGraphTestFixture *fixture, gconstpointer user_data)
{
        RestProxy *proxy;
        RestProxyCall *rest_call;
        g_autofree gchar *function_path = NULL;
        const gchar *payload;
        g_autoptr(GError) error = NULL;
        const GFBGraphTestApp *app = user_data;
        JsonNode *jnode;
        g_autoptr(JsonParser) jparser = NULL;
        g_autoptr(JsonReader) jreader = NULL;
        const gchar *access_token;

        /* Create a new user */

        proxy = rest_proxy_new (FACEBOOK_ENDPOINT, FALSE);
        rest_call = rest_proxy_new_call (proxy);

        /* Params as documented here: https://developers.facebook.com/docs/graph-api/reference/app/accounts/test-users#publish */
        rest_proxy_call_add_param (rest_call, "installed", "true");
        rest_proxy_call_add_param (rest_call, "permissions", FACEBOOK_TEST_USER_PERMISSIONS);
        rest_proxy_call_add_param (rest_call, "access_token", app->access_token);

        rest_proxy_call_set_method (rest_call, "POST");
        function_path = g_strdup_printf ("%s/accounts/test-users", app->client_id);
        rest_proxy_call_set_function (rest_call, function_path);

        rest_proxy_call_sync (rest_call, &error);
        g_assert_no_error (error);

        payload = rest_proxy_call_get_payload (rest_call);
        jparser = json_parser_new ();
        json_parser_load_from_data (jparser, payload, -1, &error);
        g_assert_no_error (error);
        jnode = json_parser_get_root (jparser);
        jreader = json_reader_new (jnode);

        json_reader_read_element (jreader, 0);
        fixture->user_id = g_strdup (json_reader_get_string_value (jreader));
        json_reader_end_element (jreader);
        json_reader_read_element (jreader, 1);
        access_token = json_reader_get_string_value (jreader);
        json_reader_end_element (jreader);
        json_reader_read_element (jreader, 3);
        fixture->user_email = g_strdup (json_reader_get_string_value (jreader));
        json_reader_end_element (jreader);

        fixture->authorizer = gfbgraph_simple_authorizer_new (access_token);

        g_clear_object (&rest_call);
        g_clear_object (&proxy);

}

static void
gfbgraph_test_fixture_teardown (GFBGraphTestFixture *fixture, G_GNUC_UNUSED gconstpointer user_data)
{
        g_autoptr(SoupSession) ssession = NULL;
        g_autoptr(SoupMessage) smessage = NULL;
        g_autofree gchar *function_path = NULL;
        guint status;

        /* Delete the test user and clean up memory */

        ssession = soup_session_new ();

        function_path = g_strdup_printf ("%s/%s", FACEBOOK_ENDPOINT, fixture->user_id);
        smessage = soup_message_new ("DELETE", function_path);
        gfbgraph_authorizer_process_message (GFBGRAPH_AUTHORIZER (fixture->authorizer), smessage);

        status = soup_session_send_message (ssession, smessage);
        g_assert_cmpint(status, ==, 200);

        g_free (fixture->user_id);
        g_free (fixture->user_email);
        g_object_unref (fixture->authorizer);
}

static void
gfbgraph_test_me (GFBGraphTestFixture *fixture, G_GNUC_UNUSED gconstpointer user_data)
{
        g_autoptr(GFBGraphUser) me = NULL;
        g_autoptr(GError) error = NULL;

        me = gfbgraph_user_get_me (GFBGRAPH_AUTHORIZER (fixture->authorizer), &error);
        g_assert_no_error (error);
        g_assert (GFBGRAPH_IS_USER (me));

        g_assert_cmpstr (fixture->user_id, ==, gfbgraph_node_get_id (GFBGRAPH_NODE (me)));
        g_assert_cmpstr (fixture->user_email, ==, gfbgraph_user_get_email (me));
}

static G_GNUC_UNUSED void
gfbgraph_test_album (GFBGraphTestFixture *fixture, G_GNUC_UNUSED gconstpointer user_data)
{
        GFBGraphUser *me;
        GFBGraphAlbum *album;
        GError *error = NULL;
        gboolean result;

        me = gfbgraph_user_get_me (GFBGRAPH_AUTHORIZER (fixture->authorizer), &error);
        g_assert_no_error (error);
        g_assert (GFBGRAPH_IS_USER (me));

        /* Create a photo album */
        album = gfbgraph_album_new ();
        gfbgraph_album_set_name (album, "Vanilla Sky");
        gfbgraph_album_set_description (album, "Great sunset photos in Mars!");
        result = gfbgraph_node_append_connection (GFBGRAPH_NODE (me),
                                                  GFBGRAPH_NODE (album),
                                                  GFBGRAPH_AUTHORIZER (fixture->authorizer),
                                                  &error);
        /* Asserting the connection */
        g_assert_no_error (error);
        g_assert (result);

        /* Asserting the Album ID */
        g_assert_cmpstr (gfbgraph_node_get_id (GFBGRAPH_NODE (album)), !=, "");
}

int
main (int argc, char **argv)
{
        GFBGraphTestApp *app = NULL;
        int test_result;

        g_test_init (&argc, &argv, NULL);

        g_log_set_always_fatal (G_LOG_LEVEL_ERROR | G_LOG_FLAG_RECURSION | G_LOG_FLAG_FATAL | G_LOG_LEVEL_CRITICAL);

        app = gfbgraph_test_app_setup ();

        g_test_add ("/GFBGraph/Me",
                    GFBGraphTestFixture,
                    app,
                    gfbgraph_test_fixture_setup,
                    gfbgraph_test_me,
                    gfbgraph_test_fixture_teardown);

#if 0
        g_test_add ("/GFBGraph/Album",
                    GFBGraphTestFixture,
                    app,
                    gfbgraph_test_fixture_setup,
                    gfbgraph_test_album,
                    gfbgraph_test_fixture_teardown);
#endif

        test_result = g_test_run ();

        if (app) {
                if (app->client_id)
                        g_free (app->client_id);
                if (app->client_secret)
                        g_free (app->client_secret);
                if (app->access_token)
                        g_free (app->access_token);
                g_free (app);
        }

        return test_result;
}
