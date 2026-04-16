add_executable(test_hit test_hit.cpp)

target_link_libraries(test_hit PRIVATE nmhit)

add_test(NAME hit_tests COMMAND test_hit)
