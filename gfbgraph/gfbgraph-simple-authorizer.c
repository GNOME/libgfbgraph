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
 * libgfbgraph is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION:gfbgraph-simple-authorizer
 * @short_description: GFBGraph simple authorization interface
 * @stability: Unstable
 * @include: gfbgraph/gfbgraph-simple-authorizer.h
 *
 * #GFBGraphSimpleAuthorizer provides an implementation of the #GFBGraphAuthorizer interface
 * for authorization using a access token provided in creation time, no refresh, just a simple
 * authorizer for test purposes, don't use in final code.
 **/

#include "gfbgraph-authorizer.h"
#include "gfbgraph-simple-authorizer.h"

enum
{
        PROP_0,

        PROP_ACCESS_TOKEN
};

struct _GFBGraphSimpleAuthorizerPrivate {
        GMutex mutex;
        gchar *access_token;
};

static void gfbgraph_simple_authorizer_init         (GFBGraphSimpleAuthorizer *obj);
static void gfbgraph_simple_authorizer_class_init   (GFBGraphSimpleAuthorizerClass *klass);
static void gfbgraph_simple_authorizer_finalize     (GObject *obj);
static void gfbgraph_simple_authorizer_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void gfbgraph_simple_authorizer_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

static void gfbgraph_simple_authorizer_iface_init (GFBGraphAuthorizerInterface *iface);

void     gfbgraph_simple_authorizer_process_call          (GFBGraphAuthorizer *iface, RestProxyCall *call);
void     gfbgraph_simple_authorizer_process_message       (GFBGraphAuthorizer *iface, SoupMessage *message);
gboolean gfbgraph_simple_authorizer_refresh_authorization (GFBGraphAuthorizer *iface, GCancellable *cancellable, GError **error);

#define GFBGRAPH_SIMPLE_AUTHORIZER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), GFBGRAPH_TYPE_SIMPLE_AUTHORIZER, GFBGraphSimpleAuthorizerPrivate))

static GObjectClass *parent_class = NULL;

G_DEFINE_TYPE_WITH_CODE (GFBGraphSimpleAuthorizer, gfbgraph_simple_authorizer, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GFBGRAPH_TYPE_AUTHORIZER, gfbgraph_simple_authorizer_iface_init));


static void
gfbgraph_simple_authorizer_init (GFBGraphSimpleAuthorizer *obj)
{
        obj->priv = GFBGRAPH_SIMPLE_AUTHORIZER_GET_PRIVATE(obj);
        g_mutex_init (&obj->priv->mutex);
}

static void
gfbgraph_simple_authorizer_class_init (GFBGraphSimpleAuthorizerClass *klass)
{
        GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

        parent_class            = g_type_class_peek_parent (klass);
        gobject_class->finalize = gfbgraph_simple_authorizer_finalize;
        gobject_class->set_property = gfbgraph_simple_authorizer_set_property;
        gobject_class->get_property = gfbgraph_simple_authorizer_get_property;

        g_type_class_add_private (gobject_class, sizeof(GFBGraphSimpleAuthorizerPrivate));

        /**
         * GFBGraphSimpleAuthorizer:access_token:
         *
         * The access token for the Facebook Graph API, normally, take it by hand in the 
         * Graph API explorer tool: https://developers.facebook.com/tools/explorer.
         **/
        g_object_class_install_property (gobject_class,
                                         PROP_ACCESS_TOKEN,
                                         g_param_spec_string ("access-token",
                                                              "The access token", "The access token for the Facebook Graph API.",
                                                              "",
                                                              G_PARAM_READABLE | G_PARAM_WRITABLE));
}

static void
gfbgraph_simple_authorizer_finalize (GObject *obj)
{
        G_OBJECT_CLASS(parent_class)->finalize (obj);
}

static void
gfbgraph_simple_authorizer_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
        GFBGraphSimpleAuthorizerPrivate *priv;

        priv = GFBGRAPH_SIMPLE_AUTHORIZER_GET_PRIVATE (object);

        switch (prop_id) {
                case PROP_ACCESS_TOKEN:
                        if (priv->access_token)
                                g_free (priv->access_token);
                        priv->access_token = g_strdup (g_value_get_string (value));
                        break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;
        }  
}

static void
gfbgraph_simple_authorizer_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
        GFBGraphSimpleAuthorizerPrivate *priv;

        priv = GFBGRAPH_SIMPLE_AUTHORIZER_GET_PRIVATE (object);

        switch (prop_id) {
                case PROP_ACCESS_TOKEN:
                        g_value_set_string (value, priv->access_token);
                        break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;
        }
}

static void
gfbgraph_simple_authorizer_iface_init (GFBGraphAuthorizerInterface *iface)
{
        iface->process_call = gfbgraph_simple_authorizer_process_call;
        iface->process_message = gfbgraph_simple_authorizer_process_message;
        iface->refresh_authorization = gfbgraph_simple_authorizer_refresh_authorization;
}

void
gfbgraph_simple_authorizer_process_call (GFBGraphAuthorizer *iface, RestProxyCall *call)
{
        GFBGraphSimpleAuthorizerPrivate *priv;

        priv = GFBGRAPH_SIMPLE_AUTHORIZER_GET_PRIVATE (GFBGRAPH_SIMPLE_AUTHORIZER (iface));

        g_mutex_lock (&priv->mutex);
        rest_proxy_call_add_param (call, "access_token", priv->access_token);
        g_mutex_unlock (&priv->mutex);
}

void
gfbgraph_simple_authorizer_process_message (GFBGraphAuthorizer *iface, SoupMessage *message)
{
        gchar *auth_value;
        SoupURI *uri;
        GFBGraphSimpleAuthorizerPrivate *priv;

        priv = GFBGRAPH_SIMPLE_AUTHORIZER_GET_PRIVATE (GFBGRAPH_SIMPLE_AUTHORIZER (iface));

        g_mutex_lock (&priv->mutex);

        uri = soup_message_get_uri (message);
        auth_value = g_strconcat ("access_token=", priv->access_token, NULL);
        soup_uri_set_query (uri, auth_value);

        g_free (auth_value);

        g_mutex_unlock (&priv->mutex);
}

gboolean
gfbgraph_simple_authorizer_refresh_authorization (GFBGraphAuthorizer *iface, GCancellable *cancellable, GError **error)
{
        return FALSE;
}

/**
 * gfbgraph_simple_authorizer_new:
 * @access_token: a const @gchar.
 *
 * Creates a new #GFBGraphAuthorizer to use with the GFBGraph library using the @access_token as access token.
 * It's only a test authorizer, don't use in final code.
 *
 * Returns: (transfer full): a #GFBGraphSimpleAuthorizer.
 **/
GFBGraphSimpleAuthorizer*
gfbgraph_simple_authorizer_new (const gchar *access_token)
{
        return GFBGRAPH_SIMPLE_AUTHORIZER (g_object_new (GFBGRAPH_TYPE_SIMPLE_AUTHORIZER, "access-token", access_token, NULL));
}
