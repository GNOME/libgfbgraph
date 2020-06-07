/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * libgfbgraph - GObject library for Facebook Graph API
 * Copyright (C) 2013-2014 Álvaro Peña <alvaropg@gmail.com>
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

/**
 * SECTION:gfbgraph-user
 * @short_description: GFBGraph User node
 * @stability: Unstable
 * @include: gfbgraph/gfbgraph.h
 *
 * #GFBGraphUser represents a <ulink url="https://developers.facebook.com/docs/reference/api/user/">user in Facebook</ulink>.
 * With the "me" functions, (see gfbgraph_user_get_me()) you can query for the logged user node.
 **/

#include <gmodule.h>
#include <json-glib/json-glib.h>

#include "gfbgraph-user.h"
#include "gfbgraph-album.h"
#include "gfbgraph-common.h"

#define ME_FUNCTION "me"

enum {
  PROP_0,
  PROP_NAME,
  PROP_EMAIL
};

struct _GFBGraphUserPrivate {
  gchar *name;
  gchar *email;
};

typedef struct {
  GFBGraphAuthorizer *authorizer;
  GList *nodes;
} GFBGraphUserConnectionAsyncData;

#define GFBGRAPH_USER_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE((o), GFBGRAPH_TYPE_USER, GFBGraphUserPrivate))

static GFBGraphNodeClass *parent_class = NULL;

G_DEFINE_TYPE (GFBGraphUser, gfbgraph_user, GFBGRAPH_TYPE_NODE);

static void
gfbgraph_user_finalize (GObject *object)
{
  GFBGraphUserPrivate *priv = GFBGRAPH_USER_GET_PRIVATE (object);

  g_free (priv->name);
  g_free (priv->email);

  G_OBJECT_CLASS(parent_class)->finalize (object);
}

