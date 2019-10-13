configure_file(input : 'credentials.ini',
               output : 'credentials.ini',
               copy: true)

test('autoptr',
  executable('autoptr', 'autoptr.c',
             include_directories: root_dir,
             dependencies: lib_deps,
             c_args: lib_c_args,
             link_with: gfbgraph_lib),
  env: [
    'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
    'G_TEST_BUILDDIR=@0@'.format(meson.current_source_dir()),
  ]
)

test('gtestutils',
  executable('gtestutils', 'gtestutils.c',
             include_directories: root_dir,
             dependencies: lib_deps,
             c_args: lib_c_args,
             link_with: gfbgraph_lib),
  env: [
    'CREDENTIALS_PATH=@0@'.format(meson.current_source_dir()),
    'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
    'G_TEST_BUILDDIR=@0@'.format(meson.current_source_dir()),
  ]
)

