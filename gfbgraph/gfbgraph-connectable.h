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

#ifndef __GFBGRAPH_CONNECTABLE_H__
#define __GFBGRAPH_CONNECTABLE_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define GFBGRAPH_TYPE_CONNECTABLE (gfbgraph_connectable_get_type ())
#define GFBGRAPH_CONNECTABLE(o) \
  (G_TYPE_CHECK_INSTANCE_CAST ((o), GFBGRAPH_TYPE_CONNECTABLE, GFBGraphConnectable))
#define GFBGRAPH_CONNECTABLE_CLASS(k) \
  (G_TYPE_CHECK_CLASS_CAST((k), GFBGRAPH_TYPE_CONNECTABLE, GFBGraphConnectableInterface))
#define GFBGRAPH_IS_CONNECTABLE(o) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((o), GFBGRAPH_TYPE_CONNECTABLE))
#define GFBGRAPH_CONNECTABLE_GET_IFACE(o) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((o), GFBGRAPH_TYPE_CONNECTABLE, GFBGraphConnectableInterface))

typedef struct _GFBGraphConnectable          GFBGraphConnectable;
typedef struct _GFBGraphConnectableInterface GFBGraphConnectableInterface;

struct _GFBGraphConnectableInterface {
  GTypeInterface parent;

  GHashTable *  connections;

  GHashTable *  (*get_connection_post_params) (GFBGraphConnectable *self,
                                               GType                node_type);
  GList *       (*parse_connected_data)       (GFBGraphConnectable  *self,
                                               const gchar          *payload,
                                               GError              **error);
};

GType gfbgraph_connectable_get_type (void) G_GNUC_CONST;

GHashTable*  gfbgraph_connectable_get_connection_post_params   (GFBGraphConnectable *self,
                                                                GType                node_type);
GList*       gfbgraph_connectable_parse_connected_data         (GFBGraphConnectable  *self,
                                                                const gchar          *payload,
                                                                GError              **error);
gboolean     gfbgraph_connectable_is_connectable_to            (GFBGraphConnectable *self,
                                                                GType                node_type);
const gchar* gfbgraph_connectable_get_connection_path          (GFBGraphConnectable *self,
                                                                GType                node_type);
GList*       gfbgraph_connectable_default_parse_connected_data (GFBGraphConnectable  *self,
                                                                const gchar          *payload,
                                                                GError              **error);

G_END_DECLS

#endif /* __GFBGRAPH_CONNECTABLE_H__ */
