{
  "targets": [
    {
      "target_name": "lizac",
      "include_dirs" : [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "sources": [
        "src/cpp/main.cc",
      ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
    }
  ]
}
