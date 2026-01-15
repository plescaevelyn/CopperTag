#include <iostream>
#include <string>

#include "coppertag/tagRule.h"

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "Usage: coppertag_generator <marker_id> <out_path>\n";
    return 2;
  }

  const int marker_id = std::stoi(argv[1]);
  const std::string out_path = argv[2];

  // IMPORTANT:
  // CopperTagRule(double size) builds a tag image of (size*2) x (size*2).
  // Use 256 -> output 512x512 which matches TAG_SIZE style.
  CopperTag::CopperTagRule rule(256.0);

  const bool ok = rule.generate(marker_id, out_path);
  return ok ? 0 : 1;
}
