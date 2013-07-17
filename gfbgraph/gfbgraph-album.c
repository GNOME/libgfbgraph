/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 8; tab-width: 8 -*-  */
/*
 * libgfbgraph - GObject library for Facebook Graph API
 * Copyright (C) 2013 Álvaro Peña <alvaropg@gmail.com>
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

/**
 * SECTION:gfbgraph-album
 * @short_description: GFBGraph Photo album node object
 * @stability: Unstable
 * @include: gfbgraph/gfbgraph.h
 *
 * #GFGraphAlbum represents the <ulink url="https://developers.facebook.com/docs/reference/api/album/">
 * photo album node in the Graph API</ulink>.
 *
 * This node is connectable to:
 *  - #GFBGraphUser
 **/

#include "gfbgraph-album.h"
#include "gfbgraph-user.h"
#include "gfbgraph-connectable.h"

enum {
        PROP_O,

        PROP_NAME,
        PROP_DESCRIPTION,
        PROP_COVER_PHOTO,
        PROP_COUNT
};

struct _GFBGraphAlbumPrivate {
        gchar *name;
        gchar *description;
        gchar *cover_photo;
        guint  count;
};

static void gfbgraph_album_init         (GFBGraphAlbum *obj);
static void gfbgraph_album_class_init   (GFBGraphAlbumClass *klass);
static void gfbgraph_album_finalize     (GObject *obj);
static void gfbgraph_album_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void gfbgraph_album_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

static void gfbgraph_album_connectable_iface_init (GFBGraphConnectableInterface *iface);
GHashTable* gfbgraph_album_get_connection_post_params (GFBGraphConnectable *self, GType node_type);

#define GFBGRAPH_ALBUM_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), GFBGRAPH_TYPE_ALBUM, GFBGraphAlbumPrivate))

static GFBGraphNodeClass *parent_class = NULL;

G_DEFINE_TYPE_WITH_CODE (GFBGraphAlbum, gfbgraph_album, GFBGRAPH_TYPE_NODE,
                         G_IMPLEMENT_INTERFACE (GFBGRAPH_TYPE_CONNECTABLE, gfbgraph_album_connectable_iface_init));

static void
gfbgraph_album_init (GFBGraphAlbum *obj)
{
	obj->priv = GFBGRAPH_ALBUM_GET_PRIVATE(obj);
}

static void
gfbgraph_album_class_init (GFBGraphAlbumClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	parent_class            = g_type_class_peek_parent (klass);

	gobject_class->finalize = gfbgraph_album_finalize;
        gobject_class->set_property = gfbgraph_album_set_property;
        gobject_class->get_property = gfbgraph_album_get_property;

	g_type_class_add_private (gobject_class, sizeof(GFBGraphAlbumPrivate));

        /**
         * GFBGraphAlbum:name:
         *
         * The album name.
         **/
        g_object_class_install_property (gobject_class,
                                         PROP_NAME,
                                         g_param_spec_string ("name",
                                                              "The title", "The name of the album",
                                                              "",
                                                              G_PARAM_READABLE | G_PARAM_WRITABLE));

        /**
         * GFBGraphAlbum:description:
         *
         * The album description given by the owner.
         **/
        g_object_class_install_property (gobject_class,
                                         PROP_DESCRIPTION,
                                         g_param_spec_string ("description",
                                                              "The description", "The description of the album",
                                                              "",
                                                              G_PARAM_READABLE | G_PARAM_WRITABLE));

        /**
         * GFBGraphAlbum:cover_photo:
         *
         * The node ID for the album cover photo. It's an ID for a #GFBGraphPhoto node.
         **/
        g_object_class_install_property (gobject_class,
                                         PROP_COVER_PHOTO,
                                         g_param_spec_string ("cover_photo",
                                                              "Cover photo", "The ID for the cover photo of the album",
                                                              "",
                                                              G_PARAM_READABLE | G_PARAM_WRITABLE));

        /**
         * GFBGraphAlbum:count:
         *
         * The number of photos in the album.
         **/
        g_object_class_install_property (gobject_class,
                                         PROP_COUNT,
                                         g_param_spec_uint ("count",
                                                           "Number of photos", "The number of photos in the album",
                                                           0, G_MAXUINT, 0,
                                                           G_PARAM_READABLE | G_PARAM_WRITABLE));

}

static void
gfbgraph_album_finalize (GObject *obj)
{
	G_OBJECT_CLASS(parent_class)->finalize (obj);
}

