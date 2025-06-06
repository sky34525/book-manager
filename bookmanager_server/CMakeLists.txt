cmake_minimum_required(VERSION 3.27)
#cmake_policy(SET CMP0167 OLD)
project(GateServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_PREFIX_PATH "/opt/homebrew")

# 查找依赖库
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(jsoncpp REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# 添加头文件搜索路径
include_directories(
        ${Boost_INCLUDE_DIRS}
        ${Protobuf_INCLUDE_DIRS}
        ${gRPC_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}  # 项目源代码
)
set(MySQLConnector_ROOT "/Users/kw/cppenv/mysql-connector-c++-9.3.0-macos15-arm64")
include_directories(${MySQLConnector_ROOT}/include)
include_directories(${MySQLConnector_ROOT}/include/mysql)
link_directories(${MySQLConnector_ROOT}/lib64)
# 添加可执行文件
add_executable(GateServer
        main.cpp

        const.h
        CServer.cpp
        CServer.h
        Httpconnection.cpp
        Httpconnection.h
        LogicSystem.cpp
        LogicSystem.h
        Singleton.h
)

# 链接所需库
target_link_libraries(GateServer
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_LIBRARIES}
        ${Protobuf_LIBRARIES}
        gRPC::grpc++
        jsoncpp_lib
        mysqlcppconn
)

set(CMAKE_INSTALL_RPATH "/opt/homebrew/opt/openssl@3/lib")