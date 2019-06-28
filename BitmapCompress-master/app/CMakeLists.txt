cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v")
set(CPP_ROOT ${CMAKE_CURRENT_LIST_DIR}/src/main/cpp)
include_directories(./libs/jpeg)
link_directories(./libs/${ANDROID_ABI})

find_library(log-lib
             log)
find_library(android-lib
             android)
find_library(bitmap-lib
             jnigraphics)


add_library(jpeg STATIC IMPORTED)
set_target_properties(jpeg
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libjpeg.a)
add_library( # Sets the name of the library.
            myjpeg

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/native-lib.cpp )

target_link_libraries( myjpeg
                       ${log-lib}
                       ${android-lib}
                       ${bitmap-lib}
                        jpeg )