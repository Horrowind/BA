add_custom_target(3_5_block_4
  COMMAND ./main ${PROJECT_SOURCE_DIR}/pics/3_5_block_4.svg --figonly > ${CMAKE_BINARY_DIR}/pics/3_5_block_4.tex
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/scripts/svg2tikz
  SOURCES ${PROJECT_SOURCE_DIR}/pics/3_5_block_4.svg
  )
add_custom_target(3_5_basis
  COMMAND ./main ${PROJECT_SOURCE_DIR}/pics/3_5_basis.svg --figonly > ${CMAKE_BINARY_DIR}/pics/3_5_basis.tex
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/scripts/svg2tikz
  SOURCES ${PROJECT_SOURCE_DIR}/pics/3_5_basis.svg
  )


add_dependencies(pdf 3_5_block_4)
add_dependencies(dvi 3_5_block_4)
add_dependencies(pdf 3_5_basis)
add_dependencies(dvi 3_5_basis)
