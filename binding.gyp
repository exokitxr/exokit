{
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
        '<(module_root_dir)/deps/exokit-bindings/node',
        '<(module_root_dir)/deps/exokit-bindings/native_app_glue',
        '<(module_root_dir)/deps/exokit-bindings/util/include',
        '<(module_root_dir)/deps/exokit-bindings/bindings/include',
        '<(module_root_dir)/deps/exokit-bindings/canvas/include',
        '<(module_root_dir)/deps/exokit-bindings/nanosvg/include',
        '<(module_root_dir)/deps/exokit-bindings/canvascontext/include',
        '<(module_root_dir)/deps/exokit-bindings/webglcontext/include',
      ],
      'libraries': [
        'glfw3.lib',
        ],
      'defines' : [
        'WIN32_LEAN_AND_MEAN',
        'VC_EXTRALEAN'
      ],
      'msvs_settings' : {
        'VCCLCompilerTool' : {
          'AdditionalOptions' : ['/O3',/std:[c++14]
        },
      },
    }
  ]
}
