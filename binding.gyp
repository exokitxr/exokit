{
  'variables': {
    'platform': '<(OS)',
  },
  'conditions': [
    # Replace gyp platform with node platform, blech
    ['platform == "mac"', {'variables': {'platform': 'darwin'}}],
    ['platform == "win"', {'variables': {'platform': 'win32'}}],
  ],
  'targets': [
    {
      'target_name': 'exokit',
      'sources': [
        'main.cpp',
        'deps/exokit-bindings/bindings/src/*.cc',
        'deps/exokit-bindings/canvas/src/*.cpp',
        'deps/exokit-bindings/nanosvg/src/*.cpp',
        'deps/exokit-bindings/canvascontext/src/*.cc',
        'deps/exokit-bindings/webglcontext/src/*.cc',
      ],
      'include_dirs': [
        "<!(node -e \"require('nan')\")",
        '<(module_root_dir)/deps/exokit-bindings/node',
        '<(module_root_dir)/deps/exokit-bindings/native_app_glue',
        '<(module_root_dir)/deps/exokit-bindings/util/include',
        '<(module_root_dir)/deps/exokit-bindings/bindings/include',
        '<(module_root_dir)/deps/exokit-bindings/canvas/include',
        '<(module_root_dir)/deps/exokit-bindings/nanosvg/include',
        '<(module_root_dir)/deps/exokit-bindings/canvascontext/include',
        '<(module_root_dir)/deps/exokit-bindings/webglcontext/include',
      ],
    }
  ]
}
