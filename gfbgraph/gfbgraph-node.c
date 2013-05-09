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

        PROP_ID
};

struct _GFBGraphNodePrivate {
        GList *connections;
        gchar *id;
};

typedef struct {
        GList *list;
        GType node_type;
        GFBGraphAuthorizer *authorizer;
} GFBGraphNodeConnectionAsyncData;

GQuark
gfbgraph_node_error_quark (void)
{
        return g_quark_from_static_string ("gfbgraph-node-error-quark");
}

static void gfbgraph_node_init         (GFBGraphNode *obj);
static void gfbgraph_node_class_init   (GFBGraphNodeClass *klass);
static void gfbgraph_node_finalize     (GObject *object);
static void gfbgraph_node_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void gfbgraph_node_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

static void gfbgraph_node_connection_async_data_free (GFBGraphNodeConnectionAsyncData *data);
static void gfbgraph_node_get_connection_nodes_async_thread (GSimpleAsyncResult *simple_async, GFBGraphNode *node, GCancellable cancellable);

#define GFBGRAPH_NODE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), GFBGRAPH_TYPE_NODE, GFBGraphNodePrivate))

static GObjectClass *parent_class = NULL;

G_DEFINE_TYPE (GFBGraphNode, gfbgraph_node, G_TYPE_OBJECT);

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
         * GFBGraphNode:id
         *
         * The node ID. All nodes have one of this.
         **/
        g_object_class_install_property (gobject_class,
                                         PROP_ID,
                                         g_param_spec_string ("id",
                                                              "The Facebook node ID", "Every node in the Facebook Graph is identified by his ID",
                                                              "",
                                                              G_PARAM_READABLE | G_PARAM_WRITABLE));
}

static void
gfbgraph_node_init (GFBGraphNode *obj)
{
        obj->priv = GFBGRAPH_NODE_GET_PRIVATE(obj);
}

static void
gfbgraph_node_finalize (GObject *object)
{
        GFBGraphNodePrivate *priv;

        priv = GFBGRAPH_NODE_GET_PRIVATE (object);

        if (priv->id)
                g_free (priv->id);

        G_OBJECT_CLASS(parent_class)->finalize (object);
}

static void
gfbgraph_node_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
        GFBGraphNodePrivate *priv;

        priv = GFBGRAPH_NODE_GET_PRIVATE (object);

        switch (prop_id) {
                case PROP_ID:
                        if (priv->id)
                                g_free (priv->id);
                        priv->id = g_strdup (g_value_get_string (value));
                        break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;
        }
}

static void
gfbgraph_node_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
        GFBGraphNodePrivate *priv;

        priv = GFBGRAPH_NODE_GET_PRIVATE (object);

        switch (prop_id) {
                case PROP_ID:
                        g_value_set_string (value, priv->id);
                        break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;
        }
}

static void
gfbgraph_node_connection_async_data_free (GFBGraphNodeConnectionAsyncData *data)
{
        g_list_free (data->list);
        g_object_unref (data->authorizer);

        g_slice_free (GFBGraphNodeConnectionAsyncData, data);
}

static void
gfbgraph_node_get_connection_nodes_async_thread (GSimpleAsyncResult *simple_async, GFBGraphNode *node, GCancellable cancellable)
{
        GFBGraphNodeConnectionAsyncData *data;
        GError *error;

        data = (GFBGraphNodeConnectionAsyncData *) g_simple_async_result_get_op_res_gpointer (simple_async);

        error = NULL;
        data->list = gfbgraph_node_get_connection_nodes (node, data->node_type, data->authorizer, &error);
        if (error != NULL)
                g_simple_async_result_take_error (simple_async, error);
}

/**
 * gfbgraph_node_new:
 *
 * Creates a new #GFBGraphNode.
 *
 * Return value: a new #GFBGraphNode; unref with g_object_unref()
 **/
GFBGraphNode*
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
 * Returns: a #GFBGraphNode or %NULL.
 **/
GFBGraphNode*
        gfbgraph_node_new_from_id (GFBGraphAuthorizer *authorizer, const gchar *id, GType node_type, GError **error)
{
        GFBGraphNode *node;
        RestProxyCall *rest_call;

        g_return_val_if_fail ((strlen (id) > 0), NULL);
        g_return_val_if_fail (GFBGRAPH_IS_AUTHORIZER (authorizer), NULL);
        g_return_val_if_fail (g_type_is_a (node_type, GFBGRAPH_TYPE_NODE), NULL);

        rest_call = gfbgraph_new_rest_call (authorizer);
        rest_proxy_call_set_method (rest_call, "GET");
        rest_proxy_call_set_function (rest_call, id);

        node = NULL;
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

        return node;        
}

/**
 * gfbgraph_node_get_connection_nodes:
 * @node: a #GFBGraphNode object which retrieve the connected nodes.
 * @node_type: a #GFBGraphNode type #GType that determines the kind of nodes to retrieve.
 * @authorizer: a #GFBGraphAuthorizer.
 * @error: (allow-none) a #GError or %NULL.
 * 
 * Retrieve the nodes of type @node_type connected to the @node object. The @node_type object must
 * implement the #GFBGraphConnectionable interface and be connectable to @node type object.
 * See gfbgraph_node_get_connection_nodes_async() for the asynchronous version of this call.
 *
 * Returns: a #GList of type @node_type objects with the found nodes.
 **/