static void
gfbgraph_user_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  GFBGraphUserPrivate *priv = GFBGRAPH_USER_GET_PRIVATE (object);

  switch (prop_id) {
    case PROP_NAME:
      if (priv->name)
        g_free (priv->name);
      priv->name = g_strdup (g_value_get_string (value));
      break;
    case PROP_EMAIL:
      if (priv->email)
        g_free (priv->email);
      priv->email = g_strdup (g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gfbgraph_user_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  GFBGraphUserPrivate *priv = GFBGRAPH_USER_GET_PRIVATE (object);

  switch (prop_id) {
    case PROP_NAME:
      g_value_set_string (value, priv->name);
      break;
    case PROP_EMAIL:
      g_value_set_string (value, priv->email);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gfbgraph_user_init (GFBGraphUser *object)
{
  object->priv = GFBGRAPH_USER_GET_PRIVATE (object);
}

static void
gfbgraph_user_class_init (GFBGraphUserClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  parent_class            = g_type_class_peek_parent (klass);
  gobject_class->finalize = gfbgraph_user_finalize;
  gobject_class->set_property = gfbgraph_user_set_property;
  gobject_class->get_property = gfbgraph_user_get_property;

  g_type_class_add_private (gobject_class, sizeof(GFBGraphUserPrivate));

  /**
   * GFBGraphUser:name:
   *
   * The full name of the user
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_NAME,
                                   g_param_spec_string ("name",
                                                        "User's full name",
                                                        "The full name of the user",
                                                        "",
                                                        G_PARAM_READABLE | G_PARAM_WRITABLE));

  /**
   * GFBGraphUser:email:
   *
   * The email of the user if available
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_EMAIL,
                                   g_param_spec_string ("email",
                                                        "User's email",
                                                        "The user primary email if available",
                                                        NULL,
                                                        G_PARAM_READABLE | G_PARAM_WRITABLE));
}

/* --- Private Functions --- */
static void
connection_async_data_free (GFBGraphUserConnectionAsyncData *data)
{
  g_object_unref (data->authorizer);

  g_slice_free (GFBGraphUserConnectionAsyncData, data);
}

static void
get_me_async_io_thread (GTask        *task,
                        gpointer      source_object,
                        gpointer      task_data,
                        GCancellable *cancellable)
{
  GFBGraphAuthorizer *authorizer = GFBGRAPH_AUTHORIZER (source_object);
  GFBGraphUser *user = NULL;
  GError *error = NULL;

  user = gfbgraph_user_get_me (authorizer, &error);
  if (user && !error)
    g_task_return_pointer (task, user, g_object_unref);
  else
    {
      /* FIXME: better way to handle error */
      if (error)
        g_task_return_error (task, error);
    }
}

static void
get_albums_async_thread (GSimpleAsyncResult *simple_async,
                         GFBGraphUser       *user,
                         GCancellable        cancellable)
{
  GFBGraphUserConnectionAsyncData *data;
  GError *error = NULL;

  data = (GFBGraphUserConnectionAsyncData *) g_simple_async_result_get_op_res_gpointer (simple_async);
  data->nodes = gfbgraph_user_get_albums (user, data->authorizer, &error);
  if (error != NULL)
    g_simple_async_result_take_error (simple_async, error);
}

/**
 * gfbgraph_user_new:
 *
 * Creates a new #GFBGraphUser.
 *
 * Returns: a new #GFBGraphUser; unref with g_object_unref()
 **/
GFBGraphUser *
gfbgraph_user_new (void)
{
  return GFBGRAPH_USER (g_object_new (GFBGRAPH_TYPE_USER, NULL));
}

/**
 * gfbgraph_user_new_from_id:
 * @authorizer: a #GFBGraphAuthorizer.
 * @id: a const #gchar with the user ID.
 * @error: (allow-none): a #GError or %NULL.
 *
 * Retrieves a user from the Facebook Graph with the give ID.
 *
 * Returns: (transfer full): a new #GFBGraphUser; unref with g_object_unref()
 **/
GFBGraphUser *
gfbgraph_user_new_from_id (GFBGraphAuthorizer  *authorizer,
                           const gchar         *id,
                           GError             **error)
{
  return GFBGRAPH_USER (gfbgraph_node_new_from_id (authorizer,
                                                   id,
                                                   GFBGRAPH_TYPE_USER,
                                                   error));
}

/**
 * gfbgraph_user_get_me:
 * @authorizer: a #GFBGraphAuthorizer.
 * @error: (allow-none): a #GError or %NULL.
 *
 * Retrieve the current user logged using the https://graph.facebook.com/me Graph API function.
 * See gfbgraph_user_get_my_async() for the asynchronous version of this call.
 *
 * Returns: (transfer full): a #GFBGraphUser with the current user information.
 **/
GFBGraphUser *
gfbgraph_user_get_me (GFBGraphAuthorizer  *authorizer,
                      GError             **error)
{
  GFBGraphUser *me = NULL;
  RestProxyCall *rest_call;
  const gchar *payload;
  gboolean result;

  g_return_val_if_fail (GFBGRAPH_IS_AUTHORIZER (authorizer), NULL);

  rest_call = gfbgraph_new_rest_call (authorizer);
  rest_proxy_call_set_function (rest_call, ME_FUNCTION);
  rest_proxy_call_set_method (rest_call, "GET");
  rest_proxy_call_add_param (rest_call, "fields", "name,email");

  result = rest_proxy_call_sync (rest_call, error);
  if (result) {
    JsonParser *parser;
    JsonNode *node;

    payload = rest_proxy_call_get_payload (rest_call);
    parser = json_parser_new ();
    if (json_parser_load_from_data (parser, payload, -1, error)) {
      node = json_parser_get_root (parser);
      me = GFBGRAPH_USER (json_gobject_deserialize (GFBGRAPH_TYPE_USER, node));
    }

    g_object_unref (parser);
  }
  g_object_unref (rest_call);

  return me;
}

/**
 * gfbgraph_user_get_me_async:
 * @authorizer: a #GFBGraphAuthorizer.
 * @cancellable: (allow-none): An optional #GCancellable object, or %NULL.
 * @callback: (scope async): A #GAsyncReadyCallback to call when the request is completed.
 * @user_data: (closure): The data to pass to @callback.
 *
 * Asynchronously retrieve the current user logged. See gfbgraph_user_get_me() for the
 * synchronous version of this call.
 *
 * When the operation is finished, @callback will be called. You can then call gfbgraph_user_get_me_finish()
 * to get the #GFBGraphUser for to the current user logged.
 **/
void
gfbgraph_user_get_me_async (GFBGraphAuthorizer  *authorizer,
                            GCancellable        *cancellable,
                            GAsyncReadyCallback  callback,
                            gpointer             user_data)
{
  GTask *task;

  g_return_if_fail (GFBGRAPH_IS_AUTHORIZER (authorizer));
  g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));
  g_return_if_fail (callback != NULL);

  task = g_task_new (authorizer,
                     cancellable,
                     callback,
                     user_data);
  g_task_run_in_thread (task, get_me_async_io_thread);

  g_object_unref (task);
}

/**
 * gfbgraph_user_get_me_async_finish:
 * @authorizer: a #GFBGraphAuthorizer.
 * @result: A #GAsyncResult.
 * @error: (allow-none): An optional #GError, or %NULL.
 *
 * Finishes an asynchronous operation started with
 * gfbgraph_user_get_me_async().
 *
 * Returns: (transfer full): a #GFBGraphUser for to the current user logged.
 **/
GFBGraphUser *
gfbgraph_user_get_me_async_finish (GFBGraphAuthorizer  *authorizer,
                                   GAsyncResult        *result,
                                   GError             **error)
{
  g_return_val_if_fail (g_task_is_valid (result, authorizer), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  return g_task_propagate_pointer (G_TASK (result), error);
}

/**
 * gfbgraph_user_get_albums:
 * @user: a #GFBGraphUser.
 * @authorizer: a #GFBGraphAuthorizer.
 * @error: (allow-none): An optional #GError, or %NULL.
 *
 * Retrieve the albums nodes owned by the @user. This functions call the function ID/albums.
 *
 * Returns: (element-type GFBGraphAlbum) (transfer full): a newly-allocated #GList with the albums nodes owned by the given user.
 **/
GList *
gfbgraph_user_get_albums (GFBGraphUser        *user,
                          GFBGraphAuthorizer  *authorizer,
                          GError             **error)
{
  g_return_val_if_fail (GFBGRAPH_IS_USER (user), NULL);
  g_return_val_if_fail (GFBGRAPH_IS_AUTHORIZER (authorizer), NULL);

  return gfbgraph_node_get_connection_nodes (GFBGRAPH_NODE (user),
                                             GFBGRAPH_TYPE_ALBUM,
                                             authorizer,
                                             error);
}

/**
 * gfbgraph_user_get_albums_async:
 * @user: a #GFBGraphUser.
 * @authorizer: a #GFBGraphAuthorizer.
 * @cancellable: (allow-none): An optional #GCancellable object, or %NULL.
 * @callback: (scope async): A #GAsyncReadyCallback to call when the request is completed.
 * @user_data: (closure): The data to pass to @callback.
 *
 * Asynchronously retrieve the albums nodes owned by the @user. See gfbgraph_user_get_albums() for the
 * synchronous version of this call.
 *
 * When the operation is finished, @callback will be called. You can then call gfbgraph_user_get_albums_async_finish()
 * to get the #GList of #GFBGraphAlbum owned by the @user.
 **/
void
gfbgraph_user_get_albums_async (GFBGraphUser        *user,
                                GFBGraphAuthorizer  *authorizer,
                                GCancellable        *cancellable,
                                GAsyncReadyCallback  callback,
                                gpointer             user_data)
{
  GSimpleAsyncResult *simple_async;
  GFBGraphUserConnectionAsyncData *data;

  g_return_if_fail (GFBGRAPH_IS_USER (user));
  g_return_if_fail (GFBGRAPH_IS_AUTHORIZER (authorizer));
  g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));
  g_return_if_fail (callback != NULL);

  simple_async = g_simple_async_result_new (G_OBJECT (user),
                                            callback,
                                            user_data,
                                            gfbgraph_user_get_albums_async);
  g_simple_async_result_set_check_cancellable (simple_async,
                                               cancellable);

  data = g_slice_new (GFBGraphUserConnectionAsyncData);
  data->nodes = NULL;
  data->authorizer = authorizer;
  g_object_ref (data->authorizer);

  g_simple_async_result_set_op_res_gpointer (simple_async,
                                             data,
                                             (GDestroyNotify)connection_async_data_free);
  g_simple_async_result_run_in_thread (simple_async,
                                       (GSimpleAsyncThreadFunc)get_albums_async_thread,
                                       G_PRIORITY_DEFAULT,
                                       cancellable);

  g_object_unref (simple_async);
}

/**
 * gfbgraph_user_get_albums_async_finish:
 * @user: a #GFBGraphUser.
 * @result: A #GAsyncResult.
 * @error: (allow-none): An optional #GError, or %NULL.
 *
 * Finishes an asynchronous operation started with
 * gfbgraph_user_get_albums_async().
 *
 * Returns: (element-type GFBGraphAlbum) (transfer full): a newly-allocated #GList of albums owned by the @user.
 **/
GList *
gfbgraph_user_get_albums_async_finish (GFBGraphUser  *user,
                                       GAsyncResult  *result,
                                       GError       **error)
{
  GSimpleAsyncResult *simple_async;
  GFBGraphUserConnectionAsyncData *data;

  g_return_val_if_fail (GFBGRAPH_IS_USER (user), NULL);
  g_return_val_if_fail (g_simple_async_result_is_valid (result,
                                                        G_OBJECT (user),
                                                        gfbgraph_user_get_albums_async), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  simple_async = G_SIMPLE_ASYNC_RESULT (result);

  if (g_simple_async_result_propagate_error (simple_async, error))
    return NULL;

  data = (GFBGraphUserConnectionAsyncData *) g_simple_async_result_get_op_res_gpointer (simple_async);
  return data->nodes;
}

/**
 * gfbgraph_user_get_name:
 * @user: a #GFBGraphUser.
 *
 * Get the user full name.
 *
 * Returns: (transfer none): a const #gchar with the user full name, or %NULL.
 **/
const gchar *
gfbgraph_user_get_name (GFBGraphUser *user)
{
  g_return_val_if_fail (GFBGRAPH_IS_USER (user), NULL);

  return user->priv->name;
}

/**
 * gfbgraph_user_get_email:
 * @user: a #GFBGraphUser.
 *
 * Get the user email. To retrieve this propertie, you need 'email' extended
 * permission.
 *
 * Returns: (transfer none): a const #gchar with the user email, or %NULL.
 **/
const gchar *
gfbgraph_user_get_email (GFBGraphUser *user)
{
  g_return_val_if_fail (GFBGRAPH_IS_USER (user), NULL);

  return user->priv->email;
}
