#ifndef STYLE_CSS_H
#define STYLE_CSS_H
#include <stdint.h>
#include <string>
#include <vector>
#define CSS(...) #__VA_ARGS__

std::vector<uint8_t> styleCss() {
  std::string css = CSS(
    body {
      padding-top: 70px;
    }
    .skyplanner-notifications {
      position: fixed;
      width: 100%;
      top: 55px;
      z-index: 999;
    }
    .dialog-popup-submenu {
      z-index: 1100!important;
    }
    .astroobjectwidget,print-no-break {
      page-break-inside:avoid;
    }
    .dropdown-menu li {
      cursor: pointer;
    }

    @media print {
      .astroobject_text,.astroobjects-info-widget {
        font-size: x-small!important;
      }
      .astroobject_title {
        font-size: 15px!important;
      }
    }
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
    .form-slider {
      vertical-align: middle;
    }
    .panel-compact {
      margin-bottom: 0px;
    }

    .checkbox-no-form-control {
      border: 0px;
      box-shadow: inherit;
      -webkit-box-shadow: inherit;
    }
  );
  return std::vector<uint8_t>(css.begin(), css.end());
}
#endif
