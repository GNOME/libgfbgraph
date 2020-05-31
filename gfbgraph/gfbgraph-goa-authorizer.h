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

#ifndef __GFBGRAPH_GOA_AUTHORIZER_H__
#define __GFBGRAPH_GOA_AUTHORIZER_H__

#include <glib-object.h>
#include <goa/goa.h>

G_BEGIN_DECLS

#define GFBGRAPH_TYPE_GOA_AUTHORIZER (gfbgraph_goa_authorizer_get_type())
#define GFBGRAPH_GOA_AUTHORIZER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GFBGRAPH_TYPE_GOA_AUTHORIZER,GFBGraphGoaAuthorizer))
#define GFBGRAPH_GOA_AUTHORIZER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GFBGRAPH_TYPE_GOA_AUTHORIZER,GFBGraphGoaAuthorizerClass))
#define GFBGRAPH_IS_GOA_AUTHORIZER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GFBGRAPH_TYPE_GOA_AUTHORIZER))
#define GFBGRAPH_IS_GOA_AUTHORIZER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GFBGRAPH_TYPE_GOA_AUTHORIZER))
#define GFBGRAPH_GOA_AUTHORIZER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS((obj),GFBGRAPH_TYPE_GOA_AUTHORIZER,GFBGraphGoaAuthorizerClass))

typedef struct _GFBGraphGoaAuthorizer        GFBGraphGoaAuthorizer;
typedef struct _GFBGraphGoaAuthorizerClass   GFBGraphGoaAuthorizerClass;
typedef struct _GFBGraphGoaAuthorizerPrivate GFBGraphGoaAuthorizerPrivate;

struct _GFBGraphGoaAuthorizer {
  GObject parent;

  /*< private >*/
  GFBGraphGoaAuthorizerPrivate *priv;
};

struct _GFBGraphGoaAuthorizerClass {
  GObjectClass parent_class;
};

GType                   gfbgraph_goa_authorizer_get_type (void) G_GNUC_CONST;
GFBGraphGoaAuthorizer*  gfbgraph_goa_authorizer_new      (GoaObject *goa_object);

G_END_DECLS

#endif /* __GFBGRAPH_GOA_AUTHORIZER_H__ */
