if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/googletest/CMakeLists.txt")
  message("Found googletest, enabling testing")
  add_subdirectory(googletest)
  add_custom_target(compile_unit_tests)
  add_custom_target(compile_integration_tests)
  add_custom_target(unit_tests COMMAND ctest --output-on-failure -R "unit.*" DEPENDS compile_unit_tests)
  add_custom_target(integration_tests COMMAND ctest --output-on-failure -R "integration.*" DEPENDS compile_integration_tests)


  function(add_test)
    #set(options)
    set(oneValueArgs NAME TYPE)
    set(multiValueArgs SOURCES TARGET_LINK_LIBRARIES)
    cmake_parse_arguments(add_test "${options}"  "${oneValueArgs}" "${multiValueArgs}" "${ARGN}")
    message("adding test: name=${add_test_NAME}, type=${add_test_TYPE}, sources: ${add_test_SOURCES}, link: ${add_test_TARGET_LINK_LIBRARIES}")
    set(test_name test_${add_test_TYPE}_${add_test_NAME})
    add_executable(${test_name} ${add_test_SOURCES})
    target_link_libraries(${test_name} ${add_test_TARGET_LINK_LIBRARIES} gtest_main)
    add_dependencies(compile_${add_test_TYPE}_tests ${test_name})
  endfunction(add_test)

  add_subdirectory(nova)



endif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/googletest/CMakeLists.txt")