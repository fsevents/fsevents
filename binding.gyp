{
  "targets": [{
    "target_name": "fse",
    "sources": ["src/fse_watch.cc", "src/fse_async.cc", "src/fse_thread.cc", "src/fse_native.cc"],
    "xcode_settings": {
      "OTHER_LDFLAGS": [
        "-framework CoreFoundation -framework CoreServices"
      ]
    },
    "include_dirs": [
      "<!(node -e \"require('nan')\")"
    ]
  },{
    "target_name": "fseconst",
    "sources": [ "src/fse_constants.cc" ],
    "xcode_settings": {
      "OTHER_LDFLAGS": [
        "-framework CoreServices"
      ]
    },
    "include_dirs": [
      "<!(node -e \"require('nan')\")"
    ]
  }]
}