static void
gfbgraph_album_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
        GFBGraphAlbumPrivate *priv;

        priv = GFBGRAPH_ALBUM_GET_PRIVATE (object);

        switch (prop_id) {
                case PROP_NAME:
                        if (priv->name)
                                g_free (priv->name);
                        priv->name = g_strdup (g_value_get_string (value));
                        break;
                case PROP_DESCRIPTION:
                        if (priv->description)
                                g_free (priv->description);
                        priv->description = g_strdup (g_value_get_string (value));
                        break;
                case PROP_COVER_PHOTO:
                        if (priv->cover_photo)
                                g_free (priv->cover_photo);
                        priv->cover_photo = g_strdup (g_value_get_string (value));
                        break;
                case PROP_COUNT:
                        priv->count = g_value_get_uint (value);
                        break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;
        }
}

static void
gfbgraph_album_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
        GFBGraphAlbumPrivate *priv;

        priv = GFBGRAPH_ALBUM_GET_PRIVATE (object);

        switch (prop_id) {
                case PROP_NAME:
                        g_value_set_string (value, priv->name);
                        break;
                case PROP_DESCRIPTION:
                        g_value_set_string (value, priv->description);
                        break;
                case PROP_COVER_PHOTO:
                        g_value_set_string (value, priv->cover_photo);
                        break;
                case PROP_COUNT:
                        g_value_set_uint (value, priv->count);
                        break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;
        }
}

static void
gfbgraph_album_connectable_iface_init (GFBGraphConnectableInterface *iface)
{
        GHashTable *connections;

        connections = g_hash_table_new (g_str_hash, g_str_equal);
        g_hash_table_insert (connections, (gpointer) g_type_name (GFBGRAPH_TYPE_USER), (gpointer) "albums");

        iface->connections = connections;
        iface->get_connection_post_params = gfbgraph_album_get_connection_post_params;
        iface->parse_connected_data = gfbgraph_connectable_default_parse_connected_data;
}

GHashTable*
gfbgraph_album_get_connection_post_params (GFBGraphConnectable *self, GType node_type)
{
        GHashTable *params;
        GFBGraphAlbumPrivate *priv;

        priv = GFBGRAPH_ALBUM_GET_PRIVATE (self);

        params = g_hash_table_new (g_str_hash, g_str_equal);
        g_hash_table_insert (params, "name", priv->name);
        if (priv->description != NULL)
                g_hash_table_insert (params, "message", priv->description);
        /* TODO: Incorpate the "privacy" param */

        return params;
}

/**
 * gfbgraph_album_new:
 *
 * Creates a new #GFBGraphAlbum.
 *
 * Returns: (transfer full): a new #GFBGraphAlbum; unref with g_object_unref() 
 **/
GFBGraphAlbum*
gfbgraph_album_new (void)
{
	return GFBGRAPH_ALBUM (g_object_new(GFBGRAPH_TYPE_ALBUM, NULL));
}

/**
 * gfbgraph_album_new_from_id:
 * @authorizer: a #GFBGraphAuthorizer.
 * @id: a const #gchar with the album ID.
 * @error: (allow-none): a #GError or %NULL.
 *
 * Retrieves an album node from the Facebook Graph with the give ID.
 *
 * Returns: (transfer full): a new #GFBGraphAlbum; unref with g_object_unref()
 **/
GFBGraphAlbum*
gfbgraph_album_new_from_id (GFBGraphAuthorizer *authorizer, const gchar *id, GError **error)
{
        return GFBGRAPH_ALBUM (gfbgraph_node_new_from_id (authorizer, id, GFBGRAPH_TYPE_ALBUM, error));
}

/**
 * gfbgraph_album_get_name:
 * @album: a #GFBGraphAlbum.
 *
 * Returns: (transfer none): the @album name, or %NULL.
 **/
const gchar*
gfbgraph_album_get_name (GFBGraphAlbum *album)
{
        g_return_val_if_fail (GFBGRAPH_IS_ALBUM (album), NULL);

        return album->priv->name;
}

/**
 * gfbgraph_album_get_description:
 * @album: a #GFBGraphAlbum.
 *
 * Returns: (transfer none): the @album description, or %NULL.
 **/
const gchar*
gfbgraph_album_get_description (GFBGraphAlbum *album)
{
        g_return_val_if_fail (GFBGRAPH_IS_ALBUM (album), NULL);

        return album->priv->description;
}

/**
 * gfbgraph_album_get_cover_photo:
 * @album: a #GFBGraphAlbum.
 *
 * Returns: (transfer none): the cover photo node ID or %NULL.
 **/
const gchar*
gfbgraph_album_get_cover_photo_id (GFBGraphAlbum *album)
{
        g_return_val_if_fail (GFBGRAPH_IS_ALBUM (album), NULL);

        return album->priv->cover_photo;
}

/**
 * gfbgraph_album_get_count:
 * @album: a #GFBGraphAlbum.
 *
 * Returns: (transfer none): the number of photos into the @album or -1 in case of error.
 **/
guint
gfbgraph_album_get_count (GFBGraphAlbum *album)
{
        g_return_val_if_fail (GFBGRAPH_IS_ALBUM (album), -1);

        return album->priv->count;
}
