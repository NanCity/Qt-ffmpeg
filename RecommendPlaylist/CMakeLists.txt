cmake_minimum_required(VERSION 3.5)
# 从Qt设计师文件列表中创建代码
set(CMAKE_AUTOUIC ON)
# 设置CMake在需要时自动运行moc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 14)
# 在相应的构建目录中查找头文件
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set Qt PATH  windows下无法调试，解决方案：将qt的的qmake配置成环境变量
#set (CMAKE_PREFIX_PATH "D:/Qt6/6.2.2/msvc2019_64")
set (CMAKE_PREFIX_PATH "D:/Qt5.14.2/5.14.2/msvc2017_64")
find_package(Qt5 COMPONENTS Widgets Network REQUIRED)
#添加根目录文件
include_directories("../")

include_directories(
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5Network_INCLUDE_DIRS}
)
# 搜索当前目录下的源文件
aux_source_directory(. DIR_LIB_SRCS)
#搜索目录下的.ui文件
file(GLOB USER_UI_PATH *.ui)
message(${DIR_LIB_SRCS})

add_library(Recommend_playlist ${DIR_LIB_SRCS} ${USER_UI_PATH}	)
target_link_libraries(Recommend_playlist Qt5::Widgets Qt5::Network )
