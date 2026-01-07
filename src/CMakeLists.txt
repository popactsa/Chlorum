file(GLOB_RECURSE SOURCES_C LIST_DIRECTORIES true *.c)
list(REMOVE_ITEM SOURCES_C ${CMAKE_CURRENT_SOURCE_DIR}/main.c)
list(LENGTH SOURCES_C SOURCES_C_SZ)
message(STATUS "Found source files: ${SOURCES_C}")
function(add_common_flags target)
    target_compile_options(${target} PRIVATE
        # "-Wall"
        # "-Werror"
        # "-Wextra"
        # "-pedantic"
        # "-march=native"
        # "-save-temps"
        # "-fverbose-asm"
        # "-masm=intel"
        # "-fdiagnostics-color=always"
        "$<$<CONFIG:Release>:-O2;-DNDEBUG>"
        "$<$<CONFIG:Debug>:-O0;-g;-fsanitize=address>"
        # "$<$<CONFIG:Coverage>:-O0;-g;-fsanitize=address;-fprofile-instr-generate;-fcoverage-mapping;-DDASH_DEBUG>"
    )
    target_link_options(${target} PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address>
        # $<$<CONFIG:Coverage>:-fsanitize=address;-fprofile-instr-generate;-fcoverage-mapping>
    )
endfunction()

add_executable(${PROJECT_NAME}_run main.c)
add_common_flags(${PROJECT_NAME}_run)

if (${SOURCES_C_SZ} GREATER 0)
    add_library(${PROJECT_NAME}_lib STATIC ${SOURCES_C})
    add_common_flags(${PROJECT_NAME}_lib)
    target_link_libraries(${PROJECT_NAME}_run PRIVATE
        ${PROJECT_NAME}_lib
    )
else()
    message(WARNING "No library compiled!")
endif()

