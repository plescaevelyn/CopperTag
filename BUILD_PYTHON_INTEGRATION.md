# CopperTag Python Integration Build Guide

This guide explains how to build the CopperTag C++ detector for use with the Python fiducial detection system.

## Overview

The CopperTag backend ([common/detectors/coppertag_backend.py](../../common/detectors/coppertag_backend.py)) automatically detects and uses the compiled C++ detector if available, falling back to template matching otherwise.

## Quick Start

```bash
# From project root
cd third_party/CopperTag
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

After building, the detector binary will be at `third_party/CopperTag/bin/coppertag_detector`.

## System Dependencies

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libeigen3-dev \
    libyaml-cpp-dev \
    libopencv-dev
```

### macOS
```bash
brew install cmake eigen yaml-cpp opencv
```

### Windows (WSL2)
Use Ubuntu/Debian instructions above in WSL2 environment.

## Detailed Build Steps

### 1. Install Dependencies

Ensure you have:
- **OpenCV** >= 4.3
- **Yaml-CPP** (any recent version)
- **Eigen3** (any recent version)
- **CMake** >= 3.10
- **C++17 compatible compiler** (GCC 7+, Clang 5+, MSVC 2017+)

### 2. Configure Build

```bash
cd third_party/CopperTag
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
```

**Note**: If CMake cannot find OpenCV, you may need to specify the path:
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DOpenCV_DIR=/path/to/opencv/build ..
```

### 3. Compile

```bash
make -j$(nproc)
```

This creates two executables in `third_party/CopperTag/bin/`:
- `coppertag_detector` - Detection binary (used by Python backend)
- `coppertag_generator` - Marker generation binary

### 4. Verify Installation

Test the detector with the provided test image:
```bash
cd ../bin
./coppertag_detector ../out_123.png
```

Expected output:
```
The CopperTag detection result is: 123
```

## Python Backend Integration

The Python backend automatically detects the compiled binary and uses it:

```python
from common.detectors import coppertag_backend

# Check detector status
info = coppertag_backend.get_detector_info()
print(f"Detector mode: {info['detector_mode']}")  # 'cpp' or 'template_matching'

# Detect markers (automatically uses C++ detector if available)
import cv2
image = cv2.imread("test.jpg")
detections = coppertag_backend.detect(image)

for det in detections:
    print(f"Marker ID: {det['marker_id']}, Confidence: {det['confidence']}")
```

## Detection Modes

### 1. C++ Detector (Preferred)
- **Accuracy**: High - uses official CopperTag algorithm
- **Features**: Reed-Solomon decoding, occlusion resilience
- **Requirements**: Compiled binary at `third_party/CopperTag/bin/coppertag_detector`
- **Performance**: ~10-50ms per image (depends on image size)

### 2. Template Matching (Fallback)
- **Accuracy**: Low - naive edge-based matching
- **Features**: Basic template correlation
- **Requirements**: Template image at `common/detectors/coppertag_template.png`
- **Performance**: ~5-20ms per image

## Troubleshooting

### CMake cannot find OpenCV

**Problem**: `CMake Error: find_package could not find OpenCV`

**Solution 1** - Install OpenCV development package:
```bash
sudo apt-get install libopencv-dev
```

**Solution 2** - Specify OpenCV path manually:
```bash
cmake -DOpenCV_DIR=/usr/local/lib/cmake/opencv4 ..
```

### Missing yaml-cpp

**Problem**: `CMake Error: find_package could not find yaml-cpp`

**Solution**:
```bash
sudo apt-get install libyaml-cpp-dev
```

### Missing Eigen3

**Problem**: `CMake Error: find_package could not find Eigen3`

**Solution**:
```bash
sudo apt-get install libeigen3-dev
```

### C++17 compiler not available

**Problem**: Compilation fails with C++17 errors

**Solution**: Upgrade compiler:
```bash
sudo apt-get install g++-9
export CXX=g++-9
cmake -DCMAKE_CXX_COMPILER=g++-9 ..
```

### Runtime: "params file not found"

**Problem**: Detector runs but cannot load `detectorParams.yaml`

**Solution**: The detector expects to run from `third_party/CopperTag/` directory. The Python backend handles this automatically by setting `cwd` in subprocess call.

## Performance Optimization

### Release Build
Always use Release mode for best performance:
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

### Multi-threaded Build
Use parallel compilation:
```bash
make -j$(nproc)
```

### Link-Time Optimization (LTO)
For maximum performance:
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON ..
make
```

## Advanced Configuration

### Detector Parameters

Edit `params/detectorParams.yaml` to tune detection:

```yaml
# Example detector parameters
min_marker_size: 20      # Minimum marker size in pixels
max_marker_size: 500     # Maximum marker size in pixels
detection_threshold: 0.6 # Detection confidence threshold
```

Refer to CopperTag documentation for full parameter list.

### Custom Output Format

To get structured output (JSON) instead of plain text, modify `example/detector.cpp`:

```cpp
// After decoding, output JSON:
std::cout << "{\"marker_id\": " << result
          << ", \"corners\": [[" << quad.cornerLoc[0].x << "," << quad.cornerLoc[0].y << "]..."
          << "]}" << std::endl;
```

Then rebuild:
```bash
make coppertag_detector
```

## References

- **CopperTag Paper**: [Original research paper](https://arxiv.org/abs/...)
- **Repository**: https://github.com/plescaevelyn/CopperTag
- **Dataset**: [Google Drive](https://drive.google.com/drive/folders/1al6iWGvUWLP2QdQiYIq_z2CAR__72UVJ)

## Contact

For CopperTag C++ library issues, contact:
- Wenzhao Chen (robotmanciu@gmail.com)

For Python integration issues, file an issue in the fiducial_data_collector repository.
