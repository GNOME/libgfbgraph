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

#include "gfbgraph-authorizer.h"

G_DEFINE_INTERFACE (GFBGraphAuthorizer, gfbgraph_authorizer, G_TYPE_OBJECT)

static void
gfbgraph_authorizer_default_init (GFBGraphAuthorizerInterface *iface)
{
}

/**
 * GFBGraph_authorizer_process_call:
 * @iface: A #GFBGraphAuthorizer.
 * @call: A #RestProxyCall.
 *
 * Adds the necessary authorization to @call.
 *
 * This method modifies @call in place and is thread safe.
 */
void
gfbgraph_authorizer_process_call (GFBGraphAuthorizer *iface, RestProxyCall *call)
{
        g_return_if_fail (GFBGRAPH_IS_AUTHORIZER (iface));
        GFBGRAPH_AUTHORIZER_GET_IFACE (iface)->process_call (iface, call);
}

/**
 * GFBGraph_authorizer_process_message:
 * @iface: A #GFBGraphAuthorizer.
 * @message: A #SoupMessage.
 *
 * Adds the necessary authorization to @message. The type of @message
 * can be DELETE, GET and POST.
 *
 * This method modifies @message in place and is thread safe.
 */
void
gfbgraph_authorizer_process_message (GFBGraphAuthorizer *iface, SoupMessage *message)
{
        g_return_if_fail (GFBGRAPH_IS_AUTHORIZER (iface));
        GFBGRAPH_AUTHORIZER_GET_IFACE (iface)->process_message (iface, message);
}

/**
 * GFBGraph_authorizer_refresh_authorization:
 * @iface: A #GFBGraphAuthorizer.
 * @cancellable: (allow-none): An optional #GCancellable object, or
 *   %NULL.
 * @error: (allow-none): An optional #GError, or %NULL.
 *
 * Synchronously forces @iface to refresh any authorization tokens
 * held by it.
 *
 * This method is thread safe.
 *
 * Returns: %TRUE if the authorizer now has a valid token.
 */
gboolean
gfbgraph_authorizer_refresh_authorization (GFBGraphAuthorizer *iface, GCancellable *cancellable, GError **error)
{
        g_return_val_if_fail (GFBGRAPH_IS_AUTHORIZER (iface), FALSE);
        return GFBGRAPH_AUTHORIZER_GET_IFACE (iface)->refresh_authorization (iface, cancellable, error);
}
