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
        'deps/exokit-bindings/webaudiocontext/src/*.cpp',
        'deps/glfw-bindings/src/*.cc',
        'deps/openvr/src/*.cpp',
      ],
      'include_dirs': [
        '<(module_root_dir)/node_modules/native-graphics-deps/include',
        '<(module_root_dir)/node_modules/native-openvr-deps/headers',
        '<(module_root_dir)/node_modules/native-canvas-deps/include/core',
        '<(module_root_dir)/node_modules/native-canvas-deps/include/config',
        '<(module_root_dir)/node_modules/native-canvas-deps/include/gpu',
        '<(module_root_dir)/node_modules/native-audio-deps/include',
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
        '<(module_root_dir)/deps/exokit-bindings/webaudiocontext/include',
        '<(module_root_dir)/deps/exokit-bindings/platform/windows/include',
        '<(module_root_dir)/deps/glfw/include',
        '<(module_root_dir)/deps/glfw-bindings/include',
        '<(module_root_dir)/deps/openvr/include',
      ],
      'library_dirs': [
        '<(module_root_dir)/node_modules/native-graphics-deps/windows/lib/x64',
        '<(module_root_dir)/node_modules/native-canvas-deps/lib/windows',
        '<(module_root_dir)/node_modules/native-audio-deps/lib/windows',
        '<(module_root_dir)/node_modules/native-openvr-deps/lib/win64',
      ],
      'libraries': [
        'opengl32.lib',
        'glew32.lib',
        'glfw3dll.lib',
        'gdiplus.lib',
        'skia.lib',
        'LabSound.lib',
        'libnyquist.lib',
        'openvr_api.lib',
      ],
      'copies': [
        {
          'destination': '<(module_root_dir)/build/Release/',
          'files': [
            '<(module_root_dir)/node_modules/native-graphics-deps/windows/lib/x64/glew32.dll',
            '<(module_root_dir)/node_modules/native-graphics-deps/windows/lib/x64/glfw3.dll',
            '<(module_root_dir)/node_modules/native-openvr-deps/bin/win64/openvr_api.dll',
          ]
        }
      ],
      'defines': ['NOMINMAX'],
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
