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

#ifndef __GFBGRAPH_ALBUM_H__
#define __GFBGRAPH_ALBUM_H__

#include <gfbgraph/gfbgraph-node.h>
#include <gfbgraph/gfbgraph-authorizer.h>

G_BEGIN_DECLS

#define GFBGRAPH_TYPE_ALBUM             (gfbgraph_album_get_type())
#define GFBGRAPH_ALBUM(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),GFBGRAPH_TYPE_ALBUM,GFBGraphAlbum))
#define GFBGRAPH_ALBUM_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),GFBGRAPH_TYPE_ALBUM,GFBGraphAlbumClass))
#define GFBGRAPH_IS_ALBUM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),GFBGRAPH_TYPE_ALBUM))
#define GFBGRAPH_IS_ALBUM_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),GFBGRAPH_TYPE_ALBUM))
#define GFBGRAPH_ALBUM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),GFBGRAPH_TYPE_ALBUM,GFBGraphAlbumClass))

typedef struct _GFBGraphAlbum        GFBGraphAlbum;
typedef struct _GFBGraphAlbumClass   GFBGraphAlbumClass;
typedef struct _GFBGraphAlbumPrivate GFBGraphAlbumPrivate;

struct _GFBGraphAlbum {
        GFBGraphNode parent;

        /*< private >*/
        GFBGraphAlbumPrivate *priv;
};

struct _GFBGraphAlbumClass {
        GFBGraphNodeClass parent_class;
};

GType          gfbgraph_album_get_type    (void) G_GNUC_CONST;
GFBGraphAlbum* gfbgraph_album_new         (void);
GFBGraphAlbum* gfbgraph_album_new_from_id (GFBGraphAuthorizer *authorizer, const gchar *id, GError **error);

G_END_DECLS

#endif /* __GFBGRAPH_ALBUM_H__ */
