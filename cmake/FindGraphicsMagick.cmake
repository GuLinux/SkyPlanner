include(FindPkgConfig)
pkg_check_modules(GRAPHICSMAGICK ImageMagick++ REQUIRED)
add_definitions(${GRAPHICSMAGICK_CFLAGS})