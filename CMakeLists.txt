cmake_minimum_required(VERSION 3.0.0)

project(oci_test_switch_session VERSION 0.1.0)

set(SOURCE_EXE switching.c oci_common.c)

if(WIN32)
    add_library(oci SHARED IMPORTED)
    set_property(TARGET oci PROPERTY IMPORTED_LOCATION "$ENV{ORACLE_HOME}/oci.dll")
    set_property(TARGET oci PROPERTY IMPORTED_IMPLIB "$ENV{ORACLE_HOME}/sdk/lib/msvc/oci.lib")
else()
    add_library(oci SHARED IMPORTED)
    set_property(TARGET oci PROPERTY IMPORTED_LOCATION "$ENV{ORACLE_HOME}/libclntsh.so")
endif ()

add_executable(${PROJECT_NAME} ${SOURCE_EXE})

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE 
    $ENV{ORACLE_HOME}/sdk/include
)

target_link_libraries(${PROJECT_NAME} oci)