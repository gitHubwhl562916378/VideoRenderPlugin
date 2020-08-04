cmake_minimum_required(VERSION 3.9)

Project(Nv12Render_Gpu VERSION 1.0
        LANGUAGES CXX CUDA)
		
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)       #设置可执行文件的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)           #设置库文件的输出目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

find_package(CUDA REQUIRED)
include_directories(
    ${CUDA_INCLUDE_DIRS}
)

set(Qt5_DIR E:/Qt/Qt5.12.4/5.12.4/msvc2017_64/lib/cmake/Qt5)
find_package(Qt5 REQUIRED Core Gui)
include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
)
set(CMAKE_AUTOMOC ON) #自动生成moc文件
set(CMAKE_AUTORCC ON) #自动处理rcc文件
set(CMAKE_AUTOUIC ON) #自动处理uic
#查找当前文件夹中的所有源代码文件，也可以通过Set命令将所有文件设置为一个变量
FILE(GLOB SRC_FILES "./*.cpp")
#同理，查找当前文件夹中所有的头文件
FILE(GLOB HEAD_FILES "./*.h") 
#同理，查找当前文件夹中所有的ui文件
FILE(GLOB UI_FILES "./*.ui")
#通过Ui文件生成对应的头文件，一定要添加
# qt5_wrap_ui(WRAP_FILES ${UI_FILES})

aux_source_directory(. SRCS)
add_library(${PROJECT_NAME} SHARED ${SRCS})
target_link_libraries(${PROJECT_NAME} ${CUDA_CUDA_LIBRARY})
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui)