#ifndef STYLE_CSS_H
#define STYLE_CSS_H
#include <stdint.h>
#include <string>
#include <vector>
#define CSS(...) #__VA_ARGS__

std::vector<uint8_t> styleCss() {
  std::string css = CSS(
    body {
    };
    legend {
    }
  );
  return std::vector<uint8_t>(css.begin(), css.end());
}
#endif