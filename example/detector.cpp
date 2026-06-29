/**************************************************************************
 * Copyright 2023 Youibot Robotics Co., Ltd. All Rights Reserved.
 * Contact: wenzhaochen (robotmanciu@gmail.com)
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *************************************************************************/

#include "coppertag/tagDetector.h"
#include "coppertag/tagRule.h"
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>

// usage: ./coppertag your_image.jpg

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "you need to specific the input image (name) / path" << std::endl;
    return -1;
  }

  /************ load the detector parameters ************/
  // Debug: print current working directory
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    std::cout << "Current working directory: " << cwd << std::endl;
  }

  // Debug: check if params file exists
  const char* params_path = "params/detectorParams.yaml";
  struct stat buffer;
  if (stat(params_path, &buffer) == 0) {
    std::cout << "Params file exists at: " << params_path << std::endl;
  } else {
    std::cerr << "ERROR: Params file NOT FOUND at: " << params_path << std::endl;
    std::cerr << "Please run detector from CopperTag root directory!" << std::endl;
    return -1;
  }

  if (!PARAMS->load_from_yaml(params_path)) {
    std::cerr << "Failed to load detector parameters. Exiting." << std::endl;
    return -1;
  }
  std::cout << "Successfully loaded detector parameters." << std::endl;

  /************ read the image from local path ************/
  cv::Mat imgInput = imread(argv[1], cv::IMREAD_GRAYSCALE);
  if (imgInput.empty()) {
    std::cerr << "ERROR: Failed to load image: " << argv[1] << std::endl;
    return -1;
  }
  std::cout << "Image loaded: " << imgInput.cols << "x" << imgInput.rows << " pixels" << std::endl;

  int rows = imgInput.rows / 2;
  int cols = imgInput.cols / 2;
  std::cout << "Detector initialized with: " << cols << "x" << rows << " (half resolution)" << std::endl;

  /************ initialize the detector / decoder ************/
  CopperTag::Detector detectorObj(rows, cols);
  // IMPORTANT: Generator uses TAG_SIZE/2 (256) to create TAG_SIZE x TAG_SIZE (512x512) image
  // Decoder must use the same coordinate space, not the image size
  CopperTag::CopperTagRule decoderObj(TAG_SIZE/2);

  /************ start detect ************/
  std::vector<CopperTag::CopperQuad> quads;
  detectorObj.detect(imgInput, quads);
  std::cout << "Detected " << quads.size() << " candidate quads" << std::endl;

  /************ start decode ************/
  int decoded_count = 0;
  for (size_t i = 0; i < quads.size(); i++) {
    const auto& quad = quads[i];
    std::cout << "Processing quad " << (i+1) << "/" << quads.size() << std::endl;

    int result;
    std::vector<bool> allBits;
    if (decoderObj.sample_bits(imgInput, quad.cornerLoc, allBits)) {
      std::cout << "  Successfully sampled " << allBits.size() << " bits" << std::endl;
      if (decoderObj.decode(allBits, result)) {
        std::cout << "The CopperTag detection result is: " << result << std::endl;
        decoded_count++;
      } else {
        std::cout << "  Failed to decode quad (Reed-Solomon decoding failed)" << std::endl;
        // Print all bits for debugging (in hex for compactness)
        std::cout << "  All " << allBits.size() << " bits (hex): ";
        for (size_t j = 0; j < allBits.size(); j += 8) {
          unsigned char byte = 0;
          for (size_t k = 0; k < 8 && (j + k) < allBits.size(); k++) {
            if (allBits[j + k]) {
              byte |= (1 << (7 - k));
            }
          }
          printf("%02X", byte);
        }
        std::cout << std::endl;
      }
    } else {
      std::cout << "  Failed to sample bits from quad" << std::endl;
    }
  }

  if (decoded_count == 0 && quads.size() > 0) {
    std::cout << "Found " << quads.size() << " quads but failed to decode any" << std::endl;
  } else if (quads.size() == 0) {
    std::cout << "No candidate quads detected in image" << std::endl;
  }

  return 1;
}