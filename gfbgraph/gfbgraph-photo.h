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

#ifndef __GFBGRAPH_PHOTO_H__
#define __GFBGRAPH_PHOTO_H__

#include <gio/gio.h>
#include <gfbgraph/gfbgraph-node.h>

G_BEGIN_DECLS

#define GFBGRAPH_TYPE_PHOTO             (gfbgraph_photo_get_type())
#define GFBGRAPH_PHOTO(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),GFBGRAPH_TYPE_PHOTO,GFBGraphPhoto))
#define GFBGRAPH_PHOTO_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),GFBGRAPH_TYPE_PHOTO,GFBGraphPhotoClass))
#define GFBGRAPH_IS_PHOTO(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),GFBGRAPH_TYPE_PHOTO))
#define GFBGRAPH_IS_PHOTO_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),GFBGRAPH_TYPE_PHOTO))
#define GFBGRAPH_PHOTO_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),GFBGRAPH_TYPE_PHOTO,GFBGraphPhotoClass))

typedef struct _GFBGraphPhoto        GFBGraphPhoto;
typedef struct _GFBGraphPhotoClass   GFBGraphPhotoClass;
typedef struct _GFBGraphPhotoPrivate GFBGraphPhotoPrivate;

struct _GFBGraphPhoto {
        GFBGraphNode parent;

        /*< private >*/
        GFBGraphPhotoPrivate *priv;
};

struct _GFBGraphPhotoClass {
        GFBGraphNodeClass parent_class;
};

typedef struct _GFBGraphPhotoImage GFBGraphPhotoImage;

struct _GFBGraphPhotoImage {
        guint  width;
        guint  height;
        gchar *source;
};

GType          gfbgraph_photo_get_type (void) G_GNUC_CONST;
GFBGraphPhoto* gfbgraph_photo_new      (void);
GFBGraphPhoto* gfbgraph_photo_new_from_id (GFBGraphAuthorizer *authorizer, const gchar *id, GError **error);
GInputStream*  gfbgraph_photo_download_default_size (GFBGraphPhoto *photo, GFBGraphAuthorizer *authorizer, GError **error);

const gchar*        gfbgraph_photo_get_name               (GFBGraphPhoto *photo);
const gchar*        gfbgraph_photo_get_default_source_uri (GFBGraphPhoto *photo);
guint               gfbgraph_photo_get_default_width      (GFBGraphPhoto *photo);
guint               gfbgraph_photo_get_default_height     (GFBGraphPhoto *photo);
GList*              gfbgraph_photo_get_images             (GFBGraphPhoto *photo);
GFBGraphPhotoImage* gfbgraph_photo_get_image_hires        (GFBGraphPhoto *photo);

G_END_DECLS

#endif /* __GFBGRAPH_PHOTO_H__ */
