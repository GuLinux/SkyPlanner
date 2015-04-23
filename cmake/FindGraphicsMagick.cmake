include(FindPkgConfig)
pkg_check_modules(GRAPHICSMAGICK GraphicsMagic++ REQUIRED)
add_definitions(${GRAPHICSMAGICK_CFLAGS})