#set(CMAKE_VERBOSE_MAKEFILE ON)
cmake_minimum_required (VERSION 2.8.9)
include(uselatex.cmake)

project(BA NONE)

#latex_get_output_path(output_dir)

set(sources
  head.tex
  preliminaries.tex
  case_3_5.tex
)



add_latex_document(ba_main.tex
  INPUTS ${sources}
  )


add_subdirectory(pics)


