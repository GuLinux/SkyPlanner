 #ifndef STYLE_CSS_H
#define STYLE_CSS_H
#include <stdint.h>
#include <string>
#include <vector>
#define CSS(...) #__VA_ARGS__

std::vector<uint8_t> styleCss() {
  std::string css = CSS(

    .skyplanner-notifications {
      position: fixed;
      width: 100%;
      top: 55px;
      z-index: 999;
    }
    .bold {
      font-weight: bold;
    }
    .menu-item-highlight span {
      color: #eeeeee;
    }
    .menu-item-highlight .dropdown-menu span {
      color: #616161;
    }
    .dialog-popup-submenu {
      z-index: 1100!important;
    }

    .navbar-toggle-fix {
      display:inherit!important;
    }
    .astroobjectwidget,print-no-break {
      page-break-inside:avoid;
    }
    .dropdown-menu li, .nav li {
      cursor: pointer;
    }
    .navbar .navbar-collapsed {
      display: none!important;
    }
    .pagination-item a {
//      min-width: 3em;
    }
    @media (min-width: 768px) {
      .navbar .navbar-collapsed {
        display: block!important;
      }
   }
   
   .resize-none {
     resize: none;
   }

   @media print {
     .planets-table {
       font-size: 0.72em;
     }
     .panel-body {
       padding: 0px;
     }
     .panel {
       margin: 0px!important;
       border: 0px;
     }
   }

   .astroobject-list-item {
//     margin-top: 5px;
     padding: 5px;
     border: 1px;
     border-color: grey;
     border-style: solid;
     margin-bottom: -1px;
   }  

   .astroobjects-table span{
     font-size: 0.9em;
   }

   .astroobject-last-list-item {
     margin-bottom: 0px;
   }

   .astroobjects-list {
     counter-reset: astroobjects 0;
   }
     .astroobjectwidget-dialog .astroobject_title {
       display:none;
     }

    @media print {
      .nav-tabs {
        display: none!important;
      }
      body {
        padding-top: 0px!important;
      }
      .astroobject_text,.astroobjects-info-widget {
        font-size: x-small!important;
      }
      .astroobject_title {
//        font-size: 15px!important;
      }
    .astroobjectwidget {
       counter-increment:astroobjects;
    }
    .astroobjects-list .astroobject_title h4:before {
       content: counter(astroobjects) ". ";
     }
    .astroobjects-list .astroobject_title b:before {
       content: counter(astroobjects) ". ";
     }
    }

    legend {
    }
    .gm-style img {
      max-width: none;
    }
    .link, .panel-heading {
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
      display: inline-block!important;
    }
    .panel-compact {
      margin-bottom: 0px;
    }

    .checkbox-no-form-control {
      border: 0px;
      box-shadow: inherit;
      -webkit-box-shadow: inherit;
    }
    .modal-layer {
      z-index: 399!important;
      position: absolute;  /* makes the div go into a position that’s absolute to the browser viewing area */
      left: 0%; /* makes the div span all the way across the viewing area */
      top: 0%; /* makes the div span all the way across the viewing area */
      background-color: black;
      -moz-opacity: 0.2; /* makes the div transparent, so you have a cool overlay effect */
      opacity: .20;
      filter: alpha(opacity=20);
      width: 100%;
      height: 100%;
    }
    .modal-layer-dark {
      -moz-opacity: 0.7; /* makes the div transparent, so you have a cool overlay effect */
      opacity: .70;
      filter: alpha(opacity=70);
    }
    .nearby-object-names a, .nearby-object-modal-body {
      padding: 3px!important;
    }
    .nearby-object-container {
      padding: 0px!important;
    }
  );
  return std::vector<uint8_t>(css.begin(), css.end());
}
#endif
