cmake_minimum_required(VERSION 3.14)
project(face_detection_plugin)

set(CMAKE_CXX_STANDARD 17)

# --- A. Cấu hình Macro Export ---
if(WIN32)
    set(API_EXPORT_MACRO "__declspec(dllexport)")
else()
    set(API_EXPORT_MACRO "__attribute__((visibility(\"default\")))")
endif()
# --------------------------------
if(MSVC)
    # 1. Enforce Static Runtime (/MT) to avoid server-side dependencies
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    
    # 2. Add /MT compile options for all configurations
    add_compile_options($<$<CONFIG:Release>:/MT>)
    add_compile_options($<$<CONFIG:Debug>:/MTd>)
    
    # 3. Replace default CMake /MD flags with /MT (Crucial step)
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      if(${flag_var} MATCHES "/MD")
        string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif()
    endforeach()
endif()

# 2. Tìm kiếm Nx Metadata SDK
set(metadataSdkDir "" CACHE PATH "Path to unpacked VMS Metadata SDK zip.")
if(metadataSdkDir STREQUAL "")
    get_filename_component(metadataSdkDir "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)
endif()

if(NOT EXISTS ${metadataSdkDir}/src/nx/sdk)
    message(FATAL_ERROR "SDK not found at: ${metadataSdkDir}")
endif()

# 3. Tìm kiếm OpenCV (Sửa đường dẫn của bạn nếu cần)
set(OpenCV_DIR "F:/Workspaces/LightJSC/NxPlugin/CustomPlugins/opencv/build")
find_package(OpenCV REQUIRED)

# 4. Build các thư viện phụ trợ của Nx
add_subdirectory(${metadataSdkDir}/nx_kit ${CMAKE_CURRENT_BINARY_DIR}/nx_kit)

file(GLOB_RECURSE SDK_SRC ${metadataSdkDir}/src/*)
add_library(nx_sdk STATIC ${SDK_SRC})
target_include_directories(nx_sdk PUBLIC ${metadataSdkDir}/src)
target_link_libraries(nx_sdk PRIVATE nx_kit)
target_compile_definitions(nx_sdk PRIVATE NX_PLUGIN_API=${API_EXPORT_MACRO})

# 5. Build Plugin face_detection_plugin
file(GLOB_RECURSE MY_PLUGIN_SRC "src/*.cpp" "src/*.h")

add_library(face_detection_plugin SHARED ${MY_PLUGIN_SRC})

target_include_directories(face_detection_plugin PRIVATE
    src
    ${metadataSdkDir}/src
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(face_detection_plugin PRIVATE
    nx_kit
    nx_sdk
    ${OpenCV_LIBS}
)

# --- B. QUAN TRỌNG: Thêm dòng này để sửa lỗi C1189 cho Plugin ---
target_compile_definitions(face_detection_plugin PRIVATE NX_PLUGIN_API=${API_EXPORT_MACRO})
# ---------------------------------------------------------------