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

#ifndef __GFBGRAPH_AUTHORIZER_H__
#define __GFBGRAPH_AUTHORIZER_H__

#include <glib-object.h>
#include <gio/gio.h>
#include <libsoup/soup.h>
#include <rest/rest-proxy-call.h>

G_BEGIN_DECLS

#define GFBGRAPH_TYPE_AUTHORIZER (gfbgraph_authorizer_get_type ())
#define GFBGRAPH_AUTHORIZER(o) \
  (G_TYPE_CHECK_INSTANCE_CAST ((o), GFBGRAPH_TYPE_AUTHORIZER, GFBGraphAuthorizer))
#define GFBGRAPH_IS_AUTHORIZER(o) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((o), GFBGRAPH_TYPE_AUTHORIZER))
#define GFBGRAPH_AUTHORIZER_GET_IFACE(o) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((o), GFBGRAPH_TYPE_AUTHORIZER, GFBGraphAuthorizerInterface))

typedef struct _GFBGraphAuthorizer          GFBGraphAuthorizer;
typedef struct _GFBGraphAuthorizerInterface GFBGraphAuthorizerInterface;

/**
 * GFBGraphAuthorizerInterface:
 * @parent: The parent interface.
 * @process_call: A method to append authorization headers to a a #RestProxyCall.
 * @process_message: A method to append authorization headers to a #SoupMessage.
 * @refresh_authorization: A synchronous method to force a refresh of any authorization
 *  tokes held by the authorizer. It should return %TRUE on succes.
 *
 * Interface structure for #GFBGraphAuthorizer. All methos should be thread safe.
 **/
struct _GFBGraphAuthorizerInterface {
  GTypeInterface parent;

  void      (*process_call)           (GFBGraphAuthorizer *iface,
                                       RestProxyCall      *call);
  void      (*process_message)        (GFBGraphAuthorizer *iface,
                                       SoupMessage        *message);
  gboolean  (*refresh_authorization)  (GFBGraphAuthorizer  *iface,
                                       GCancellable        *cancellable,
                                       GError             **error);
};

GType    gfbgraph_authorizer_get_type              (void) G_GNUC_CONST;

void     gfbgraph_authorizer_process_call          (GFBGraphAuthorizer *iface,
                                                    RestProxyCall      *call);
void     gfbgraph_authorizer_process_message       (GFBGraphAuthorizer *iface,
                                                    SoupMessage        *message);
gboolean gfbgraph_authorizer_refresh_authorization (GFBGraphAuthorizer  *iface,
                                                    GCancellable        *cancellable,
                                                    GError             **error);

G_END_DECLS

#endif /* __GFBGRAPH_AUTHORIZER_H__ */
