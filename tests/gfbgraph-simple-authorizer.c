#include "gfbgraph-simple-authorizer.h"
#include "credentials.h"

#include <gfbgraph/gfbgraph-authorizer.h>

struct _GFBGraphSimpleAuthorizerPrivate {
        GMutex mutex;
};

static void gfbgraph_simple_authorizer_init       (GFBGraphSimpleAuthorizer *obj);
static void gfbgraph_simple_authorizer_class_init (GFBGraphSimpleAuthorizerClass *klass);
static void gfbgraph_simple_authorizer_finalize   (GObject *obj);

static void gfbgraph_simple_authorizer_iface_init (GFBGraphAuthorizerInterface *iface);

void     gfbgraph_simple_authorizer_process_call          (GFBGraphAuthorizer *iface, RestProxyCall *call);
void     gfbgraph_simple_authorizer_process_message       (GFBGraphAuthorizer *iface, SoupMessage *message);
gboolean gfbgraph_simple_authorizer_refresh_authorization (GFBGraphAuthorizer *iface, GCancellable *cancellable, GError **error);

#define GFBGRAPH_SIMPLE_AUTHORIZER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), GFBGRAPH_TYPE_SIMPLE_AUTHORIZER, GFBGraphSimpleAuthorizerPrivate))

static GObjectClass *parent_class = NULL;

G_DEFINE_TYPE_WITH_CODE (GFBGraphSimpleAuthorizer, gfbgraph_simple_authorizer, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GFBGRAPH_TYPE_AUTHORIZER, gfbgraph_simple_authorizer_iface_init));


static void
gfbgraph_simple_authorizer_init (GFBGraphSimpleAuthorizer *obj)
{
        obj->priv = GFBGRAPH_SIMPLE_AUTHORIZER_GET_PRIVATE(obj);
        g_mutex_init (&obj->priv->mutex);
}

static void
gfbgraph_simple_authorizer_class_init (GFBGraphSimpleAuthorizerClass *klass)
{
        GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

        parent_class            = g_type_class_peek_parent (klass);
        gobject_class->finalize = gfbgraph_simple_authorizer_finalize;

        g_type_class_add_private (gobject_class, sizeof(GFBGraphSimpleAuthorizerPrivate));
}

static void
gfbgraph_simple_authorizer_finalize (GObject *obj)
{
        G_OBJECT_CLASS(parent_class)->finalize (obj);
}

static void
gfbgraph_simple_authorizer_iface_init (GFBGraphAuthorizerInterface *iface)
{
        iface->process_call = gfbgraph_simple_authorizer_process_call;
        iface->process_message = gfbgraph_simple_authorizer_process_message;
        iface->refresh_authorization = gfbgraph_simple_authorizer_refresh_authorization;
}

void
gfbgraph_simple_authorizer_process_call (GFBGraphAuthorizer *iface, RestProxyCall *call)
{
        GFBGraphSimpleAuthorizerPrivate *priv;

        priv = GFBGRAPH_SIMPLE_AUTHORIZER_GET_PRIVATE (GFBGRAPH_SIMPLE_AUTHORIZER (iface));

        g_mutex_lock (&priv->mutex);
        rest_proxy_call_add_param (call, "access_token", GFBGRAPH_TEST_ACCESS_TOKEN);
        g_mutex_unlock (&priv->mutex);
}

void
gfbgraph_simple_authorizer_process_message (GFBGraphAuthorizer *iface, SoupMessage *message)
{
        gchar *auth_value;
        SoupURI *uri;
        GFBGraphSimpleAuthorizerPrivate *priv;

        priv = GFBGRAPH_SIMPLE_AUTHORIZER_GET_PRIVATE (GFBGRAPH_SIMPLE_AUTHORIZER (iface));

        g_mutex_lock (&priv->mutex);

        uri = soup_message_get_uri (message);
        auth_value = g_strconcat ("access_token=", GFBGRAPH_TEST_ACCESS_TOKEN, NULL);
        soup_uri_set_query (uri, auth_value);

        g_free (auth_value);

        g_mutex_unlock (&priv->mutex);
}

gboolean
gfbgraph_simple_authorizer_refresh_authorization (GFBGraphAuthorizer *iface, GCancellable *cancellable, GError **error)
{
        return FALSE;
}

GFBGraphSimpleAuthorizer*
gfbgraph_simple_authorizer_new (void)
{
        return GFBGRAPH_SIMPLE_AUTHORIZER (g_object_new (GFBGRAPH_TYPE_SIMPLE_AUTHORIZER, NULL));
}