GList*
gfbgraph_node_get_connection_nodes (GFBGraphNode *node, GType node_type, GFBGraphAuthorizer *authorizer, GError **error)
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
                             "The given node type (%s) don't implement connetionable interface", g_type_name (node_type));
                return NULL;
        }

        if (gfbgraph_connectable_is_connectable_to (GFBGRAPH_CONNECTABLE (connected_node), G_OBJECT_TYPE (node)) == FALSE) {
                g_set_error (error, GFBGRAPH_NODE_ERROR,
                             GFBGRAPH_NODE_ERROR_NO_CONNECTABLE,
                             "The given node type (%s) can't connect with the node", g_type_name (node_type));
                return NULL;
        }

        rest_call = gfbgraph_new_rest_call (authorizer);
        rest_proxy_call_set_method (rest_call, "GET");
        function_path = g_strdup_printf ("%s/%s",
                                         priv->id,
                                         gfbgraph_connectable_get_connection_path (GFBGRAPH_CONNECTABLE (connected_node),
                                                                                      G_OBJECT_TYPE (node)));
        rest_proxy_call_set_function (rest_call, function_path);

        if (rest_proxy_call_sync (rest_call, error)) {
                const gchar *payload;

                payload = rest_proxy_call_get_payload (rest_call);
                nodes_list = gfbgraph_connectable_parse_connected_data (GFBGRAPH_CONNECTABLE (connected_node), payload, error);
        } else {
                return NULL;
        }

        /* We don't need this node again */
        g_clear_object (&connected_node);
        g_free (function_path);


        return nodes_list;
}

/**
 * gfbgraph_node_get_connection_nodes_async:
 * @node: A #GFBGraphNode object which retrieve the connected nodes.
 * @node_type: a #GFBGraphNode type #GType that must implement the #GFBGraphConnectionable interface.
 * @authorizer: a #GFBGraphAuthorizer.
 * @cancellable: (allow-none): An optional #GCancellable object, or %NULL.
 * @callback: (scope async): A #GAsyncReadyCallback to call when the request is completed.
 * @user_data: (closure); The data to pass to @callback.
 * 
 * Asynchronously retrieve the list of nodes of type @node_type connected to the @node object. See
 * gfbgraph_node_get_connection_nodes() for the synchronous version of this call.
 *
 * When the operation is finished, @callback will be called. You can then call gfbgraph_node_get_connection_nodes_finish()
 * to get the list of connected nodes.
 **/
void
gfbgraph_node_get_connection_nodes_async (GFBGraphNode *node, GType node_type, GFBGraphAuthorizer *authorizer, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data)
{
        GSimpleAsyncResult *result;
        GFBGraphNodeConnectionAsyncData *data;

        g_return_if_fail (GFBGRAPH_IS_NODE (node));
        g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));
	g_return_if_fail (callback != NULL);

        result = g_simple_async_result_new (G_OBJECT (node), callback, user_data, gfbgraph_node_get_connection_nodes_async);
        g_simple_async_result_set_check_cancellable (result, cancellable);

        data = g_slice_new (GFBGraphNodeConnectionAsyncData);
        data->list = NULL;
        data->node_type = node_type;
        data->authorizer = authorizer;
        g_object_ref (data->authorizer);

        g_simple_async_result_set_op_res_gpointer (result, data, (GDestroyNotify) gfbgraph_node_connection_async_data_free);
        g_simple_async_result_run_in_thread (result, (GSimpleAsyncThreadFunc) gfbgraph_node_get_connection_nodes_async_thread, G_PRIORITY_DEFAULT, cancellable);

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
 * Returns: a #GList of type #node_type objects with the found nodes.
 **/
GList*
gfbgraph_node_get_connection_nodes_async_finish (GFBGraphNode *node, GAsyncResult *result, GError **error)
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
 **/
gboolean
gfbgraph_node_append_connection (GFBGraphNode *node, GFBGraphNode *connect_node, GFBGraphAuthorizer *authorizer, GError **error)
{
        GFBGraphNodePrivate *priv;
        RestProxyCall *rest_call;
        GHashTable *params;
        gchar *function_path;

        g_return_val_if_fail (GFBGRAPH_IS_NODE (node), FALSE);
        g_return_val_if_fail (GFBGRAPH_IS_NODE (connect_node), FALSE);
        g_return_val_if_fail (GFBGRAPH_IS_AUTHORIZER (authorizer), FALSE);
        /* TODO: Check if connect_node is connectable to node */

        priv = GFBGRAPH_NODE_GET_PRIVATE (node);

        rest_call = gfbgraph_new_rest_call (authorizer);
        rest_proxy_call_set_method (rest_call, "POST");
        function_path = g_strdup_printf ("%s/%s",
                                         priv->id,
                                         gfbgraph_connectable_get_connection_path (GFBGRAPH_CONNECTABLE (connect_node),
                                                                                   G_OBJECT_TYPE (node)));
        rest_proxy_call_set_function (rest_call, function_path);

        params = gfbgraph_connectable_get_connection_post_params (GFBGRAPH_CONNECTABLE (connect_node), G_OBJECT_TYPE (node));
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

                payload = rest_proxy_call_get_payload (rest_call);
                /* TODO: Parse result (the ID) and put to the connect_node */
        } else {
                return FALSE;
        }

        g_free (function_path);

        return TRUE;
}
