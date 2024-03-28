cmake_minimum_required(VERSION 3.27)
project(Test_Task)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt5 COMPONENTS
        Core
        REQUIRED)

add_executable(Test_Task main.cpp)
target_link_libraries(Test_Task
        Qt5::Core
)

