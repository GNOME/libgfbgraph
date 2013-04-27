#ifndef __GFBGRAPH_SIMPLE_AUTHORIZER_H__
#define __GFBGRAPH_SIMPLE_AUTHORIZER_H__

#include <glib-object.h>

G_BEGIN_DECLS

/* convenience macros */
#define GFBGRAPH_TYPE_SIMPLE_AUTHORIZER             (gfbgraph_simple_authorizer_get_type())
#define GFBGRAPH_SIMPLE_AUTHORIZER(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),GFBGRAPH_TYPE_SIMPLE_AUTHORIZER,GFBGraphSimpleAuthorizer))
#define GFBGRAPH_SIMPLE_AUTHORIZER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),GFBGRAPH_TYPE_SIMPLE_AUTHORIZER,GFBGraphSimpleAuthorizerClass))
#define GFBGRAPH_IS_SIMPLE_AUTHORIZER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),GFBGRAPH_TYPE_SIMPLE_AUTHORIZER))
#define GFBGRAPH_IS_SIMPLE_AUTHORIZER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),GFBGRAPH_TYPE_SIMPLE_AUTHORIZER))
#define GFBGRAPH_SIMPLE_AUTHORIZER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),GFBGRAPH_TYPE_SIMPLE_AUTHORIZER,GFBGraphSimpleAuthorizerClass))

typedef struct _GFBGraphSimpleAuthorizer        GFBGraphSimpleAuthorizer;
typedef struct _GFBGraphSimpleAuthorizerClass   GFBGraphSimpleAuthorizerClass;
typedef struct _GFBGraphSimpleAuthorizerPrivate GFBGraphSimpleAuthorizerPrivate;

struct _GFBGraphSimpleAuthorizer {
        GObject parent;

        GFBGraphSimpleAuthorizerPrivate *priv;
};

struct _GFBGraphSimpleAuthorizerClass {
        GObjectClass parent_class;
};

GType                     gfbgraph_simple_authorizer_get_type (void) G_GNUC_CONST;
GFBGraphSimpleAuthorizer* gfbgraph_simple_authorizer_new      (void);

G_END_DECLS

#endif /* __GFBGRAPH_SIMPLE_AUTHORIZER_H__ */
