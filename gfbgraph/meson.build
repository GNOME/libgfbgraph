lib_sources = [
  'gfbgraph-album.c',
  'gfbgraph-authorizer.c',
  'gfbgraph-common.c',
  'gfbgraph-connectable.c',
  'gfbgraph-goa-authorizer.c',
  'gfbgraph-node.c',
  'gfbgraph-photo.c',
  'gfbgraph-simple-authorizer.c',
  'gfbgraph-user.c',
]

lib_headers = [
  'gfbgraph.h',
  'gfbgraph-album.h',
  'gfbgraph-authorizer.h',
  'gfbgraph-common.h',
  'gfbgraph-connectable.h',
  'gfbgraph-goa-authorizer.h',
  'gfbgraph-node.h',
  'gfbgraph-photo.h',
  'gfbgraph-simple-authorizer.h',
  'gfbgraph-user.h',
]

lib_deps = [
  dependency('glib-2.0'),
  dependency('gio-2.0'),
  dependency('json-glib-1.0'),
  dependency('rest-0.7'),
  dependency('libsoup-2.4'),
  dependency('goa-1.0'),
]

lib_c_args = [
  '-DG_LOG_DOMAIN="GFBGraph"',
  '-g',
  '-Og'
]

extra_c_args = [
  '-DLIBSOUP_USE_UNSTABLE_REQUEST_API',
  '-DGOA_API_IS_SUBJECT_TO_CHANGE'
]

gfbgraph_lib = shared_library('gfbgraph-' + api_version,
  lib_sources,
  include_directories: root_dir,
  dependencies: lib_deps,
  c_args: lib_c_args + extra_c_args,
  install: true,
)

install_headers(lib_headers, subdir: 'gfbgraph')

