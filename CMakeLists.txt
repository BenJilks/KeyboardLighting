cmake_minimum_required(VERSION 3.5)
project(KeyboardLighting)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

add_compile_definitions(libusb)
find_library(LIBUSB_LIBRARY
    NAMES usb-1.0
    PATH_SUFFIXES "lib" "lib32" "lib64"
)

set(COMMON_SOURCES
    src/keyboard.cpp src/keyboard.h
    src/keyboard_layout.h
    src/stb_image.h
)

add_executable(text src/keyboard_text.cpp ${COMMON_SOURCES})
target_link_libraries(text ${LIBUSB_LIBRARY})

add_executable(image src/keyboard_image.cpp ${COMMON_SOURCES})
target_link_libraries(image ${LIBUSB_LIBRARY})

add_executable(mania src/keyboard_mania.cpp ${COMMON_SOURCES})
target_link_libraries(mania ${LIBUSB_LIBRARY})
