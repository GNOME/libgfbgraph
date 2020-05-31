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

#ifndef __GFBGRAPH_NODE_H__
#define __GFBGRAPH_NODE_H__

#include <glib-object.h>
#include <gfbgraph/gfbgraph-authorizer.h>

G_BEGIN_DECLS

#define GFBGRAPH_TYPE_NODE (gfbgraph_node_get_type())
#define GFBGRAPH_NODE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GFBGRAPH_TYPE_NODE,GFBGraphNode))
#define GFBGRAPH_NODE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GFBGRAPH_TYPE_NODE,GFBGraphNodeClass))
#define GFBGRAPH_IS_NODE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GFBGRAPH_TYPE_NODE))
#define GFBGRAPH_IS_NODE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GFBGRAPH_TYPE_NODE))
#define GFBGRAPH_NODE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS((obj),GFBGRAPH_TYPE_NODE,GFBGraphNodeClass))

#define GFBGRAPH_NODE_ERROR            gfbgraph_node_error_quark ()

typedef struct _GFBGraphNode        GFBGraphNode;
typedef struct _GFBGraphNodeClass   GFBGraphNodeClass;
typedef struct _GFBGraphNodePrivate GFBGraphNodePrivate;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GFBGraphNode, g_object_unref)

struct _GFBGraphNode {
  GObject parent;

  /*< private >*/
  GFBGraphNodePrivate *priv;
};

struct _GFBGraphNodeClass {
  GObjectClass parent_class;
};

typedef enum {
  GFBGRAPH_NODE_ERROR_NO_CONNECTIONABLE = 1,
  GFBGRAPH_NODE_ERROR_NO_CONNECTABLE
} GFBGraphNodeError;

GType          gfbgraph_node_get_type    (void) G_GNUC_CONST;
GQuark         gfbgraph_node_error_quark (void) G_GNUC_CONST;
GFBGraphNode*  gfbgraph_node_new         (void);

GFBGraphNode*  gfbgraph_node_new_from_id (GFBGraphAuthorizer  *authorizer,
                                          const gchar         *id,
                                          GType                node_type,
                                          GError             **error);

const gchar*   gfbgraph_node_get_id           (GFBGraphNode *node);
const gchar*   gfbgraph_node_get_link         (GFBGraphNode *node);
const gchar*   gfbgraph_node_get_created_time (GFBGraphNode *node);
const gchar*   gfbgraph_node_get_updated_time (GFBGraphNode *node);

void           gfbgraph_node_set_id           (GFBGraphNode *node,
                                               const gchar  *id);

GList*         gfbgraph_node_get_connection_nodes (GFBGraphNode        *node,
                                                   GType                node_type,
                                                   GFBGraphAuthorizer  *authorizer,
                                                   GError             **error);
void           gfbgraph_node_get_connection_nodes_async (GFBGraphNode        *node,
                                                         GType                node_type,
                                                         GFBGraphAuthorizer  *authorizer,
                                                         GCancellable        *cancellable,
                                                         GAsyncReadyCallback  callback,
                                                         gpointer             user_data);
GList*         gfbgraph_node_get_connection_nodes_async_finish (GFBGraphNode  *node,
                                                                GAsyncResult  *result,
                                                                GError       **error);
gboolean       gfbgraph_node_append_connection (GFBGraphNode        *node,
                                                GFBGraphNode        *connect_node,
                                                GFBGraphAuthorizer  *authorizer,
                                                GError             **error);

G_END_DECLS

#endif /* __GFBGRAPH_NODE_H__ */
