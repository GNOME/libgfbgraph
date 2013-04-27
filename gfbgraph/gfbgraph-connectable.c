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

#include "gfbgraph-connectable.h"
#include "gfbgraph-node.h"

#include <json-glib/json-glib.h>

G_DEFINE_INTERFACE (GFBGraphConnectable, gfbgraph_connectable, GFBGRAPH_TYPE_NODE)

static void
gfbgraph_connectable_default_init (GFBGraphConnectableInterface *iface)
{
        iface->connections = NULL;

        iface->get_connection_post_params = NULL;
        iface->parse_connected_data = NULL;
}

static GHashTable*
get_connections (GFBGraphConnectableInterface *iface)
{
        GHashTable *connections;

        connections = iface->connections;
        /* If no connections... Why you implement this iface? */
        g_assert (g_hash_table_size (connections) > 0);

        return connections;
}

GHashTable*
gfbgraph_connectable_get_connection_post_params (GFBGraphConnectable *self, GType node_type)
{
        GFBGraphConnectableInterface *iface;

        g_return_val_if_fail (GFBGRAPH_IS_CONNECTABLE (self), NULL);
        g_return_val_if_fail (g_type_is_a (node_type, GFBGRAPH_TYPE_NODE), NULL);
        g_return_val_if_fail (gfbgraph_connectable_is_connectable_to (self, node_type), NULL);

        iface = GFBGRAPH_CONNECTABLE_GET_IFACE (self);
        g_assert (iface->get_connection_post_params != NULL);

        return iface->get_connection_post_params (self, node_type);
}

GList*
gfbgraph_connectable_parse_connected_data (GFBGraphConnectable *self, const gchar *payload, GError **error)
{
        GFBGraphConnectableInterface *iface;

        g_return_val_if_fail (GFBGRAPH_IS_CONNECTABLE (self), NULL);

        iface = GFBGRAPH_CONNECTABLE_GET_IFACE (self);
        g_assert (iface->parse_connected_data != NULL);

        return iface->parse_connected_data (self, payload, error);
}

gboolean
gfbgraph_connectable_is_connectable_to (GFBGraphConnectable *self, GType node_type)
{
        GFBGraphConnectableInterface *iface;
        GHashTable *connections;

        g_return_val_if_fail (GFBGRAPH_IS_CONNECTABLE (self), FALSE);
        g_return_val_if_fail (g_type_is_a (node_type, GFBGRAPH_TYPE_NODE), FALSE);

        iface = GFBGRAPH_CONNECTABLE_GET_IFACE (self);

        connections = get_connections (iface);
        return g_hash_table_contains (connections, g_type_name (node_type));
}

const gchar*
gfbgraph_connectable_get_connection_path (GFBGraphConnectable *self, GType node_type)
{
        GFBGraphConnectableInterface *iface;
        GHashTable *connections;

        g_return_val_if_fail (GFBGRAPH_IS_CONNECTABLE (self), NULL);
        g_return_val_if_fail (g_type_is_a (node_type, GFBGRAPH_TYPE_NODE), NULL);
        g_return_val_if_fail (gfbgraph_connectable_is_connectable_to (self, node_type), NULL);

        iface = GFBGRAPH_CONNECTABLE_GET_IFACE (self);

        connections = get_connections (iface);
        return (const gchar *) g_hash_table_lookup (connections, g_type_name (node_type));
}

GList*
gfbgraph_connectable_default_parse_connected_data (GFBGraphConnectable *self, const gchar *payload, GError **error)
{
        GList *nodes_list = NULL;
        JsonParser *jparser;
        GType node_type;

        node_type = G_OBJECT_TYPE (self);

        jparser = json_parser_new ();
        if (json_parser_load_from_data (jparser, payload, -1, error)) {
                JsonNode *root_jnode;
                JsonObject *main_jobject;
                JsonArray *nodes_jarray;
                int i = 0;

                root_jnode = json_parser_get_root (jparser);
                main_jobject = json_node_get_object (root_jnode);
                nodes_jarray = json_object_get_array_member (main_jobject, "data");
                for (i = 0; i < json_array_get_length (nodes_jarray); i++) {
                        JsonNode *jnode;
                        GFBGraphNode *node;

                        jnode = json_array_get_element (nodes_jarray, i);
                        node = GFBGRAPH_NODE (json_gobject_deserialize (node_type, jnode));
                        nodes_list = g_list_append (nodes_list, node);
                }
        }

        g_clear_object (&jparser);

        return nodes_list;
}
