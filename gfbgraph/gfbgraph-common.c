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

#include "gfbgraph-common.h"

#include <rest/rest-proxy.h>

#define FACEBOOK_ENDPOINT "https://graph.facebook.com"

/**
 * gfbgraph_new_rest_call:
 * @authorizer: a #GFBGraphAuthorizer.
 *
 * Create a new #RestProxyCall pointing to the Facebook Graph API url (https://graph.facebook.com)
 * and processed by the authorizer to allow queries.
 *
 * Returns: (transfer full): a new #RestProxyCall or %NULL in case of error.
 **/
RestProxyCall*
gfbgraph_new_rest_call (GFBGraphAuthorizer *authorizer)
{
        RestProxy *proxy;
        RestProxyCall *rest_call;

        g_return_val_if_fail (GFBGRAPH_IS_AUTHORIZER (authorizer), NULL);

        proxy = rest_proxy_new (FACEBOOK_ENDPOINT, FALSE);
        rest_call = rest_proxy_new_call (proxy);

        gfbgraph_authorizer_process_call (authorizer, rest_call);

        g_object_unref (proxy);

        return rest_call;
}
