project('libgfbgraph', 'c',
        version: '0.2.4',
        meson_version: '>=0.48',
        license: 'LGPL2.1')

gnome = import('gnome')

gfbgraph_version = meson.project_version()
version_split = gfbgraph_version.split('.')
MAJOR_VERSION = version_split[0]
MINOR_VERSION = version_split[1]
MICRO_VERSION = version_split[2]

api_version = '@0@.@1@'.format(MAJOR_VERSION, MINOR_VERSION)

root_dir = include_directories('.')

subdir('gfbgraph')

subdir('tests')

