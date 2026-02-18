{
  "targets": [
    {
      "target_name": "electron_printer",
      "sources": [
        "src/main.cpp",
        "src/print.cpp",
        "src/printer_factory.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "defines": [ "NAPI_CPP_EXCEPTIONS" ],
      "conditions": [
        ['OS=="win"', {
          "sources": ["src/windows_printer.cpp"],
          "libraries": ["winspool.lib"],
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1
            }
          }
        }],
        ['OS=="mac"', {
          "sources": ["src/mac_printer.cpp"],
          "libraries": ["-lcups"],
          "include_dirs": [
            "/usr/include/cups"
          ],
          "xcode_settings": {
            "OTHER_CFLAGS": ["-Wall"],
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "CLANG_CXX_LIBRARY": "libc++",
            "MACOSX_DEPLOYMENT_TARGET": "10.7"
          }
        }],
        ['OS=="linux"', {
          "sources": ["src/linux_printer.cpp"],
          "libraries": ["-lcups"],
          "include_dirs": [
            "/usr/include/cups"
          ],
          "cflags": [
            "-Wall",
            "-fexceptions"
          ],
          "cflags_cc": [
            "-fexceptions"
          ]
        }]
      ]
    }
  ]
}
