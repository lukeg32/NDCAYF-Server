#https://www.glfw.org/docs/latest/build_guide.html#build_link_cmake_package

cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(NDCAYF-Server LANGUAGES CXX)

find_package(assimp REQUIRED)
find_package(Bullet REQUIRED)
find_package(OpenAL REQUIRED)


add_executable(NDCAYF-Server
                main.cpp
                util/networking/server.hpp
                util/networking/server.cpp
                util/networking/serverOOP.hpp
                util/networking/serverOOP.cpp
                #util/networking/serverEdit.hpp
                #util/networking/serverEdit.cpp
                #util/object/object.cpp
                #util/object/object.h
                util/json.hpp
                util/bulletDebug/collisiondebugdrawer.cpp
                util/bulletDebug/collisiondebugdrawer.hpp

)

target_include_directories(NDCAYF-Server PUBLIC ${ASSIMP_INCLUDE_DIRS})
target_link_libraries(NDCAYF-Server ${ASSIMP_LIBRARIES})

target_include_directories(NDCAYF-Server PUBLIC ${BULLET_INCLUDE_DIRS})
target_link_libraries(NDCAYF-Server ${BULLET_LIBRARIES})


target_include_directories(NDCAYF-Server PRIVATE
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/util"
                          )


target_include_directories(NDCAYF-Server PUBLIC ${OpenAL_INCLUDE_DIR})
target_link_libraries(NDCAYF-Server ${OPENAL_LIBRARY})
