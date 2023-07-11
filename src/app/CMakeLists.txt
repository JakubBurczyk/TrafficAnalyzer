set(NAME "TrafficAnalyzer")

include(${PROJECT_SOURCE_DIR}/cmake/StaticAnalyzers.cmake)

file(COPY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include_directories(include)

set(MAIN_FILE src/main.cpp)

add_executable(${NAME} ${MAIN_FILE})

if (WIN32)
  # Copy DLL files on Windows to the target App build folder.
  add_custom_command(TARGET ${NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL2::SDL2>
    $<TARGET_FILE_DIR:${NAME}>)
endif ()

target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(${NAME} PRIVATE cxx_std_20)
target_link_libraries(${NAME} ${OpenCV_LIBS} project_warnings Core)
