# SeedLib

A modern C++ project template.

## Usage

### Method 1: FetchContent (Header-Only)
```cmake
include(FetchContent)
FetchContent_Declare(
    seedlib
    GIT_REPOSITORY https://github.com/yourusername/seedlib.git
    GIT_TAG v0.1.0
)
FetchContent_MakeAvailable(seedlib)

target_link_libraries(your_target PRIVATE seedlib::seedlib)
```

### Method 2: Installation
```bash
git clone https://github.com/yourusername/seedlib.git
cd seedlib
mkdir build && cd build
cmake ..
cmake --build .
sudo cmake --install .
```

Then in your CMakeLists.txt:
```cmake
find_package(seedlib 0.1.0 REQUIRED)
target_link_libraries(your_target PRIVATE seedlib::seedlib)
```

## Logging

```cpp
#include <seedlib/logging_config.hpp>
#include <seedlib/logging.hpp>

// Initialize with runtime configuration
seedlib::logging::init("myapp");

// Log messages
auto logger = seedlib::logging::get_logger();
logger->set_level(DEFAULT_LOG_LEVEL);
logger->debug("Debug message");
logger->info("Info message");
logger->warn("Warning message");
logger->error("Error message");
```
