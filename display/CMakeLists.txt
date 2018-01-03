add_library(display oscilloscope.cc spectral_envelope.cc spectrogram.cc)
add_library(display_helpers axes.cc)
target_link_libraries(display animation glutils)
target_link_libraries(display_helpers animation glutils)
