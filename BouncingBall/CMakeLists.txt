# CMakeList.txt : CMake project for BlobHockey, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (BouncingBall "BouncingBall.cpp" "BouncingBall.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET BouncingBall PROPERTY CXX_STANDARD 20)
endif()

find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(BouncingBall PRIVATE SDL3::SDL3)
