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

#include "gfbgraph-photo.h"
#include "gfbgraph-connectable.h"
#include "gfbgraph-album.h"

#define LIBSOUP_USE_UNSTABLE_REQUEST_API
#include <libsoup/soup.h>
#include <libsoup/soup-request.h>
#include <libsoup/soup-request-http.h>
#include <libsoup/soup-requester.h>

enum {
        PROP_0,

        PROP_NAME,
        PROP_SOURCE,
        PROP_HEIGHT,
        PROP_WIDTH
};

struct _GFBGraphPhotoPrivate {
        gchar *name;
        gchar *source;
        guint  width;
        guint  height;
};

static void gfbgraph_photo_init         (GFBGraphPhoto *obj);
static void gfbgraph_photo_class_init   (GFBGraphPhotoClass *klass);
static void gfbgraph_photo_finalize     (GObject *obj);
static void gfbgraph_photo_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void gfbgraph_photo_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

static void gfbgraph_photo_connectable_iface_init     (GFBGraphConnectableInterface *iface);
GHashTable* gfbgraph_photo_get_connection_post_params (GFBGraphConnectable *self, GType node_type);

#define GFBGRAPH_PHOTO_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), GFBGRAPH_TYPE_PHOTO, GFBGraphPhotoPrivate))

static GFBGraphNodeClass *parent_class = NULL;

G_DEFINE_TYPE_WITH_CODE (GFBGraphPhoto, gfbgraph_photo, GFBGRAPH_TYPE_NODE,
                         G_IMPLEMENT_INTERFACE (GFBGRAPH_TYPE_CONNECTABLE, gfbgraph_photo_connectable_iface_init));

static void
gfbgraph_photo_init (GFBGraphPhoto *obj)
{
        obj->priv = GFBGRAPH_PHOTO_GET_PRIVATE(obj);
}

static void
gfbgraph_photo_class_init (GFBGraphPhotoClass *klass)
{
        GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

        parent_class            = g_type_class_peek_parent (klass);
        gobject_class->finalize = gfbgraph_photo_finalize;
        gobject_class->set_property = gfbgraph_photo_set_property;
        gobject_class->get_property = gfbgraph_photo_get_property;

        g_type_class_add_private (gobject_class, sizeof(GFBGraphPhotoPrivate));

        g_object_class_install_property (gobject_class,
                                         PROP_NAME,
                                         g_param_spec_string ("name",
                                                              "The photo name", "The name given by the user to the photo",
                                                              "",
                                                              G_PARAM_READABLE | G_PARAM_WRITABLE));

        g_object_class_install_property (gobject_class,
                                         PROP_SOURCE,
                                         g_param_spec_string ("source",
                                                              "The URI for the photo", "The URI for the photo, with a maximum width or height of 720px",
                                                              "",
                                                              G_PARAM_READABLE | G_PARAM_WRITABLE));

        g_object_class_install_property (gobject_class,
                                         PROP_WIDTH,
                                         g_param_spec_uint ("width",
                                                            "Photo width", "The photo width",
                                                            0, G_MAXUINT, 0,
                                                            G_PARAM_READABLE | G_PARAM_WRITABLE));

        g_object_class_install_property (gobject_class,
                                         PROP_HEIGHT,
                                         g_param_spec_uint ("height",
                                                            "Photo height", "The photo height",
                                                            0, G_MAXUINT, 0,
                                                            G_PARAM_READABLE | G_PARAM_WRITABLE));
}

static void
gfbgraph_photo_finalize (GObject *obj)
{
        G_OBJECT_CLASS(parent_class)->finalize (obj);
}

static void
gfbgraph_photo_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
        GFBGraphPhotoPrivate *priv;

        priv = GFBGRAPH_PHOTO_GET_PRIVATE (object);

        switch (prop_id) {
                case PROP_NAME:
                        if (priv->name)
                                g_free (priv->name);
                        priv->name = g_strdup (g_value_get_string (value));
                        break;
                case PROP_SOURCE:
                        if (priv->source)
                                g_free (priv->source);
                        priv->source = g_strdup (g_value_get_string (value));
                        break;
                case PROP_WIDTH:
                        priv->width = g_value_get_uint (value);
                        break;
                case PROP_HEIGHT:
                        priv->height = g_value_get_uint (value);
                        break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;
        }
}

static void
gfbgraph_photo_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
        GFBGraphPhotoPrivate *priv;

        priv = GFBGRAPH_PHOTO_GET_PRIVATE (object);

        switch (prop_id) {
                case PROP_NAME:
                        g_value_set_string (value, priv->name);
                        break;
                case PROP_SOURCE:
                        g_value_set_string (value, priv->source);
                        break;
                case PROP_WIDTH:
                        g_value_set_uint (value, priv->width);
                        break;
                case PROP_HEIGHT:
                        g_value_set_uint (value, priv->height);
                        break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;
        }
}

static void
gfbgraph_photo_connectable_iface_init (GFBGraphConnectableInterface *iface)
{
        GHashTable *connections;

        connections = g_hash_table_new (g_str_hash, g_str_equal);
        g_hash_table_insert (connections, (gpointer) g_type_name (GFBGRAPH_TYPE_ALBUM), (gpointer) "photos");

        iface->connections = connections;
        iface->get_connection_post_params = gfbgraph_photo_get_connection_post_params;
        iface->parse_connected_data = gfbgraph_connectable_default_parse_connected_data;
}

GHashTable*
gfbgraph_photo_get_connection_post_params (GFBGraphConnectable *self, GType node_type)
{
        GHashTable *params;
        GFBGraphPhotoPrivate *priv;

        priv = GFBGRAPH_PHOTO_GET_PRIVATE (self);

        params = g_hash_table_new (g_str_hash, g_str_equal);
        g_hash_table_insert (params, "message", priv->name);
        /* TODO: Incorpate the "source" param (multipart/form-data) */

        return params;
}

GFBGraphPhoto*
gfbgraph_photo_new (void)
{
        return GFBGRAPH_PHOTO(g_object_new(GFBGRAPH_TYPE_PHOTO, NULL));
}

GInputStream*
gfbgraph_photo_download_default_size (GFBGraphPhoto *photo, GFBGraphAuthorizer *authorizer)
{
        GInputStream *stream = NULL;
        SoupSession *session;
        SoupRequester *requester;
        SoupRequest *request;
        SoupMessage *message;
        GError *error = NULL;
        GFBGraphPhotoPrivate *priv;

        g_return_val_if_fail (GFBGRAPH_IS_PHOTO (photo), NULL);
        g_return_val_if_fail (GFBGRAPH_IS_AUTHORIZER (authorizer), NULL);

        priv = GFBGRAPH_PHOTO_GET_PRIVATE (photo);

        session = soup_session_sync_new ();
        requester = soup_requester_new ();
        soup_session_add_feature (session, SOUP_SESSION_FEATURE (requester));

        request = soup_requester_request (requester, priv->source, &error);
        if (request != NULL) {
                message = soup_request_http_get_message (SOUP_REQUEST_HTTP (request));
                gfbgraph_authorizer_process_message (authorizer, message);

                stream = soup_request_send (request, NULL, &error);
                if (stream != NULL) {
                        g_object_weak_ref (G_OBJECT (stream), (GWeakNotify) g_object_unref, session);
                }

                g_clear_object (&message);
                g_clear_object (&request);
        }

        g_clear_object (&requester);

        return stream;
}
