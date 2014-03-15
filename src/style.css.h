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
    .navbar {
      margin-bottom: 0px;
    }
    .blog-link-cell {
      display: inline;
      right: 0px;
      bottom: 0px;
      position: fixed;
      margin-bottom: 0px;
    }

    .image-inverse {
      -webkit-filter: invert(100%);
    }
    .nav-tabs .close {
      display: none;
    }
  );
  return std::vector<uint8_t>(css.begin(), css.end());
}
#endif
