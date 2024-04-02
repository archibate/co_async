function(enable_std_modules target)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if (ON)
            target_compile_options(${target} PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,Clang>:-fmodules$<SEMICOLON>-fbuiltin-module-map$<SEMICOLON>-fimplicit-module-maps$<SEMICOLON>-stdlib=libc++>)
            target_link_options(${target} PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,Clang>:-stdlib=libc++>)
        elseif (OFF)
            set(module_map ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/clang_std_modules_source/std.modulemap)
            target_compile_options(${target} PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,Clang>:-fmodules$<SEMICOLON>-fprebuilt-module-path=${CMAKE_BINARY_DIR}/clang_std_modules/$<SEMICOLON>-fbuiltin-module-map$<SEMICOLON>-fno-implicit-module-maps$<SEMICOLON>-fmodule-map-file=${module_map}$<SEMICOLON>-stdlib=libc++>)
            target_link_options(${target} PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,Clang>:-stdlib=libc++>)
            if (NOT TARGET clang_std_modules)
                add_custom_command(
                OUTPUT ${CMAKE_BINARY_DIR}/clang_std_modules/std.pcm
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/clang_std_modules
                COMMAND ${CMAKE_CXX_COMPILER} -x c++ -Xclang -emit-module -std=c++20 -stdlib=libc++ -O3 -DNDEBUG ${CMAKE_CXX_FLAGS} -c ${module_map} -o ${CMAKE_BINARY_DIR}/clang_std_modules/std.pcm -fmodules -fmodule-name=std -fno-implicit-module-maps
                MAIN_DEPENDENCY ${module_map}
                DEPENDS ${CMAKE_CURRENT_FUNCTION_LIST_FILE}
                WORKING_DIRECTORY ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/clang_std_modules_source
                COMMENT "Building Clang std modules"
                VERBATIM)
                add_custom_target(clang_std_modules DEPENDS ${CMAKE_BINARY_DIR}/clang_std_modules/std.pcm)
            endif()
            add_dependencies(${target} clang_std_modules)
        else()
            target_compile_options(${target} PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,Clang>:-fmodules$<SEMICOLON>-fprebuilt-module-path=${CMAKE_SOURCE_DIR}/std/$<SEMICOLON>-fbuiltin-module-map$<SEMICOLON>-fno-implicit-module-maps$<SEMICOLON>-stdlib=libc++>)
file(READ ${CMAKE_SOURCE_DIR}/std/std.modmap modmap)
            string(REPLACE "\n" " " modmap ${modmap})
            string(REPLACE "-fmodule-file=std/" "-fmodule-file=${CMAKE_SOURCE_DIR}/std/" modmap ${modmap})
            target_compile_options(${target} PRIVATE ${modmap})
            target_link_options(${target} PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,Clang>:-stdlib=libc++>)
        endif()
    endif()
endfunction()
