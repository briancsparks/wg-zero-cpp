# Root CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(seedlib VERSION 0.1.0 LANGUAGES CXX C)

# Version Info
set(SEEDLIB_VERSION_MAJOR 1)
set(SEEDLIB_VERSION_MINOR 0)
set(SEEDLIB_VERSION_PATCH 0)
set(SEEDLIB_VERSION "${SEEDLIB_VERSION_MAJOR}.${SEEDLIB_VERSION_MINOR}.${SEEDLIB_VERSION_PATCH}")

# Modern C++ requirements
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

# Enable sanitizers in Debug mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -fno-omit-frame-pointer")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined")
endif()

# Only fetch dependencies if we're the main project
# This prevents dependency duplication when we're fetched
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  # Load dependencies from separate file
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Dependencies.cmake)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/coverage.cmake)
endif()

# Logging
# In CMakeLists.txt

# Logging options
option(ENABLE_DEBUG_LOGGING "Enable debug level logging" ON)
option(ENABLE_FILE_LOGGING "Enable logging to file" ON)
option(ENABLE_ASYNC_LOGGING "Enable asynchronous logging" ON)
option(ENABLE_CONSOLE_LOGGING "Enable console logging" ON)
option(ENABLE_SYSLOG_LOGGING "Enable syslog logging" OFF)
option(ENABLE_STACKTRACE_LOGGING "Enable stack traces in error logs" ON)

# Logging configuration
set(LOG_FILE_PATH "logs/seedlib.log" CACHE STRING "Path to log file")
set(LOG_MAX_SIZE "10MB" CACHE STRING "Maximum size of log file before rotation")
set(LOG_MAX_FILES 5 CACHE STRING "Number of rotated log files to keep")
set(LOG_ROTATION_TIME "daily" CACHE STRING "Log rotation interval")

# Log patterns
set(LOG_CONSOLE_PATTERN "[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%t] %v" CACHE STRING "Console log pattern")
set(LOG_FILE_PATTERN "[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] %v" CACHE STRING "File log pattern")

# Log levels
set(LOG_LEVEL_DEVELOPMENT "debug" CACHE STRING "Log level for development")
set(LOG_LEVEL_TESTING "info" CACHE STRING "Log level for testing")
set(LOG_LEVEL_PRODUCTION "warn" CACHE STRING "Log level for production")

# Performance settings
set(LOG_QUEUE_SIZE 8192 CACHE STRING "Size of async logging queue")
set(LOG_THREAD_COUNT 1 CACHE STRING "Number of logging threads")
set(LOG_FLUSH_INTERVAL_MS 1000 CACHE STRING "Log flush interval in milliseconds")

# Stacktrace settings
set(STACKTRACE_MAX_DEPTH 64 CACHE STRING "Maximum depth of stack traces")

# Optional structured logging metadata (comma-separated list)
set(LOG_STRUCTURED_METADATA "timestamp,thread_id,logger_name" CACHE STRING "Structured logging metadata fields")



# ---------------------------------------------------------------------------------------
# SEEDLIB TODO: Add your sources here

# Library target
add_library(${PROJECT_NAME}
    src/calculator.cpp
    src/string_utils.cpp
    src/include/seedlib/logging.hpp
    src/include/seedlib/url.hpp
    src/url_impl.hpp
)
# ---------------------------------------------------------------------------------------

# Create an alias target that matches the namespace export name
# This allows the same target name to be used whether the library
# is fetched or installed
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
      spdlog::spdlog
    PRIVATE
      ${CMAKE_CURRENT_BINARY_DIR}/src/include/seedlib
)


# CLI application
add_executable(${PROJECT_NAME}_cli
    apps/main.cpp
    src/include/seedlib/logging.hpp
    src/include/seedlib/url.hpp
    src/url_impl.hpp
)
target_link_libraries(${PROJECT_NAME}_cli
    PRIVATE
      ${PROJECT_NAME}
      ${CMAKE_CURRENT_BINARY_DIR}/src/include
)



# Tests
# Only include tests if we're the main project
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  enable_testing()
  add_subdirectory(tests)
endif()


# Installation and export configuration only if requested
# FetchContent users typically don't need this
option(SEEDLIB_INSTALL "Install seedlib" ${CMAKE_PROJECT_NAME_IS_TOP_LEVEL})
if (SEEDLIB_INSTALL)

  include(GNUInstallDirs)
  install(TARGETS ${PROJECT_NAME}
      EXPORT ${PROJECT_NAME}Targets
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  install(DIRECTORY include/
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  # Export targets
  install(EXPORT ${PROJECT_NAME}Targets
      FILE ${PROJECT_NAME}Targets.cmake
      NAMESPACE ${PROJECT_NAME}::
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  )

  # Generate and install package config
  include(CMakePackageConfigHelpers)
  configure_package_config_file(
      cmake/${PROJECT_NAME}Config.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
      INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  )

  write_basic_package_version_file(
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
      VERSION ${PROJECT_VERSION}
      COMPATIBILITY SameMajorVersion
  )
endif()
