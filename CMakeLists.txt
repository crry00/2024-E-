

cmake_minimum_required(VERSION 3.16)

project(Myproject)

find_package(OpenCV REQUIRED)

add_executable(main hsvcolor.cpp)
target_link_libraries(
    main
    PRIVATE ${OpenCV_LIBS}
)



    