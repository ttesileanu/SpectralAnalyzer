add_library(interface spectrum.cc)
target_link_libraries(interface display display_helpers input)
target_link_libraries(interface ${PA_LIBRARIES})

# the executable target
add_executable(spectrum main.cc)
target_link_libraries(spectrum animation display display_helpers input interface processor sdl glutils utils)

target_link_libraries(spectrum ${Boost_LIBRARIES})
target_link_libraries(spectrum ${SDL_LIBRARY})
target_link_libraries(spectrum ${OPENGL_LIBRARIES})
target_link_libraries(spectrum ${PA_LIBRARIES})
target_link_libraries(spectrum ${FFTWF_LIBRARIES})
