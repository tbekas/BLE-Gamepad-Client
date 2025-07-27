find_package(Doxygen REQUIRED)

configure_file("index.md.in" "index.md" @ONLY)

set(DOXYGEN_MARKDOWN_ID_STYLE "GITHUB")
set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_CURRENT_BINARY_DIR}/index.md")
set(DOXYGEN_GENERATE_LATEX "NO")
set(DOXYGEN_CLASS_GRAPH "NO")

doxygen_add_docs(
        my_doc
        "${PROJECT_SOURCE_DIR}/docs"
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/include"
)
