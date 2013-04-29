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

#include "gfbgraph-authorizer.h"
#include "gfbgraph-goa-authorizer.h"

enum {
        PROP_O,

        PROP_GOA_OBJECT
};

struct _GFBGraphGoaAuthorizerPrivate {
        GMutex mutex;
        GoaObject *goa_object;
        gchar *access_token;
};

static void gfbgraph_goa_authorizer_class_init            (GFBGraphGoaAuthorizerClass *klass);
static void gfbgraph_goa_authorizer_init                  (GFBGraphGoaAuthorizer *object);
static void gfbgraph_goa_authorizer_finalize              (GObject *object);
static void gfbgraph_goa_authorizer_dispose               (GObject *object);
static void gfbgraph_goa_authorizer_set_property          (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void gfbgraph_goa_authorizer_get_property          (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

static void gfbgraph_goa_authorizer_iface_init            (GFBGraphAuthorizerInterface *iface);
void        gfbgraph_goa_authorizer_process_call          (GFBGraphAuthorizer *iface, RestProxyCall *call);
void        gfbgraph_goa_authorizer_process_message       (GFBGraphAuthorizer *iface, SoupMessage *message);
gboolean    gfbgraph_goa_authorizer_refresh_authorization (GFBGraphAuthorizer *iface, GCancellable *cancellable, GError **error);

static void gfbgraph_goa_authorizer_set_goa_object        (GFBGraphGoaAuthorizer *self, GoaObject *goa_object);

#define GFBGRAPH_GOA_AUTHORIZER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), GFBGRAPH_TYPE_GOA_AUTHORIZER, GFBGraphGoaAuthorizerPrivate))

static GObjectClass *parent_class = NULL;

G_DEFINE_TYPE_WITH_CODE (GFBGraphGoaAuthorizer, gfbgraph_goa_authorizer, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GFBGRAPH_TYPE_AUTHORIZER, gfbgraph_goa_authorizer_iface_init));

static void
gfbgraph_goa_authorizer_class_init (GFBGraphGoaAuthorizerClass *klass)
{
	GObjectClass *gobject_class;
	gobject_class = (GObjectClass*) klass;

	parent_class            = g_type_class_peek_parent (klass);
	gobject_class->finalize = gfbgraph_goa_authorizer_finalize;
        gobject_class->get_property = gfbgraph_goa_authorizer_get_property;
        gobject_class->set_property = gfbgraph_goa_authorizer_set_property;
        gobject_class->dispose = gfbgraph_goa_authorizer_dispose;

        g_object_class_install_property (gobject_class,
                                         PROP_GOA_OBJECT,
                                         g_param_spec_object ("goa-object",
                                                              "GoaObject",
                                                              "The GOA account to authenticate.",
                                                              GOA_TYPE_OBJECT,
                                                              G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));

	g_type_class_add_private (gobject_class, sizeof(GFBGraphGoaAuthorizerPrivate));
}

static void
gfbgraph_goa_authorizer_init (GFBGraphGoaAuthorizer *object)
{
	object->priv = GFBGRAPH_GOA_AUTHORIZER_GET_PRIVATE(object);
        g_mutex_init (&object->priv->mutex);
}

static void
gfbgraph_goa_authorizer_finalize (GObject *object)
{
	G_OBJECT_CLASS(parent_class)->finalize (object);
}

static void
gfbgraph_goa_authorizer_dispose (GObject *object)
{
        GFBGraphGoaAuthorizerPrivate *priv;

        priv = GFBGRAPH_GOA_AUTHORIZER_GET_PRIVATE (object);

        g_clear_object (&priv->goa_object);

        G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
gfbgraph_goa_authorizer_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
        switch (prop_id) {
                case PROP_GOA_OBJECT:
                        gfbgraph_goa_authorizer_set_goa_object (GFBGRAPH_GOA_AUTHORIZER (object), GOA_OBJECT (g_value_get_object (value)));
                        break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;
        }
}

static void
gfbgraph_goa_authorizer_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
        GFBGraphGoaAuthorizerPrivate *priv;

        priv = GFBGRAPH_GOA_AUTHORIZER_GET_PRIVATE (object);

        switch (prop_id) {
                case PROP_GOA_OBJECT:
                        g_value_set_object (value, priv->goa_object);
                        break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;
        }
}

