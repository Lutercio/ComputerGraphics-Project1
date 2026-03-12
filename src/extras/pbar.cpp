#include <iostream>

#include "pbar.hpp"

// float ProgressBar::progress=0.f;
size_t ProgressBar::bar_width = 50;  //!< Size of progress bar in screen character.

void ProgressBar::set_size(size_t sz) { bar_width = sz; }

void ProgressBar::update(float progress) {
  if (progress < 0.F or progress > 1.F) {
    return;
  }
  if (progress <= 1.0) {
    std::cout << "    [";
    size_t pos = bar_width * progress;
    for (auto i(0U); i < bar_width; ++i) {
      if (i < pos) {
        std::cout << "=";
      } else if (i == pos) {
        std::cout << ">";
      } else {
        std::cout << " ";
      }
    }
    std::cout << "] " << int((progress + 0.01) * 100.0) << " %\r";
    std::cout.flush();
  }
}
