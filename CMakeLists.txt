cmake_minimum_required(VERSION 3.0.0)
project(Test VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(main main.cpp connectionPool.cpp connection.cpp)
target_link_libraries(main pthread mysqlclient)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)