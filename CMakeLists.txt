#set(CMAKE_VERBOSE_MAKEFILE ON)
cmake_minimum_required (VERSION 2.8.9)
include(uselatex.cmake)

project(BA NONE)

#latex_get_output_path(output_dir)

add_custom_target(3_5_block_4
  COMMAND ./main ${PROJECT_SOURCE_DIR}/pics/3_5_basis.svg --figonly > ${CMAKE_BINARY_DIR}/3_5_block_4.tex
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/scripts/svg2tikz
  SOURCES ${PROJECT_SOURCE_DIR}/pics/3_5_basis.svg
  )

set(sources
  head.tex
  preliminaries.tex
  case_3_5.tex
)

add_latex_document(ba_main.tex
  INPUTS ${sources}
  )

add_dependencies(pdf 3_5_block_4)
add_dependencies(dvi 3_5_block_4)
