include(FetchContent)

# Catch2 for unit testing
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)

# Google Test for unit testing
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

# Make all dependencies available
FetchContent_MakeAvailable(catch2 googletest)

# Optional: Add any dependency-specific configuration here
# For example, you might want to set some variables or options for the dependencies
set(BUILD_GMOCK OFF CACHE BOOL "Disable GMock" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Disable installation of GTest" FORCE)

# Logging --------------------------------------------------------------------------------------------

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(spdlog)


# Configure logging options
option(ENABLE_DEBUG_LOGGING "Enable debug level logging" ON)
option(ENABLE_FILE_LOGGING "Enable logging to file" ON)
option(ENABLE_ASYNC_LOGGING "Enable asynchronous logging" ON)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include/seedlib/logging_config.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/src/include/seedlib/logging_config.hpp
    @ONLY
)

# DBs --------------------------------------------------------------------------------------------

# Options for database support
option(ENABLE_POSTGRES "Enable PostgreSQL support" OFF)
option(ENABLE_MYSQL "Enable MySQL support" OFF)
option(ENABLE_SQLITE "Enable SQLite support" ON)
option(ENABLE_REDIS "Enable Redis support" OFF)

# SQLite (always included as fallback)
FetchContent_Declare(
    sqlitecpp
    GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
    GIT_TAG 3.2.1
)
set(SQLITECPP_RUN_CPPLINT OFF)
FetchContent_MakeAvailable(sqlitecpp)

if(ENABLE_POSTGRES)
  find_package(libpqxx REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE libpqxx::pqxx)
  target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_POSTGRESQL)
endif()

if(ENABLE_REDIS)
  FetchContent_Declare(
      redis-plus-plus
      GIT_REPOSITORY https://github.com/sewenew/redis-plus-plus.git
      GIT_TAG master
  )
  FetchContent_MakeAvailable(redis-plus-plus)
  target_link_libraries(${PROJECT_NAME} PRIVATE redis++)
  target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_REDIS)
endif()
