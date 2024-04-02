function(enable_std_modules target)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${target} PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,Clang>:-fmodules$<SEMICOLON>-fprebuilt-module-path=${CMAKE_BINARY_DIR}/clang_std_modules/$<SEMICOLON>-stdlib=libc++>)
        target_link_options(${target} PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,Clang>:-stdlib=libc++>)
        if (NOT TARGET clang_std_modules)
            add_custom_target(clang_std_modules
        ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/clang_std_modules
        COMMAND ${CMAKE_CXX_COMPILER} -x c++ -Xclang -emit-module -std=c++20 -stdlib=libc++ -O3 -DNDEBUG ${CMAKE_CXX_FLAGS} -c std.modulemap -o ${CMAKE_BINARY_DIR}/clang_std_modules/std.pcm -fmodules -fmodule-name=std
        BYPRODUCTS ${CMAKE_BINARY_DIR}/clang_std_modules/std.pcm
        WORKING_DIRECTORY ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/clang_std_modules_source
            )
        endif()
        add_dependencies(${target} clang_std_modules)
    endif()
endfunction()
