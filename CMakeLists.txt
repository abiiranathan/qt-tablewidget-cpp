cmake_minimum_required(VERSION 3.2)

project(tableWidget LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(TEST_BINARY ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core PrintSupport)

add_library(tableWidget STATIC
  tableWidget_global.h
  tableWidget.h
  delegates.h
)

target_include_directories(tableWidget
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)


target_link_libraries(tableWidget PUBLIC Qt6::Widgets Qt6::Core Qt6::PrintSupport)
target_compile_definitions(tableWidget PRIVATE TABLEWIDGET_LIBRARY)

# Generate the export file
install(TARGETS tableWidget
  EXPORT tableWidget
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# Export the project
install(EXPORT tableWidget
  FILE tableWidget.cmake
  NAMESPACE tableWidget::
  DESTINATION lib/cmake/tableWidget
)

# Install the header files to the installation directory
install(FILES tableWidget.h delegates.h tableWidget_global.h DESTINATION include)

# Install config file
install(FILES tableWidget-config.cmake DESTINATION lib/cmake/tableWidget)


# Check if compiled with TEST_BINARY flag
# -DTEST_BINARY=ON

if(TEST_BINARY)
    # Create executable target for testing
    add_executable(application_test
        mainwindow.h mainwindow.cpp mainwindow.ui main.cpp
    )

    target_link_libraries(application_test PRIVATE tableWidget)
    target_compile_definitions(application_test PRIVATE TABLEWIDGET_LIBRARY)
endif()

