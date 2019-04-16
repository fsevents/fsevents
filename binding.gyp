{
  "conditions": [
    ['OS=="mac"', {
      "targets": [{
        "target_name": "fsevents",
        "sources": [ "src/fsevents.c", "src/rawfsevents.c" ],
        "xcode_settings": {
          "OTHER_LDFLAGS": [
            "-Wl,-bind_at_load",
            "-framework CoreFoundation -framework CoreServices"
          ]
        }
      }, {
        "target_name": "action_after_build",
        "type": "none",
        "dependencies": ["fsevents"],
        "copies": [{
          "files": ["<(PRODUCT_DIR)/fsevents.node"],
          "destination": "./"
        }]
      }]
    }]
  ]
}
