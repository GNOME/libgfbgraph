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
 * SECTION:gfbgraph-node
 * @short_description: GFBGraph Node object
 * @stability: Unstable
 * @include: gfbgraph/gfbgraph.h
 *
 * #GFBGraphNode is the base class for the nodes in the Facebook Graph API, such a Album,
 * a Photo or a User. Only usefull to expand the current library functionality creating
 * new nodes based on it.
 *
 * This object provide the common functions to manage the relations between nodes trough the
 * #GFBGraphConnectable interface. See #gfbgraph_node_get_connection_nodes and #gfbgraph_node_append_node
 **/

#include <rest/rest-proxy-call.h>
#include <json-glib/json-glib.h>
#include <string.h>

#include "gfbgraph-common.h"
#include "gfbgraph-connectable.h"
#include "gfbgraph-node.h"

enum
{
  PROP_0,
  PROP_ID,
  PROP_LINK,
  PROP_CREATEDTIME,
  PROP_UPDATEDTIME
};

struct _GFBGraphNodePrivate {
  GList *connections;
  gchar *id;
  gchar *link;
  gchar *created_time;
  gchar *updated_time;
};

typedef struct {
  GList *list;
  GType node_type;
  GFBGraphAuthorizer *authorizer;
} GFBGraphNodeConnectionAsyncData;

#define GFBGRAPH_NODE_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE((o), GFBGRAPH_TYPE_NODE, GFBGraphNodePrivate))

static GObjectClass *parent_class = NULL;

G_DEFINE_TYPE (GFBGraphNode, gfbgraph_node, G_TYPE_OBJECT);

GQuark
gfbgraph_node_error_quark (void)
{
  return g_quark_from_static_string ("gfbgraph-node-error-quark");
}

static void
gfbgraph_node_finalize (GObject *object)
{
  GFBGraphNodePrivate *priv = GFBGRAPH_NODE_GET_PRIVATE (object);

  if (priv->id)
    g_free (priv->id);
  if (priv->link)
    g_free (priv->link);
  if (priv->created_time)
    g_free (priv->created_time);

  G_OBJECT_CLASS(parent_class)->finalize (object);
}

