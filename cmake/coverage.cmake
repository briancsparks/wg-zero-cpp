# In root CMakeLists.txt or a separate Coverage.cmake

option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

if(ENABLE_COVERAGE)
  # Add coverage flags
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

  # Create coverage target
  add_custom_target(coverage
      # Clear any previous coverage data
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
      COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/coverage/*

      # Run tests
      COMMAND ctest --output-on-failure

      # Generate coverage data
      COMMAND lcov --capture --directory . --output-file ${CMAKE_BINARY_DIR}/coverage/coverage.info

      # Filter out system headers and test files
      COMMAND lcov
      --remove ${CMAKE_BINARY_DIR}/coverage/coverage.info
      '/usr/include/*' '/usr/lib/*' '*/tests/*' '*/external/*'
      --output-file ${CMAKE_BINARY_DIR}/coverage/coverage.info

      # Generate HTML report
      COMMAND genhtml ${CMAKE_BINARY_DIR}/coverage/coverage.info
      --output-directory ${CMAKE_BINARY_DIR}/coverage/html
      --title "${PROJECT_NAME} Code Coverage"
      --legend --show-details --branch-coverage

      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Generating code coverage report..."
  )
endif()
