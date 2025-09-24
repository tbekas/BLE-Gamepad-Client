include(FetchContent)
FetchContent_Declare(
        doxygen-awesome-css
        URL https://github.com/jothepro/doxygen-awesome-css/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(doxygen-awesome-css)
FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

find_package(Doxygen REQUIRED)

set(MAINPAGE_FILE "${CMAKE_CURRENT_BINARY_DIR}/index.md")
file(READ "${PROJECT_SOURCE_DIR}/README.md" README_CONTENT)
string(PREPEND README_CONTENT "# Overview\n")
file(WRITE "${MAINPAGE_FILE}" "${README_CONTENT}")

set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${MAINPAGE_FILE}")
set(DOXYGEN_MARKDOWN_ID_STYLE "GITHUB")
set(DOXYGEN_GENERATE_LATEX "NO")
set(DOXYGEN_CLASS_GRAPH "NO")
set(DOXYGEN_GENERATE_TREEVIEW "YES")
set(DOXYGEN_DISABLE_INDEX "NO")
set(DOXYGEN_FULL_SIDEBAR "NO")
set(DOXYGEN_PAGE_OUTLINE_PANEL "NO")
set(DOXYGEN_HTML_EXTRA_STYLESHEET "${AWESOME_CSS_DIR}/doxygen-awesome.css" "${AWESOME_CSS_DIR}/doxygen-awesome-sidebar-only.css")
set(DOXYGEN_IMAGE_PATH "${PROJECT_SOURCE_DIR}/docs")

doxygen_add_docs(
        lib_docs
        "${MAINPAGE_FILE}"
        "${PROJECT_SOURCE_DIR}/docs"
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/include"
)
