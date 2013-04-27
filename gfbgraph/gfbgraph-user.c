/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 8; tab-width: 8 -*-  */
/*
 * libgfbgraph - GObject library for Facebook Graph API
 * Copyright (C) 2013 Álvaro Peña <alvaropg@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libginstapaper is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gfbgraph-user.h"
#include "gfbgraph-album.h"
#include "gfbgraph-common.h"

#include <json-glib/json-glib.h>

#define ME_FUNCTION "me"

enum {
        PROP_0,

        PROP_NAME
};

struct _GFBGraphUserPrivate {
        gchar *name;
};

typedef struct {
        GFBGraphUser *user;
} GFBGraphUserAsyncData;

static void gfbgraph_user_init         (GFBGraphUser *obj);
static void gfbgraph_user_class_init   (GFBGraphUserClass *klass);
static void gfbgraph_user_finalize     (GObject *obj);
static void gfbgraph_user_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void gfbgraph_user_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

/* Private functions */
static void gfbgraph_user_async_data_free (GFBGraphUserAsyncData *data);
static void gfbgraph_user_get_me_async_thread (GSimpleAsyncResult *simple_async, GFBGraphAuthorizer *authorizer, GCancellable cancellable);

#define GFBGRAPH_USER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), GFBGRAPH_TYPE_USER, GFBGraphUserPrivate))

static GFBGraphNodeClass *parent_class = NULL;

G_DEFINE_TYPE (GFBGraphUser, gfbgraph_user, GFBGRAPH_TYPE_NODE);

static void
gfbgraph_user_init (GFBGraphUser *obj)
{
	obj->priv = GFBGRAPH_USER_GET_PRIVATE(obj);
}

static void
gfbgraph_user_class_init (GFBGraphUserClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	parent_class            = g_type_class_peek_parent (klass);
	gobject_class->finalize = gfbgraph_user_finalize;
        gobject_class->set_property = gfbgraph_user_set_property;
        gobject_class->get_property = gfbgraph_user_get_property;

	g_type_class_add_private (gobject_class, sizeof(GFBGraphUserPrivate));

        g_object_class_install_property (gobject_class,
                                         PROP_NAME,
                                         g_param_spec_string ("name",
                                                              "User's full name", "The full name of the user",
                                                              "",
                                                              G_PARAM_READABLE | G_PARAM_WRITABLE));
}

static void
gfbgraph_user_finalize (GObject *obj)
{
	G_OBJECT_CLASS(parent_class)->finalize (obj);
}

static void
gfbgraph_user_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
        GFBGraphUserPrivate *priv;

        priv = GFBGRAPH_USER_GET_PRIVATE (object);

        switch (prop_id) {
                case PROP_NAME:
                        if (priv->name)
                                g_free (priv->name);
                        priv->name = g_strdup (g_value_get_string (value));
                        break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;
        }
}

static void
gfbgraph_user_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
        GFBGraphUserPrivate *priv;

        priv = GFBGRAPH_USER_GET_PRIVATE (object);

        switch (prop_id) {
                case PROP_NAME:
                        g_value_set_string (value, priv->name);
                        break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;
        }
}

static void
gfbgraph_user_async_data_free (GFBGraphUserAsyncData *data)
{
	g_object_unref (data->user);

	g_slice_free (GFBGraphUserAsyncData, data);
}

static void
gfbgraph_user_get_me_async_thread (GSimpleAsyncResult *simple_async, GFBGraphAuthorizer *authorizer, GCancellable cancellable)
{
        GFBGraphUserAsyncData *data;
        GError *error;

        data = (GFBGraphUserAsyncData *) g_simple_async_result_get_op_res_gpointer (simple_async);

        error = NULL;
        data->user = gfbgraph_user_get_me (authorizer, &error);
        if (error != NULL)
                g_simple_async_result_take_error (simple_async, error);
}

GFBGraphUser*
gfbgraph_user_new (void)
{
	return GFBGRAPH_USER (g_object_new (GFBGRAPH_TYPE_USER, NULL));
}

GFBGraphUser*
gfbgraph_user_get_me (GFBGraphAuthorizer *authorizer, GError **error)
{
        GFBGraphUser *me;
        RestProxyCall *rest_call;
        const gchar *payload;
        gboolean result;

        g_return_val_if_fail (GFBGRAPH_IS_AUTHORIZER (authorizer), NULL);

        me = NULL;

        rest_call = gfbgraph_new_rest_call (authorizer);
        rest_proxy_call_set_function (rest_call, ME_FUNCTION);
        rest_proxy_call_set_method (rest_call, "GET");

        result = rest_proxy_call_sync (rest_call, error);
        if (result) {
                JsonParser *parser;
                JsonNode *node;

                payload = rest_proxy_call_get_payload (rest_call);
                parser = json_parser_new ();
                if (json_parser_load_from_data (parser, payload, -1, error)) {
                        node = json_parser_get_root (parser);
                        me = GFBGRAPH_USER (json_gobject_deserialize (GFBGRAPH_TYPE_USER, node));
                }

                g_object_unref (parser);
        }

        return me;
}

void
gfbgraph_user_get_me_async (GFBGraphAuthorizer *authorizer, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data)
{
        GSimpleAsyncResult *simple_async;
        GFBGraphUserAsyncData *data;

        g_return_if_fail (GFBGRAPH_IS_AUTHORIZER (authorizer));
        g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));
	g_return_if_fail (callback != NULL);

        simple_async = g_simple_async_result_new (G_OBJECT (authorizer), callback, user_data, gfbgraph_user_get_me_async);
        g_simple_async_result_set_check_cancellable (simple_async, cancellable);

        data = g_slice_new (GFBGraphUserAsyncData);
        data->user = NULL;

        g_simple_async_result_set_op_res_gpointer (simple_async, data, (GDestroyNotify) gfbgraph_user_async_data_free);
        g_simple_async_result_run_in_thread (simple_async, (GSimpleAsyncThreadFunc) gfbgraph_user_get_me_async_thread, G_PRIORITY_DEFAULT, cancellable);

        g_object_unref (simple_async);
}

GFBGraphUser*
gfbgraph_user_get_me_async_finish (GFBGraphAuthorizer *authorizer, GAsyncResult *result, GError **error)
{
        GSimpleAsyncResult *simple_async;
        GFBGraphUserAsyncData *data;

        g_return_val_if_fail (g_simple_async_result_is_valid (result, G_OBJECT (authorizer), gfbgraph_user_get_me_async), NULL);
        g_return_val_if_fail (error == NULL || *error == NULL, NULL);

        simple_async = G_SIMPLE_ASYNC_RESULT (result);

        if (g_simple_async_result_propagate_error (simple_async, error))
                return NULL;

        data = (GFBGraphUserAsyncData *) g_simple_async_result_get_op_res_gpointer (simple_async);
        return data->user;
}
