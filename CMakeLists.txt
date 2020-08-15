cmake_minimum_required (VERSION 2.6)
project (good_robot)

IF(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
list(APPEND CMAKE_PREFIX_PATH "${good_robot_SOURCE_DIR}/sdl")
list(APPEND CMAKE_PREFIX_PATH "${good_robot_SOURCE_DIR}/glew")
list(APPEND CMAKE_PREFIX_PATH "${good_robot_SOURCE_DIR}/freetype")
list(APPEND CMAKE_PREFIX_PATH "${good_robot_SOURCE_DIR}/openal")
list(APPEND CMAKE_PREFIX_PATH "${good_robot_SOURCE_DIR}/openil")
set(Boost_USE_STATIC_LIBS ON)
ELSEIF(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
# Suppress warning of c_str/c++String mapping & deprecated functions
add_definitions(-Wno-write-strings -Wno-deprecated-declarations)
# Add debugging symbols to build
add_definitions(-g)
# Uncomment LINKER line for linker debugging
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -v")
# Specify that we are using the C++11 standard for GCC/Clang
list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
ENDIF()

file(GLOB good_robot_SRC
	"*.cpp"
)

add_executable(good_robot WIN32 ${good_robot_SRC})

add_definitions(-DCMAKE_BUILD)

set(SDL_BUILDING_LIBRARY ON)
# use pkg-config to find SDL2
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
target_link_libraries (good_robot ${SDL2_LIBRARIES})
include_directories (${SDL2_INCLUDE_DIRS})

set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
target_link_libraries (good_robot ${OPENGL_LIBRARIES})
include_directories (${OPENGL_INCLUDE_DIR})

find_package(Boost REQUIRED COMPONENTS system filesystem)
target_link_libraries (good_robot ${Boost_LIBRARIES})
include_directories (${Boost_INCLUDE_DIRS})

find_package(GLEW REQUIRED)
target_link_libraries (good_robot ${GLEW_LIBRARIES})
include_directories (${GLEW_INCLUDE_DIRS})

find_package(Freetype REQUIRED)
target_link_libraries (good_robot ${FREETYPE_LIBRARIES})
include_directories (${FREETYPE_INCLUDE_DIRS})
include_directories (${FREETYPE_INCLUDE_DIR_ft2build})

find_package(OpenAL REQUIRED)
target_link_libraries (good_robot ${OPENAL_LIBRARY})
include_directories (${OPENAL_INCLUDE_DIR})

find_package(DevIL) #Ignore error for ILUT_LIBRARIES
target_link_libraries (good_robot ${IL_LIBRARIES})
include_directories (${IL_INCLUDE_DIR})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/openal")
find_package(ALUT REQUIRED)
target_link_libraries (good_robot ${ALUT_LIBRARIES})
include_directories (${ALUT_INCLUDE_DIRS})

set(STEAMWORKS_SDK "${CMAKE_CURRENT_SOURCE_DIR}/steamworks/")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/steamworks")
find_package(STEAMWORKS REQUIRED)
target_link_libraries (good_robot ${STEAMWORKS_LIBRARY})
include_directories (${STEAMWORKS_INCLUDE_DIR})

include_directories ("${good_robot_SOURCE_DIR}/rapidxml")
