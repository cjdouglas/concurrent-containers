cmake_minimum_required (VERSION 3.15)

# Configuration options
option(
	CDS_BUILD_TESTS
	"If ON, cds will build a test executable."
	OFF
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# In source guard
include(Prelude)

# Enable hot reload on MSVC if applicable
include(MSVCHotReload)

project (
	"concurrent-data-structures"
	VERSION 0.1.0
	DESCRIPTION "Thread-safe data structures"
	LANGUAGES CXX
)

# Configure cds-core library.
add_library(
	cds-core
	INTERFACE
)
target_include_directories(cds-core INTERFACE include)
set_property(TARGET cds-core PROPERTY LINKER_LANGUAGE CXX)

if (CDS_BUILD_TESTS)
	add_subdirectory(test)
endif()
