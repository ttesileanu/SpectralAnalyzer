add_library(input fake_input.cc pa_input.cc)
target_link_libraries(input ${PA_LIBRARIES})
