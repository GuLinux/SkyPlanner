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
    .gm-style img {
      max-width: none;
    }
    .link {
      cursor: pointer;
    }
    .object-difficulty-box {
      font-family: "monospace";
      border-style:solid;
      border-width:5px;
    }
    .modal-body {
      max-height: inherit!important;
    }
    .form-inline * {
      margin-right: 5px;
    }
    .btn-group button {
      margin-right: 0px;
    }
  );
  return std::vector<uint8_t>(css.begin(), css.end());
}
#endif
