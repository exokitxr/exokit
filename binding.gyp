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
        'deps/exokit-bindings/platform/windows/src/*.cpp',
        'deps/glfw-bindings/src/*.cc',
      ],
      'include_dirs': [
        '<(module_root_dir)/node_modules/native-graphics-deps/include',
        '<(module_root_dir)/deps/exokit-bindings',
        '<(module_root_dir)/deps/exokit-bindings/utf8',
        '<(module_root_dir)/deps/exokit-bindings/node',
        '<(module_root_dir)/deps/exokit-bindings/native_app_glue',
        '<(module_root_dir)/deps/exokit-bindings/util/include',
        '<(module_root_dir)/deps/exokit-bindings/bindings/include',
        '<(module_root_dir)/deps/exokit-bindings/canvas/include',
        '<(module_root_dir)/deps/exokit-bindings/nanosvg/include',
        '<(module_root_dir)/deps/exokit-bindings/canvascontext/include',
        '<(module_root_dir)/deps/exokit-bindings/webglcontext/include',
        '<(module_root_dir)/deps/exokit-bindings/platform/windows/include',
        '<(module_root_dir)/deps/glfw/include',
        '<(module_root_dir)/deps/glfw-bindings/include',
      ],
      'library_dirs': [
        '<(module_root_dir)/node_modules/native-graphics-deps/windows/lib/x64',
        '<(module_root_dir)/deps/glfw/src/Release',
      ],
      'libraries': [
        'opengl32.lib',
        'glew32.lib',
        'glfw3dll.lib',
        'gdiplus.lib',
      ],
      'msvs_settings' : {
        'VCCLCompilerTool' : {
          'AdditionalOptions' : ['/O2','/Oy','/GL','/GF','/Gm-','/EHsc','/MT','/GS','/Gy','/GR-','/Gd']
        },
        'VCLinkerTool' : {
          'AdditionalOptions' : ['/OPT:REF','/OPT:ICF','/LTCG']
        },
      },
    }
  ]
}
