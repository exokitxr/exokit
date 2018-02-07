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
      ],
      'libraries': [
        '-framework OpenGL',
        '-lglew', # brew install
        '-lglfw', # brew install
        # '-lglfw3', # for local build
        '-framework Cocoa',
        '-F <(module_root_dir)/node_modules/native-openvr-deps/bin/osx64',
        '-framework OpenVR',
      ],
      'link_settings': {
        'libraries': [
          '-Wl,-rpath,<(module_root_dir)/node_modules/native-openvr-deps/bin/osx64',
          '-framework OpenVR',
        ],
      },
      'copies': [
        {
          'destination': '<(module_root_dir)/build/Release/',
          'files': [
          ]
        }
      ],
    }
  ]
}
