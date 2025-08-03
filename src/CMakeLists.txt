file(GLOB_RECURSE SOURCES_CXX LIST_DIRECTORIES true *.hpp *.cpp)
list(REMOVE_ITEM SOURCES_CXX ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
file(GLOB_RECURSE SOURCES_C LIST_DIRECTORIES true *.h *.c)
set(SOURCES ${SOURCES_CXX} ${SOURCES_C})
add_executable(${PROJECT_NAME}_run main.cpp)
add_library(${PROJECT_NAME}_lib STATIC ${SOURCES})
# find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${PROJECT_NAME}_run PRIVATE
    ${PROJECT_NAME}_lib
    # Boost::boost
    # Boost::system
)
function(add_common_flags target)
    target_compile_options(${target} PRIVATE
        "-Wall"
        "-Werror"
        "-Wextra"
        "-pedantic"
        "-march=native"
        "-fdiagnostics-color=always"
        "$<$<CONFIG:Release>:-O2;-DNDEBUG>"
        "$<$<CONFIG:Debug>:-O0;-g;-fsanitize=address>"
        # "$<$<CONFIG:Coverage>:-O0;-g;-fsanitize=address;-fprofile-instr-generate;-fcoverage-mapping;-DDASH_DEBUG>"
    )
    target_link_options(${target} PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address>
        # $<$<CONFIG:Coverage>:-fsanitize=address;-fprofile-instr-generate;-fcoverage-mapping>
    )
endfunction()
add_common_flags(${PROJECT_NAME}_run)
add_common_flags(${PROJECT_NAME}_lib)
