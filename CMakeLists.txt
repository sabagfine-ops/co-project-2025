cmake_minimum_required(VERSION 3.26)

project(asm VERSION 1.0.0 LANGUAGES C)

# Add warnings dispatched on MSVC and LINUX
if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall)
endif()

add_executable(asm
  asm_functions.c
  main.c
)