static void
gfbgraph_goa_authorizer_iface_init (GFBGraphAuthorizerInterface *iface)
{
        iface->process_call = gfbgraph_goa_authorizer_process_call;
        iface->process_message = gfbgraph_goa_authorizer_process_message;
        iface->refresh_authorization = gfbgraph_goa_authorizer_refresh_authorization;
}

void
gfbgraph_goa_authorizer_process_call (GFBGraphAuthorizer *iface, RestProxyCall *call)
{
        GFBGraphGoaAuthorizerPrivate *priv = GFBGRAPH_GOA_AUTHORIZER_GET_PRIVATE (GFBGRAPH_GOA_AUTHORIZER (iface));

        g_mutex_lock (&priv->mutex);

        if (priv->access_token != NULL)
                rest_proxy_call_add_param (call, "access_token", priv->access_token);

        g_mutex_unlock (&priv->mutex);
}

void
gfbgraph_goa_authorizer_process_message (GFBGraphAuthorizer *iface, SoupMessage *message)
{
        gchar *auth_value;
        SoupURI *uri;
        GFBGraphGoaAuthorizerPrivate *priv;

        priv = GFBGRAPH_GOA_AUTHORIZER_GET_PRIVATE (GFBGRAPH_GOA_AUTHORIZER (iface));

        g_mutex_lock (&priv->mutex);

        uri = soup_message_get_uri (message);
        auth_value = g_strconcat ("access_token=", priv->access_token, NULL);
        soup_uri_set_query (uri, auth_value);

        g_free (auth_value);

        g_mutex_unlock (&priv->mutex);
}

gboolean
gfbgraph_goa_authorizer_refresh_authorization (GFBGraphAuthorizer *iface, GCancellable *cancellable, GError **error)
{
        GFBGraphGoaAuthorizerPrivate *priv;
        GoaAccount *account;
        GoaOAuth2Based *oauth2_based;
        gboolean ret_val;

        priv = GFBGRAPH_GOA_AUTHORIZER_GET_PRIVATE (GFBGRAPH_GOA_AUTHORIZER (iface));
        ret_val = FALSE;

        g_mutex_lock (&priv->mutex);

        g_free (priv->access_token);
        priv->access_token = NULL;

        account = goa_object_peek_account (priv->goa_object);
        oauth2_based = goa_object_peek_oauth2_based (priv->goa_object);

        if (goa_account_call_ensure_credentials_sync (account, NULL, cancellable, error))
                if (goa_oauth2_based_call_get_access_token_sync (oauth2_based, &priv->access_token, NULL, cancellable, error))
                        ret_val = TRUE;

        g_mutex_unlock (&priv->mutex);
        return ret_val;
}

static void
gfbgraph_goa_authorizer_set_goa_object (GFBGraphGoaAuthorizer *self, GoaObject *goa_object)
{
        GoaAccount *account;
        GoaOAuth2Based *oauth2_based;
        GFBGraphGoaAuthorizerPrivate *priv;

        g_return_if_fail (GOA_IS_OBJECT (goa_object));

        priv = GFBGRAPH_GOA_AUTHORIZER_GET_PRIVATE (self);

        oauth2_based = goa_object_peek_oauth2_based (goa_object);
        g_return_if_fail (oauth2_based != NULL && GOA_IS_OAUTH2_BASED (oauth2_based));

        account = goa_object_peek_account (goa_object);
        g_return_if_fail (account != NULL && GOA_IS_ACCOUNT (account));

        g_object_ref (goa_object);
        priv->goa_object = goa_object;
}

/**
 * gfbgraph_goa_authorizer_new:
 * @goa_object: A #GoaObject representing a Facebook account.
 *
 * Creates a new #GFBGraphGoaAuthorizer using @goa_object as account.
 *
 * Returns: A new #GFBGraphGoaAuthorizer. Use g_object_unref() to free it.
 */
GFBGraphGoaAuthorizer*
gfbgraph_goa_authorizer_new (GoaObject *goa_object)
{
	return GFBGRAPH_GOA_AUTHORIZER (g_object_new (GFBGRAPH_TYPE_GOA_AUTHORIZER, "goa-object", goa_object, NULL));
}
