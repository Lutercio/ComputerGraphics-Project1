#ifndef PBAR_H
#define PBAR_H

#include <iostream>

class ProgressBar {
private:
  static size_t bar_width;  //!< Size of progress bar in screen character.

public:
  static void set_size(size_t sz_ = 50);
  static void update(float);
};

#endif /* COMMON_H */
