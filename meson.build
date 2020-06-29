project('CObject', 'c', version: '0.1.0')

cc = meson.get_compiler('c')

include_folders = ['include']

sources = [ './src/object.c' ]

inc = include_directories(include_folders)

CObject = static_library(
    'CObject',
    sources,
    include_directories: inc,
    install: true
)

CObject_shared = shared_library(
    'CObject',
    sources,
    version: '0.1.0',
    include_directories: inc,
    install: true
)

CObject_dep = declare_dependency(
    include_directories: inc,
    link_with: CObject_shared
)