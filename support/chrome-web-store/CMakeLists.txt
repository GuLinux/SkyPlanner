add_custom_target(manifest zip -r -9 ${CMAKE_BINARY_DIR}/skyplanner-manifest.zip 
  ${CMAKE_SOURCE_DIR}/support/chrome-web-store/manifest.json 
  ${CMAKE_SOURCE_DIR}/support/chrome-web-store/skyplanner-logo-v1-128.png 
  ${CMAKE_SOURCE_DIR}/support/chrome-web-store/_locales )
