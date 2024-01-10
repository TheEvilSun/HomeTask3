cmake_minimum_required(VERSION 3.14)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.0.${PATCH_VERSION})

project(HomeTask3 VERSION ${PROJECT_VERSION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

configure_file(version.h.in version.h)

Include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.0.1
)

FetchContent_MakeAvailable(Catch2)

add_subdirectory(Allocators)
add_subdirectory(Containers)
add_executable(HomeTask3 main.cpp)
add_executable(AllocatorTest AllocatorTest.cpp)

target_link_libraries(HomeTask3 PUBLIC
    Allocators
    Containers
)
target_link_libraries(AllocatorTest PUBLIC
    Catch2::Catch2WithMain
    Allocators
    Containers
)

target_include_directories(HomeTask3 PUBLIC
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/Allocators"
    "${PROJECT_SOURCE_DIR}/Containers"
)

target_include_directories(AllocatorTest PUBLIC
    "${PROJECT_SOURCE_DIR}/Allocators"
    "${PROJECT_SOURCE_DIR}/Containers"
)

install(TARGETS HomeTask3 RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT alservol@gmail.com)

include(CPack)

enable_testing()

add_test(AllocatorTest AllocatorTest)
