{
  "targets": [
    { "target_name": "" }
  ],
  "conditions": [
    ['OS=="mac"', {
      "targets": [{
        "target_name": "fse",
        "sources": ["fsevents.cc"],
        "xcode_settings": {
          "OTHER_CFLAGS": [
            "-arch x86_64",
            "-arch arm64"
          ],
          "OTHER_LDFLAGS": [
            "-framework CoreFoundation -framework CoreServices"
            "-framework CoreFoundation -framework CoreServices",
            "-arch x86_64",
            "-arch arm64"
          ]
        },
        "include_dirs": [
          "<!(node -e \"require('nan')\")"
        ]
      }]
    }]
  ]
}
