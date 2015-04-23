include(FindPkgConfig)
pkg_check_modules(GRAPHICSMAGICK GraphicsMagick++ REQUIRED)
add_definitions(${GRAPHICSMAGICK_CFLAGS})