cmake_minimum_required(VERSION 3.5)
#set project Name
set (PRO_NAME Music)
PROJECT(${PRO_NAME})

#创建构建目录
file(MAKE_DIRECTORY bin lyric photowall Userpix)
#设置编译器
set(CMAKE_CXX_COMPILE "g++")
# ADD_COMPILE_OPTIONS(-Wall -std=c++11 -g )

# 从Qt设计师文件列表中创建代码
set(CMAKE_AUTOUIC ON)
# 设置CMake在需要时自动运行moc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 14)
# 在相应的构建目录中查找头文件
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#设置构建类型，及相应的编译选项
IF(NOT CMAKE_BUILD_TYPE)
#设置为Release模式
#SET(CMAKE_BUILD_TYPE Release)
# Release模式下的编译指令
#SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

#设置为Debug模式
SET(CMAKE_BUILD_TYPE "Debug")
#Debug模式下的编译指令
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
ENDIF()
#执行外部命令,不能在程序启动的时候启动脚本
#execute_process(COMMAND  node ${CMAKE_CURRENT_SOURCE_DIR}/NeteaseCloudMusicApi/app.js  OUTPUT_VARIABLE ls_ret)
#message(STATUS "node app.js:\n ${ls_ret}")

#设置执行文件输出目录
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
message("*********************** begin **************************")
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/ffmpeg/include)
file(GLOB libs_path ${PROJECT_SOURCE_DIR}/ffmpeg/lib/*.lib)

#file(GLOB OpenSSLx86 ${PROJECT_SOURCE_DIR}/OpenSSL/Win_x86/lib/*.lib)
#message(${OpenSSLx86})
#message(${libs_path})
message("*********************** end **************************")

#set Qt PATH  windows下无法调试，解决方案：将qt的的qmake配置成环境变量
set (CMAKE_PREFIX_PATH D:/Qt6/6.2.2/msvc2019_64)

# 搜索当前目录下的源文件
aux_source_directory(. DIR_SRCS)
#搜索目录下的.ui文件
file(GLOB USER_UI_PATH ./*.ui)
# 寻找Qt5的库
find_package(Qt5 COMPONENTS Widgets Multimedia Network REQUIRED)
#添加Qt的资源文件，必须放在find_package之后，不然无法识别qt5_add_resources函数
#set(QRC_SOURCE_FILES resource.qrc)
#qt_add_resources(QRC_FILES ${QRC_SOURCE_FILES})
#添加子目录
add_subdirectory(photowall)
add_subdirectory(RecommendPlaylist)
add_subdirectory(NewMusicSudi)

qt5_add_resources(QRC_SOURCE_FILES resource.qrc)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/OpenSSL/Win_x64/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/OpenSSL/Win_x86/include)
file(GLOB SSL_x64 ${PROJECT_SOURCE_DIR}/OpenSSL/Win_x64/lib/*.lib)
file(GLOB SSL_x86 ${PROJECT_SOURCE_DIR}/OpenSSL/Win_x64/lib/*.lib)
message("x64 = " ${SSL_x64})
message("x86= " ${SSL_x86})
add_executable(${PRO_NAME} ${DIR_SRCS} ${QRC_SOURCE_FILES} ${USER_UI_PATH})
# 链接Qt5的库
target_link_libraries(${PRO_NAME} PRIVATE  Qt5::Multimedia Qt5::Widgets Qt5::Network photowall Recommend_playlist 
NewMusicSudi ${libs_path} 
${SSL_x64} ${SSL_x86}
)
 

#动态库 target_link_libraries(${PRO_NAME}  PRIVATE ${libs_path}   Qt5::Multimedia Qt5::Sql  Qt5::Widgets Qt5::Network )

#设置可执行文件
