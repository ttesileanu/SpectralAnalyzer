# the executable target 1
add_executable(animator_tests animator_tests.cc)
target_link_libraries(animator_tests animation sdl utils glutils)

target_link_libraries(animator_tests ${Boost_LIBRARIES})
target_link_libraries(animator_tests ${SDL_LIBRARY})
target_link_libraries(animator_tests ${OPENGL_LIBRARIES})

# the executable target 2
add_executable(axes_tests axes_tests.cc)
target_link_libraries(axes_tests animation sdl utils display_helpers glutils)

target_link_libraries(axes_tests ${Boost_LIBRARIES})
target_link_libraries(axes_tests ${SDL_LIBRARY})
target_link_libraries(axes_tests ${OPENGL_LIBRARIES})
