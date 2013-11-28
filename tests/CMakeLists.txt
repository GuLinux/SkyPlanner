if(Boost_UNIT_TEST_FRAMEWORK_LIBRARY)
  add_definitions(-DBOOST_TEST_DYN_LINK)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})
  add_custom_target(compile_unit_tests)
  add_custom_target(compile_integration_tests)
  add_custom_target(unit_tests COMMAND ctest --output-on-failure -R "unit.*" DEPENDS compile_unit_tests)
  add_custom_target(integration_tests COMMAND ctest --output-on-failure -R "integration.*" DEPENDS compile_integration_tests)
  
  function(register_test name type)
    set(test_name ${type}_test_${name})
    add_executable(${test_name} EXCLUDE_FROM_ALL ${ARGN} )
    add_test(${type}_${name} ${test_name})
    add_dependencies(compile_${type}_tests ${test_name})
  endfunction()
  
  function(link_unit_test name)
    target_link_libraries(unit_test_${name}  ${ARGN} ${Boost_LIBRARIES})
  endfunction()
  
  function(link_integration_test name)
    target_link_libraries(integration_test_${name}  ${ARGN} ${Boost_LIBRARIES})
  endfunction()
  
  function(register_unit_test name)
    register_test(${name} unit  ${ARGN})
  endfunction()
  
  function(register_integration_test name)
    register_test(${name} integration  ${ARGN})
  endfunction()
  
  
  register_unit_test(simple_test simple_test.cpp)
  include(${CMAKE_SOURCE_DIR}/src/aaplus/source_files.cmake)
  link_unit_test(simple_test)
  register_unit_test(ephemeris
    ephemeris_test.cpp
    ${CMAKE_SOURCE_DIR}/src/ephemeris.cpp
    ${CMAKE_SOURCE_DIR}/src/types.cpp
    ${AAPLUS_SRCS}
  )
  link_unit_test(ephemeris)
endif(Boost_UNIT_TEST_FRAMEWORK_LIBRARY)

