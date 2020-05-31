/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * libgfbgraph - GObject library for Facebook Graph API
 * Copyright (C) 2013 Álvaro Peña <alvaropg@gmail.com>
 *               2020 Leesoo Ahn <yisooan@fedoraproject.org>
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
 * SECTION:gfbgraph-connectable
 * @title: GFBGraphConnectable
 * @short_description: Connectable interface for nodes
 * @include: gfbgraph/gfbgraph.h
 *
 * #GFBGraphConnectable interface allow the connection between nodes.
 * You can see the posible (not necesary implemented) connections in
 * the section "Connections" in any node object in the
 * <ulink url="https://developers.facebook.com/docs/reference/api/">Facebook Graph API documentation</ulink>
 **/

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

static GHashTable *
get_connections (GFBGraphConnectableInterface *iface)
{
  /* The GHashTable contains the connections for a node.
   * The key must be the g_type_name() of a GFBGRAPH_TYPE_NODE, and the value
   * must be the function name to call in order to retrieve the nodes connected
   * to the GFBGraphNode indicated by GFBGRAPH_TYPE_NODE.
   */
  GHashTable *connections;

  connections = iface->connections;
  /* If no connections... Why you implement this iface? */
  g_assert (g_hash_table_size (connections) > 0);

  return connections;
}

/**
 * gfbgraph_connectable_get_connection_post_params:
 * @self: a #GFBGraphConnectable.
 * @node_type: a #GType, required a #GFBGRAPH_TYPE_NODE or children.
 *
 * Get the params to be inserted in a request to the Facebook Graph API
 * in order to append the node @self to a node of type @node_type.
 *
 * Returns: (transfer full): A string based #GHashTable with the params and his values or %NULL.
 **/
GHashTable *
gfbgraph_connectable_get_connection_post_params (GFBGraphConnectable *self,
                                                 GType                node_type)
{
  GFBGraphConnectableInterface *iface;

  g_return_val_if_fail (GFBGRAPH_IS_CONNECTABLE (self), NULL);
  g_return_val_if_fail (g_type_is_a (node_type, GFBGRAPH_TYPE_NODE), NULL);
  g_return_val_if_fail (gfbgraph_connectable_is_connectable_to (self, node_type), NULL);

  iface = GFBGRAPH_CONNECTABLE_GET_IFACE (self);
  g_assert (iface->get_connection_post_params != NULL);

  return iface->get_connection_post_params (self, node_type);
}

/**
 * gfbgraph_connectable_parse_connected_data:
 * @self: a #GFBGraphConnectable.
 * @payload: a const #gchar with the response string from the Facebook Graph API.
 * @error: (allow-none): a #GError.
 *
 * Parse the response contained in @payload when a gfbgraph_node_get_connection_nodes() was
 * executed.
 *
 * Returns: (element-type GFBGraphNode) (transfer full): a newly-allocated #GList of #GFBGraphNode created from the @payload or %NULL.
 **/
GList *
gfbgraph_connectable_parse_connected_data (GFBGraphConnectable  *self,
                                           const gchar          *payload,
                                           GError              **error)
{
  GFBGraphConnectableInterface *iface;

  g_return_val_if_fail (GFBGRAPH_IS_CONNECTABLE (self), NULL);

  iface = GFBGRAPH_CONNECTABLE_GET_IFACE (self);
  g_assert (iface->parse_connected_data != NULL);

  return iface->parse_connected_data (self, payload, error);
}


/**
 * gfbgraph_connectable_is_connectable_to:
 * @self: a #GFBGraphConnectable.
 * @node_type: a #GType, required a #GFBGRAPH_TYPE_NODE or children.
 *
 * Check if @self object, normally a #GFBGraphNode implementing the #GFBGraphConnectable interface,
 * has the possibility to be connected to another node of type @node_type.
 *
 * Returns: %TRUE in case that the @self object can be connected to a node of type @node_type,
 * %FALSE otherwise.
 **/
gboolean
gfbgraph_connectable_is_connectable_to (GFBGraphConnectable *self,
                                        GType                node_type)
{
  GFBGraphConnectableInterface *iface;
  GHashTable *connections;

  g_return_val_if_fail (GFBGRAPH_IS_CONNECTABLE (self), FALSE);
  g_return_val_if_fail (g_type_is_a (node_type, GFBGRAPH_TYPE_NODE), FALSE);

  iface = GFBGRAPH_CONNECTABLE_GET_IFACE (self);
  connections = get_connections (iface);

  return g_hash_table_contains (connections, g_type_name (node_type));
}

/**
 * gfbgraph_connectable_get_connection_path:
 * @self: a #GFBGraphConnectable.
 * @node_type: a #GType, required a #GFBGRAPH_TYPE_NODE or children.
 *
 * Get the Facebook Graph API function path to retrieve the nodes connected with @node_type
 * managed by the #GFBGraphConnectable object.
 *
 * Returns: (transfer none): a const #gchar with the function path or %NULL.
 **/
const gchar *
gfbgraph_connectable_get_connection_path (GFBGraphConnectable *self,
                                          GType                node_type)
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

/**
 * gfbgraph_connectable_default_parse_connected_data:
 * @self: a #GFBGraphConnectable.
 * @payload: a const #gchar with the response string from the Facebook Graph API.
 * @error: (allow-none): a #GError or %NULL.
 *
 * In most cases, #GFBGraphConnectable implementers can use this function in order to parse
 * the response when a gfbgraph_node_get_connection_nodes() is executed and the
 * gfbgraph_connectable_parse_connected_data() was called.
 *
 * Normally, Facebook Graph API returns the connections in the same way, using JSON objects,
 * with a root object called "data".
 *
 * Returns: (element-type GFBGraphNode) (transfer full): a newly-allocated #GList of #GFBGraphNode with the same #GType as @self.
 **/
GList *
gfbgraph_connectable_default_parse_connected_data (GFBGraphConnectable  *self,
                                                   const gchar          *payload,
                                                   GError              **error)
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
