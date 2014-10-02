file(GLOB strings *.xml)
install(FILES ${strings} DESTINATION share/${CMAKE_PROJECT_NAME}/strings)
install(DIRECTORY html DESTINATION share/${CMAKE_PROJECT_NAME}/strings)