static void
gfbgraph_node_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  GFBGraphNodePrivate *priv = GFBGRAPH_NODE_GET_PRIVATE (object);

  switch (prop_id) {
    case PROP_ID:
      if (priv->id)
        g_free (priv->id);
      priv->id = g_strdup (g_value_get_string (value));
      break;
    case PROP_LINK:
      if (priv->link)
        g_free (priv->link);
      priv->link = g_strdup (g_value_get_string (value));
      break;
    case PROP_CREATEDTIME:
      if (priv->created_time)
        g_free (priv->created_time);
      priv->created_time = g_strdup (g_value_get_string (value));
      break;
    case PROP_UPDATEDTIME:
      if (priv->updated_time)
        g_free (priv->updated_time);
      priv->updated_time = g_strdup (g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gfbgraph_node_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  GFBGraphNodePrivate *priv = GFBGRAPH_NODE_GET_PRIVATE (object);

  switch (prop_id) {
    case PROP_ID:
      g_value_set_string (value, priv->id);
      break;
    case PROP_LINK:
      g_value_set_string (value, priv->link);
      break;
    case PROP_CREATEDTIME:
      g_value_set_string (value, priv->created_time);
      break;
    case PROP_UPDATEDTIME:
      g_value_set_string (value, priv->updated_time);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gfbgraph_node_class_init (GFBGraphNodeClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  parent_class            = g_type_class_peek_parent (klass);

  gobject_class->finalize = gfbgraph_node_finalize;
  gobject_class->set_property = gfbgraph_node_set_property;
  gobject_class->get_property = gfbgraph_node_get_property;

  g_type_class_add_private (gobject_class, sizeof(GFBGraphNodePrivate));

  /**
   * GFBGraphNode:id:
   *
   * The node ID. All nodes have one of this.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_ID,
                                   g_param_spec_string ("id",
                                                        "The Facebook node ID",
                                                        "Every node in the Facebook Graph is identified by his ID",
                                                        "",
                                                        G_PARAM_READABLE | G_PARAM_WRITABLE));

  /**
   * GFBGraphNode:link:
   *
   * The node link. An URL to the node on Facebook.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_LINK,
                                   g_param_spec_string ("link",
                                                        "The link to the node",
                                                        "A link (url) to the node on Facebook",
                                                        "",
                                                        G_PARAM_READABLE | G_PARAM_WRITABLE));

  /**
   * GFBGraphNode:created_time:
   *
   * The time the node was initially published. Is an ISO 8601 encoded date.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_CREATEDTIME,
                                   g_param_spec_string ("created_time",
                                                        "The node creation time",
                                                        "An ISO 8601 encoded date when the node was initially published",
                                                        "",
                                                        G_PARAM_READABLE | G_PARAM_WRITABLE));

  /**
   * GFBGraphNode:updated_time:
   *
   * The last time the node was updated. Is an ISO 8601 encoded date.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_UPDATEDTIME,
                                   g_param_spec_string ("updated_time",
                                                        "The node updated time",
                                                        "An ISO 8601 encoded date when the node was updated",
                                                        "",
                                                        G_PARAM_READABLE | G_PARAM_WRITABLE));
}

static void
gfbgraph_node_init (GFBGraphNode *obj)
{
  obj->priv = GFBGRAPH_NODE_GET_PRIVATE(obj);
}

/* --- Private Functions --- */
static void
connection_async_data_free (GFBGraphNodeConnectionAsyncData *data)
{
  g_list_free (data->list);
  g_object_unref (data->authorizer);

  g_slice_free (GFBGraphNodeConnectionAsyncData, data);
}

static void
get_connection_nodes_async_thread (GSimpleAsyncResult *simple_async,
                                   GFBGraphNode       *node,
                                   GCancellable        cancellable)
{
  GFBGraphNodeConnectionAsyncData *data;
  GError *error = NULL;

  data = (GFBGraphNodeConnectionAsyncData *)g_simple_async_result_get_op_res_gpointer (simple_async);
  data->list = gfbgraph_node_get_connection_nodes (node,
                                                   data->node_type,
                                                   data->authorizer,
                                                   &error);
  if (error != NULL)
    g_simple_async_result_take_error (simple_async, error);
}

/**
 * gfbgraph_node_new:
 *
 * Creates a new #GFBGraphNode.
 *
 * Returns: (transfer full): a new #GFBGraphNode; unref with g_object_unref()
 **/
GFBGraphNode *
gfbgraph_node_new (void)
{
  return GFBGRAPH_NODE (g_object_new (GFBGRAPH_TYPE_NODE, NULL));
}

/**
 * gfbgraph_node_new_from_id:
 * @id: a const #gchar with the node ID.
 * @node_type: a #GFBGraphNode type #GType.
 * @authorizer: a #GFBGraphAuthorizer.
 * @error: (allow-none): a #GError or %NULL.
 *
 * Retrieve a node object as a #GFBgraphNode of #node_type type, with the given @id from the Facebook Graph.
 *
 * Returns: (transfer full): a #GFBGraphNode or %NULL.
 **/
GFBGraphNode *
gfbgraph_node_new_from_id (GFBGraphAuthorizer  *authorizer,
                           const gchar         *id,
                           GType                node_type,
                           GError             **error)
{
  GFBGraphNode *node = NULL;
  RestProxyCall *rest_call;

  g_return_val_if_fail ((strlen (id) > 0), NULL);
  g_return_val_if_fail (GFBGRAPH_IS_AUTHORIZER (authorizer), NULL);
  g_return_val_if_fail (g_type_is_a (node_type, GFBGRAPH_TYPE_NODE), NULL);

  rest_call = gfbgraph_new_rest_call (authorizer);
  rest_proxy_call_set_method (rest_call, "GET");
  rest_proxy_call_set_function (rest_call, id);

  if (rest_proxy_call_sync (rest_call, error)) {
    JsonParser *jparser;
    JsonNode *jnode;
    const gchar *payload;

    payload = rest_proxy_call_get_payload (rest_call);
    jparser = json_parser_new ();
    if (json_parser_load_from_data (jparser, payload, -1, error)) {
      jnode = json_parser_get_root (jparser);
      node = GFBGRAPH_NODE (json_gobject_deserialize (node_type, jnode));
    }

    g_object_unref (jparser);
  }

  g_object_unref (rest_call);

  return node;
}

/**
 * gfbgraph_node_get_id:
 * @node: a #GFBGraphNode.
 *
 * Gets the Facebook Graph unique node ID.
 *
 * Returns: (transfer none): the node ID.
 **/
const gchar *
gfbgraph_node_get_id (GFBGraphNode *node)
{
  g_return_val_if_fail (GFBGRAPH_IS_NODE (node), NULL);

  return node->priv->id;
}

/**
 * gfbgraph_node_get_url:
 * @node: a #GFBGraphNode.
 *
 * Gets the node link to the Facebook web page.
 *
 * Returns: (transfer none): the URL.
 **/
const gchar *
gfbgraph_node_get_link (GFBGraphNode *node)
{
  g_return_val_if_fail (GFBGRAPH_IS_NODE (node), NULL);

  return node->priv->link;
}

/**
 * gfbgraph_node_get_created_time:
 * @node: a #GFBGraphNode.
 *
 * Gets a node created time.
 *
 * Returns: (transfer none): an ISO 8601 encoded date when the node was initially published.
 **/
const gchar *
gfbgraph_node_get_created_time (GFBGraphNode *node)
{
  g_return_val_if_fail (GFBGRAPH_IS_NODE (node), NULL);

  return node->priv->created_time;
}

/**
 * gfbgraph_node_get_updated_time:
 * @node: a #GFBGraphNode.
 *
 * Gets a node updated time.
 *
 * Returns: (transfer none): an ISO 8601 encoded date when the node was updated.
 **/
const gchar *
gfbgraph_node_get_updated_time (GFBGraphNode *node)
{
  g_return_val_if_fail (GFBGRAPH_IS_NODE (node), NULL);

  return node->priv->updated_time;
}

/**
 * gfbgraph_node_set_id:
 * @node: a #GFBGraphNode.
 * @id: a const pointer to a #gchar.
 *
 * Sets the ID for a node. Just useful when a new node is created
 * and the Graph API returns the ID of the new created node.
 **/
void
gfbgraph_node_set_id (GFBGraphNode *node,
                      const gchar  *id)
{
  g_return_if_fail (GFBGRAPH_IS_NODE (node));
  g_return_if_fail (id != NULL);

  g_object_set (G_OBJECT (node),
                "id", id,
                NULL);
}

/**
 * gfbgraph_node_get_connection_nodes:
 * @node: a #GFBGraphNode object which retrieve the connected nodes.
 * @node_type: a #GFBGraphNode type #GType that determines the kind of nodes to retrieve.
 * @authorizer: a #GFBGraphAuthorizer.
 * @error: (allow-none): a #GError or %NULL.
 *
 * Retrieve the nodes of type @node_type connected to the @node object. The @node_type object must
 * implement the #GFBGraphConnectionable interface and be connectable to @node type object.
 * See gfbgraph_node_get_connection_nodes_async() for the asynchronous version of this call.
 *
 * Returns: (element-type GFBGraphNode) (transfer full): a newly-allocated #GList of type @node_type objects with the found nodes.
 **/
GList *
gfbgraph_node_get_connection_nodes (GFBGraphNode        *node,
                                    GType                node_type,
                                    GFBGraphAuthorizer  *authorizer,
                                    GError             **error)
{
  GFBGraphNodePrivate *priv;
  GList *nodes_list = NULL;
  GFBGraphNode *connected_node;
  RestProxyCall *rest_call;
  gchar *function_path;

  g_return_val_if_fail (GFBGRAPH_IS_NODE (node), NULL);
  g_return_val_if_fail (g_type_is_a (node_type, GFBGRAPH_TYPE_NODE), NULL);
  g_return_val_if_fail (GFBGRAPH_IS_AUTHORIZER (authorizer), NULL);

  priv = GFBGRAPH_NODE_GET_PRIVATE (node);

  /* Dummy node just for test */
  connected_node = g_object_new (node_type, NULL);
  if (GFBGRAPH_IS_CONNECTABLE (connected_node) == FALSE) {
    g_set_error (error, GFBGRAPH_NODE_ERROR,
                 GFBGRAPH_NODE_ERROR_NO_CONNECTABLE,
                 "The given node type (%s) doesn't implement connectable interface",
                 g_type_name (node_type));
    return NULL;
  }

  if (gfbgraph_connectable_is_connectable_to (GFBGRAPH_CONNECTABLE (connected_node), G_OBJECT_TYPE (node)) == FALSE) {
    g_set_error (error, GFBGRAPH_NODE_ERROR,
                 GFBGRAPH_NODE_ERROR_NO_CONNECTABLE,
                 "The given node type (%s) can't connect with the node",
                 g_type_name (node_type));
    return NULL;
  }

  rest_call = gfbgraph_new_rest_call (authorizer);
  rest_proxy_call_set_method (rest_call, "GET");
  function_path = g_strdup_printf ("%s/%s",
                                   priv->id,
                                   gfbgraph_connectable_get_connection_path (GFBGRAPH_CONNECTABLE (connected_node),
                                                                             G_OBJECT_TYPE (node)));
  rest_proxy_call_set_function (rest_call, function_path);
  g_free (function_path);

  if (rest_proxy_call_sync (rest_call, error)) {
    const gchar *payload = rest_proxy_call_get_payload (rest_call);
    nodes_list = gfbgraph_connectable_parse_connected_data (GFBGRAPH_CONNECTABLE (connected_node), payload, error);
  }

  /* We don't need this node again */
  g_object_unref (connected_node);
  g_object_unref (rest_call);

  return nodes_list;
}

/**
 * gfbgraph_node_get_connection_nodes_async:
 * @node: A #GFBGraphNode object which retrieve the connected nodes.
 * @node_type: a #GFBGraphNode type #GType that must implement the #GFBGraphConnectionable interface.
 * @authorizer: a #GFBGraphAuthorizer.
 * @cancellable: (allow-none): An optional #GCancellable object, or %NULL.
 * @callback: (scope async): A #GAsyncReadyCallback to call when the request is completed.
 * @user_data: (closure): The data to pass to @callback.
 *
 * Asynchronously retrieve the list of nodes of type @node_type connected to the @node object. See
 * gfbgraph_node_get_connection_nodes() for the synchronous version of this call.
 *
 * When the operation is finished, @callback will be called. You can then call gfbgraph_node_get_connection_nodes_finish()
 * to get the list of connected nodes.
 **/
void
gfbgraph_node_get_connection_nodes_async (GFBGraphNode        *node,
                                          GType                node_type,
                                          GFBGraphAuthorizer  *authorizer,
                                          GCancellable        *cancellable,
                                          GAsyncReadyCallback  callback,
                                          gpointer             user_data)
{
  GSimpleAsyncResult *result;
  GFBGraphNodeConnectionAsyncData *data;

  g_return_if_fail (GFBGRAPH_IS_NODE (node));
  g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));
  g_return_if_fail (callback != NULL);

  result = g_simple_async_result_new (G_OBJECT (node),
                                      callback,
                                      user_data,
                                      gfbgraph_node_get_connection_nodes_async);
  g_simple_async_result_set_check_cancellable (result, cancellable);

  data = g_slice_new (GFBGraphNodeConnectionAsyncData);
  data->list = NULL;
  data->node_type = node_type;
  data->authorizer = authorizer;
  g_object_ref (data->authorizer);

  g_simple_async_result_set_op_res_gpointer (result,
                                             data,
                                             (GDestroyNotify)connection_async_data_free);
  g_simple_async_result_run_in_thread (result,
                                       (GSimpleAsyncThreadFunc)get_connection_nodes_async_thread,
                                       G_PRIORITY_DEFAULT,
                                       cancellable);

  g_object_unref (result);
}

/**
 * gfbgraph_node_get_connection_nodes_async_finish:
 * @node: A #GFBGraphNode.
 * @result: A #GAsyncResult.
 * @error: (allow-none): An optional #GError, or %NULL.
 *
 * Finishes an asynchronous operation started with
 * gfbgraph_node_get_connection_nodes_async().
 *
 * Returns: (element-type GFBGraphNode) (transfer full): a newly-allocated #GList of type #node_type objects with the found nodes.
 **/
GList*
gfbgraph_node_get_connection_nodes_async_finish (GFBGraphNode  *node,
                                                 GAsyncResult  *result,
                                                 GError       **error)
{
  GSimpleAsyncResult *simple_async;
  GFBGraphNodeConnectionAsyncData *data;

  g_return_val_if_fail (g_simple_async_result_is_valid (result, G_OBJECT (node), gfbgraph_node_get_connection_nodes_async), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  simple_async = G_SIMPLE_ASYNC_RESULT (result);

  if (g_simple_async_result_propagate_error (simple_async, error))
    return NULL;

  data = (GFBGraphNodeConnectionAsyncData *) g_simple_async_result_get_op_res_gpointer (simple_async);
  return data->list;
}

/**
 * gfbgraph_node_append_connection:
 * @node: A #GFBGraphNode.
 * @connect_node: A #GFBGraphNode.
 * @authorizer: A #GFBGraphAuthorizer.
 * @error: (allow-none): An optional #GError, or %NULL.
 *
 * Appends @connect_node to @node. @connect_node must implement the #GFBGraphConnectable interface
 * and be connectable to @node GType.
 *
 * Returns: TRUE on sucess, FALSE if an error ocurred.
 **/
gboolean
gfbgraph_node_append_connection (GFBGraphNode        *node,
                                 GFBGraphNode        *connect_node,
                                 GFBGraphAuthorizer  *authorizer,
                                 GError             **error)
{
  GFBGraphNodePrivate *priv;
  RestProxyCall *rest_call;
  GHashTable *params;
  gchar *function_path;
  gboolean success = FALSE;

  g_return_val_if_fail (GFBGRAPH_IS_NODE (node), FALSE);
  g_return_val_if_fail (GFBGRAPH_IS_NODE (connect_node), FALSE);
  g_return_val_if_fail (GFBGRAPH_IS_AUTHORIZER (authorizer), FALSE);

  if (GFBGRAPH_IS_CONNECTABLE (connect_node) == FALSE) {
    g_set_error (error, GFBGRAPH_NODE_ERROR,
                 GFBGRAPH_NODE_ERROR_NO_CONNECTABLE,
                 "The given node type (%s) doesn't implement connectable interface",
                 G_OBJECT_TYPE_NAME (connect_node));
    return FALSE;
  }

  if (gfbgraph_connectable_is_connectable_to (GFBGRAPH_CONNECTABLE (connect_node), G_OBJECT_TYPE (node)) == FALSE) {
    g_set_error (error, GFBGRAPH_NODE_ERROR,
                 GFBGRAPH_NODE_ERROR_NO_CONNECTABLE,
                 "The given node type (%s) can't append a %s connection",
                 G_OBJECT_TYPE_NAME (node),
                 G_OBJECT_TYPE_NAME (connect_node));
    return FALSE;
  }

  priv = GFBGRAPH_NODE_GET_PRIVATE (node);

  rest_call = gfbgraph_new_rest_call (authorizer);
  rest_proxy_call_set_method (rest_call, "POST");
  function_path = g_strdup_printf ("%s/%s",
                                   priv->id,
                                   gfbgraph_connectable_get_connection_path (GFBGRAPH_CONNECTABLE (connect_node),
                                                                             G_OBJECT_TYPE (node)));
  rest_proxy_call_set_function (rest_call, function_path);
  g_free (function_path);

  params = gfbgraph_connectable_get_connection_post_params (GFBGRAPH_CONNECTABLE (connect_node),
                                                            G_OBJECT_TYPE (node));
  if (g_hash_table_size (params) > 0) {
    GHashTableIter iter;
    const gchar *key;
    const gchar *value;

    g_hash_table_iter_init (&iter, params);
    while (g_hash_table_iter_next (&iter, (gpointer *) &key, (gpointer *) &value)) {
      rest_proxy_call_add_param (rest_call, key, value);
    }
  }

  if (rest_proxy_call_sync (rest_call, error)) {
    const gchar *payload;
    JsonParser *jparser;
    JsonNode *jnode;
    JsonReader *jreader;

    payload = rest_proxy_call_get_payload (rest_call);
    /* Parssing the new ID */
    jparser = json_parser_new ();
    json_parser_load_from_data (jparser, payload, -1, error);
    jnode = json_parser_get_root (jparser);
    jreader = json_reader_new (jnode);

    json_reader_read_element (jreader, 0);
    gfbgraph_node_set_id (connect_node,
        json_reader_get_string_value (jreader));
    json_reader_end_element (jreader);

    g_object_unref (jreader);
    g_object_unref (jparser);
    success = TRUE;
  }
  g_object_unref (rest_call);

  return success;
}
