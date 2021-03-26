#https://www.glfw.org/docs/latest/build_guide.html#build_link_cmake_package

cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

project(NDCAYF-Server LANGUAGES CXX)

find_package(assimp REQUIRED)
find_package(Bullet REQUIRED)
find_package(OpenAL REQUIRED)


add_executable(NDCAYF-Server
                main.cpp
                util/networking/server.cpp
                util/networking/server.hpp
                #util/networking/serverOOP.cpp
                #util/networking/serverOOP.hpp

                util/networking/TCP/TCPListener.hpp
                util/networking/TCP/TCPListener.cpp
                util/networking/TCP/TCP.hpp
                util/networking/TCP/TCP.cpp

                util/networking/TCP/Listeners.hpp
                util/networking/TCP/Listeners.cpp

                util/networking/TCP/FileGet.hpp
                util/networking/TCP/FileGet.cpp

                util/networking/TCP/FileUpload.hpp
                util/networking/TCP/FileUpload.cpp

                util/networking/TCP/MusicStreamer.hpp
                util/networking/TCP/MusicStreamer.cpp
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
