{
  'targets': [
    {
      'target_name': 'exokit',
      'sources': [
        'main.cpp',
        '<!@(ls -1 deps/exokit-bindings/bindings/src/*.cc)',
        '<!@(ls -1 deps/exokit-bindings/canvas/src/*.cpp)',
        '<!@(ls -1 deps/exokit-bindings/nanosvg/src/*.cpp)',
        '<!@(ls -1 deps/exokit-bindings/canvascontext/src/*.cc)',
        '<!@(ls -1 deps/exokit-bindings/webglcontext/src/*.cc)',
        '<!@(ls -1 deps/exokit-bindings/platform/macos/src/*.cpp)',
        '<!@(ls -1 deps/glfw-bindings/src/*.cc)',
        '<!@(ls -1 deps/openvr/src/*.cpp)',
      ],
      'include_dirs': [
        '<(module_root_dir)/node_modules/native-graphics-deps/include',
        '<(module_root_dir)/node_modules/native-openvr-deps/headers',
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
        '<(module_root_dir)/deps/exokit-bindings/platform/macos/include',
        '<(module_root_dir)/deps/glfw/include',
        '<(module_root_dir)/deps/glfw-bindings/include',
        '<(module_root_dir)/deps/openvr/include',
      ],
      'library_dirs': [
        # '<(module_root_dir)/deps/glfw/src',
        # '<(module_root_dir)/node_modules/native-openvr-deps/lib/win64',
      ],
      'libraries': [
        '-lglew', # brew install
        '-lglfw', # brew install
        # '-lglfw3',
        '-framework Cocoa'
        # 'openvr_api.lib',
      ],
      'copies': [
        {
          'destination': '<(module_root_dir)/build/Release/',
          'files': [
            # '<(module_root_dir)/node_modules/native-openvr-deps/bin/win64/openvr_api.dll',
          ]
        }
      ],
    }
  ]
}
