{
  "conditions": [
    ['OS=="mac"', {
      "targets": [{
        "target_name": "fsevents",
        "sources": [ "src/fsevents.c", "src/runner.c" ],
        "xcode_settings": {
          "OTHER_LDFLAGS": [
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
