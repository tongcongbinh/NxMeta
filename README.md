## Face Detection Plugin (Windows)

A face analytics solution for **Nx Meta 6.0** that provides Nx-compliant metadata, including object class, normalized bounding boxes, and stable track IDs. The architecture builds on the Metadata SDK and uses OpenCV DNN for inference with the `model.onnx` network. It targets deployments that need real-time overlays on the Nx Client or server-side event processing.

### Environment Requirements
| Component | Version | Download Link |
|---|---|---|
| Nx Meta Client & Server | `6.0.6.41837` | https://meta.nxvms.com/download/releases/windows |
| Metadata SDK | `6.0.6.41837` | https://meta.nxvms.com/download/releases/sdk |
| Visual Studio | `2022/2026` | https://visualstudio.microsoft.com/downloads/ |
| CMake | `4.2.0` | https://cmake.org/download/ |
| OpenCV (Windows) | `4.12.0` | https://opencv.org/releases/ |

### High-Level Architecture
- `plugin.*`: registers the plugin with Nx Server, declares the taxonomy (typeLibrary), and returns the engine.
- `engine.*`: manages the lifecycle of `DeviceAgent`, requests YUV420 streams.
- `device_agent.*`: consumes decoded frames, converts to BGR, calls `FaceDetector`, and emits metadata with track IDs.
- `FaceDetector.*`: loads the ONNX model, performs preprocessing, inference, and NMS-based postprocessing.
- `model.onnx`: YOLO-based face detection model.

### Windows Build Notes
Using the static runtime (/MT) keeps the DLL self-contained on the server. Add the block below to `CMakeLists.txt`:
```
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
```

### Project Layout
```
face_detection_plugin/
├─ src/
│  ├─ device_agent.cpp/.h
│  ├─ engine.cpp/.h
│  ├─ FaceDetector.cpp/.h
│  ├─ plugin.cpp/.h
├─ CMakeLists.txt
└─ model.onnx
```

### Build Steps
1. Launch the **x64 Native Tools Command Prompt** (VS 2022/2026).  
2. Change directory to the plugin root.

Generate the build system:
```
cmake -S <PluginFolder> -B <BuildFolder> -A x64 -DmetadataSdkDir=<SdkFolder>
```

If the plugin depends on OpenCV (e.g., this Face Detection module):
```
cmake -S <PluginFolder> -B <BuildFolder> -A x64 ^
  -DOpenCV_DIR="...\opencv\build" ^
  -DmetadataSdkDir=<SdkFolder>
```

Build:
```
cmake --build <BuildFolder> --config Release
```

Clean before reconfiguring:
```
rmdir /s /q <BuildFolder>
```

Sample workflow for this plugin:
```
cmake -S samples/sample_analytics_plugin -B build_static_fix -A x64 -DmetadataSdkDir=sdk/metadata_sdk
```
```
cmake --build build_static_fix --config Release
```

### Deployment on Nx Server
After a Release build, copy the resulting files into the media server’s plugin directory; keeping them in a dedicated subfolder helps manage dependencies:
```
C:\Program Files\Network Optix\Nx Meta\MediaServer\plugins\
```
Required files:
- `face_detection_plugin.dll`
- `model.onnx`
- `opencv_world4120.dll` (matching the Release/Debug configuration)

Activation procedure:
1. Stop and restart the Nx Media Server service to reload manifests.
2. In Nx Client, open **Camera Settings → Plugins** and enable **Face Detection Plugin** for the desired cameras.
3. Ensure the Nx Client overlay (“Display Objects”) is enabled so bounding boxes are visible.