/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * libgfbgraph - GObject library for Facebook Graph API
 * Copyright (C) 2018-2020 Leesoo Ahn <yisooan@fedoraproject.org>
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

/*
 * This code is based on glib/tests/autoptr.c
 */

#include <glib.h>

#include <gfbgraph/gfbgraph.h>

static void
test_gfbgraph_album (void)
{
  g_autoptr (GFBGraphAlbum) val = NULL;

  val = gfbgraph_album_new ();
  g_assert_nonnull (val);
}

static void
test_gfbgraph_node (void)
{
  g_autoptr (GFBGraphNode) val = NULL;

  val = gfbgraph_node_new ();
  g_assert_nonnull (val);
}

static void
test_gfbgraph_photo (void)
{
  g_autoptr (GFBGraphPhoto) val = NULL;

  val = gfbgraph_photo_new ();
  g_assert_nonnull (val);
}

static void
test_gfbgraph_user (void)
{
  g_autoptr (GFBGraphUser) val = NULL;

  val = gfbgraph_user_new ();
  g_assert_nonnull (val);
}

int
main (int   argc,
    char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/GFBGraph/autoptr/Album", test_gfbgraph_album);
  g_test_add_func ("/GFBGraph/autoptr/Node", test_gfbgraph_node);
  g_test_add_func ("/GFBGraph/autoptr/Photo", test_gfbgraph_photo);
  g_test_add_func ("/GFBGraph/autoptr/User", test_gfbgraph_user);

  return g_test_run ();
}
