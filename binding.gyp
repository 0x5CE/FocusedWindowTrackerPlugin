{
  "targets": [
    {
      "target_name": "wintracker",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [ "source\main.cc", "source\imageUtilities.cpp" ],
      "include_dirs": [
	"source\\",
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
    }
  ]
}
