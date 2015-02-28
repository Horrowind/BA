#set(CMAKE_VERBOSE_MAKEFILE ON)
cmake_minimum_required (VERSION 2.8.9)
include(uselatex.cmake)

project(BA NONE)

#latex_get_output_path(output_dir)

#set(LATEX_USE_SYNCTEX)
set(LATEX_USE_SYNCTEX ON)

set(sources
  head.tex
  title.tex
  tableofcontents.tex
  introduction.tex
  preliminaries.tex
  arcs.tex
  case_3_5.tex
  case_3_6.tex
  case_3_4.tex
  case_5_7.tex
  case_5_8.tex
  negative_results.tex
  general_remarks.tex
  appendix.tex
  declaration.tex
)


add_latex_document(ba_main.tex
  INPUTS ${sources}
  IMAGES TU.png
  BIBFILES bibliography.bib
  DEFAULT_PDF
  )
