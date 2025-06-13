file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.hpp *.cpp)
set(SOURCES ${SOURCES})
add_executable(${PROJECT_NAME}_run ${SOURCES})
add_library(${PROJECT_NAME}_lib STATIC ${SOURCES})
function(add_common_flags target)
    target_compile_options(${target} PRIVATE
        "-Wall"
        "-Werror"
        "-Wextra"
        "-pedantic"
        "-march=native"
        "-fdiagnostics-color=always"
        "$<$<CONFIG:Release>:-O2>"
        "$<$<CONFIG:Debug>:-O0;-g;-fsanitize=address>"
    )
    target_link_options(${target} PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address>
    )
endfunction()
add_common_flags(${PROJECT_NAME}_run)
add_common_flags(${PROJECT_NAME}_lib)
