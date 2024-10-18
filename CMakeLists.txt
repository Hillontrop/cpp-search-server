# Минимальная версия CMake
cmake_minimum_required(VERSION 3.11)

# Название проекта
project(SearchServer LANGUAGES CXX)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Указываем все исходные файлы проекта
set(SOURCES_CPP
    search_server/src/document.cpp
    search_server/src/main.cpp
    search_server/src/process_queries.cpp
    search_server/src/read_input_functions.cpp
    search_server/src/request_queue.cpp
    search_server/src/search_server.cpp
    search_server/src/string_processing.cpp
)

# Создаём исполняемый файл
add_executable(searchServer ${SOURCES_CPP})

# Указываем путь к заголовочным файлам для searchServer
target_include_directories(searchServer PRIVATE 
    ${CMAKE_SOURCE_DIR}/search_server/include
)
